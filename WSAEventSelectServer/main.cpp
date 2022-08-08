#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// WSAEventSelect
// ���� : WSACreateEvent (manual-reset + non-signaled ���·� ����)
// ���� : WSACloseEvent
// ���� ���� : WSAWaitForMultipleEvents
// ��ü���� ��Ʈ��ũ �̺�Ʈ �˾Ƴ��� : WSAEnumNetworkEvents

// - ��Ʈ��ũ �̺�Ʈ
// FD_ACCEPT : Ŭ���̾�Ʈ ����
// FD_READ
// FD_WRITE
// FD_CLOSE
// FD_CONNECT : ���� ���� �Ϸ�
// FD_OOB

struct Session
{
	SOCKET socket;
	char recvBuffer[100];
	// ��Ÿ ������...
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

	// listenSocket�� WSAEVENT, ������ ��Ʈ��ũ �̺�Ʈ�� ����
	// listen�̹Ƿ� FD_ACCEPT, FD_CLOSE ����
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
		// �̺�Ʈ�� �����Ͽ� �߻��� �̺�Ʈ�� index�� ������
		int index = WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
		{
			cout << "WSAWaitFailed" << endl;
			continue;
		}
		index -= WSA_WAIT_EVENT_0;

		// � ��Ʈ��ũ �̺�Ʈ�� �߻��Ͽ����� �˾Ƴ�
		WSANETWORKEVENTS networkEvents;
		if (WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
		{
			cout << "WSAEnumNetworkEvents error : " << WSAGetLastError() << endl;
			continue;
		}

		// ��Ʈ��ũ �̺�Ʈ�� accept�� ���
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

		// ��Ʈ��ũ �̺�Ʈ�� read�� ���
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

		// FD_CLOSE ó��
		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			// TODO. Remove Socket
		}
	}

	WSACleanup();

	return 0;
}