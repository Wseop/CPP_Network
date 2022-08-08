#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// 5초마다 thread 생성 (최대 10개)
// 생성된 thread는 1초마다 메세지 전송
void ClientThread(SOCKADDR_IN* serverAddr, int threadId)
{
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "socket error : " << WSAGetLastError() << endl;
		return;
	}

	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		cout << "ioctlsocket error : " << WSAGetLastError() << endl;
		return;
	}

	while (true)
	{
		if (connect(clientSocket, (SOCKADDR*)serverAddr, sizeof(*serverAddr)) == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				continue;
			if (err == WSAEISCONN)
				break;

			cout << "connect error : " << err << endl;
			return;
		}
	}

	char sendBuffer[100] = "Hello I'm Thread ";
	strcat_s(sendBuffer, to_string(threadId).c_str());
	WSAEVENT wsaEvent = WSACreateEvent();
	WSAOVERLAPPED overlapped;
	overlapped.hEvent = wsaEvent;

	while (true)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = sendBuffer;
		wsaBuf.len = 100;

		DWORD sendLen = 0;
		DWORD flags = 0;
		if (WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr) == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSA_IO_PENDING)
			{
				WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);
			}
			else
			{
				cout << "WSASend error : " << err << endl;
				return;
			}
		}
		cout << threadId << " thread send message" << endl;

		this_thread::sleep_for(1s);
	}
}

int main(void)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);

	vector<thread> threads;
	for (int i = 0; i < 10; i++)
	{
		threads.push_back(thread(ClientThread, &serverAddr, i));
		this_thread::sleep_for(5s);
	}
	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}
	WSACleanup();

	return 0;
}