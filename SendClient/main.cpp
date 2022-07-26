// send, recv는 blocking 계열 함수
// blocking 여부는 내부 buffer의 상태에 따라 결정됨
/*
	       [client]             |              [server]
	<user>        <kernel>      |     <kernel>          <user>
	send()  --->  SendBuffer    ->    RecvBuffer  --->  recv()
	recv()  <---  RecvBuffer    <-    SendBuffer  <---  send()
*/

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// 5초마다 자신의 threadId를 서버로 전송
void SendThread(SOCKET* clientSocket, int threadId)
{
	string sendData = "I'm Thread " + to_string(threadId);
	
	while (true)
	{
		if (send(*clientSocket, sendData.c_str(), sendData.size() + 1, 0) == SOCKET_ERROR)
		{
			cout << "send error @" << threadId << " : " << WSAGetLastError() << endl;
			return;
		}

		cout << "send data : " << sendData << "(" << sendData.size() + 1 << ")" << endl;

		this_thread::sleep_for(1s);
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

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "socket error : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);

	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "connect error : " << WSAGetLastError() << endl;
		return 0;
	}

	cout << "Connected to server" << endl;

	vector<thread> threads;
	for (int i = 0; i < 5; i++)
	{
		threads.push_back(thread(SendThread, &clientSocket, i));
	}
	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}

	closesocket(clientSocket);
	WSACleanup();

	return 0;
}