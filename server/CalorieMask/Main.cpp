#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
int main()
{
	WSAData data;
	WSAStartup(MAKEWORD(2, 1), &data);
	SOCKET s = socket(AF_INET, SOCK_DGRAM, NULL);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = INADDR_ANY;
	bind(s, (SOCKADDR*)&addr, sizeof(addr));
	int addrSz = sizeof(addr);
	while (true) {
		struct {
			INT16 errorCode;
			INT16 co2;
			float wnd;
		}data;
		if (int sz = recvfrom(s, (char*)&data, sizeof(data), 0, (SOCKADDR*)&addr, &addrSz) != SOCKET_ERROR)
		{
			printf("Wind: %f Co2: %d Errors: %d\n", data.wnd, data.co2, data.errorCode);
		}
	}
}