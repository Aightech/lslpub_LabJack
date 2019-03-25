#include "tcp.h"
#include <stdio.h>
#include <errno.h>

#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#endif

#define	DEBUG 0

void printPacket(const unsigned	char *packet, int size)
{
	int i = 0;
	printf("PACKET (%d): ", size);
	if(size > 0)
	{
		for(i = 0; i < size; i++)
			printf("%02X ", packet[i]);
	}
	printf("\n");
}

TCP_SOCKET openTCP(const char *ipAddress, int port)
{
	TCP_SOCKET sock;
	struct sockaddr_in address;
	
	struct hostent *he;

#ifdef WIN32
	WSADATA	info;

	if (WSAStartup(MAKEWORD(1,1), &info) != 0)
	{
		printf("Error: Cannot initilize winsock\n");
		return INVALID_SOCKET;
	}
#endif
	printf("%d\n", IPPROTO_TCP);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket. Exiting\n");
		return INVALID_SOCKET;
	}
	
	address.sin_family=AF_INET;
	address.sin_port=htons(port);


	he = gethostbyname(ipAddress);
	address.sin_addr = *((struct in_addr *)he->h_addr);


	if((connect(sock,(struct sockaddr *)&address,sizeof(address))) < 0)
	{
		fprintf(stderr, "Could not connect to %s:%d\n", inet_ntoa(address.sin_addr), port);
		return INVALID_SOCKET;
	}

	return sock;
}

int	setCommTimeoutTCP(TCP_SOCKET sock, int seconds)
{
	int tvSize = 0;
#ifdef WIN32
	int tv = 0;
	tv = seconds*1000;
	tvSize = sizeof(int);
#else
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	tvSize = sizeof(struct timeval);
#endif
	if(setsockopt(sock,	SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, tvSize) <	0)
	{
		printf("Error setting the RCV timeout.");
		return -1;
	}
	if(setsockopt(sock,	SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, tvSize) <	0)
	{
		printf("Error setting the SND timeout.");
		return -1;
	}
	return 0;
}

int writeTCP(TCP_SOCKET sock, const unsigned char *packet, int size)
{
	int ret = 0;
	ret = send(sock, (const char *)packet, size, 0);
	if(ret != size)
	{
		printf("Unexpected write response size: Response = %d, Expected = %d\n", ret, size);
		if(ret >= 0)
			printPacket(packet, ret);
	}
	if(DEBUG)
	{
		printf("WRITE ");
		printPacket(packet, size);
	}
	return ret;
}

int	readTCP(TCP_SOCKET sock, unsigned char *packet,	int	size)
{
	int ret = 0;
	ret = recv(sock, (char *)packet, size, 0);
	if(ret != size)
	{
        if(ret < 0 && errno == EINTR)
			printf("\nTCP read interrupted.");
		else
			printf("Unexpected read response size: Response = %d, Expected = %d\n", ret, size);

		if(ret >= 0)
			printPacket(packet, ret);
	}
	if(DEBUG)
	{
		printf("READ ");
		printPacket(packet, ret);
	}
	return ret;
}

int	closeTCP(TCP_SOCKET	sock)
{
	int err = 0;
	if(sock == INVALID_SOCKET)
		return -1;
#ifdef WIN32
	err = closesocket(sock);
	WSACleanup();
	return err;
#else
	return close(sock);
#endif
}
