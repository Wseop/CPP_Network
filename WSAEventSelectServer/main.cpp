#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// WSAEventSelect
// 생성 : WSACreateEvent (manual-reset + non-signaled 상태로 시작)
// 삭제 : WSACloseEvent
// 상태 감지 : WSAWaitForMultipleEvents
// 구체적인 네트워크 이벤트 알아내기 : WSAEnumNetworkEvents

// - 네트워크 이벤트
// FD_ACCEPT : 클라이언트 접속
// FD_READ
// FD_WRITE
// FD_CLOSE
// FD_CONNECT : 연결 절차 완료
// FD_OOB

struct Session
{
	SOCKET socket;
	char recvBuffer[100];
	// 기타 정보들...
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

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;

	// listenSocket과 WSAEVENT, 관찰할 네트워크 이벤트를 연동
	// listen이므로 FD_ACCEPT, FD_CLOSE 연동
	WSAEVENT listenEvent = WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket });
	if (WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		cout << "WSAEventSelect error : " << WSAGetLastError() << endl;
		return 0;
	}

	while (true)
	{
		// 이벤트를 감지하여 발생한 이벤트의 index를 가져옴
		int index = WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
		{
			cout << "WSAWaitFailed" << endl;
			continue;
		}
		index -= WSA_WAIT_EVENT_0;

		// 어떤 네트워크 이벤트가 발생하였는지 알아냄
		WSANETWORKEVENTS networkEvents;
		if (WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
		{
			cout << "WSAEnumNetworkEvents error : " << WSAGetLastError() << endl;
			continue;
		}

		// 네트워크 이벤트가 accept인 경우
		if (networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
			{
				cout << "FD_ACCEPT error" << endl;
				continue;
			}

			SOCKADDR_IN clientAddr;
			memset(&clientAddr, 0, sizeof(clientAddr));
			int addrLen = sizeof(clientAddr);

			SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket == INVALID_SOCKET)
			{
				cout << "accept error : " << WSAGetLastError() << endl;
				continue;
			}

			WSAEVENT clientEvent = WSACreateEvent();
			wsaEvents.push_back(clientEvent);
			sessions.push_back(Session{ clientSocket });
			if (WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_CLOSE) == SOCKET_ERROR)
			{
				cout << "WSAEventSelect : " << WSAGetLastError() << endl;
				return 0;
			}
		}

		// 네트워크 이벤트가 read인 경우
		if (networkEvents.lNetworkEvents & FD_READ)
		{
			if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				cout << "FD_READ error" << endl;
				continue;
			}

			Session& s = sessions[index];
			int recvLen = recv(s.socket, s.recvBuffer, 100, 0);
			if (recvLen == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cout << "recv error : " << WSAGetLastError() << endl;
				return 0;
			}
			cout << s.recvBuffer << endl;
		}

		// FD_CLOSE 처리
		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			// TODO. Remove Socket
		}
	}

	WSACleanup();

	return 0;
}