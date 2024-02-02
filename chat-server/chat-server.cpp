#include "../common/refs.h"
#include <map>
#include <thread>

using port_t = short;



struct clientInfo {
	clientInfo() = default;
	clientInfo(std::string_view clUserName , SOCKET s) : userid(clUserName), clSocket(s) {	
	}
	std::string userid;
	SOCKET clSocket = INVALID_SOCKET;
};

std::map<SOCKET, clientInfo> clientList;
std::vector<std::thread> workers;

int acceptClient(SOCKET s);

void *newClThread(SOCKET s, std::string_view cl_name);
void msg_to_all(char buffer[], int buffSize, SOCKET s);
bool isPM(char* buffer);
std::string totalUsers();

// a very basic server
int main() {
	WsaWrapper wsa;





	int rc;
	port_t port = 3232; // some random port

	socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); assert(s > 0);

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	rc = bind(s, (sockaddr*)&addr, sizeof(addr)); assert(rc == 0);
	rc = listen(s, 8);



	char buf[128];
	socklen_t buflen = sizeof(buf);
	while (serverIsRunning) {
		socket_t cli = accept(s, (sockaddr*)buf, &buflen);		
		if (cli != INVALID_SOCKET)
			acceptClient(cli);
	}

	closesocket(s);
}

int acceptClient(SOCKET s)
{
	char buffer[16];
	ZeroMemory(&buffer, 16);
	int bytes = recv(s, buffer, sizeof(buffer), 0);
	if (bytes > 0)
	{
		std::string userID = buffer;

		clientList[s] = clientInfo(userID, s);
		printf("User %s accepted on socket: %d\n",userID.data(), (int)s);
		printf("Online Users: %d\n", static_cast<int>(clientList.size()));
		bytes = send(s, "OK", 2, 0);
		workers.push_back(std::thread(&newClThread, s, userID));
		

	}
	else {
		printf("User not accepted\n");
	}

	return -1;
}

void *newClThread(SOCKET s, std::string_view cl_name)
{
	char buffer[128];
	ZeroMemory(buffer, 128);
	while (true) {

		int nBytes = recv(s, buffer, 128, 0);
		//if buffer = "-list" the server should list all users... to this one.
		if (nBytes > 0)
		{
			if (strcmp(buffer, "-list") == 0)
			{
				printf("user asks for clientlist --- debug log\n ");
				send(s, totalUsers().c_str(), 128, 0);
			}
			else if (isPM(buffer))
			{
				printf("its a PM --- debug log\n ");
				//function to send msg to specific client
				
			}
			else
				msg_to_all(buffer, nBytes, s);
			
			
		}
			
		if (nBytes == 0)
		{
			printf("Client [%s] disconnected.\n", cl_name.data());
			//std::map<SOCKET, clientInfo>::iterator it;
			clientList.erase(s);
			break;
		}

	}
	return 0;
}

void msg_to_all(char buffer[],int buffSize, SOCKET s)
{





	for (auto it = clientList.begin(); it != clientList.end(); ++it)
	{
		
		if (it->first != s)
		{
			send(it->first, buffer, 128, 0);

			printf("sending msg [%s] to: %s\n", buffer,it->second.userid.c_str());
		}


	}

}

bool isPM(char *buffer)
{
	if ((buffer[0] == '-') && (buffer[1] == 'p') && (buffer[2] == 'm') && (buffer[3]== ' '))
		return true;
	else
		return false;
}

std::string totalUsers()
{

	std::string users = "--------------\nOnline Users: ";
	users.append(std::to_string(clientList.size()));

	users.append("\n--------------\n");

	for (auto it = clientList.begin(); it != clientList.end(); it++)
	{
		users.append("\t" + it->second.userid + "\n");

	}

	users.append("--------------\n");

	return users;
}