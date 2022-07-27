#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(void)
{
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		return 0;
	}

	// ���� ����ϴ� socket option��...
	
	// 1) ���� ���� (SOL_SOCKET)
	
	// SO_KEEPALIVE
	// �ֱ������� ���� ���¸� Ȯ�� (TCP Only)
	bool enable = true;
	setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
	
	// SO_LINGER
	// socket close �� buffer�� ���� ������ ó���� ���� ���� ����
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;    // ���ð�
	setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	// SEND, RECV Buffer ũ�� ����
	int bufferSize;
	int optionLen = sizeof(bufferSize);
	getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, &optionLen);
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, optionLen);

	// SO_REUSEADDR
	// ip�ּҿ� port ����
	enable = true;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));

	// 2) TCP ���� (IPPROTO_TCP)

	// Nagle �˰��� on/off
	// �����Ͱ� ����� ���϶� ���� ����ߴٰ� �� ���� �����ϴ� ���
	// ���� ��Ŷ�� ���� �����Ǵ� ���� ������������, �ٷ� ������ ���� �ʱ� ������ ���� �ð����� ���ظ� �� ���� ����
	enable = true;
	setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

	return 0;
}