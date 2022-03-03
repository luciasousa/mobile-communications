from network import LoRa
import socket
import time
import pycom
import machine
import math

# Initializing the variable lora in the mode LoRa.LORA 
# bypasses the LoRaWAN layer, allowing its usage using only the LoRa radio capabilities
lora = LoRa(mode=LoRa.LORA, region=LoRa.EU868)
# create a raw LoRa socket that will allow sending and receiving messages between the devices
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
# set as non-blocking to prevent being blocked if there’s no data received
s.setblocking(False)

enable_led = True

stats = lora.stats()
print(stats)
print('RSSI: '+str(stats[1])) # Received signal strength indication
print('SNR: '+str(stats[2]))  # Signal-to-noise ratio

pycom.heartbeat(False)

# Temperature Sensor
# Connect G3 (P16) on the expansion board to the middle pin on the TMP36.
# Connect 3V3 on the expansion board to the left pin on the TMP36.
# Connect GND on the expansion board to the right pin on the TMP36.

adc = machine.ADC()
apin = adc.channel(pin='P16')

while True :

    # get stats
    stats = lora.stats()
    print(stats)

    # get received signal strength indication
    rssi = stats[1]
    print('RSSI: '+str(rssi)+' dBm') 
    # get signal-to-noise ratio
    snr = stats[2]
    print('SNR: '+str(snr)+' dB')  

    # get duty cycle
    duty_cycle = 0.99*stats[7]

    # get tx power
    power = stats[6]
    print('Power: '+str(power)+' mW')

    # get temperature from pin G3=P16
    millivolts = apin.voltage()
    degC = (millivolts - 500.0) / 10.0
    print("Temperature: "+ str(degC)+" C")

    # change led color according to SNR value
    if stats[2] > 5:
        pycom.rgbled(0x00FF00)  # Green
    elif stats[2] > 0:
        pycom.rgbled(0x0000FF)  # Blue
    else:
        pycom.rgbled(0xFF0000)  # Red

    # send message to node B "true/false,temperature"
    s.send(str(enable_led)+","+str(degC)) 
    print('[Node A] Changing led to {} '.format('On ' if enable_led else 'Off '))
    enable_led = not enable_led

    # receive message from node B
    message_received = s.recv(300*8)
    if message_received == b'':
        print('No response from Node B yet ... ')
    else:
        print(message_received.decode())
        
    time.sleep(5) 