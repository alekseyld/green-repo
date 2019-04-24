PC_DEBUG = False

import control
control.PC_DEBUG = PC_DEBUG

import os

import esp
esp.osdebug(None)

import gc
gc.collect()

if not PC_DEBUG:
    #from machine import Pin
    import network

def setupSTA(settings):
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(False)
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        #print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(settings['wifi-name'], settings['wifi-pass'])
        import time
        i = time.time()
        while not sta_if.isconnected():
            #print('.', end='')
            if time.time() - i > 40:
                setupAp(settings)
                break

def setupAp(settings):
     sta_if = network.WLAN(network.STA_IF)
     sta_if.active(False)
     ap_if = network.WLAN(network.AP_IF)
     ap_if.active(True)
     ap_if.config(essid=settings['ap-name'], password=settings['ap-pass'])

def setupNetwork():     
    if not "settings.json" in os.listdir():
        open("settings.json", 'w+').close()

    settings = control.getSettings()

    if settings['mode'] == 'sta' and not settings['wifi-name'] == '' and not settings['wifi-pass'] == '':
        setupSTA(settings)
    else:
        setupAp(settings)

if not PC_DEBUG:
    setupNetwork()

control.start_web()
