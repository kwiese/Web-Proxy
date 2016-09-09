#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct clientSock
{
	int *sockets;
};

struct receiverInfo {
	char *hostSplit;
	char hostNoSplit[1024];
	char *ip;
	int port;
};

char **getTypes(char stringToSplit[])
{
	//splits up string based on spaces
	char **type = malloc(2*sizeof *type);
	const char d[] = " ";
	char *token;
	token = strtok(stringToSplit, d);
	int i = 0;
	while(token != NULL)
	{
		type[i] = token;
		token = strtok(NULL, d);
		i++;
	}
	return type;
}

int fileSize(int fd)
{
	//grabs the file size (allows for sending files)
	struct stat stat_struct;
	if (fstat(fd, &stat_struct) == -1)
		return (-1);
		
	return (int) stat_struct.st_size;
}

void sendMsg(int fd, char *msg)
{
	//sends a message to the client
	int size = strlen(msg);
	int numBytesSent;
	printf("Size of Message: %d\n", size);
	do
	{
		//printf("Message:\n%s\n", msg);
		numBytesSent = send(fd, msg, size, MSG_NOSIGNAL);
		printf("Bytes Sent: %d\n", numBytesSent);
		if(numBytesSent <= 0) break;
		size -= numBytesSent;
		msg += numBytesSent;
	}while(size > 0);
}
int uriCheck(char *request, int fd)
{
	//checks to make sure the uri requested is valid
	char *req = request;
	char temp[500];
	char uri[500];
	char *p1, *p2;
	p2 = strstr(req, " HTTP");
	p1 = strchr(req, '/');
	int n = p2 - p1;
	strncpy(temp, request + 4, n);
	strcpy(uri, temp);
	if(strlen(uri) == 0)
	{
		strcat(uri, "/index.html");
	}
	char *chr = uri;
	printf("URI: %s", uri);
	char *allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=";
	while(*chr)
	{
		if(strchr(allowed, *chr) == NULL)
		{
			char str[500];
			printf("HTTP/1.1 400 Bad Request: Invalid Method: %s\n", uri);
			sprintf(str, "HTTP/1.1 400 Bad Request: Invalid URI: %s\rnContent-Type: text/html\r\n\r\n", uri);
			sendMsg(fd, str);
			sendMsg(fd, "<html><head><title>400 Invalid URI</head></title>");
			sendMsg(fd, "<body><p>400 Unsuported URI</p></body></html>");
			return -1;
		}
		chr++;
	}
	return 1;
}

void send500(int fd)
{
	//sends 500 error if server error occurs
	printf("HTTP/1.1 500 Internal Server Error: cannot allocate memory\n");
	//sendMsg(fd, "HTTP/1.1 500 Internal Server Error: cannot allocate memory\r\n");
}

int methodCheck(char *request, int fd)
{
	//checks to make sure it is a GET request
	char *req = request;
	//char str[1024];
	if(strncmp(req, "GET ", 4) == 0)
	{
		return 1;
	}
	else
	{
		int n;
		char method[1024];
		n = strchr(req, '/') - req;
		strncpy(method, req, n);
		strncpy(method, method, strlen(method) - 1);
		printf("HTTP/1.1 400 Bad Request: Invalid Method: %s\n", method);
		/*
		sprintf(str, "HTTP/1.1 400 Bad Request: Invalid Method: %s\rnContent-Type: text/html\r\n\r\n", method);
		sendMsg(fd, str);
		sendMsg(fd, "<html><head><title>400 Invalid Method</head></title>");
		sendMsg(fd, "<body><p>400 Unsuported Method Type</p></body></html>");*/
		return -1;
	}
	
	return 1;
}

