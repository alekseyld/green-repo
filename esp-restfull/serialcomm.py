import uasyncio as asyncio
from uasyncio.synchro import Lock

lockObj = Lock()

async def killer():
    await asyncio.sleep(10)

async def writeAsync(mes, lock):
    await lock.acquire()
    print(mes + ";")
    result = input()
    lock.release()
    return result
    
def write(mes):
    loop = asyncio.get_event_loop()
    
    loop.create_task(writeAsync(mes, lockObj))

    loop.run_until_complete(killer())

    
#from machine import UART
#import time

#uart = UART(1, 115200);
#uart.init(115200, bits=8, parity=None, stop=1);
    
#from machine import UART
#import time

#uart = UART(1, 115200);
#uart.init(115200, bits=8, parity=None, stop=1);

#while True:
#    print('is console')
#
    #uart.write('privet andrey;')
#    time.sleep_ms(500)

    #i = 0
    #while True:
    #    
    #    print(mes)
	#	#print('getstate all')
    #    i += 1
    #    if i >= 1000:
    #        return "not responce"
    #    try:    
    #        return input()
    #    except OSError:
    #        pass
    #    time.sleep_ms(50)

    #uart.write('privet andrey;')
    #time.sleep_ms(500)