#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>


const char error_501[138]="HTTP/1.0 501 Not Implemented\r\nContent-type: text/html\r\n\r\n""<html><body><b>501</b> Operation not supported</body></html>\r\n";
const size_t BUF_LEN = 1024;

int get_line(int sock, char *buf, int size);

// Something unexpected happened. Report error and terminate.
void sysErr( char *msg, int exitCode )
{
	fprintf( stderr, "%s\n\t%s\n", msg, strerror( errno ) );
	exit( exitCode );
}

// The user entered something stupid. Tell him.
void usage( char *argv0 )
{
	printf( "usage : %s portnumber\n", argv0 );
	exit( 0 );
}

int main(int argc, char **argv)
{
    // Setup for all used variables
	int connfd, sockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addrLen = sizeof(struct sockaddr_in);
	char revBuff[BUF_LEN];
	size_t len;

	// Setup of the Socket for TCP Communication
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd==-1){
        printf("Socket Creation not successfull \n");
        exit(-1);
	}
	else {
        printf("Socket created \n");
	}
	// Initialization of Server Struct with port and address
	bzero(&server_addr,addrLen);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( (u_short)atoi(argv[1] ) );

	// Check for right number of arguments
	if ( argc < 2 ) usage( argv[0] );


	// Binding the created Socket to the Server
	if ( bind( sockfd, (struct sockaddr *) &server_addr, addrLen ) == -1 ) {
		sysErr( "Server Fault : BIND", -2 );
	}

	while ( true ) {
		memset(revBuff, 0, BUF_LEN);

		// Wait for incoming TCP Connection Requests
		if ( (listen(sockfd,10) ) !=0 ){
            printf("Listen from Server failed \n");
            exit(-1);
		}
		else{
            printf("Listening.... \n");
		}
        // Accepting a single connection on connfd
		connfd = accept(sockfd,(struct sockaddr *) &client_addr, &addrLen);
		if (connfd < 0){
            printf("Server accept failed \n");
            exit(-1);
		}
		else {
            printf("Connection Succesfull \n");
		}

        // Reading a Message from the Client
		while ((len > 0) && strcmp("\n", revBuff)) {
			len = get_line(connfd, revBuff, BUF_LEN-1);
			printf("%s",revBuff);
		}

        // Sending back the Message unchanged
		printf("Send Response\n");
		if(write( connfd, error_501, strlen(error_501) ) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
		}
		// Closes currently accepted connection
		close(connfd);
	}

	close( sockfd );
	exit(0);
}

int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                /* DEBUG printf("%02X\n", c); */
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';
    return(i);
}