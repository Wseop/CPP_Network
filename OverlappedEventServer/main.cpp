#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Overlapped IO (비동기 + 논블로킹) - Event 방식

struct Session
{
	SOCKET socket;
	char recvBuffer[100];
	WSAOVERLAPPED overlapped;
};

void RecvThread(Session session)
{
	while (true)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = session.recvBuffer;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;
		if (WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSA_IO_PENDING)
			{
				WSAWaitForMultipleEvents(1, &session.overlapped.hEvent, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
			}
			else
			{
				cout << "WSARecv error : " << err << endl;
				break;
			}

			cout << wsaBuf.buf << endl;
		}
	}

	closesocket(session.socket);
	WSACloseEvent(session.overlapped.hEvent);
}

int main(void)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "socket error : " << WSAGetLastError() << endl;
		return 0;
	}

	u_long on = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		cout << "ioctlsocket error : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);

	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "bind error : " << WSAGetLastError() << endl;
		return 0;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "listen error : " << WSAGetLastError() << endl;
		return 0;
	}

	// 지속적으로 클라이언트의 연결을 확인 (accept)
	// 연결된 클라이언트에서 메세지를 보내오면 콘솔 출력
	vector<thread> threads;

	while (true)
	{
		SOCKADDR_IN clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK)
			{
				cout << "accept error : " << err << endl;
				continue;
			}
			// pending accept
			cout << ".";
			this_thread::sleep_for(100ms);
		}
		else {
			Session session;
			session.socket = clientSocket;
			WSAEVENT wsaEvent = WSACreateEvent();
			session.overlapped.hEvent = wsaEvent;
			threads.push_back(thread(RecvThread, session));

			cout << "Client Connected." << endl;
		}
	}
	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}

	WSACleanup();

	return 0;
}