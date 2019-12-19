#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

//Preprocessor defines
#define VERSION "0.5"
#define SERVERNAME "Hermes Team 09"
#define HTTP_HEADER "HTTP/1.0 %i %s\r\nServer: %s/%s\r\nConnection: close\r\nContent-type: text/html\r\n\r\n"
#define HTTP_BODY "<html><body><b>501</b> Not Implemented </body></html>\r\n"
#define HTTP_404 "<html><body><h1>404</h1> Not Found </body></html>\r\n"
#define HTTP_OK "<html><body>Dies ist die eine Fake Seite des Webservers!</body></html>\r\n"
#define HTML_FILES_PATH "./html/"

//Maximum possible size for requests
const size_t BUF_LEN = 1024;

// Struct for creation of HTTP headers
typedef struct http_code{
    int number;
    char *reason;
} status_code;

//Function prototypes
int get_line(int sock, char *buf, int size);
int set_code(status_code* error, int number);
int process_Request(int socket);
int send_File(char *filename,int socket);
void create_header(status_code* code,char** header);
void send_status(int statuscode, int sock);
void usage( char *argv0 );
void sysErr( char *msg, int exitCode );

//Main program
int main(int argc, char **argv)
{
    // Setup for all used variables
	int connfd, sockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addrLen = sizeof(struct sockaddr_in);
    char str[INET_ADDRSTRLEN];
    pid_t id;
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

    // Check for right number of arguments
	if ( argc < 2 ) usage( argv[0] );
    printf("Build %s Date: %s %s\n",VERSION,__DATE__,__TIME__);

	// Setup of the Socket for TCP Communication
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd==-1){
        sysErr("Socket Creation not successfull",-1);
	}
	else {
        printf("Socket created \n");
	}

	// Initialization of server struct with port and address
	bzero(&server_addr,addrLen);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( (u_short)atoi(argv[1] ) );

	// Binding the created socket to the server
    printf("Binding\n");
	if ( bind( sockfd, (struct sockaddr *) &server_addr, addrLen ) == -1 ) {
		sysErr( "Server Fault : BIND", -2 );
	}

    // Preparing while loop and creation of child processes
    signal(SIGCHLD, SIG_IGN);
	while ( true ) {

		// Wait for incoming TCP connection requests
		if ( (listen(sockfd,10) ) !=0 ){
            sysErr("Listen from Server failed",-1);
		}
		else{
            printf("Listening.... \n");
		}

        // Accepting a single connection on connfd
		connfd = accept(sockfd,(struct sockaddr *) &client_addr, &addrLen);
        id=fork();

        //Child process executes the requested connection code so the main process can accept the next request
        if(id==-1){
            sysErr("Fork failed",-5);
        }
        else if(id==0){
            if (connfd < 0){
                sysErr("Server accept failed",-1);
            }
            else {
                printf("Connection Succesfull IP:%s\n",str);

                // Processing the current request
                process_Request(connfd);
            }
            // Close conection
            printf("Close Connection\n");
            close(connfd);
            exit(0);
        }
        close(connfd);
	}

	close( sockfd );
	exit(0);
}

//Function Implementations
int process_Request(int socket)
{
    // Declaration of used variables in this function
    size_t len;
    int line;
    char revBuff[BUF_LEN];
    char delimiter[2]=" ";
    char *token;
    int usedbuf;
    int flag;
    char reqfile[100];
    
    usedbuf=8192-1;
    line=0;
    flag=0;

    // Reading a message from the client
    do {
        len = get_line(socket, revBuff, BUF_LEN-1);

        // Check if the entire request exceeds he 8KB limit
        usedbuf=usedbuf-len;
        if (usedbuf <= 0){
            send_status(400,socket);
            return 1;
        }
        printf("%s",revBuff);

        //Only check at the first line
        if( line == 0 ){

            token = strtok( revBuff , delimiter );
            // Check for get request
            if( strncmp( token , "GET" , 3 ) == 0 ) {
                token=strtok(NULL,delimiter);
                flag=1;
                strncpy(reqfile,token,100-1);
            }
            line++;
        }
        
    }while(((len > 0) && strcmp("\n", revBuff)));

    // Start function to send the requested file to the client
    if(flag==1){
        send_File(reqfile,socket);
    }
    //if there is no get request send 501
    else{
        send_status(501,socket);
        return 1;
    }

    return 0;
}

int send_File(char *filename,int socket)
{
    // Declaration of used variables in this function
    FILE *fp;
    char text[201];
    char *file;
    file=(char *)malloc(strlen(filename)*sizeof(char)+1);
    file[0]='.';

    // Check if the requested file wants to escape the designated area
    if(strstr(filename,"..")!=NULL){
        send_status(404,socket);
	free(file);
	return 1;
    }

    // Change to direotory of the stored files
    chdir(HTML_FILES_PATH);
    strcat(file,filename);
    fp = fopen(file,"r+");

    // Check if the requested file exists
    // If not send error
    if(fp==NULL || errno==EISDIR){
        send_status(404,socket);
        free(file);
        return 1;
    }
    // Else send the requested file
    else{
        send_status(200,socket);
        while(fgets(text,200,fp)!=NULL){   
            if(write( socket, text, strlen(text) ) ==-1 ){
                sysErr("Server Fault: SENDTO", -4);
		    }
        }
        fclose(fp);
        free(file);
    }
    return 0;
}

void create_header(status_code* code,char** header)
{
    //Get Size of formated header string and allocate the needed Memoryspace
    int len=snprintf(NULL,0,HTTP_HEADER,code->number,code->reason,SERVERNAME,VERSION)+2;
    *header=(char*)malloc(len*sizeof(char));
    snprintf(*header,len,HTTP_HEADER,code->number,code->reason,SERVERNAME,VERSION);
}

void send_status(int statuscode, int sock){
    // Send the corresponding error as a header
    char *header;
    status_code code;
    set_code(&code,statuscode);
    create_header(&code,&header);
    if(write( sock, header, strlen(header))==-1){
        sysErr("Server Fault: SENDTO",-4);
    }
    // Individual pages for better visibility of the send error
    if(statuscode ==501){
        if(write(sock,HTTP_BODY,strlen(HTTP_BODY)) ==-1){
            sysErr("Server Fault: SENDTO",-4);
        }
    }
    if(statuscode ==404){
        if(write(sock,HTTP_404,strlen(HTTP_404)) ==-1){
            sysErr("Server Fault: SENDTO",-4);
        }
    }
}

// The user entered something stupid. Tell him.
void usage( char *argv0 )
{
	printf( "usage : %s portnumber\n", argv0 );
	exit( 0 );
}

int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    // Reads the send in HTTP request
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

// Something unexpected happened. Report error and terminate.
void sysErr( char *msg, int exitCode )
{
	fprintf( stderr, "%s\n\t%s\n", msg, strerror( errno ) );
	exit( exitCode );
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
