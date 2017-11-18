import os
import datetime
import pytz
import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from aws_kit_common import *

def main():
    # Create or load a root CA key pair
    print('\nLoading root CA key')
    root_ca_priv_key = load_or_create_key(ROOT_CA_KEY_FILENAME)

    # Create root CA certificate
    print('\nGenerating self-signed root CA certificate')
    builder = x509.CertificateBuilder()
    builder = builder.serial_number(random_cert_sn(16))
    # Please note that the name of the root CA is also part of the signer certificate and thus, it's
    # part of certificate definition in the SAMG55 firmware (g_cert_elements_1_signer). If this name is
    # changed, it will also need to be changed in the firmware. The cert2certdef.py utility script can
    # help with regenerating the cert_def_1_signer.c file after making changes.
    builder = builder.issuer_name(x509.Name([
        x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, u'Example Inc'),
        x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, u'Example Root CA')]))
    builder = builder.not_valid_before(datetime.datetime.now(tz=pytz.utc))
    builder = builder.not_valid_after(builder._not_valid_before.replace(year=builder._not_valid_before.year + 25))
    builder = builder.subject_name(builder._issuer_name)
    builder = builder.public_key(root_ca_priv_key.public_key())
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(root_ca_priv_key.public_key()),
        critical=False)
    builder = builder.add_extension(
        x509.BasicConstraints(ca=True, path_length=None),
        critical=True)
    # Self-sign certificate
    root_ca_cert = builder.sign(
        private_key=root_ca_priv_key,
        algorithm=hashes.SHA256(),
        backend=crypto_be)

    # Write root CA certificate to file
    with open(ROOT_CA_CERT_FILENAME, 'wb') as f:
        print('    Saving to ' + f.name)
        f.write(root_ca_cert.public_bytes(encoding=serialization.Encoding.PEM))

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)