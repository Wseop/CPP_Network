// Socket Programming
// SocketClient와 함께 실행

#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(void)
{
	// winsock 초기화
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	// listen socket 생성
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		cout << "listen socket error : " << error << endl;
		return 0;
	}

	// 서버 주소 세팅
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);

	// bind (생성한 소켓과 주소를 연동)
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		cout << "bind error : " << error << endl;
		return 0;
	}

	// listen 시작
	if (listen(listenSocket, 10) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		cout << "listen error : " << error << endl;
		return 0;
	}

	// accept client & TODO Something
	while (true)
	{
		// 연결된 클라이언트의 소켓과 주소 정보
		SOCKADDR_IN clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int addrLen = sizeof(clientAddr);

		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			cout << "accept error : " << error << endl;
			return 0;
		}

		// 연결된 클라이언트의 ip주소를 출력
		char ipAddress[16];
		inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client connected! IP : " << ipAddress << endl;
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}