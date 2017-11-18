import argparse
import hid
from mchp_aws_zt_kit import MchpAwsZTKitDevice
from sim_hid_device import SimMchpAwsZTHidDevice
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

    print('\nOpening AWS Zero-touch Kit Device')
    device = MchpAwsZTKitDevice(hid.device())
    #device = MchpAwsZTKitDevice(SimMchpAwsZTHidDevice())
    device.open()

    print('\nInitializing Kit')
    resp = device.init()
    print('    ATECC508A SN: %s' % resp['deviceSn'])

    print('\nSetting WiFi Information')
    device.set_wifi(ssid=args.ssid[0], psk=args.password[0])

    kit_info = read_kit_info()
    kit_info['wifi_ssid'] = args.ssid[0]
    kit_info['wifi_password'] = args.password[0]
    save_kit_info(kit_info)

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)
