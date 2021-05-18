/*
		Source: http://www.mario-konrad.ch/blog/programming/multithread/tutorial-04.html
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
#include <mutex> // mutex

#define BUFFER_SIZE 100
#define TIMEOUT 2

int globalCount = 0; // global message counter
mutex mtx01; // globalCount


/*
    This class defines data types and methods used by the message entity.
*/
class CMessage {
private:
  char messageType;
  string senderName;
  int dataValue;
  time_t timeStamp;
public:
  CMessage( char messBuffer[] );
  void print ( void );
  char getMessageType ( void ) { return messageType; }
  string getSenderName ( void ) { return senderName; }
  int getDataValue ( void ) { return dataValue; }
  time_t getTimeStamp ( void ) { return timeStamp; }
};

/*
    Constructor
*/
CMessage::CMessage( char messBuffer[] ) {
  messageType = messBuffer[0];
  senderName = "";
  for ( unsigned i = 1; i < 11; i++ ) {
    if  ( ( messBuffer[i] > 47 && messBuffer[i] < 58 ) ||
          ( messBuffer[i] > 64 && messBuffer[i] < 91 ) ||
          ( messBuffer[i] > 96 && messBuffer[i] < 123 ) ) {
      senderName.push_back( messBuffer[i] );
    } else {
      senderName.push_back( '?' );
    }
  }
  dataValue = 0;
  for ( unsigned i = 11; i < 16; i++ ) {
    dataValue += ( (int)messBuffer[i] - 48 ) * ( pow( 10, ( 15 - i ) ) );
  }
  double tmp = 0;
  for ( unsigned i = 16; i < 32; i++ ) {
    tmp += ( pow( 10, 31 - i ) ) * ( (int)messBuffer[i] - 48 );
  }
    timeStamp = tmp / 1000000;
}

//------------------------------------------------------------------------------

void CMessage::print ( void ) {
  cout << this->getMessageType() << "::";
  cout << this->getSenderName() << "::";
  cout << this->getDataValue() << "::";
  cout << this->getTimeStamp() << endl;
}

//******************************************************************************


typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
} connection_t;

void * threadFunction( void * ptr )
{
	int len;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0);
	conn = (connection_t *)ptr;

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;
	fd_set sockets;
	int selectReturnValue;
	char buffer[ BUFFER_SIZE ];

	while ( true ) {
		FD_ZERO( &sockets );
		FD_SET( conn->sock, &sockets );
		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 0;

		selectReturnValue = select( conn->sock + 1, &sockets, NULL, NULL, &timeout );
		if ( selectReturnValue < 0 ) {
			cout << "Select failure" << endl;
			close( conn->sock );
			break;
		}

		if ( !FD_ISSET( conn->sock, &sockets ) ) {
			cout << "Connection timeout" << endl;
			close( conn->sock );
			break;
		}

		int bytesRead = recv( conn->sock, buffer, BUFFER_SIZE, 0 );
		if ( bytesRead <= 0 ) {
			//cout << "Reading from socket failure" << endl;
			close( conn->sock );
			break;
		}
		buffer[ bytesRead ] = '\0';

		CMessage message( buffer );

		if ( message.getMessageType() == 's' && message.getDataValue() == 2 ) {
			mtx01.lock();
			cout << "Message count: " << globalCount << endl;
			mtx01.unlock();
			break;
		}

		if ( message.getMessageType() == 's' && message.getDataValue() == 1 ) {
			mtx01.lock();
			cout << "Message count: " << globalCount << endl;
			mtx01.unlock();
			cout << "Exiting now." << endl;
			exit(0);
		}

		mtx01.lock();
		globalCount++;
		mtx01.unlock();

	}

	free(conn);
	pthread_exit(0);
}

int main(int argc, char ** argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;


	/* check for command line arguments */
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " port" << endl;
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		cout << argv[0] << ": error: wrong parameter: port" << endl;
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		cout << argv[0] << ": error: cannot create socket" << endl;
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		cout << argv[0] << ": error: cannot bind socket to port " << port << endl;
		return -4;
	}

	/* listen on port */
	if (listen(sock, 5) < 0)
	{
		cout << argv[0] << ": error: cannot listen on port" << endl;
		return -5;
	}

	cout << argv[0] << ": ready and listening" << endl;

	while ( true )
	{
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = accept(sock, &connection->address, &connection->addr_len);
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, threadFunction, (void *)connection);
			pthread_detach(thread);
		}
	}

	return 0;
}
