#sudo kill $(sudo lsof -t -i:80)

import webpage
import serialcomm

import socket
import gc
import json

import uasyncio as asyncio

def importRe():
    try:
        import ure as re
    except:
        import re

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
    return serialcomm.write('setmode ' + parsedRequest['params']['mode'])

def getState(parsedRequest):
    return serialcomm.write('getstate ' + parsedRequest['params']['node'])

def setState(parsedRequest):
    return serialcomm.write('setstate ' + parsedRequest['params']['node'] + ' ' + parsedRequest['params']['state'])

def f(parsedRequest):
    return serialcomm.write('message from esp')#parsedRequest['params']['param1']
    #return 'hello ' + parsedRequest['rest']

def getAdminResponse(stripedRequest):
    if (len(stripedRequest) > 7):
        saveSettings(parseSettings(stripedRequest))
        
    return webpage.adminPage(getSettings())
    
REST_METHODS = {
    'setmode' : setMode,
    'getstate' : getState,
    'setstate' : setState,
    'admin' : getAdminResponse
}

# --- end RESTfull API section ---

def sendResponse(conn, response, end = False):
    try:
        conn.sendall(response)
    except:
        printD('Error on send response')
        pass
    
    if end:
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

def generate_headers(response_code):
    header = ''
    if response_code == 200:
        header += 'HTTP/1.1 200 OK\n'
    elif response_code == 404:
        header += 'HTTP/1.1 404 Not Found\n'
    
    time_now = time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
    header += 'Date: {now}\n'.format(now=time_now)
    header += 'Server: Simple-Python-Server\n'
    header += 'Connection: close\n\n' # Signal that connection will be closed after completing the request
    return header
    
def web_handler(conn, stripedRequest):
    respBody = b'_'
    code = b'';

    headers = b'Connection: keep-alive' 
    
    printD(stripedRequest)

    #if stripedRequest.find('admin', 0, 20) != -1:
        #header = generate_headers(200)
        #respBody = getAdminResponse(stripedRequest)
        
        #sendResponse(conn, respBody, True)
    #else:
    try:
        parsedRequest = parseRequest(stripedRequest)
        
        printD(parsedRequest)
        
        method = parsedRequest['restMethod']
        
        if method == "admin":
            respBody = REST_METHODS[method](stripedRequest)
            headers += b'\nContent-Type: text/html;charset=UTF-8'
            
        else:
            respBody = REST_METHODS[method](parsedRequest).encode('utf-8')
            headers += b'\nContent-Type: application/json;charset=UTF-8'
        
        code = b'200 OK'
        
    except KeyError:
        code = b'404 Not Found'
        respBody = b'method not supported'
        
    headers += b'\nContent-Length: ' + str(len(respBody)).encode('utf-8')
    
    sendResponse(conn, b'HTTP/1.1 ' + code + b'\n')
    sendResponse(conn, headers)
    sendResponse(conn, b'\n')
    sendResponse(conn, b'\n' + respBody, True)
        
def accept_client(s):
    conn, addr = s.accept()
	
    printD('Got a connection from %s' % str(addr))
	
    request = conn.recv(1024)
    request = stripRequect(str(request))
	
    printD('URL = "%s"' % request)
	
    loop = asyncio.get_event_loop()
    loop.run_until_complete(web_handler(conn, request))

async def web_loop(s):
    while True:
        try:
            accept_client(s)
	        
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
    
    loop = asyncio.get_event_loop()
    loop.create_task(web_loop(s)) # Schedule ASAP
    loop.run_forever()

gc.enable()
