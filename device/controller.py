# controller.py by Jan Dvorak

# Controller is launched with the following command-line arguments:
# python3 controller.py server_IP_address server_port message_number
#
# server_IP_address -- IP address or 'localhost'
# server_port -- port number from 1 to 65535
# message_number -- 1 or 2; 1 = print out message count and quit
#                           2 = print out message count, continue receiving messages

import socket
import sys
import random
import time
from datetime import datetime

# processing of command-line argument(s)
if len( sys.argv ) < 4:
    print ( "Incorrect number of command-line arguments. Please launch the script as:" )
    print ( "python3 controller.py serveraddress serverport messagetype" )

serverIPAddress = sys.argv[1]
serverPort = int( sys.argv[2] )
messageType = int( sys.argv[3] )

if (serverIPAddress == "localhost"):
    SERVER_IP = "127.0.0.1"
else:
    SERVER_IP = serverIPAddress

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
