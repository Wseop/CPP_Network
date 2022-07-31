#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

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

	// socket non-blocking setting
	u_long on = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		cout << "non-blocking enable error : " << WSAGetLastError() << endl;
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

	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));
	int addrLen = sizeof(clientAddr);

	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			// non-blocking handling
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << ".";
				continue;
			}

			cout << "accept error : " << WSAGetLastError() << endl;
			return 0;
		}

		cout << endl << "Client Connected!" << endl;

		while (true)
		{
			char recvBuffer[100];
			int recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{
				// non-blocking handling
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					cout << ".";
					continue;
				}

				cout << "recv error : " << WSAGetLastError() << endl;
				return 0;
			}

			cout << endl << "Client Message : " << recvBuffer << endl;
		}
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}