# challenge
Eaton coding challenge

============================================================================

The problem to solve is following:

* You monitor devices, which are sending data to you.

* Each device have a unique name.

* Each device produces measurements.

The challenge is:

* Compute number of messages you got or read from the devices.

The solution can be in any language (preferably C++).

The scope is open, you must decide how the devices will work in your system.

The solution should be posted on GitHub or a similar page for a review.

Please add documentation explaining us how to run your code.

============================================================================

1) My proposed implementation:

Devices communicate with me (the Server) over the TCP protocol. This is my arbitrary decision but it helps me to achieve the level of service when each message intended to be sent will be received exactly once (i.e. no message should be lost, no message should be received more than once). Given the fact that the objective of the challenge is to "compute number of messages", I consider this level of service appropriate. Furthermore, it should be useful during testing (knowing the number of messages sent from the devices will allow me to assess the correctness of the computed number of messages).

Considering the scope of the challenge I am not attempting to implement any standard industry protocol for the communication with the IoT devices over the TCP. My devices use a simple proprietary protocol that is described in documentation.

For the purposes of development and testing, I use very simple software simulation of devices written in python.

The Server is written in C++

Protocol:

Byte 0x00: message type
Values: "m" ... measurement
        "s" ... service message

Bytes from 0x01 to 0x0a: sender name
Values: 10 characters ([a..z], [A..Z], [0..9])

Bytes from 0x0b to 0x0f: data
Five bytes of data, integer number from 0 to 65535
In case of service message, this is where the message is encoded:
Values: "00000" ... stop the server program
        "00001" ... print the current number of messages and continue operating

Bytes from 0x10 to 0x29: timestamp
Example: 2021-05-16 16:22:35.914273
