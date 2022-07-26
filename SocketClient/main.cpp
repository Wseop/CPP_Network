// Socket Programming
// SocketServer와 함께 실행

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
	// socket 생성
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
	// winsock 초기화
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup error" << endl;
		return 0;
	}

	// 연결할 주소 세팅 (접속할 서버의 주소)
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	// host to network short
	// endian 변환
	// host(일반적으로 pc)는 little endian을 사용하고 network상에서는 big endian을 사용함
	serverAddr.sin_port = htons(7777);

	// TODO Something
	// 1초마다 스레드 하나씩 생성해서 서버로 연결
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