#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 1000;

// Client로 부터 받은 데이터를 출력
void RecvThread(SOCKET* clientSocket)
{
	while (true)
	{
		char recvBuffer[BUFFER_SIZE];

		int recvLen = recv(*clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLen <= 0)
		{
			cout << "recv error : " << WSAGetLastError() << endl;
			return;
		}

		// 문자열이 합쳐져서 오기 때문에 전체 문자열 출력을 위해 \0를 공백 처리
		// 마지막 \0은 유지
		for (int i = 0; i < recvLen - 1; i++)
		{
			if (recvBuffer[i] == '\0')
				recvBuffer[i] = ' ';
		}

		cout << "Recv Data : " << recvBuffer << "(" << recvLen << ")" << endl;
	}
}

int main(void)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "startup error" << endl;
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

	if (listen(listenSocket, 10) == SOCKET_ERROR)
	{
		cout << "listen error : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));
	int addrLen = sizeof(clientAddr);

	vector<thread> threads;
	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "accept error : " << WSAGetLastError() << endl;
			return 0;
		}
		threads.push_back(thread(RecvThread, &clientSocket));
	}

	return 0;
}