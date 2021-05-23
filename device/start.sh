#!/bin/bash

echo ""

python3 device.py device9999 localhost 12345 2 &
python3 device.py device0000 localhost 12345 1 &
python3 device.py device0001 localhost 12345 4
wait
python3 controller.py localhost 12345 2
python3 device.py device0060 localhost 12345 2
#wait
python3 controller.py localhost 12345 1
