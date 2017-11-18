import argparse
import hid
from mchp_aws_zt_kit import MchpAwsZTKitDevice
from sim_hid_device import SimMchpAwsZTHidDevice
from aws_kit_common import *

def main():
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Reset the board to a factory state')
    args = parser.parse_args()

    print('\nOpening AWS Zero-touch Kit Device')
    device = MchpAwsZTKitDevice(hid.device())
    #device = MchpAwsZTKitDevice(SimMchpAwsZTHidDevice())
    device.open()

    print('\nInitializing Kit')
    resp = device.init()
    print('    ATECC508A SN: %s' % resp['deviceSn'])

    print('\nResetting Device')
    device.reset_kit()

    print('\nDone')

try:
    main()
except AWSZTKitError as e:
    print(e)
