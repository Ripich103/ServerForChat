#ifndef SERVERCLASS
#define SERVERCLASS

#include "SIncludes.h"





class ServerClass
{
public:
	ServerClass() = default;

	~ServerClass() = default;

	int StartServer();

private:

	void recieveData(SOCKET clientSock, const std::string& username);

	void sendData(SOCKET senderSock, const std::string& msg);

	std::atomic<bool> running;
	std::vector<SOCKET> clients;
	std::mutex clientsMutex;

	WSADATA WsaData;
	SOCKET ServSocket;
	WORD port;
	int erStat;
};

#endif // SERVERCLASS
