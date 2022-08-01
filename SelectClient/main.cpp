#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// 5초마다 thread 생성 (최대 10개)
// 생성된 thread는 1초마다 메세지 전송
void ClientThread(SOCKADDR_IN* serverAddr, int threadId)
{
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Thread " << threadId << " socket error : " << WSAGetLastError() << endl;
		return;
	}

	if (connect(clientSocket, (SOCKADDR*)serverAddr, sizeof(*serverAddr)) == SOCKET_ERROR)
	{
		cout << "Thread " << threadId << " connect error : " << WSAGetLastError() << endl;
		return;
	}

	string sendBuffer = "Hello I'm Thread " + to_string(threadId);
	while (true)
	{
		if (send(clientSocket, sendBuffer.c_str(), sendBuffer.size() + 1, 0) == SOCKET_ERROR)
		{
			cout << "Thread " << threadId << " send error : " << WSAGetLastError() << endl;
			return;
		}

		this_thread::sleep_for(1s);
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

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);

	vector<thread> threads;
	for (int i = 0; i < 10; i++)
	{
		threads.push_back(thread(ClientThread, &serverAddr, i));
		this_thread::sleep_for(5s);
	}
	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}

	WSACleanup();

	return 0;
}