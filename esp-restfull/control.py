#sudo kill $(sudo lsof -t -i:80)

import webpage
import serialcomm

import socket
import gc
import json

def importRe():
    if PC_DEBUG:
        import re
    else:
        import ure as re

def printD(mes):
    if PC_DEBUG:
        print('DEBUG LOG - ' + str(mes))

def parsePinNum(url):
    m = re.search('/?(\d)=', url)
    if m:
        try:
            return int(m.group(1))
        except ValueError:
            return -1
    else:
        return -1

# --- start settings section ---
        
def saveSettings(settings):
    f = open('settings.json', 'w')
    f.write(json.dumps(settings))
    f.close()

def getSettings():
    f = open('settings.json', 'r')
    sjson = f.read()
    f.close()

    if len(sjson) > 0:
        return json.loads(sjson)
    
    return {'mode':'ap',
            'wifi-name':'',
            'wifi-pass':'',
            'ap-name':'ESP_Alekseyld',
            'ap-pass':'123456789'
            }

def normalizeSettings(settings):
    if not 'mode' in settings:
        settings['mode'] = 'ap'
    if not 'wifi-name' in settings:
        settings['wifi-name'] = ''
    if not 'wifi-pass' in settings:
        settings['wifi-name'] = ''
    if not 'ap-name' in settings:
        settings['wifi-name'] = 'ESP_Alekseyld' 
    if not 'ap-pass' in settings:
        settings['wifi-name'] = '123456789'
    return settings

def parseSettings(request):
    return normalizeSettings(
        dict(x.split('=') for x in request.replace('/admin?','').split('&'))
    )

# --- end settings section  ---

# --- start RESTfull API section ---

def setMode(parsedRequest):
    printD('/setMode to ' + parsedRequest['params']['mode'])
    
    return parsedRequest['restMethod'] + parsedRequest['params']['mode']

def getState(parsedRequest):
    return 'not realized'

def setState(parsedRequest):
    return 'not realized'

def f(parsedRequest):
    return serialcomm.write('message from esp')#parsedRequest['params']['param1']
    #return 'hello ' + parsedRequest['rest']

REST_METHODS = {
    'setmode' : setMode,
    'getstate' : getState,
    'setstate' : setState
}

# --- end RESTfull API section ---

def getAdminResponse(request):
    if (len(request) > 7):
        saveSettings(parseSettings(request))
        
    return webpage.adminPage(getSettings())
   
def sendResponse(conn, response):
    try:
        conn.sendall(response)
    except:
        printD('Error on send response')
        pass
         
    conn.close()
    gc.collect()   

def stripRequect(request):
    indexHTTP = request.find('HTTP')
    if indexHTTP != -1:
        request = request[0:indexHTTP]

    indexGET = request.find('GET')

    if indexGET != -1:
        return request[indexGET + 3:len(request)].strip()
    else:
        indexPOST = request.find('POST')
        if indexPOST != -1:
            return request[indexPOST + 4:len(request)].strip()
    return request.strip()

def parseRequest(stripedRequest):
    parsedRequest = {}

    if stripedRequest.find('?') != -1:
        paramIndex = stripedRequest.find('?')
    
        parsedRequest['restMethod'] = stripedRequest[1:paramIndex]
        
        params = {}
        paramsString = stripedRequest[paramIndex + 1: len(stripedRequest)]
        
        for paramsArray in paramsString.split('&'):
            paramsPair = paramsArray.split('=')
            params[paramsPair[0]] = paramsPair[1]
        
        parsedRequest['params'] = params
        
    else:
        parsedRequest['restMethod'] = stripedRequest[1:len(stripedRequest)]
        
    return parsedRequest
    
def web_handler(conn, stripedRequest):
    response = b''

    printD(stripedRequest)

    if stripedRequest.find('admin', 0, 20) != -1:
        response = getAdminResponse(stripedRequest)
    else:
        try:
            parsedRequest = parseRequest(stripedRequest)

            printD(parsedRequest)
            
            response = REST_METHODS[parsedRequest['restMethod']](parsedRequest).encode('utf-8')
            
        except KeyError:
            printD('eerror')
            response = 'HTTP/1.1 404 Not Found method not supported'.encode('utf-8')
        
    sendResponse(conn, response)
        
def web_loop(s):
    while True:
        try:
            conn, addr = s.accept()
            
            printD('Got a connection from %s' % str(addr))
            
            request = conn.recv(1024)
            request = stripRequect(str(request))
            
            printD('URL = "%s"' % request)
            
            web_handler(conn, request)

        except Exception as ex:
            template = "An exception of type {0} occurred. Arguments:\n{1!r}"
            message = template.format(type(ex).__name__, ex.args)
            printD(message)
            gc.collect()

def start_web():
    importRe()

    s = socket.socket()#socket.AF_INET, socket.SOCK_STREAM
    addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
    s.bind(addr)
    s.listen(1)

    web_loop(s)

gc.enable()
