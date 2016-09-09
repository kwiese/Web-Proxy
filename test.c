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

struct ret {
	char *hostSplit;
	char hostNoSplit[1024];
	char *ip;
	int port;
};

int portCheck(char *request);
char **getTypes(char stringToSplit[]);
int hostname_to_ip(char *hostname , char *ip);
struct ret portCheck2(char *request);
int main()
{
	char *noPort = "GET http://www.yahoo.com/ HTTP/1.0\nHost: www.google.com\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:41.0) Gecko/20100101 Firefox/41.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive";
	//char *withPort = "GET http://www.bbc.com/ HTTP/1.0\nHost: www.bbc.com:4040\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:41.0) Gecko/20100101 Firefox/41.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive";
	//char *ptr = strstr(testStr, "Host: ");
	//int port = portCheck(noPort);
	//printf("Port: %d\n", port);
	struct ret test;
	test = portCheck2(noPort);
	if(strlen(test.hostNoSplit) > 0)
	{
		printf("Port: %d, IP: %s, hostNoSplit: %s\n", test.port, test.ip, test.hostNoSplit);
	}
	else if(strlen(test.hostSplit) > 0)
	{
		printf("Port: %d, IP: %s, hostSplit: %s\n", test.port, test.ip, test.hostSplit);
	}
	return 0;
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

int portCheck(char *request)
{
	char *ptr;
	char *req = request;
	char str[1024];
	int port = 80;
	char *hostname;
	char ip[100];
	
	ptr = strstr(req, "Host: ");
	int start = ptr - req;
	start += 6;
	printf("%d\n", start);
	int end = start;
	int i = 0;
	while(req[end] != '\n')
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
		char **split;
		split = getTypes(str);
		port = atoi(split[1]);
		hostname = split[0];
		hostname_to_ip("www.google.com", ip);
		printf("%s resolved to %s\n", hostname, ip);
	}
	else
	{
		hostname_to_ip("www.google.com", ip);
		printf("%s resolved to %s\n", str, ip);
	}
	
	return port;
}

struct ret portCheck2(char *request)
{
	char *ptr;
	char *req = request;
	char str[1024];
	int port = 80;
	
	char ip[100];
	struct ret a;
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
		a.hostSplit = hostname;
	}
	else
	{
		hostname_to_ip(str, ip);
		printf("%s resolved to %s\n", str, ip);
		strcpy(a.hostNoSplit, str);
	}
	a.ip = ip;
	a.port = port;
	 
	return a;
}

char **getTypes(char stringToSplit[])
{
	//splits up string based on spaces
	char **type = malloc(2*sizeof *type);
	const char d[] = ":";
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


//puts("receiving from web");
					/*
					timer = time(NULL);
					while((rb = recv(webSock, buffer, 8192, MSG_DONTWAIT)) < 0)
					{
						if(((((long long) (time(NULL) - timer)) > 1)))
						{
							puts("timeout");
							timeout = 1;
							break;
						}
					}
					if(timeout == 1) break;*/

