// chat-client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../common/refs.h"
#include <thread>

using port_t = short;

// a very simple client

struct clientInfo
{
	std::string userId;
};

void recvMessages(SOCKET s);
int connectInit(SOCKET s, std::string_view usrId);


int main(int argc , char *argv[]) {
	WsaWrapper wsa;

	int rc;
	port_t port = 3232; // some random port

	socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); assert(s > 0);

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	rc = connect(s, (sockaddr*)&addr, sizeof(addr)); 
	if (rc == 0)
		connectInit(s, argv[1]);
	else
		printf("Connecting to  server failed..\n");
	

	closesocket(s);
}

int connectInit(SOCKET s, std::string_view usrId)
{
	char auth[16];
	char buffer[128];
	ZeroMemory(buffer, 128);
	std::string msg;
	int bytes = send(s, usrId.data(), (int)usrId.size(), 0);
	if (bytes == SOCKET_ERROR)
		printf("Failed to send()... ERR: %d", WSAGetLastError());

	bytes = recv(s, auth, 2, 0);

	if (bytes == 2)
	{
		printf("Successful aproved by server...\n");

		std::thread t1(recvMessages, s);
	

		while (serverIsRunning) {

			
			//std::cin >> buffer;

			std::cin.getline(buffer, sizeof(buffer));
			if (strcmp(buffer,"exit") == 0)
				break;

			bytes = send(s, buffer, sizeof(buffer), 0);


		}

		t1.join();
	}
		
	else
		printf("Nickname not approved..bytes recv: %d\n",bytes);

	return 0;
}

void recvMessages(SOCKET s)
{
	char buffer[128];
	int byte = 0;

	while (true)
	{
		ZeroMemory(buffer, 128);
		byte = recv(s, buffer, sizeof(buffer), 0);

		if (byte > 0)
		{
			printf("%s \n", buffer);
		}

		if (byte == 0)
		{
			printf("Disconnected from Server...\n");
			break;
		}
	}
}