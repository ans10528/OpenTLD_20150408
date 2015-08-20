//#include <stdio.h>
//#include <stdlib.h>

// link with Ws2_32.lib
//#pragma comment(lib,"Ws2_32.lib")
//#include <WinSock2.h>
//#include <ws2tcpip.h>


#include <opencv\cv.h>

#include "ImAcq.h"


class SocketImageStream
{
public:
	SocketImageStream();
	~SocketImageStream();
	int newSocketImageStream(ImAcq *imAcq);
	IplImage SocketImage;

	//std::thread mThread;

private:
	//int ImageSocketClient(ImAcq *imAcq, SOCKET ClientSocket);
};
