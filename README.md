# Eaton coding challenge

**1) My understanding of the challenge**

Write a server console application in C++ that will receive communication over the network from multiple client devices simultaneously. Implement the server in a manner allowing to use a shared variable containing the message count.

**2) My proposed implementation:**

* Server console application listening on a TCP port
* Use of multithreading
* Global variable containing the message count protected by mutex
* The server application is controlled over the TCP port from another application

**3) Details:**

I chose the TCP protocol as it allows me to achieve the level of service when each message intended to be sent will be received exactly once (i.e. no message should be lost, no message should be received more than once). Given the fact that the objective of the challenge is to "compute number of messages", I consider this level of service appropriate. Choice of the TCP protocol should be also useful during testing (knowing the number of messages sent from the devices will allow me to assess the correctness of the computed number of messages).

Considering the scope of the challenge I have decided to implement just my own, very simple proprietary communication protocol that is described below (no industry-standard protocol). I have no claims about the usability of my protocol in any environment and for any purposes except for this test project.

For the purposes of development and testing, I use very simple software simulation of devices written in python. The device.py script is located in ./device folder. The device.py script is launched with several command-line arguments, viz. device name, server IP address, server port, period.

Example: python3 device.py device9999 localhost 12345 2

The meaning of the period argument: This number indicates how long should the device wait before sending a new message. If 'x' is given, the device will wait x/1000 sec. Each device is hard-coded to send exactly 1000 messages and terminate.

The server application has no built-in user interface. Instead, it is controlled over the TCP port using another application (any other application that is capable of sending the correctly formatted messages). The ./device folder contains controller.py script, which can be used for this purpose. Only two commands are implemented: 2 = print out the message count and continue counting, 1 = print out the message count and stop.

The ./device folder also contains a bash script (start.sh) that can be used to test the server.


**4) Main features of the server application (server.cpp)**

* Application written in C++, although a lot of code is more C than C++ and the OOP principles have not actually been much used.

* The application is launched with one command-line argument, which is the port number (use 12345 if you want to test the server using the start.sh script).

* The application uses multithreading so that it can communicate simultaneously with multiple devices and a couple of global variables so that it can keep track of the message count, its status (and the port number). Mutexes are used to protect the global variables against uncontrolled simultaneous writing.

* When the application receives the stop command from the controller.py (message no. 1), the respective thread changes the global status variable and then sends a message to the server (to the main or parent thread) so that it makes it leave the socket accept() line. The thread then discovers that it should terminate, therefore frees the allocated memory and terminates.

* The application has been tested using valgrind for memory leak errors.


**5) Implemented communication protocol**

Byte range | Length | Field | Values
-----------|--------|-------|-------
0x00 | 1 | Message type | "m" ... measurement, "s" ... service message
0x01-0x0a | 10 | Sender name | String of 10 characters ([a..z], [A..Z], [0..9])
0x0b-0x0f | 5 | Measured data ("m" messages), control command ("s" messages) | Integer from 0 to 99999
