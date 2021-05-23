/*
    server.cpp by Jan Dvořák

    As even a bare and minimalistic TCP client/server application is quite
    complex and contains a relatively standard set of code, I have not tried to
    write the entire application from the scratch. Instead, I partly reused
    one of my previously written student projects ( in Computer Networks subject ),
    which was based on a "skeleton" application by Viktor.Cerny@fit.cvut.cz
    and I have also utilized some code from the following website:
    http://www.mario-konrad.ch/blog/programming/multithread/tutorial-04.html
*/

#include <iostream>
using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath> // pow()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // htons(), htonl()
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <strings.h> // bzero()
#include <netdb.h>
#include <cstring>
#include <mutex>

#define BUFFER_SIZE 64 // amend the value if needed (e.g. for larger messages)
#define TIMEOUT 2 // the timeout constant should be increased if this value
                  // proves to be insufficient

//------------------------------------------------------------------------------
//      Global variables

int globalCount = 0; // global message counter
mutex mtx01; // mutex used for the globalCount variable

int globalStatus = 0; // global indicator of application status 0 = running,
                      // 1 = terminating
mutex mtx02; // mutex used for the globalStatus variable

int globalPort; // purpose of storing the port number to a global variable is to
                // make it accessible to the thread from which the main thread
                // shall be eventually terminated


//------------------------------------------------------------------------------
/*
    CMessage: class defining data types and methods of the message entity

    I have written and used the print() method. I leave it inentionally here,
    although I am aware that no code that is not used in an application should
    be left in it. In this case, you can use it e.g. for testing purposes.
*/
class CMessage {
private:
  char messageType;
  string senderName;
  int dataValue;
public:
  CMessage ( char messBuffer[] );  // Special constructor for creating new
                                  // instance directly from message buffer
  void print ( void );
  char getMessageType ( void ) { return messageType; }
  string getSenderName ( void ) { return senderName; }
  int getDataValue ( void ) { return dataValue; }
};

//------------------------------------------------------------------------------
CMessage::CMessage ( char messBuffer[] ) {
  messageType = messBuffer[0];
  senderName = "";
  for ( unsigned i = 1; i < 11; i++ ) {
    if  ( ( messBuffer[i] > 47 && messBuffer[i] < 58 ) ||
          ( messBuffer[i] > 64 && messBuffer[i] < 91 ) ||
          ( messBuffer[i] > 96 && messBuffer[i] < 123 ) ) {
      senderName.push_back ( messBuffer[i] );
    } else {
      senderName.push_back ( '?' );
    }
  }
  dataValue = 0;
  for ( unsigned i = 11; i < 16; i++ ) {
    dataValue += ( ( int )messBuffer[i] - 48 ) * ( pow ( 10, ( 15 - i ) ) );
  }
}

//------------------------------------------------------------------------------

void CMessage::print ( void ) {
  cout << this->getMessageType() << "::";
  cout << this->getSenderName() << "::";
  cout << this->getDataValue() << endl;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
} connection_t;

