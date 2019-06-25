from argparse import ArgumentParser
import hid
from mchp_aws_zt_kit import MchpAwsZTKitDevice
from sim_hid_device import SimMchpAwsZTHidDevice
from aws_kit_common import *


def kit_set_wifi(ssid, password, is_sim=False):
    print('\nOpening AWS Zero-touch Kit Device')
    if not is_sim:
        device = MchpAwsZTKitDevice(hid.device())
    else:
        device = MchpAwsZTKitDevice(SimMchpAwsZTHidDevice())
    device.open()

    print('\nInitializing Kit')
    resp = device.init()
    print('    ATECCx08A SN: %s' % resp['deviceSn'])

    print('\nSetting WiFi Information')
    device.set_wifi(ssid=ssid, psk=password)

    kit_info = read_kit_info()
    kit_info['wifi_ssid'] = ssid
    kit_info['wifi_password'] = password
    save_kit_info(kit_info)

    print('\nDone')


if __name__ == '__main__':
    # Create argument parser to document script use
    parser = ArgumentParser(description='Set the WiFi connection settings on the demo board.')
    parser.add_argument(
        '--ssid',
        dest='ssid',
        required=True,
        metavar='name',
        help='WiFi network name'
    )
    parser.add_argument(
        '--password',
        dest='password',
        default=None,
        metavar='pw',
        help='WiFi network password'
    )
    parser.add_argument(
        '--sim',
        help='Use a simulated device instead.',
        action='store_true'
    )
    args = parser.parse_args()

    try:
        kit_set_wifi(ssid=args.ssid, password=args.password, is_sim=args.sim)
    except AWSZTKitError as e:
        # Print kit errors without a stack trace
        print(e)
