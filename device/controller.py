# controller.py by Jan Dvorak
import socket
import sys
import random
import time
from datetime import datetime

# processing of command-line argument(s)
if len( sys.argv ) < 3:
    print ( "Incorrect number of command-line arguments. Please launch the script as:" )
    print ( "python3 controller.py serverport messagetype" )

serverPort = int( sys.argv[1] )
messageType = int( sys.argv[2] )

# constants
SERVER_IP = "127.0.0.1"   # IP address of the server
DEVICE_IP = "127.0.0.1"   # IP address of the device


# socket
socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
socket.connect( ( SERVER_IP, serverPort ) )

random.seed()

while True:
    data = messageType
    now = datetime.now()
    timestamp = int( datetime.timestamp( now ) * 1000000 )
    message = "scontroller" + str( data ).zfill( 5 ) + str( timestamp )
    socket.sendall(  bytes( message, 'utf-8' )  )