//------------------------------------------------------------------------------
/*
    The thread function - all the work performed in threads is done here
    Its return type is void * and the argument is void *. The pointed entity
    is of connection_t type ( see just above ).
*/
void * threadFunction( void * ptr )
{
	connection_t * conn;

  // close the thread if the function argument is not available,
	if ( !ptr ) {
    free ( conn );
    pthread_exit ( 0 );
  }

  // otherwise give the argument the required shape ( i.e. connection_t )
	conn = ( connection_t * )ptr;

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;
	fd_set sockets;
	int selectReturnValue;
	char buffer[ BUFFER_SIZE ];

  // The active connection loop should be kept while the connection is needed
	while ( true ) {

    // When the globalStatus is not 0, the thread should terminate
    mtx02.lock();
    if ( globalStatus != 0 ) {
      mtx02.unlock();
      close ( conn->sock );
      free ( conn );
      pthread_exit ( 0 );
    }
    mtx02.unlock();

		FD_ZERO ( &sockets );
		FD_SET ( conn->sock, &sockets );
		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 0;

    // Socket select and check
		selectReturnValue = select ( conn->sock + 1, &sockets, NULL, NULL, &timeout );
		if ( selectReturnValue < 0 ) {
			cout << "Select failure" << endl;
			close ( conn->sock );
			break;
		}

    // Connection timeout check
		if ( !FD_ISSET ( conn->sock, &sockets ) ) {
			cout << "Connection timeout" << endl;
			close ( conn->sock );
			break;
		}

    // Receiving data from the socket
		int bytesRead = recv ( conn->sock, buffer, BUFFER_SIZE, 0 );
		if ( bytesRead <= 0 ) {
			close ( conn->sock );
			break;
		}
    // Addition of the terminating zero to the data read
		buffer[ bytesRead ] = '\0';

    // Construction of CMessage instance from the buffer
		CMessage message ( buffer );

    // If the message if of the "service" type and the value is "2"
    // the application prints out the number of messages from devices
    // and continues working
		if ( message.getMessageType() == 's' && message.getDataValue() == 2 ) {
      close ( conn->sock );
			cout << "Message count: " << globalCount << endl;
			break;
		}

    // If the message if of the "service" type and the value is "1"
    // the application prints out the number of messages from devices
    // and is terminated
		if ( message.getMessageType() == 's' && message.getDataValue() == 1 ) {
      close ( conn->sock );
			cout << "Message count: " << globalCount << endl;
			cout << "Exiting now." << endl;
      free ( conn );
      mtx02.lock();
      globalStatus = 1;
      mtx02.unlock();

      // The purpose of the following section is to ensure a correct termination
      // of the main thread, which waits stuck at the socket accept() command.
      // The idea is to send a message from this thread to the main thread, thus
      // making the main thread to leave the accept() line and make it check
      // the globalStatus varible and evantually to correctly terminate.

      // Connect to the server port
      // If this attempt fails, exit( 0 ) is called, though it may
      // result in memory leak
      int s = socket ( AF_INET, SOCK_STREAM, 0 );
      if ( s < 0 ) {
        cout << "Unable to create socket!" << endl;
        exit ( 0 );
      }

      // Preparation for connecting to the parent thread on the localhost
      // using the globalPort number as a port number
      socklen_t sockAddrSize;
    	struct sockaddr_in serverAddr;
    	sockAddrSize = sizeof ( struct sockaddr_in );
    	bzero ( ( char * ) &serverAddr, sockAddrSize );
    	serverAddr.sin_family = AF_INET;
    	serverAddr.sin_port = htons ( globalPort );
    	struct hostent *host;
    	host = gethostbyname ( "localhost" );
    	memcpy ( &serverAddr.sin_addr, host->h_addr, host->h_length );

    	// Connecting...
      // If this attempt fails, exit( 0 ) is called, though it may
      // result in memory leak
    	if ( connect ( s, ( struct sockaddr * ) &serverAddr, sockAddrSize ) < 0 ) {
    		cout << "Connection not succesfull!" << endl;
    		close ( s );
    		exit ( 0 );
    	}

      // Preparation of the message used to awaken the main thread
      char * buf;
      buf = ( char * ) malloc ( 2 );
      buf[0] = 'a';
      buf[1] = '\0';

      // Sending of the message to the main thread
      // If this attempt fails, exit( 0 ) is called, though it may
      // result in memory leak
      if ( send ( s, buf, 1, 0 ) < 0 ) {
  			cout << "Unable to send data!" << endl;
  			close ( s );
  			exit ( 0 );
  		}

      // Free memory allocated for the message
      free ( buf );

      // Terminating the thread
			pthread_exit ( 0 );
		}

    // This is to ensure that messages other than 'm' messages from devices
    // are not counted. This includes 's' messages other than nos. 1 and 2.
    if ( message.getMessageType() == 'm' ) {

      // global counter of messages is incremented
      // mutex must be used here to prevent message losses
  		mtx01.lock();
  		globalCount++;
  		mtx01.unlock();
    } else {

      // Message of unknown type causes closing of socket
      close ( conn->sock );
      break;
    }
	}

	free ( conn );
	pthread_exit ( 0 );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main ( int argc, char ** argv ) {

  int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;


	// Check of command-line arguments
	if ( argc != 2 ) {
    cout << "Incorrect number of command line arguments!" << endl;
		cout << "Usage: ./server port_number" << endl;
		return -1;
	}

	// Check of the port number ( should be > 0 and <= 65535 )
  port = atoi ( argv[1] );
	if ( port <= 0 || port > 65535 ) {
		cout << "Error: Wrong parameter: port number (should be > 0 and <= 65535)" << endl;
		return -2;
	} else {
    globalPort = port;
  }

	// Create socket
	sock = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( sock <= 0 ) {
		cout << "Error: Unable to create socket" << endl;
		return -3;
	}

	// Bind socket to port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons ( port );
	if ( bind ( sock, ( struct sockaddr * ) &address, sizeof ( struct sockaddr_in ) ) < 0 ) {
		cout << "Error: Unable to bind socket to port " << port << endl;
		return -4;
	}

	// Listen on port
	if ( listen ( sock, 5 ) < 0 ) {
		cout << "Error: Unable to listen on port" << endl;
		return -5;
	}

  // The loop that runs until it is ended
	while ( true ) {
		// Accept incoming connections
		connection = ( connection_t * ) malloc ( sizeof ( connection_t ) );

    // The following two lines have been added to prevent
    // uninitialized value warnings from being displayed by valgrind
    connection->address = {0};
    connection->addr_len = {0};
		connection->sock = accept ( sock, &connection->address, &connection->addr_len );

    // When the globalStatus is not 0, the thread should terminate
    mtx02.lock();
    if ( globalStatus != 0 ) {
      mtx02.unlock();
      free ( connection );
      pthread_exit ( 0 );
    }
    mtx02.unlock();

		if ( connection->sock <= 0 ) {
			free ( connection );
		} else {
			// Start a new thread, no waiting
			pthread_create ( &thread, 0, threadFunction, ( void * )connection );
			pthread_detach ( thread );
		}
	}

	return 0;
}
