#include <opencv\cv.h>
#include <thread>
#include "ImAcq.h"

class SocketImageStream
{
public:
	SocketImageStream();
	~SocketImageStream();
	int newSocketImageStream(ImAcq *imAcq);
	IplImage SocketImage;

	std::thread mThread;

private:

};
