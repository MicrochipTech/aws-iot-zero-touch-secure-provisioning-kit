import os
import datetime
import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from aws_kit_common import *
from ca_create_signer_csr import add_signer_extensions

def main():
    print('\nLoading signer CA CSR')
    if not os.path.isfile(SIGNER_CA_CSR_FILENAME):
        raise AWSZTKitError('Failed to find signer CA CSR file, ' + SIGNER_CA_CSR_FILENAME + '. Have you run ca_create_signer_csr first?')
    with open(SIGNER_CA_CSR_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        signer_ca_csr = x509.load_pem_x509_csr(f.read(), crypto_be)
    if not signer_ca_csr.is_signature_valid:
        raise RuntimeError('Signer CA CSR has invalid signature.')

    print('\nLoading root CA key')
    if not os.path.isfile(ROOT_CA_KEY_FILENAME):
        raise AWSZTKitError('Failed to find root CA key file, ' + ROOT_CA_KEY_FILENAME + '. Have you run ca_create_root first?')
    with open(ROOT_CA_KEY_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        root_ca_priv_key = serialization.load_pem_private_key(
            data=f.read(),
            password=None,
            backend=crypto_be)

    print('\nLoading root CA certificate')
    if not os.path.isfile(ROOT_CA_CERT_FILENAME):
        raise AWSZTKitError('Failed to find root CA certificate file, ' + ROOT_CA_CERT_FILENAME + '. Have you run ca_create_root first?')
    with open(ROOT_CA_CERT_FILENAME, 'rb') as f:
        print('    Loading from ' + f.name)
        root_ca_cert = x509.load_pem_x509_certificate(f.read(), crypto_be)

    # Create signer CA certificate
    print('\nGenerating signer CA certificate from CSR')
    # Please note that the structure of the signer certificate is part of certificate definition in the SAMG55 firmware
    # (g_cert_elements_1_signer). If any part of it is changed, it will also need to be changed in the firmware.
    # The cert2certdef.py utility script can help with regenerating the cert_def_1_signer.c file after making changes.
    builder = x509.CertificateBuilder()
    builder = builder.serial_number(random_cert_sn(16))
    builder = builder.issuer_name(root_ca_cert.subject)
    builder = builder.not_valid_before(datetime.datetime.now(tz=pytz.utc))
    builder = builder.not_valid_after(builder._not_valid_before.replace(year=builder._not_valid_before.year + 10))
    builder = builder.subject_name(signer_ca_csr.subject)
    builder = builder.public_key(signer_ca_csr.public_key())
    builder = add_signer_extensions(
        builder=builder,
        authority_cert=root_ca_cert)
    # Sign signer certificate with root
    signer_ca_cert = builder.sign(
        private_key=root_ca_priv_key,
        algorithm=hashes.SHA256(),
        backend=crypto_be)

    # Write signer CA certificate to file
    with open(SIGNER_CA_CERT_FILENAME, 'wb') as f:
        print('    Saving to ' + f.name)
        f.write(signer_ca_cert.public_bytes(encoding=serialization.Encoding.PEM))

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)