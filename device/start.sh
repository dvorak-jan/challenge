#!/bin/bash

echo ""

python3 device.py device0000 12345 2 &
python3 device.py device0001 12345 1 &
python3 device.py device0002 12345 2 &
python3 device.py device0003 12345 1 &
wait
python3 controller.py 12345 2
