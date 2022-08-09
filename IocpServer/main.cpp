#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <cassert>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Session
{
	SOCKET socket;
	char recvBuffer[100];
};

enum IO_TYPE
{
	READ,
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped;
	int type;	// IO_TYPE
};

void WorkerThread(int threadId, HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		cout << "[Thread " << threadId << "] Wait Job..." << endl;
		bool ret = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, 
			(PULONG_PTR)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);
		if (ret == FALSE || bytesTransferred == 0)
			continue;
		
		assert(overlappedEx->type == IO_TYPE::READ);

		cout << "[Thread " << threadId << "]" << session->recvBuffer << endl;

		// recv �ٽ� ���
		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;
		WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
	}
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

	vector<Session*> sessions;
	// CompletionPort ����
	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// WorkerThread ���� (����Ǹ� GetQueuedCompletionStatus���� �۾� ���)
	vector<thread> threads;
	for (int i = 0; i < 5; i++)
	{
		threads.push_back(thread(WorkerThread, i, iocpHandle));
	}

	// main thread�� accept ���
	while (true)
	{
		SOCKADDR_IN clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "accept error : " << WSAGetLastError() << endl;
			continue;
		}

		Session* session = new Session();
		session->socket = clientSocket;
		sessions.push_back(session);
		cout << "Client Connected." << endl;

		// clientSocket�� CompletionPort�� ���
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)session, 0);

		// recv��� (�񵿱�IO �۾� ���)
		// ��ϵ� �۾��� �Ϸ�� WorkerThread���� ������ ó��
		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;
		WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
	}

	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}

	WSACleanup();

	return 0;
}