import os
import re
import time
import binascii
import json
import cryptography
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.serialization import Encoding
from cryptography.hazmat.primitives.asymmetric import ec
from aws_kit_common import crypto_be, load_or_create_key

class SimMchpAwsZTHidDevice:
    def __init__(self):
        self.report_size = 64
        self.cmd_buff = b''
        self.send_reports = []
        self.key = load_or_create_key('sim-device.key')

        if os.path.isfile('sim-device.json'):
            with open('sim-device.json', 'rb') as f:
                self.state = json.loads(f.read().decode('ascii'))
        else:
            self.state = {'sn':'0123112233445566A5'}
            self.save_state()

    def save_state(self):
        with open('sim-device.json', 'w') as f:
            f.write(json.dumps(self.state, indent=4, sort_keys=True))

    def open(self, vendor_id=0, product_id=0, serial_number=None):
        return

    def write(self, buff):
        self.cmd_buff += buff[1:] # skip report id
        # Kit protocol command is complete on newline
        if b'\n' in self.cmd_buff:
            self.process_cmd(self.cmd_buff.decode('ascii'))
            self.cmd_buff = b'' # clear command buffer for next command

    def read(self, max_length, timeout_ms=0):
        if len(self.send_reports) <= 0:
            # No reports to send, mimic the timeout, then return empty list
            time.sleep(timeout_ms / float(1000))
            return []
        return self.send_reports.pop(0)

    def process_cmd(self, raw_cmd):
        m = re.search('^([^(]+)\(([^)]*)\)', raw_cmd)
        target = m.group(1)
        data = m.group(2)
        if target == 'board:app':
            self.process_app_cmd(data)
        else:
            self.send_reply('C0()') # Unknown command

    def process_app_cmd(self, raw_app_cmd):
        cmd = json.loads(binascii.a2b_hex(raw_app_cmd).decode('ascii'))
        if cmd['method'] == 'init':
            self.sim_init(cmd)
        elif cmd['method'] == 'genCsr':
            self.sim_gen_csr(cmd)
        elif cmd['method'] == 'saveCredentials':
            self.sim_save_credentials(cmd)
        elif cmd['method'] == 'setWifi':
            self.sim_set_wifi(cmd)
        else:
            self.send_app_reply_error(cmd['id'], 2, 'Unknown command')

    def send_reply(self, data):
        # Send reply in report size chunks and pad out small remainder chunks with nulls
        for i in range(0, len(data), self.report_size):
            chunk = bytes(data[i:i+self.report_size], encoding='ascii')
            self.send_reports.append(chunk + b'\x00'*(self.report_size - len(chunk)))

    def send_kit_reply(self, status, data):
        status_str = binascii.b2a_hex(status.to_bytes(1, byteorder='little', signed=False)).decode('ascii')
        data_str = binascii.b2a_hex(data).decode('ascii')
        self.send_reply('%s(%s)\n' % (status_str, data_str))

    def send_app_reply_error(self, id, error_code, error_msg):
        reply = {'id':id, 'result':None, 'error':{'error_code':error_code ,'error_msg':error_msg}}
        self.send_kit_reply(0, bytes(json.dumps(reply), encoding='ascii'))

    def send_app_reply(self, id, results):
        self.send_kit_reply(0, bytes(json.dumps({'id':id, 'result':results, 'error':None}), encoding='ascii'))

    def sim_init(self, cmd):
        if cmd['params']['version'] != '2.0.0':
            send_app_reply_error(cmd['id'], 256, 'Unsupported version')
        results = {}
        results['deviceSn'] = self.state['sn']
        pub_nums = self.key.public_key().public_numbers()
        pubkey =  pub_nums.x.to_bytes(32, byteorder='big', signed=False)
        pubkey += pub_nums.y.to_bytes(32, byteorder='big', signed=False)
        results['devicePublicKey'] = binascii.b2a_hex(pubkey).decode('ascii')
        self.send_app_reply(cmd['id'], results)

    def sim_gen_csr(self, cmd):
        builder = x509.CertificateSigningRequestBuilder()
        builder = builder.subject_name(x509.Name([
            x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, u'Example Inc'),
            x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, u'Example Device')]))
        csr = builder.sign(
            private_key=self.key,
            algorithm=hashes.SHA256(),
            backend=crypto_be)

        results = {'csr' : binascii.b2a_hex(csr.public_bytes(encoding=Encoding.DER)).decode('ascii')}
        self.send_app_reply(cmd['id'], results)

    def sim_save_credentials(self, cmd):
        self.state['hostName'] = cmd['params']['hostName']
        self.state['deviceCert'] = cmd['params']['deviceCert']
        self.state['signerCert'] = cmd['params']['signerCert']
        self.state['signerCaPublicKey'] = cmd['params']['signerCaPublicKey']
        self.save_state()
        self.send_app_reply(cmd['id'], {})

    def sim_set_wifi(self, cmd):
        self.state['ssid'] = cmd['params']['ssid']
        self.state['psk']  = cmd['params']['psk']
        self.save_state()
        self.send_app_reply(cmd['id'], {})
