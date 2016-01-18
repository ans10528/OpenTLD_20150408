
#include <time.h>
#include <stdio.h>
#include <stdlib.h>   // Needed for _wtoi

#include "TargetPosSocket.h"

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>


#include <opencv\cv.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3155"
#define DEFAULT_REMOTE_PORT "3156"



TargetPosSocket::TargetPosSocket()
{

}

TargetPosSocket::~TargetPosSocket()
{

}


int TargetPosSocketClient(CvPoint *targetPos, int *targetPos_Updated, SOCKET ClientSocket)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iSendResult;
	int iResult;

	char *data_PosX_Buf = new char[4];
	char *data_PosY_Buf = new char[4];
	char *data_Request = new char[]{5};
	char *data_Request_Buf = new char[1];
	int dataLen = 0;



	// 對照c#端 - 系統時間
	//time_t t = time(0);
	//printf("%d", t);
	//system("pause");
	//return 0;

	// C# ---------------------------------------------------
	iSendResult = send(ClientSocket, new char[1]{0}, 1, 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		shutdown(ClientSocket, 2);
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}



	// Receive until the peer shuts down the connection
	do {

		//等待用戶端要求傳送座標資訊
		if (data_Request_Buf[0] != data_Request[0])
		{
			iResult = recv(ClientSocket, data_Request_Buf, 1, 0);
			printf("recv TargetPos request  (if %d == %d).\n", data_Request[0], data_Request_Buf[0]);
		}
		if (data_Request_Buf[0] == data_Request[0])
		{
			//確認座標是否可使用
			if (*targetPos_Updated == 1) 
			{
				data_Request_Buf[0] = 0;

				int PosX = targetPos->x
					, PosY = targetPos->y;

				*targetPos_Updated = 0;

				for (int i = 0; i < 4; i++)
				{
					data_PosX_Buf[i] = (PosX >> ((3-i) * 8) & 255);
					data_PosY_Buf[i] = (PosY >> ((3-i) * 8) & 255);
				}

				send(ClientSocket, data_PosX_Buf, 4, 0);
				send(ClientSocket, data_PosY_Buf, 4, 0);
				printf("TargetPos Reply = %d,%d\n", PosX, PosY);
			}
		}





		//dataLen = (unsigned char) data_Len_Buf[0] << 24 | (unsigned char) data_Len_Buf[1] << 16 | (unsigned char) data_Len_Buf[2] << 8 | (unsigned char) data_Len_Buf[3];



		if (iResult == SOCKET_ERROR || iResult == 0) {
			printf("ClientSocketDisconnection %d\n", WSAGetLastError());
			shutdown(ClientSocket, 2);
			closesocket(ClientSocket);
			return 1;
		}
		//測試影像接收是否正常
		//Mat cppImg;
	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		shutdown(ClientSocket, 2);
		closesocket(ClientSocket);
		//WSACleanup();
		return 1;
	}

	// cleanup
	shutdown(ClientSocket, 2);
	closesocket(ClientSocket);
	return 0;
}

int TargetPosSocket::newTargetPosSocket(CvPoint *targetPos, int *targetPos_Updated)
{

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;



	std::cout << "TargetPosSocket" << std::endl;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("TargetPos WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("TargetPos getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("TargetPos socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("TargetPos bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("TargetPos listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	int clientResult = 0;
	do
	{
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL); //同步
		if (ClientSocket == INVALID_SOCKET) {
			printf("TargetPos accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		std::cout << "TargetPosClientConnect" << std::endl;
		clientResult = TargetPosSocketClient(targetPos,targetPos_Updated,ClientSocket);
		std::cout << "TargetPosClientClose" << std::endl;

	} while (clientResult != 0);

	std::cout << "TargetPos ListenSocketClose" << std::endl;

	// No longer need server socket
	closesocket(ListenSocket);
	WSACleanup();

	//system("PAUSE");
	return 1;
}
