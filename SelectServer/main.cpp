// session : 연결된 client = 1 : 1

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
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[100];
	int recvLen = 0;
};

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

	vector<Session> sessions;
	fd_set reads;

	while (true)
	{
		// fd_set 초기화
		FD_ZERO(&reads);

		// socket 등록 - listen socket
		FD_SET(listenSocket, &reads);
		// socket 등록 - 연결된 client socket
		for (Session& s : sessions)
		{
			if (s.socket != INVALID_SOCKET && s.recvLen == 0)
			{
				FD_SET(s.socket, &reads);
			}
		}

		if (select(0, &reads, nullptr, nullptr, nullptr) == SOCKET_ERROR)
		{
			cout << "select error : " << WSAGetLastError() << endl;
			return 0;
		}

		// listen socket 체크
		if (FD_ISSET(listenSocket, &reads))
		{
			SOCKADDR_IN clientAddr;
			memset(&clientAddr, 0, sizeof(clientAddr));
			int addrLen = sizeof(clientAddr);
			SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connected!" << endl;
				sessions.push_back(Session{ clientSocket });
			}
		}
		// 연결된 client socket 체크
		for (auto sIter = sessions.begin(); sIter != sessions.end(); sIter++)
		{
			if (FD_ISSET(sIter->socket, &reads))
			{
				int recvLen = recv(sIter->socket, sIter->recvBuffer, sizeof(sIter->recvBuffer), 0);
				if (recvLen <= 0)
				{
					cout << "recv error : " << WSAGetLastError() << endl;
					closesocket(sIter->socket);
					sessions.erase(sIter);
					continue;
				}

				sIter->recvLen = recvLen;
				cout << sIter->recvBuffer << endl;
				sIter->recvLen = 0;
			}
		}
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}