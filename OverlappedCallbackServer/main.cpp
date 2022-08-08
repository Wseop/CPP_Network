#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Session
{
	// OVERLAPPED 구조체를 가장 먼저 선언
	WSAOVERLAPPED overlapped;
	SOCKET socket;
	char recvBuffer[100];
};

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	// OVERLAPPED 구조체 캐스팅하여 데이터 추출
	Session* session = reinterpret_cast<Session*>(overlapped);
	cout << "[Callback] " << session->recvBuffer << endl;
}

void RecvFunc(Session session)
{
	while (true)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = session.recvBuffer;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;
		if (WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, RecvCallback) == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSA_IO_PENDING)
			{
				// Alertable Wait
				SleepEx(INFINITE, TRUE);
			}
			else
			{
				cout << "WSARecv error : " << err << endl;
				break;
			}
		}
		else
		{
			cout << session.recvBuffer << endl;
		}
	}

	closesocket(session.socket);
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

	vector<thread> threads;

	// main thread는 accept만...
	// 클라이언트가 연결되면 새 thread를 생성하여 RecvFunc를 실행
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
		else
		{
			Session session;
			session.socket = clientSocket;
			threads.push_back(thread(RecvFunc, session));

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