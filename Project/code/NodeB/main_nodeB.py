from network import LoRa
from machine import Pin
from pycom import heartbeat
import math
import socket
import time
import sys


lora = LoRa(mode=LoRa.LORA, region=LoRa.EU868)
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
s.setblocking(False)
#i = 0
led = Pin('P9',mode=Pin.OUT)

while True :
    message_received = s.recv(64)
    
    print(message_received)
    message_list = list(message_received.decode().split(','))
    print(message_list)
    if len(message_list) >= 2:
        # send message usado para enviar um informação de tamanho (size) para testar o impacto do tamanho do pacote (max size -> 256bytes)
        #i=0
        #size = 250
        #send_message = ''
        #while (i<size-1):
        #    send_message += 'a'
        #    i+=1
        #send_message += 'b'
        if message_list[0] == 'True':
            print('Led On')
            led.value(0)
            s.send('[Node B] Led is On')
        elif message_list[0] == 'False':
            print('Led Off')
            led.value(1)
            s.send('[Node B] Led is Off')
        duty_cycle = 0.099*lora.stats()[7]
        print('Temperature',message_list[1],'\n')
        print(lora.stats())
        print('Power', lora.stats()[6])
        print('Time On Air', lora.stats()[7])
        print('SNR', lora.stats()[2])
        print('RSSI', lora.stats()[1])
    else:
        print('No message received from Node A')
    time.sleep(5)
    