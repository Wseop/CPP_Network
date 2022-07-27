#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void SendThread(SOCKET* clientSocket, SOCKADDR_IN* serverAddr, int threadId)
{
	string sendData = "Hello I'm Thread " + to_string(threadId);

	while (true)
	{
		if (sendto(*clientSocket, sendData.c_str(), sendData.size() + 1, 0, (SOCKADDR*)serverAddr, sizeof(*serverAddr)) == SOCKET_ERROR)
		{
			cout << "sendto error : " << WSAGetLastError() << endl;
			return;
		}
		cout << "Send Data : " << sendData << "(" << sendData.size() + 1 << ")" << endl;

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

	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
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

	// UDP 방식 클라이언트 : connect 없이 바로 송수신
	vector<thread> threads;
	for (int i = 0; i < 5; i++)
	{
		threads.push_back(thread(SendThread, &clientSocket, &serverAddr, i));
	}
	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}

	return 0;
}