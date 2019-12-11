#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#define VERSION "0.1"
#define DATE "Mon, 11 Dec 2069 20:04:20 UTC+1"
#define SERVERNAME "Hermes (Unix)"
#define HTTP_HEADER "HTTP/1.0 %i %s\r\nDATE: %s\r\nServer: %s/%s\r\nContent-type: text/html\r\n\r\n"
#define HTTP_BODY "<html><body><b>%i</b> %s </body></html>\r\n"
#define MAX_LENGTH 200

const size_t BUF_LEN = 1024;

typedef struct http_code{
    int number;
    char *reason;
} status_code;

int get_line(int sock, char *buf, int size);
int set_code(status_code* error, int number);

void create_header(status_code* code,char** header)
{
    //Get Size of formated Header String and allocate the needed Memoryspace
    int len=snprintf(NULL,0,HTTP_HEADER,code->number,code->reason,DATE,SERVERNAME,VERSION)+2;
    *header=(char*)malloc(len*sizeof(char));
    snprintf(*header,len,HTTP_HEADER,code->number,code->reason,DATE,SERVERNAME,VERSION);
}

void create_body(status_code* code,char** body){
    int len;
    //Get Size of formated Body String and allocate the needed Memoryspace
    len=snprintf(NULL,0,HTTP_BODY,code->number,code->reason);
    *body=(char*)malloc(len*sizeof(char));
    snprintf(*body,MAX_LENGTH,HTTP_BODY,code->number,code->reason);
}
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
    char *head=NULL;
    char *body=NULL;
	size_t len;
    status_code code;
    

    set_code(&code,501);
    create_header(&code,&head);
    create_body(&code,&body);
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
        
		if(write( connfd, head, strlen(head) ) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
		}
        printf("Send Head\n");
        if(write( connfd, body, strlen(body) ) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
		}
        printf("Send Body\n");
		// Closes currently accepted connection
		close(connfd);
	}

	close( sockfd );
    free(head);
    free(body);
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

int set_code(status_code* error, int number)
{
	switch (number) {
	case 200:
		error->number = 200;
		error->reason = "OK";
		break;

	case 201:
		error->number = 201;
		error->reason = "Created";
		break;

	case 202:
		error->number = 202;
		error->reason = "Accepted";
		break;

	case 204:
		error->number = 204;
		error->reason = "No Content";
		break;

	case 301:
		error->number = 301;
		error->reason = "Moved Permanently";
		break;

	case 302:
		error->number = 302;
		error->reason = "Moved Temporarily";
		break;

	case 304:
		error->number = 304;
		error->reason = "Not Modified";
		break;

	case 400:
		error->number = 400;
		error->reason = "Bad Request";
		break;

	case 401:
		error->number = 401;
		error->reason = "Unauthorized";
		break;

	case 403:
		error->number = 403;
		error->reason = "Forbidden";
		break;

	case 404:
		error->number = 404;
		error->reason = "Not Found";
		break;

	case 500:
		error->number = 500;
		error->reason = "Internal Server Error";
		break;

	case 501:
		error->number = 501;
		error->reason = "Not Implemented";
		break;
		
	case 502:
		error->number = 502;
		error->reason = "Bad Gateway";
		break;

	case 503:
		error->number = 503;
		error->reason = "Service Unavailable";
		break;
		
	default:
		return -1;
	}
	return 0;
	
}