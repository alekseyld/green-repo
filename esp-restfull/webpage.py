
title = 'Панель управления'
titleEncode = title.encode('utf-8')

def adminPage(settings = {'mode':'ap',
                          'wifi-name':'',
                          'wifi-pass':'',
                          'ap-name':'ESP_Alekseyld',
                          'ap-pass':'123456789'
                          }):
    return header(False) + adminContent(settings) + footer()

def footer():
    return b'</div><hr color="#EBEBEB"><footer>@Alekseyld 2019</footer>'
    
def adminContent(settings):
    isSTA = settings['mode'].find('sta') != -1
    
    return b'<form action="/admin">\xd0\xa0\xd0\xb5\xd0\xb6\xd0\xb8\xd0\xbc \xd1\x80\xd0\xb0\xd0\xb1\xd0\xbe\xd1\x82\xd1\x8b:<br><input type="radio" name="mode" value="sta" %s> \xd0\xa0\xd0\xb5\xd0\xb6\xd0\xb8\xd0\xbc WiFi<br><input type="radio" name="mode" value="ap" %s> \xd0\xa0\xd0\xb5\xd0\xb6\xd0\xb8\xd0\xbc "\xd0\xa2\xd0\xbe\xd1\x87\xd0\xba\xd0\xb0 \xd0\xb4\xd0\xbe\xd1\x81\xd1\x83\xd0\xbf\xd0\xb0"<br><br><label for="wifi-name">\xd0\x9d\xd0\xb0\xd0\xb7\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5 WiFi:</label><input type="text" name="wifi-name" value="%s"><br><label for="wifi-pass">\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c \xd0\xbe\xd1\x82 WiFi:</label><input type="password" name="wifi-pass" value="%s"><br><br><label for="ap-name">\xd0\x9d\xd0\xb0\xd0\xb7\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5 \xd1\x82\xd0\xbe\xd1\x87\xd0\xba\xd0\xb8 \xd0\xb4\xd0\xbe\xd1\x81\xd1\x82\xd1\x83\xd0\xbf\xd0\xb0:</label><input type="text" name="ap-name" value="%s"><br><label for="ap-pass" type="password">\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c \xd0\xbe\xd1\x82 \xd1\x82\xd0\xbe\xd1\x87\xd0\xba\xd0\xb8 \xd0\xb4\xd0\xbe\xd1\x81\xd1\x82\xd1\x83\xd0\xbf\xd0\xb0:</label><input type="password" name="ap-pass" value="%s"><br><br><input type="submit" value="\xd0\xa1\xd0\xbe\xd1\x85\xd1\x80\xd0\xb0\xd0\xbd\xd0\xb8\xd1\x82\xd1\x8c"></form>' % ((b'checked' if isSTA else b''), (b'checked' if not isSTA else b''), settings['wifi-name'].encode('utf-8'), settings['wifi-pass'].encode('utf-8'), settings['ap-name'].encode('utf-8'), settings['ap-pass'].encode('utf-8'))

def header(isPanel):
    return b'<!DOCTYPE html><html><head><title>%s</title><meta name="viewport" content="width=device-width, initial-scale=1"><meta http-equiv="Content-Type"content="text/html; charset=utf-8"/><style>%s</style></head><div class="panel"><p class="header-panel">%s</p>' % (titleEncode, cssAdmin(),b'\xd0\x9f\xd0\xb0\xd0\xbd\xd0\xb5\xd0\xbb\xd1\x8c \xd1\x83\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f' if isPanel else b'\xd0\x9d\xd0\xb0\xd1\x81\xd1\x82\xd1\x80\xd0\xbe\xd0\xb9\xd0\xba\xd0\xb8')

def cssAdmin():
    return b'html{font-family:Roboto;display:inline-block;margin:0 auto}h1{color:#767676;float:left;margin-left:10px}p{font-size:1.5rem}.button-menu{display:inline-block;border:none;border-radius:4px;padding:12px 40px;text-decoration:none;font-size:1.5rem;margin:2px;cursor:pointer;color:#3F79B5}.active{background-color:#3F79B5;color:white}.right{margin-top:.5em;float:right}.clear{clear:both}.panel{background-color:#EEE;margin-top:20px;margin-left:10px;margin-right:10px;padding-right:10px;padding-left:10px;padding-top:1px;border:none;border-radius:4px;padding-bottom:20px}.header-panel{text-align:center;color:#7F7F7F;font-weight:100}footer{margin-left:10px;margin-right:10px;color:#A9A9A9;padding:5px}input[type=text],input[type=password]{width:100%;padding:12px 20px;margin:8px 1px 0 1px;box-sizing:border-box}input[type=submit]{font-size:1em}a{text-decoration:none}@media (max-width:600px){h1{font-size:1.5em;text-align:center;float:none}.right{display:block;float:none;width:100%}.button-menu{display:block;margin:0,auto;font-size:1.5rem;padding:10px 15px}}'    

