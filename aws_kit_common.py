import os
import binascii
import json
import cryptography
import pytz
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec

ROOT_CA_FILENAME_BASE = 'root-ca'
ROOT_CA_KEY_FILENAME  = ROOT_CA_FILENAME_BASE + '.key'
ROOT_CA_CERT_FILENAME = ROOT_CA_FILENAME_BASE + '.crt'

SIGNER_CA_FILENAME_BASE = 'signer-ca'
SIGNER_CA_KEY_FILENAME  = SIGNER_CA_FILENAME_BASE + '.key'
SIGNER_CA_CSR_FILENAME  = SIGNER_CA_FILENAME_BASE + '.csr'
SIGNER_CA_CERT_FILENAME = SIGNER_CA_FILENAME_BASE + '.crt'
SIGNER_CA_VER_CERT_FILENAME = SIGNER_CA_FILENAME_BASE + '-verification.crt'

KIT_INFO_FILENAME = 'kit-info.json'

DEVICE_FILENAME_BASE  = 'device'
DEVICE_CSR_FILENAME   = DEVICE_FILENAME_BASE + '.csr'
DEVICE_CERT_FILENAME  = DEVICE_FILENAME_BASE + '.crt'

KP_APP_CMD = 'board:app'

# Setup cryptography 
crypto_be = cryptography.hazmat.backends.default_backend()

class AWSZTKitError(RuntimeError):
    pass

def read_kit_info():
    if not os.path.isfile(KIT_INFO_FILENAME):
        return {}
    with open(KIT_INFO_FILENAME, 'r') as f:
        json_str = f.read()
        if not json_str:
            return {} # Empty file
        return json.loads(json_str)

def save_kit_info(kit_info):
    with open(KIT_INFO_FILENAME, 'w') as f:
        f.write(json.dumps(kit_info, indent=4, sort_keys=True))

def load_or_create_key(filename, verbose=True):
    # Create or load a root CA key pair
    priv_key = None
    if os.path.isfile(filename):
        # Load existing key
        with open(filename, 'rb') as f:
            if verbose:
                print('    Loading from ' + f.name)
            priv_key = serialization.load_pem_private_key(
                data=f.read(),
                password=None,
                backend=crypto_be)
    if priv_key == None:
        # No private key loaded, generate new one
        if verbose:
            print('    No key file found, generating new key')
        priv_key = ec.generate_private_key(ec.SECP256R1(), crypto_be)
        # Save private key to file
        with open(filename, 'wb') as f:
            if verbose:
                print('    Saving to ' + f.name)
            pem_key = priv_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption())
            f.write(pem_key)
    return priv_key

def random_cert_sn(size):
    """Create a positive, non-trimmable serial number for X.509 certificates"""
    raw_sn = bytearray(os.urandom(size))
    raw_sn[0] = raw_sn[0] & 0x7F # Force MSB bit to 0 to ensure positive integer
    raw_sn[0] = raw_sn[0] | 0x40 # Force next bit to 1 to ensure the integer won't be trimmed in ASN.1 DER encoding
    return int.from_bytes(raw_sn, byteorder='big', signed=False)

def device_cert_sn(size, builder):
    """Cert serial number is the SHA256(Subject public key + Encoded dates)"""

    # Get the public key as X and Y integers concatenated
    pub_nums = builder._public_key.public_numbers()
    pubkey =  pub_nums.x.to_bytes(32, byteorder='big', signed=False)
    pubkey += pub_nums.y.to_bytes(32, byteorder='big', signed=False)

    # Get the encoded dates
    expire_years = 0
    enc_dates = bytearray(b'\x00'*3)
    enc_dates[0] = (enc_dates[0] & 0x07) | ((((builder._not_valid_before.year - 2000) & 0x1F) << 3) & 0xFF)
    enc_dates[0] = (enc_dates[0] & 0xF8) | ((((builder._not_valid_before.month) & 0x0F) >> 1) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0x7F) | ((((builder._not_valid_before.month) & 0x0F) << 7) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0x83) | (((builder._not_valid_before.day & 0x1F) << 2) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0xFC) | (((builder._not_valid_before.hour & 0x1F) >> 3) & 0xFF)
    enc_dates[2] = (enc_dates[2] & 0x1F) | (((builder._not_valid_before.hour & 0x1F) << 5) & 0xFF)
    enc_dates[2] = (enc_dates[2] & 0xE0) | ((expire_years & 0x1F) & 0xFF)
    enc_dates = bytes(enc_dates)

    # SAH256 hash of the public key and encoded dates
    digest = hashes.Hash(hashes.SHA256(), backend=crypto_be)
    digest.update(pubkey)
    digest.update(enc_dates)
    raw_sn = bytearray(digest.finalize()[:size])
    raw_sn[0] = raw_sn[0] & 0x7F # Force MSB bit to 0 to ensure positive integer
    raw_sn[0] = raw_sn[0] | 0x40 # Force next bit to 1 to ensure the integer won't be trimmed in ASN.1 DER encoding
    return int.from_bytes(raw_sn, byteorder='big', signed=False)

def datetime_to_iso8601(dt):
    tz = dt.strftime('%z')
    if tz:
        tz = tz[0:3] + ':' + tz[3:6] # Add colon between timezone minutes and seconds
    return dt.strftime("%Y-%m-%dT%H:%M:%S") + tz

def all_datetime_to_iso8601(d):
    """Traverse a dict or list and convert all datetime objects to ISO8601 strings."""
    if isinstance(d, dict):
        for key,value in d.items():
            if hasattr(value, 'strftime'):
                d[key] = datetime_to_iso8601(value)
            if isinstance(value, dict) or isinstance(value, list):
                d[key] = all_datetime_to_iso8601(value)
    elif isinstance(d, list):
        for i in range(len(d)):
            if hasattr(d[i], 'strftime'):
                d[i] = datetime_to_iso8601(d[i])
            if isinstance(d[i], dict) or isinstance(d[i], list):
                d[i] = all_datetime_to_iso8601(d[i])
    return d