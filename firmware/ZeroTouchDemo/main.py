# Zero Touch Demo
# Created at 2017-10-03 08:49:48.182639

import streams
import json
from wireless import wifi

from microchip.winc1500 import winc1500 as wifi_driver
from microchip.ateccx08a import ateccx08a

from aws.iot import iot

import helpers

new_resource('thing.conf.json')

LED_PINS = [D20, D21, 19]
BTN_PINS = [D22, D16, D17]
btn_states = ['up', 'up', 'up']

def button_pressed(btn_pin):
    btn_id = BTN_PINS.index(btn_pin)
    btn_states[btn_id] = 'up' if btn_states[btn_id] == 'down' else 'down'
    thing.update_shadow({'button' + str(btn_id+1): btn_states[btn_id]})

def prepare_pins():
    for led_pin in LED_PINS:
        pinMode(led_pin, OUTPUT)
        digitalWrite(led_pin, HIGH)
    for btn_pin in BTN_PINS:
        pinMode(btn_pin, INPUT_PULLUP)
        onPinFall(btn_pin, button_pressed, btn_pin, debounce=300)

@native_c('_load_certificates', ['csrc/certificates.c'])
def load_certificates():
    pass

def shadow_callback(requested):
    for led, state in requested.items():
        led_id = int(led[-1]) - 1
        digitalWrite(LED_PINS[led_id], 0 if state == 'on' else 1)
    return requested

prepare_pins()

streams.serial()
wifi_driver.init(SPI0, D12, D6, D2, D7, D3)

thing_conf = helpers.load_thing_conf()

print('> connect to wifi')
wifi.link(thing_conf['wifi_ssid'],wifi.WIFI_WPA2, thing_conf['wifi_password'])

print('> init crypto')
ateccx08a.hwcrypto_init(I2C0 + thing_conf['i2cdrv'], thing_conf['keyslot'], i2c_addr=thing_conf['i2caddr'])
crypto = ateccx08a.ATECC508A(I2C0 + thing_conf['i2cdrv'], thing_conf['i2caddr'])

load_certificates()
clicert = ateccx08a.read_certificate(0)
signercert = ateccx08a.read_certificate(1)
clicert = clicert[:-1] + signercert # concatenate certificates

# create aws iot thing instance, connect to mqtt broker, set shadow update callback and start mqtt reception loop
thing = iot.Thing(thing_conf['endpoint'], thing_conf['mqttid'], clicert, '', thingname=thing_conf['thingname'])
print('> connecting to mqtt broker...')

while True:
    try:
        thing.mqtt.connect()
        break
    except Exception as e:
        print('> connection failed: retrying for JITR')
        sleep(1000)

thing.on_shadow_request(shadow_callback)
thing.mqtt.loop()

print('> connected')
