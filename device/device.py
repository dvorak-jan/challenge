# device.py by Jan Dvorak
import socket
import sys
import random
import time
from datetime import datetime

# processing of command-line argument(s)
if len( sys.argv ) < 5:
    print ( "Incorrect number of command-line arguments. Please launch the script as:" )
    print ( "python3 device.py devicename serveraddress serverport period" )

deviceName = sys.argv[1]
deviceName = deviceName + "0000000000"
deviceName = deviceName[0:10]
serverIPAddress = sys.argv[2]
serverPort = int( sys.argv[3] )
period = int( sys.argv[4] )

if (serverIPAddress == "localhost"):
    SERVER_IP = "127.0.0.1"
else:
    SERVER_IP = serverIPAddress


# socket
socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
socket.connect( ( SERVER_IP, serverPort ) )

random.seed()

counter = 0;
while True:
    time.sleep(  period/1000  )
    data = int( random.uniform( 0, 99999 ) )
    now = datetime.now()
    timestamp = int( datetime.timestamp( now ) * 1000000 )
    message = "m" + deviceName + str( data ).zfill( 5 ) + str( timestamp )
    sentDataAmount = socket.send(  bytes( message, 'utf-8' )  )
    if (sentDataAmount == 32):
        counter = counter + 1

    if ( counter == 1000 ):
        break
