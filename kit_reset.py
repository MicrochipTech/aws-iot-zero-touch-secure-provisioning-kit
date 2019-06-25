from argparse import ArgumentParser
import hid
from mchp_aws_zt_kit import MchpAwsZTKitDevice
from sim_hid_device import SimMchpAwsZTHidDevice
from aws_kit_common import *


def kit_reset(is_sim=False):
    print('\nOpening AWS Zero-touch Kit Device')
    if not is_sim:
        device = MchpAwsZTKitDevice(hid.device())
    else:
        device = MchpAwsZTKitDevice(SimMchpAwsZTHidDevice())
    device.open()

    print('\nInitializing Kit')
    resp = device.init()
    print('    ATECCx08A SN: %s' % resp['deviceSn'])

    print('\nResetting Device')
    device.reset_kit()

    print('\nDone')


if __name__ == '__main__':
    # Create argument parser to document script use
    parser = ArgumentParser(description='Reset the board to an unprovisioned state')
    parser.add_argument(
        '--sim',
        help='Use a simulated device instead.',
        action='store_true'
    )
    args = parser.parse_args()

    try:
        kit_reset(is_sim=args.sim)
    except AWSZTKitError as e:
        # Print kit errors without a stack trace
        print(e)
