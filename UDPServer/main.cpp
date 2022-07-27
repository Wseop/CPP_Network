#include <iostream>
#include <vector>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 100;

int main(void)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "socket error : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);

	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "bind error : " << WSAGetLastError() << endl;
		return 0;
	}

	// UDP ��� ���� : listen�� accept ���� �ٷ� �ۼ���
	while (true)
	{
		SOCKADDR_IN clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int addrLen = sizeof(clientAddr);

		char recvBuffer[BUFFER_SIZE];
		int recvLen = recvfrom(serverSocket, recvBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddr, &addrLen);
		if (recvLen <= 0)
		{
			cout << "recvfrom error : " << WSAGetLastError();
			return 0;
		}

		// UDP ����� Boundary ������ �����ϱ� ������ 
		// �������� �����Ͱ� �� ���� �͵� �˾Ƽ� �����ؼ� �޾ƿ�
		// cf.) RecvServer �ڵ�
		cout << "Recv Data : " << recvBuffer << "(" << recvLen << ")" << endl;
	}

	return 0;
}