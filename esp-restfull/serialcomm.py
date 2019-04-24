#from machine import UART
#import time

#uart = UART(1, 115200);
#uart.init(115200, bits=8, parity=None, stop=1);

def write(mes):
    print(mes + ";")
    return input()
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
    

    
#from machine import UART
#import time

#uart = UART(1, 115200);
#uart.init(115200, bits=8, parity=None, stop=1);

#while True:
#    print('is console')
#
    #uart.write('privet andrey;')
#    time.sleep_ms(500)