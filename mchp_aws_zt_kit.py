import re
import binascii
import json

DEVICE_HID_VID = 0x04d8
DEVICE_HID_PID = 0x0f32
KIT_VERSION = "2.0.0"

#class MchpAwsZTKitDevice(hid.device):
class MchpAwsZTKitDevice():
    def __init__(self, device):
        super(MchpAwsZTKitDevice, self).__init__()
        self.device = device
        self.report_size = 64
        self.next_app_cmd_id = 0
        self.app_responses = {}
        self.kit_reply_regex = re.compile('^([0-9a-zA-Z]{2})\\(([^)]*)\\)')

    def open(self, vendor_id=DEVICE_HID_VID, product_id=DEVICE_HID_PID):
        """Opens HID device for the AWS Zero-touch Kit. Adjusts default VID/PID for the kit."""
        self.app_responses = {}
        return self.device.open(vendor_id, product_id)

    def raw_write(self, data):
        """Write arbitrary number of bytes to the kit."""
        # Break up data into hid report chunks
        for i in range(0, len(data), self.report_size):
            chunk = data[i:i+self.report_size]
            # Prepend fixed report ID of 0, then pad with EOT characters
            self.device.write(b'\x00' + chunk + b'\x04'*(self.report_size - len(chunk)))

    def kit_write(self, target, data):
        """Write a kit protocol command to the device."""
        self.raw_write(bytes('%s(%s)\n' % (target, binascii.b2a_hex(data).decode('ascii')), encoding='ascii'))

    def kit_write_app(self, method, params=None):
        """Write an app-specific command to the device."""
        if params is None:
            params = {} # No params should be encoded as an empty object
        cmd = {'method':method, 'params':params, 'id':self.next_app_cmd_id}
        self.next_app_cmd_id = self.next_app_cmd_id + 1
        self.kit_write('board:app', bytes(json.dumps(cmd), encoding='ascii'))
        return self.next_app_cmd_id - 1

    def kit_read(self, timeout_ms=0):
        """Wait for a kit protocol response to be returned."""
        # Kit protocol data is all ascii
        data = []
        # Read until a newline is encountered after printable data
        while 10 not in data:
            chunk = self.device.read(self.report_size, timeout_ms=timeout_ms)
            if len(chunk) <= 0:
                raise RuntimeError('Timeout (>%d ms) waiting for reply from kit device.' % timeout_ms)
            if len(data) == 0:
                # Disregard any initial non-printable characters
                for i in range(0, len(chunk)):
                    if chunk[i] > 32:
                        break
                chunk = chunk[i:]
            data += chunk
        data = data[:data.index(10)+1] # Trim any data after the newline
        return ''.join(map(chr, data)) # Convert data from list of integers into string

    def parse_kit_reply(self, data):
        """Perform basic parsing of the kit protocol replies.

        - XX(YYZZ...)
        - where XX is the status code in hex and YYZZ is the reply data
        """
        match = self.kit_reply_regex.search(data)
        if match is None:
            raise ValueError('Unable to parse kit protocol reply: %s' % data)
        return {'status': int(match.group(1), 16), 'data': match.group(2)}

    def kit_read_app(self, id):
        """Read an application specific command response."""
        while id not in self.app_responses:
            data = self.kit_read()
            kit_resp = self.parse_kit_reply(data)
            if kit_resp['status'] != 0:
                raise RuntimeError('Kit protocol error. Received reply %s' % data)
            app_resp = json.loads(binascii.a2b_hex(kit_resp['data']).decode('ascii'))
            self.app_responses[app_resp['id']] = app_resp

        app_resp = self.app_responses[id]
        del self.app_responses[id]
        return app_resp

    def kit_read_app_no_error(self, id):
        """Read an application specific command response and throw an error if
           the response indicates a command error."""
        resp = self.kit_read_app(id)
        if resp['error'] is not None:
            raise MchpAwsZTKitError(resp['error'])
        return resp

    def init(self, kit_version=KIT_VERSION):
        """Initialize the device for the demo."""
        id = self.kit_write_app('init', {'version':kit_version})
        resp = self.kit_read_app_no_error(id)
        return resp['result']

    def gen_csr(self):
        """Request a CSR from the device."""
        id = self.kit_write_app('genCsr')
        resp = self.kit_read_app_no_error(id)
        return resp['result']

    def save_credentials(self, host_name, device_cert, signer_cert, signer_ca_public_key):
        """Save credentials and connection information to the device."""
        params = {}
        params['hostName']= host_name
        params['deviceCert'] = binascii.b2a_hex(device_cert).decode('ascii')
        params['signerCert'] = binascii.b2a_hex(signer_cert).decode('ascii')
        params['signerCaPublicKey'] = binascii.b2a_hex(signer_ca_public_key).decode('ascii')
        id = self.kit_write_app('saveCredentials', params)
        id = self.kit_read_app_no_error(id)

    def set_wifi(self, ssid, psk):
        """Save the Wifi settings to the device."""
        id = self.kit_write_app('setWifi', {'ssid':ssid, 'psk':psk})
        id = self.kit_read_app_no_error(id)

    def reset_kit(self):
        """Reset the kit to factory state, deleting all information."""
        id = self.kit_write_app('resetKit')
        resp = self.kit_read_app_no_error(id)

    def get_status(self):
        """Get the current status of the kit."""
        id = self.kit_write_app('getStatus')
        resp = self.kit_read_app_no_error(id)
        return resp['result']

class MchpAwsZTKitError(Exception):
    def __init__(self, error_info):
        self.error_code = error_info['error_code']
        self.error_msg  = error_info['error_msg']
        super(MchpAwsZTKitError, self).__init__('Kit error %d: %s' % (self.error_code, self.error_msg))
