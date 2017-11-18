import os
import datetime
import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from aws_kit_common import *

def main():
    # Load or create a signer CA key pair
    print('\nLoading signer CA key')
    signer_ca_priv_key = load_or_create_key(SIGNER_CA_KEY_FILENAME)

    print('\nGenerating signer CA CSR')
    builder = x509.CertificateSigningRequestBuilder()
    # Please note that the name of the signer is part of certificate definition in the SAMG55 firmware
    # (g_cert_elements_1_signer). If this name is changed, it will also need to be changed in the firmware.
    # The cert2certdef.py utility script can help with regenerating the cert_def_1_signer.c file after making changes.
    builder = builder.subject_name(x509.Name([
        x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, u'Example Inc'),
        x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, u'Example Signer FFFF')]))
    builder = builder.add_extension(
        x509.BasicConstraints(ca=True, path_length=0),
        critical=True)
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=True,
            crl_sign=True,
            encipher_only=False,
            decipher_only=False),
        critical=True)
    signer_ca_csr = builder.sign(
        private_key=signer_ca_priv_key,
        algorithm=hashes.SHA256(),
        backend=crypto_be)

    # Save CSR
    with open(SIGNER_CA_CSR_FILENAME, 'wb') as f:
        print('    Saving to ' + f.name)
        f.write(signer_ca_csr.public_bytes(encoding=serialization.Encoding.PEM))

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)