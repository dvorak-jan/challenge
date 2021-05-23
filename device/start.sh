#!/bin/bash

echo ""

# By launching this file from the folder in which it is located,
# the following devices are started. Each of them then sends 1000 messages
# to the server. The first 10 devices are run simultaneously. They are
# followed by a controller message 2, command to write out the number
# of messages received so far, then two more devices are launched and finally
# the controller message 1 is sent, i.e. command to write out the number
# of messages and stop the application.

# Devices are launched with the following command-line arguments:
# device.py device_name server_IP_address server_port interval
#
# device_name -- 10 characters (will be shortened to 10 chars or supplemented by
#                zeros to 10 chars)
# server_IP_address -- IP address or 'localhost'
# server_port -- port number from 1 to 65535
# interval -- interval / 1000 (in seconds) is the period of time the device waits
#             before sending the next message; 1 = wait 0,0001 second,
#             1000 = wait 1 second, etc.

# Controller is launched with the following command-line arguments:
# python3 controller.py server_IP_address server_port message_number
#
# server_IP_address -- IP address or 'localhost'
# server_port -- port number from 1 to 65535
# message_number -- 1 or 2; 1 = print out message count and quit
#                           2 = print out message count, continue receiving messages

python3 device.py device9999 localhost 12345 2 &
python3 device.py device0088 localhost 12345 1 &
python3 device.py device0089 localhost 12345 3 &
python3 device.py device0090 localhost 12345 5 &
python3 device.py device0001 localhost 12345 4 &
python3 device.py device0002 localhost 12345 2 &
python3 device.py device0003 localhost 12345 1 &
python3 device.py device0004 localhost 12345 3 &
python3 device.py device0005 localhost 12345 2 &
python3 device.py device0006 localhost 12345 3 &
wait
python3 controller.py localhost 12345 2
python3 device.py device0060 localhost 12345 2 &
python3 device.py device0061 localhost 12345 3
python3 controller.py localhost 12345 1
