import urllib
import urllib.request as urllib2

import hashlib
import base64
import time
import binascii
import json

import threading

from wxpy import *

def signature(secret, params):
    canstring = ''
    params = sorted(params.items(), key=lambda item:item[0])
    for k,v in params:
        if(k!='sign' and k!='key' and v !=''):
            canstring+=k+'='+v+'&'
    canstring = canstring[:-1]
    canstring += secret
    md5 = hashlib.md5(canstring.encode()).digest()
    return base64.b64encode(md5)

def getWeatherInfo():
    timeStamp = int(time.time())
    params = {'location':'118.82,31.88', 'username':'HE1805241315181878', 't':str(timeStamp)}
    key = '2d3d6d18755946beb2abe0c9202df731'
    sign = signature(secret=key, params=params)
    print(sign)
    url = 'https://free-api.heweather.com/s6/weather/now?'
    for k in params:
        if(params[k]!=''):
            url += k+'='+str(params[k])+'&'
    url += 'sign='+bytes.decode(sign)
    print(url)
    req = urllib2.urlopen(url)
    respone=req.read().decode('utf-8')
    data = json.loads(respone)

    info = data['HeWeather6']
    loc = info[0]['basic']['location']
    wea = info[0]['now']['cond_txt']
    tem = info[0]['now']['fl']

    message = "江宁 " + str(wea) + " " + "温度: " + str(tem)

    return message

def sendMessage(bot, my_friend):

    currT = time.localtime(time.time())

    if currT.tm_hour >= 23:
        return
    else:
        message = getWeatherInfo()
        message += " 爱你的小管家: " + str(time.strftime("%H:%M:%S", currT))

        my_friend.send(message)

        # timer = threading.Timer(3600, sendMessage)
        # timer.start()
        time.sleep(3600)
        sendMessage(bot, my_friend)





bot = Bot(console_qr=-2)
my_friend = bot.friends().search('Black Mascara', sex=FEMALE, city='淮安')[0]
sendMessage(bot, my_friend)