int hostname_to_ip(char *hostname , char *ip)
{
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
	
    if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
 
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
     
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

struct receiverInfo getReceiverInfo(char *request)
{
	char *ptr;
	char *req = request;
	char str[1024];
	int port = 80;
	
	char ip[100];
	struct receiverInfo info;
	ptr = strstr(req, "Host: ");
	int start = ptr - req;
	start += 6;
	printf("%d\n", start);
	int end = start;
	int i = 0;
	while(req[end] != '\r')
	{
		str[i] = req[end];
		end++;
		i++;
	}
	str[i] = '\0';
	printf("%s\n", str);
	ptr = 0;
	ptr = NULL;
	ptr = strchr(str, ':');
	if(ptr != NULL)
	{
		char *hostname;
		char **split;
		split = getTypes(str);
		port = atoi(split[1]);
		hostname = split[0];
		hostname_to_ip(hostname, ip);
		printf("%s resolved to %s\n", hostname, ip);
		info.hostSplit = hostname;
	}
	else
	{
		hostname_to_ip(str, ip);
		printf("%s resolved to %s\n", str, ip);
		strcpy(info.hostNoSplit, str);
	}
	info.ip = ip;
	info.port = port;
	 
	return info;
}



void con(void *args)
{
	//set up variables
	struct clientSock* arguments = args;
	int* temp = arguments->sockets;
	int fd = *temp;
	char request[8192], *ptr;
	//time_t timer = 0;
	recv(fd, request, 8192, 0);
	char cp[8192];
	while(recv(fd, cp, 8192, MSG_DONTWAIT) > 0)
	{
		int i;
		strcat(request, cp);
		for(i = 0; i < 8192; i++)
		{
			cp[i] = 0;
		}
	}
	//printf("Request: %s\n", request);
	if(strlen(request) > 0)
	{
		ptr = strstr(request, " HTTP/");
		if(ptr == NULL)
		{
			puts("Not HTTP Request!");
		}
		else
		{
			int methodOk = methodCheck(request, fd);
			//int uriOk = uriCheck(request, fd);
			//int methodOk = 1;
			int uriOk = 1;
			if(methodOk == 1 && uriOk == 1);
			{
				struct receiverInfo recvInf = getReceiverInfo(request);
				int webSock;
				struct sockaddr_in server;
				webSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(webSock < 0)
				{
				  printf("could not create web socket");
				}
				puts("Web socekt created");

				server.sin_addr.s_addr = inet_addr(recvInf.ip);
				server.sin_family = AF_INET;
				server.sin_port = htons(recvInf.port);

				if(connect(webSock, (struct sockaddr *)&server, sizeof(server)) < 0)
				{
					perror("connect failed. Error");
					return;
				}
				puts("Web socket connected");
				
				sendMsg(webSock, request);
				
				char buffer[8192];
				ssize_t rb;
				ssize_t amt;
				ssize_t offset;
				while(1)
				{
					rb = recv(webSock, buffer, 8192, 0);
					if(rb <= 0)
					{
						if(rb == 0) break;
						if(errno == EINTR || errno == EAGAIN) continue;
						perror("receive error");
						break;
					}
					offset = 0;
					amt = rb;
					while(amt > 0)
					{
						rb = send(fd, buffer + offset, amt, 0);
						if(rb < 0)
						{
							if (errno == EINTR || errno == EAGAIN) continue;
							perror("write error");
							break;
						}
						offset += rb;
						printf("Bytes Sent: %zd\n", rb);
						amt -= rb; 
					}
				}
				shutdown(webSock, SHUT_RDWR);
			}
		}
	}
	//puts("done out");
	free(arguments);
	pthread_exit((void *)0);
}

int main(int argc, char *argv[])
{
	//parse ws.conf and set up variables
	
	int sock;
	int listener;
	int port;
	struct sockaddr_in cli_addr;
	struct sockaddr_in serv_addr;
	socklen_t cli_len = sizeof(cli_addr);
	//make server socket
	port = atoi(argv[1]);
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Error on socket creation");
		exit(-1);
	}
	//zero adn set attributes of server struct
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	//bind the socket
	if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 )
	{
		send500(sock);
		perror("Error on bind");
		exit(-1);
	}
	//listen for connections on the server socket
	if(listen(sock, 5) == -1)
	{
		send500(sock);
		perror("Error on listen");
		exit(-1);
	}
	while(1)
	{
		//accept and make client socket
		struct clientSock *clisocket = malloc(sizeof(struct clientSock));
		listener = accept(sock, (struct sockaddr*)&cli_addr, &cli_len);
		clisocket->sockets = &listener;
		printf("got connection\n");
		if(listener == -1)
		{
			send500(sock);
			perror("Error on accept");
		}
		//allow for concurrent connections using pthreads
		pthread_t tid;
		int rc;
		rc = pthread_create(&tid, NULL,(void *) con, (void *) clisocket);
		if(rc)
		{
			send500(sock);
			perror("Error in pthread_create");
			close(sock);
			exit(EXIT_FAILURE);
		}
	}
	shutdown(sock, SHUT_RDWR);
	return 0;
}
