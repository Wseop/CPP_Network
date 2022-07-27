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

	// 자주 사용하는 socket option들...
	
	// 1) 소켓 레벨 (SOL_SOCKET)
	
	// SO_KEEPALIVE
	// 주기적으로 연결 상태를 확인 (TCP Only)
	bool enable = true;
	setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
	
	// SO_LINGER
	// socket close 시 buffer에 남은 데이터 처리를 위해 지연 종료
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;    // 대기시간
	setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	// SEND, RECV Buffer 크기 관련
	int bufferSize;
	int optionLen = sizeof(bufferSize);
	getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, &optionLen);
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, optionLen);

	// SO_REUSEADDR
	// ip주소와 port 재사용
	enable = true;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));

	// 2) TCP 레벨 (IPPROTO_TCP)

	// Nagle 알고리즘 on/off
	// 데이터가 충분히 쌓일때 까지 대기했다가 한 번에 전송하는 방식
	// 작은 패킷이 자주 생성되는 것을 방지해주지만, 바로 응답을 하지 않기 때문에 반응 시간에서 손해를 볼 수도 있음
	enable = true;
	setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

	return 0;
}