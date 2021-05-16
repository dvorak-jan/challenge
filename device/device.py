# device.py by Jan Dvorak
import socket
import sys
import random
import time
from datetime import datetime

# processing of command-line argument(s)
if len(sys.argv) < 4:
    print ("Incorrect number of command-line arguments. Please launch the script as:")
    print ("sudo python3 device.py devicename serverport period")

deviceName = sys.argv[1]
serverPort = int(sys.argv[2])
period = int(sys.argv[3])

# constants
SERVER_IP = "127.0.0.1"   # IP address of the server
DEVICE_IP = "127.0.0.1"   # IP address of the device


# socket
socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((SERVER_IP, serverPort))

while True:
    time.sleep(period)
    data = random.randint(0, 65535)
    dateTimeObj = datetime.now()
    message = "m" + deviceName + str(data).zfill(5) + str(dateTimeObj)
    socket.sendall( bytes(message, 'utf-8') )
