#include "Client.h"


// extremely basic client connection. 
// will be replaced with actual client connection
int init_socket(char* ip_address) {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	const char* clientBuffer = "Testing";
	char recvbuf[BUFFER_LENGTH];
	int iResult;
	int recvbuflen = BUFFER_LENGTH;

	// socket creation
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		// Creating socket failed
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	// This is usually a bad choice for high latency applications like games
	// I will probably give considering to doing a UDP socket instead
	hints.ai_protocol = IPPROTO_TCP;

	// resolve server
	iResult = getaddrinfo(ip_address, PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return 1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// create socket to connect to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			return 1;
		}
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		return 1;
	}
	// send initial buffer
	iResult = send(ConnectSocket, clientBuffer, (int)strlen(clientBuffer), 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	} while (iResult > 0);


	// cleanup socket
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;

}