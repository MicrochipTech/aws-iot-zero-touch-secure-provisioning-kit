import tkinter as tk
import argparse
import time
import json
import boto3
import botocore
from aws_kit_common import *

def aws_interact_gui(aws_profile='default'):
    # REad kit info for the device serial number, which is used as the thing name
    kit_info = read_kit_info()
    if 'thing_name' not in kit_info:
        raise AWSZTKitError('thing_name not found in %s. Have you run kit_provision yet?' % KIT_INFO_FILENAME)

    # Create an AWS session with the credentials from the specified profile
    print('\nInitializing AWS IoTDataPlane client')
    try:
        aws_session = boto3.session.Session(profile_name=args.profile)
    except botocore.exceptions.ProfileNotFound as e:
        if args.profile == 'default':
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure" to setup profile.')
        else:
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure --profile %s" to setup profile.' % args.profile)
    # Create a client to the AWS IoT data service
    aws_iot_data = aws_session.client('iot-data')
    print('    Profile:  %s' % aws_session.profile_name)
    print('    Region:   %s' % aws_session.region_name)
    print('    Endpoint: %s' % aws_iot_data._endpoint)

    # Create the GUI
    root = tk.Tk()
    root.wm_title('Microchip AWS Zero-Touch Kit')
    app = Application(master=root, aws_iot_data=aws_iot_data, thing_name=kit_info['thing_name'])
    app.mainloop()

class Application(tk.Frame):
    def __init__(self, aws_iot_data, thing_name, master=None):
        self.aws_iot_data = aws_iot_data
        self.thing_name = thing_name
        tk.Frame.__init__(self, master)
        self.pack()
        self.create_widgets()
        self.state = ''
        self.on_update()

    def create_widgets(self):
        self.main_frame = tk.Frame()
        self.main_frame.pack(fill='x', side='top')

        self.status_frame = tk.Frame(bd=2, relief=tk.SUNKEN)
        self.status_frame.pack(fill='x', side='bottom')

        self.status_label = tk.Label(master=self.status_frame, text='Status')
        self.status_label.textvariable = tk.StringVar()
        self.status_label['textvariable'] = self.status_label.textvariable
        self.status_label.pack(side='left')

        self.frames = []
        self.led_checkbuttons = []
        self.button_radiobuttons = []
        self.button_labels = []
        for i in range(0,3):
            self.frames.append(tk.Frame(master=self.main_frame))
            self.frames[i].pack(side='left', padx=10)

            self.led_checkbuttons.append(tk.Checkbutton(master=self.frames[i], text=('LED %d' % (i+1))))
            self.led_checkbuttons[i].variable = tk.IntVar()
            self.led_checkbuttons[i]['variable'] = self.led_checkbuttons[i].variable
            self.led_checkbuttons[i]['command']  = lambda led_index=i: self.led_click(led_index)
            self.led_checkbuttons[i].pack(side='top')

            self.button_radiobuttons.append(tk.Radiobutton(master=self.frames[i], value=1))
            self.button_radiobuttons[i].variable = tk.IntVar()
            self.button_radiobuttons[i]['variable'] = self.button_radiobuttons[i].variable
            self.button_radiobuttons[i].pack(side='left')
            self.button_radiobuttons[i].config(state=tk.DISABLED)

            self.button_labels.append(tk.Label(master=self.frames[i], text=('BUTTON %d' % (i+1))))
            self.button_labels[i].pack(side='left')

    def led_click(self, led_index):
        if self.led_checkbuttons[led_index].variable.get() == 1:
            led_value = 'on'
        else:
            led_value = 'off'

        msg = {'state' : {'desired' : {('led%d' % (led_index+1)) : led_value}}}
        print('update_thing_shadow(): %s\n' % json.dumps(msg))
        self.aws_iot_data.update_thing_shadow(thingName=self.thing_name, payload=json.dumps(msg))

    def on_update(self):
        try:
            response = self.aws_iot_data.get_thing_shadow(thingName=self.thing_name)

            self.shadow = json.loads(response['payload'].read().decode('ascii'))
            curr_state = self.shadow['state']
            if curr_state != self.state:
                self.state = curr_state
                self.status_label.textvariable.set(self.thing_name)

                print('get_thing_shadow(): state changed\n%s\n' % json.dumps(self.shadow, sort_keys=True))
                
                for i in range(0, 3):
                    if 'reported' in curr_state:
                        button_label = 'button%d' % (i+1)
                        if button_label in curr_state['reported']:
                            if curr_state['reported'][button_label] == 'down':
                                self.button_radiobuttons[i].variable.set(1)
                            else:
                                self.button_radiobuttons[i].variable.set(0)
                    
                    if 'desired' in curr_state:
                        led_label = 'led%d' % (i+1)
                        if led_label in curr_state['desired']:
                            if curr_state['desired'][led_label] == 'on':
                                self.led_checkbuttons[i].variable.set(1)
                            else:
                                self.led_checkbuttons[i].variable.set(0)
                
        except botocore.exceptions.ClientError as e:
            if e.response['Error']['Code'] == 'ResourceNotFoundException':
                if self.state != 'no thing shadow':
                    self.state = 'no thing shadow'
                    status_msg = e.response['Error']['Message'] + '. The device may not have successfully connected to AWS yet.'
                    self.status_label.textvariable.set(status_msg)
                    print(status_msg)
            else:
                raise

        self.after(500, self.on_update)

if __name__ == '__main__':
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Interact with the thing through AWS')
    parser.add_argument(
        '--profile',
        dest='profile',
        nargs='?',
        default='default',
        metavar='name',
        help='AWS profile name (uses default if omitted)')
    args = parser.parse_args()

    try:
        aws_interact_gui(aws_profile=args.profile)
    except AWSZTKitError as e:
        # Capture kit errors and just display message instead of full stack trace
        print(e)
