import os
import datetime
import binascii
import json
import argparse

import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec

from aws_kit_common import *

def main():
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Set the WiFi connection settings on the demo board.')
    parser.add_argument(
        '--ssid',
        dest='ssid',
        nargs=1,
        required=True,
        metavar='name',
        help='WiFi network name')
    parser.add_argument(
        '--password',
        dest='password',
        nargs=1,
        default=[None],
        metavar='pw',
        help='WiFi network password')
    args = parser.parse_args()
    ssid, password = args.ssid[0], args.password[0]

    # # Create argument parser to document script use
    # parser = argparse.ArgumentParser(description='Provisions the kit by requesting a CSR and returning signed certificates.')
    # args = parser.parse_args()

    print('\nLoading root CA certificate')
    if not os.path.isfile(ROOT_CA_CERT_FILENAME):
        raise AWSZTKitError('Failed to find root CA certificate file, ' + ROOT_CA_CERT_FILENAME + '. Have you run ca_create_root first?')
    with open(ROOT_CA_CERT_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        root_ca_cert = x509.load_pem_x509_certificate(f.read(), crypto_be)

        with open(ROOT_CA_CERT_FILENAME + '.public', 'wb') as f:
            print('    Saving to ' + f.name)
            f.write(root_ca_cert.public_key().public_bytes(encoding=serialization.Encoding.PEM,
                                                format=serialization.PublicFormat.SubjectPublicKeyInfo))

    print('\nLoading signer CA key')
    if not os.path.isfile(SIGNER_CA_KEY_FILENAME):
        raise AWSZTKitError('Failed to find signer CA key file, ' + SIGNER_CA_KEY_FILENAME + '. Have you run ca_create_signer_csr first?')
    with open(SIGNER_CA_KEY_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        signer_ca_priv_key = serialization.load_pem_private_key(
            data=f.read(),
            password=None,
            backend=crypto_be)

    print('\nLoading signer CA certificate')
    with open(DEVICE_CSR_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        device_csr =  f.read()
    if not os.path.isfile(SIGNER_CA_CERT_FILENAME):
        raise AWSZTKitError('Failed to find signer CA certificate file, ' + SIGNER_CA_CERT_FILENAME + '. Have you run ca_create_signer first?')
    with open(SIGNER_CA_CERT_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        signer_ca_cert = x509.load_pem_x509_certificate(f.read(), crypto_be)

    print('\nLoading device CSR')
    device_csr = x509.load_pem_x509_csr(device_csr, crypto_be)
    if not device_csr.is_signature_valid:
        raise AWSZTKitError('Device CSR has invalid signature.')

    print('\nGenerating device certificate from CSR')
    # Build certificate
    builder = x509.CertificateBuilder()
    builder = builder.issuer_name(signer_ca_cert.subject)
    builder = builder.not_valid_before(datetime.datetime.now(tz=pytz.utc).replace(minute=0,second=0)) # Device cert must have minutes and seconds set to 0
    builder = builder.not_valid_after(datetime.datetime(3000, 12, 31, 23, 59, 59)) # Should be year 9999, but this doesn't work on windows
    builder = builder.subject_name(device_csr.subject)
    builder = builder.public_key(device_csr.public_key())
    # Device certificate is generated from certificate dates and public key
    builder = builder.serial_number(random_cert_sn(9))
    # Add in extensions specified by CSR
    for extension in device_csr.extensions:
        builder = builder.add_extension(extension.value, extension.critical)
    # Subject Key ID is used as the thing name and MQTT client ID and is required for this demo
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(builder._public_key),
        critical=False)
    issuer_ski = signer_ca_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier)
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(issuer_ski),
        critical=False)

    # Sign certificate 
    device_cert = builder.sign(
        private_key=signer_ca_priv_key,
        algorithm=hashes.SHA256(),
        backend=crypto_be)


    kit_info = read_kit_info()
    # Find the subject key ID for use as the thing name
    is_subject_key_id_found = False
    for extension in device_cert.extensions:
        if extension.oid._name != 'subjectKeyIdentifier':
            continue # Not the extension we're looking for, skip
        kit_info['thingname'] = binascii.b2a_hex(extension.value.digest).decode('ascii')
        kit_info['mqttid'] = binascii.b2a_hex(extension.value.digest).decode('ascii')
        is_subject_key_id_found = True
    if not is_subject_key_id_found:
        raise RuntimeError('Could not find the subjectKeyIdentifier extension in the device certificate.')

    # Save certificate for reference
    with open(DEVICE_CERT_FILENAME, 'wb') as f:
        print('    Saving to ' + f.name)
        f.write(device_cert.public_bytes(encoding=serialization.Encoding.PEM))

    print('\nUpdating WiFi settings')
    kit_info['wifi_ssid'] = ssid
    kit_info['wifi_password'] = password
    print('    SSID:     %s' % kit_info['wifi_ssid'])
    disp_password = 'None'
    if kit_info['wifi_password'] is not None:
        disp_password = '*'*len(kit_info['wifi_password'])
    print('    Password: %s' % disp_password)
    save_kit_info(kit_info)

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)