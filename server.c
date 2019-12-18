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

const size_t BUF_LEN = 1024;

typedef struct http_code{
    int number;
    char *reason;
} status_code;

//Functionsprototypes
int get_line(int sock, char *buf, int size);
int set_code(status_code* error, int number);
int process_Request(int socket);
int send_File(char *filename,int socket,status_code *code);
void create_header(status_code* code,char** header);
void create_header_404(status_code* code,char** header);
void send_OK(status_code* code,int sock);
void send_Error(status_code* code,char* header,int sock);
void send_404(status_code *code, int sock);
void usage( char *argv0 );
void sysErr( char *msg, int exitCode );

//Main Program
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
	//size_t len;

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
	// Initialization of Server Struct with port and address
	bzero(&server_addr,addrLen);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons( (u_short)atoi(argv[1] ) );


    printf("Binding\n");
	// Binding the created Socket to the Server
	if ( bind( sockfd, (struct sockaddr *) &server_addr, addrLen ) == -1 ) {
		sysErr( "Server Fault : BIND", -2 );
	}
    signal(SIGCHLD, SIG_IGN);
	while ( true ) {
		// Wait for incoming TCP Connection Requests
		if ( (listen(sockfd,10) ) !=0 ){
            sysErr("Listen from Server failed",-1);
		}
		else{
            printf("Listening.... \n");
		}
        // Accepting a single connection on connfd
		connfd = accept(sockfd,(struct sockaddr *) &client_addr, &addrLen);
        id=fork();
        //id=0;
        if(id==-1){
            sysErr("Fork failed",-5);
        }
        else if(id==0){
            //printf("Fork succesfull");
            if (connfd < 0){
                sysErr("Server accept failed",-1);
            }
            else {
                printf("Connection Succesfull IP:%s\n",str);
                process_Request(connfd);
            }

            printf("Close Connection\n");
            close(connfd);
            exit(0);
        }
        close(connfd);
	}

	close( sockfd );
	exit(0);
}

//Function Definition

int process_Request(int socket)
{
    size_t len;
    int line;
    char revBuff[BUF_LEN];
    char *head=NULL;
    char delimiter[2]=" ";
    char *token;
    char resource[50];
    status_code code={0,NULL};

    line=0;
    // Reading a Message from the Client
    do {
        len = get_line(socket, revBuff, BUF_LEN-1);
        printf("%s",revBuff);

        //Only check at the First Line
        if( line == 0 ){
            token = strtok( revBuff , delimiter );
            while( token != NULL ) {
                if( strncmp( token , "GET" , 3 ) == 0 ) {
                    token = strtok( NULL , delimiter);
                    strcpy(resource,token);
                    send_File(token,socket,&code);
                    token = NULL;
                }
                else{
                    send_Error(&code,head,socket);
                    return 1;
                }
            }
            line++;
        }
    }while(((len > 0) && strcmp("\n", revBuff)));
    return 0;
}

int send_File(char *filename,int socket, status_code *code)
{
    FILE *fp;
    char text[201];
    char *file;
    file=(char *)malloc(strlen(filename)*sizeof(char)+1);
    file[0]='.';
    if(strstr(filename,"..")!=NULL){
    	send_404(code,socket);
	free(file);
	return 1;
    }
    chdir(HTML_FILES_PATH);
    strcat(file,filename);
    fp = fopen(file,"r+");
    if(fp==NULL || errno==EISDIR){
        send_404(code,socket);
        free(file);
        return 1;
    }
    else{
        send_OK(code,socket);
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
    //Get Size of formated Header String and allocate the needed Memoryspace
    int len=snprintf(NULL,0,HTTP_HEADER,code->number,code->reason,SERVERNAME,VERSION)+2;
    *header=(char*)malloc(len*sizeof(char));
    snprintf(*header,len,HTTP_HEADER,code->number,code->reason,SERVERNAME,VERSION);
}

void send_OK(status_code* code,int sock)
{
    char *header;
    set_code(code,200);
    create_header(code,&header);
    if(write( sock, header, strlen(header)) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
	}
}

void send_Error(status_code* code,char* header,int sock)
{
    set_code(code,501);
    create_header(code,&header);
    if(write( sock, header, strlen(header)) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
	}
}

void send_404(status_code* code,int sock)
{
    char *header;
    set_code(code,404);
    create_header(code,&header);
    if(write( sock, header, strlen(header)) ==-1 ){
            sysErr("Server Fault: SENDTO", -4);
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
