/**
 * Name: tcp.h
 * Desc: Provides TCP functions.
 * Auth: LabJack Corp.
**/
#ifndef TCP_H_
#define TCP_H_

#ifdef WIN32
#include <winsock.h>
typedef SOCKET TCP_SOCKET;
#else
#include <sys/socket.h>
typedef int TCP_SOCKET;
#define INVALID_SOCKET -1
#endif

#define TCP_MAX_PACKET_BYTES 1040

//For debugging purposes. Prints a packet/array to the terminal.
void printPacket(const unsigned char *packet, int size);

//Opens a TCP socket to a device. Returns the socket, or INVALID_SOCKET on
//error.
//ipAddress: A char array string representing the IP address of the device.
//port: The port of the device. This should be 502 for command/response mode
//      operations or 702 for auto response mode streaming.
TCP_SOCKET openTCP(const char *ipAddress, int port);

//Sets the write/read TCP communication timeouts.
//sock: The device's socket.
//seconds: The timeout in seconds.
int setCommTimeoutTCP(TCP_SOCKET sock, int seconds);

//Writes/sends a packet to the device. Returns -1 on error, 0 on success.
//sock: The device's socket.
//packet: The packet to send to the device. This is an unsigned char array.
//size: The number of bytes to send.
int writeTCP(TCP_SOCKET sock, const unsigned char *packet, int size);

//Reads/retreives a packet from a device. Returns -1 on error, 0 on success.
//sock: The device's socket.
//packet: The packet from the the device. This is a returned unsigned char
//        array.
//size: The number of bytes to read.
int readTCP(TCP_SOCKET sock, unsigned char *packet, int size);

//Closes a socket. Returns -1 on error, 0 on success.
int closeTCP(TCP_SOCKET sock);

#endif
