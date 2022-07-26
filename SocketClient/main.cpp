// Socket Programming
// SocketServer�� �Բ� ����

#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void ThreadFunc(SOCKADDR_IN* serverAddr)
{
	// socket ����
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		cout << "socket error : " << error << endl;
		return;
	}

	// connect!
	if (connect(clientSocket, (SOCKADDR*)serverAddr, sizeof(*serverAddr)) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		cout << "connect error : " << error << endl;
		return;
	}

	cout << "Connected to server!" << endl;

	while (true)
	{
		this_thread::sleep_for(10s);
	}
}

int main(void)
{
	// winsock �ʱ�ȭ
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	// ������ �ּ� ���� (������ ������ �ּ�)
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	// host to network short
	// endian ��ȯ
	// host(�Ϲ������� pc)�� little endian�� ����ϰ� network�󿡼��� big endian�� �����
	serverAddr.sin_port = htons(7777);

	// TODO Something
	// 1�ʸ��� ������ �ϳ��� �����ؼ� ������ ����
	vector<thread> threads;
	while (true)
	{
		threads.push_back(thread(ThreadFunc, &serverAddr));
		this_thread::sleep_for(1s);
	}
	for (thread& t : threads)
		t.join();

	WSACleanup();

	return 0;
}