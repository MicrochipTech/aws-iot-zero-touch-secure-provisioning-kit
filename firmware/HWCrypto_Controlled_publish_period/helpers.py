# -*- coding: utf-8 -*-
# @Author: Lorenzo
# @Date:   2017-10-03 10:56:02
# @Last Modified by:   Lorenzo
# @Last Modified time: 2017-10-03 14:43:57

import json

def load_from_resource(mresource):
    mstream = open(mresource)
    barray = bytearray()
    while True:
        rd = mstream.read(1)
        if not rd:
            barray.append(0x0)
            break
        barray.append(rd[0])
    return barray

def load_thing_conf():
    confstream = open('resource://thing.conf.json')
    conf = ''
    while True:
        line = confstream.readline()
        if not line:
            break
        conf += line
    return json.loads(conf)
