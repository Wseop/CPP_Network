#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <thread>

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

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "socket error : " << WSAGetLastError() << endl;
		return 0;
	}

	// socket non-blocking setting
	u_long on = 1;
	if (ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		cout << "non-blocking enable error : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);

	while (true)
	{
		if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			// non-blocking handling
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << ".";
				continue;
			}
			// ¿¬°áµÆÀ¸¸é break
			if (WSAGetLastError() == WSAEISCONN)
			{
				cout << endl << "Connected to Server!" << endl;
				break;
			}
			
			cout << endl << "connect error : " << WSAGetLastError() << endl;
			return 0;
		}
	}

	char sendBuffer[100] = "Hello World";
	while (true)
	{
		if (send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR)
		{
			// non-blocking handling
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << ".";
				continue;
			}

			cout << endl << "send error : " << WSAGetLastError() << endl;
			return 0;
		}

		cout << endl << "Send!" << endl;

		this_thread::sleep_for(1s);
	}

	closesocket(clientSocket);
	WSACleanup();

	return 0;
}