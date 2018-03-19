import os
import datetime
import argparse
import re
import pytz
import boto3
import botocore
import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from aws_kit_common import *

def main():
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Register a signer CA with AWS IoT')
    parser.add_argument(
        '--profile',
        dest='profile',
        nargs='?',
        default='default',
        metavar='name',
        help='AWS profile name (uses default if omitted)')
    args = parser.parse_args()

    kit_info = read_kit_info()

    # Read the signer CA key file needed to sign the verification certificate
    print('\nReading signer CA key file, %s' % SIGNER_CA_KEY_FILENAME)
    if not os.path.isfile(SIGNER_CA_KEY_FILENAME):
        raise AWSZTKitError('Failed to find signer CA key file, ' + SIGNER_CA_KEY_FILENAME + '. Have you run ca_create_signer_csr first?')
    with open(SIGNER_CA_KEY_FILENAME, 'rb') as f:
        signer_ca_priv_key = serialization.load_pem_private_key(
            data=f.read(),
            password=None,
            backend=crypto_be)

    # Read the signer CA certificate to be registered with AWS IoT
    print('\nReading signer CA certificate file, %s' % SIGNER_CA_CERT_FILENAME)
    if not os.path.isfile(SIGNER_CA_CERT_FILENAME):
        raise AWSZTKitError('Failed to find signer CA certificate file, ' + SIGNER_CA_CERT_FILENAME + '. Have you run ca_create_signer first?')
    with open(SIGNER_CA_CERT_FILENAME, 'rb') as f:
        signer_ca_cert = x509.load_pem_x509_certificate(f.read(), crypto_be)

    # Create an AWS session with the credentials from the specified profile
    print('\nInitializing AWS IoT client')
    try:
        aws_session = boto3.session.Session(profile_name=args.profile)
    except botocore.exceptions.ProfileNotFound as e:
        if args.profile == 'default':
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure" to setup profile.')
        else:
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure --profile %s" to setup profile.' % args.profile)
    # Create a client to the AWS IoT service
    aws_iot = aws_session.client('iot')
    print('    Profile:  %s' % aws_session.profile_name)
    print('    Region:   %s' % aws_session.region_name)
    print('    Endpoint: %s' % aws_iot._endpoint)


    # Request a registration code required for registering a CA certificate (signer)
    print('\nGetting CA registration code from AWS IoT')
    reg_code = aws_iot.get_registration_code()['registrationCode']
    print('    Code: %s' % reg_code)

    # Generate a verification certificate around the registration code (subject common name)
    print('\nGenerating signer CA AWS verification certificate')
    builder = x509.CertificateBuilder()
    builder = builder.serial_number(random_cert_sn(16))
    builder = builder.issuer_name(signer_ca_cert.subject)
    builder = builder.not_valid_before(datetime.datetime.now(tz=pytz.utc))
    builder = builder.not_valid_after(builder._not_valid_before.replace(day=builder._not_valid_before.day + 1))
    builder = builder.subject_name(x509.Name([x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, reg_code)]))
    builder = builder.public_key(signer_ca_cert.public_key())
    signer_ca_ver_cert = builder.sign(
        private_key=signer_ca_priv_key,
        algorithm=hashes.SHA256(),
        backend=crypto_be)

    # Write signer CA certificate to file for reference
    with open(SIGNER_CA_VER_CERT_FILENAME, 'wb') as f:
        print('    Saved to ' + f.name)
        f.write(signer_ca_ver_cert.public_bytes(encoding=serialization.Encoding.PEM))

    print('\nRegistering signer CA with AWS IoT')
    try:
        # TODO: Provide options when this fails (already exists, too many CAs with same name, etc...)
        response = aws_iot.register_ca_certificate(
            caCertificate=signer_ca_cert.public_bytes(encoding=serialization.Encoding.PEM).decode('ascii'),
            verificationCertificate=signer_ca_ver_cert.public_bytes(encoding=serialization.Encoding.PEM).decode('ascii'),
            setAsActive=True,
            allowAutoRegistration=True)
        ca_id  = response['certificateId']
    except botocore.exceptions.ClientError as e:
        if e.response['Error']['Code'] == 'ResourceAlreadyExistsException':
            print('    This CA certificate already exists in AWS IoT')
            ca_id = re.search('ID:([0-9a-zA-Z]+)', e.response['Error']['Message']).group(1)
        else:
            raise
    print('    ID: ' + ca_id)

    print('\nGetting AWS IoT device endpoint')
    response = aws_iot.describe_ca_certificate(certificateId=ca_id)
    # Replace the response datetime objects with an ISO8601 string so the dict is json serializable
    all_datetime_to_iso8601(response)

    kit_info['certificateDescription'] = response['certificateDescription']
    response = aws_iot.describe_endpoint()
    kit_info['endpointAddress'] = response['endpointAddress']
    print('    Hostname: ' + kit_info['endpointAddress'])

    save_kit_info(kit_info)

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)