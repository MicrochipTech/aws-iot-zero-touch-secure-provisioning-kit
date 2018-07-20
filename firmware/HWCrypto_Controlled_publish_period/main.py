# AWS IoT Controlled publish period 
# Created at 2017-10-03 08:49:48.182639

import streams
import json
from wireless import wifi

from microchip.winc1500 import winc1500 as wifi_driver
from microchip.ateccx08a import ateccx08a

from aws.iot import iot

import helpers

new_resource('thing.conf.json')

@native_c('_load_certificates', ['csrc/certificates.c'])
def load_certificates():
    pass

# define a callback for shadow updates
def shadow_callback(requested):
    global publish_period
    print('requested publish period:', requested['publish_period'])
    publish_period = requested['publish_period']
    return {'publish_period': publish_period}

streams.serial()
wifi_driver.init(SPI0, D12, D6, D2, D7, D3)

thing_conf = helpers.load_thing_conf()

print('> connect to wifi')
# place here your wifi configuration
wifi.link(thing_conf['wifi_ssid'],wifi.WIFI_WPA2, thing_conf['wifi_password'])

print('> init crypto')
ateccx08a.hwcrypto_init(I2C0 + thing_conf['i2cdrv'], thing_conf['keyslot'], i2c_addr=thing_conf['i2caddr'])
crypto = ateccx08a.ATECC508A(I2C0 + thing_conf['i2cdrv'], thing_conf['i2caddr'])

load_certificates()
clicert = ateccx08a.read_certificate(0)
signercert = ateccx08a.read_certificate(1)

# concatenate certificates
clicert = clicert[:-1] + signercert

publish_period = 1000

# create aws iot thing instance, connect to mqtt broker, set shadow update callback and start mqtt reception loop
thing = iot.Thing(thing_conf['endpoint'], thing_conf['mqttid'], clicert, '', thingname=thing_conf['thingname'])
print('connecting to mqtt broker...')

while True:
    try:
        thing.mqtt.connect()
        break
    except Exception as e:
        print('> connection failed: retrying for JITR')
        sleep(1000)

thing.on_shadow_request(shadow_callback)
thing.mqtt.loop()

thing.update_shadow({'publish_period': publish_period})

while True:
    print('publish random sample...')
    thing.mqtt.publish(thing.thingname + "/samples", json.dumps({ 'random': crypto.random_cmd()[0] }))
    sleep(publish_period)
