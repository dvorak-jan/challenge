# Eaton coding challenge

**1) My understanding of the challenge**

Write a server console application in C++ that will receive communication over the network from multiple client devices simultaneously. Implement the server in a manner allowing to use a shared variable containing the message count.

**2) My proposed implementation:**

* Server console application listening on a TCP port
* Use of multithreading
* Global variable containing the message count protected by mutex
* Control of the server application over the TCP port from another application

**3) Details:**

I chose the TCP protocol as it allows me to achieve the level of service when each message intended to be sent will be received exactly once (i.e. no message should be lost, no message should be received more than once). Given the fact that the objective of the challenge is to "compute number of messages", I consider this level of service appropriate. Choice of the TCP protocol should be also useful during testing (knowing the number of messages sent from the devices will allow me to assess the correctness of the computed number of messages).

Considering the scope of the challenge I have decided to implement just my own, very simple proprietary communication protocol that is described below (no industry-standard protocol).

For the purposes of development and testing, I use very simple software simulation of devices written in python.

The server application has no built-in user interface. Instead, it is controlled over the TCP port using another application (a python script). Only two basic functionalities will be implemented: print the message count, stop the server.


**4) Device Communiation Protocol**

Byte range | Length | Field | Values
-----------|--------|-------|-------
0x00 | 1 | Message type | "m" ... measurement, "s" ... service message
0x01-0x05 | 5 | Message number | Integer from 0 to 99999
0x06-0x0f | 10 | Sender name | String of 10 characters ([a..z], [A..Z], [0..9])
0x10-0x1f | 16 | Measured data ("m" messages), control command ("s" messages) | Integer from 0 to 9999999999999999
0x20-0x2f | 16 | Timestamp | Integer transformed from timestamp value
