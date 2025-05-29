#include "ServerClass.h"

void ServerClass::sendData(SOCKET senderSock, const std::string& msg)
{
	std::lock_guard<std::mutex> lock(clientsMutex);
	for (SOCKET client : clients) {
		if (client != senderSock) {
			send(client, msg.c_str(), msg.size(), 0);
		}
	}
}

void ServerClass::recieveData(SOCKET clientSock, const std::string& username)
{
	std::vector<char> buffer(1024);
	while (running) {
		int bytesReceived = recv(clientSock, buffer.data(), buffer.size(), 0);
		if (bytesReceived > 0) {
			std::string msg(buffer.begin(), buffer.begin() + bytesReceived);
			if (msg == "EndConnTrue!")
			{
				std::string discmesg = "\n[" + username + "] disconnected\n";
				send(clientSock, discmesg.c_str(), discmesg.size(), 0);

				std::cout << "\n[" << username << "] requested disconnect.\n";
				{
					std::lock_guard<std::mutex> lock(clientsMutex);
					clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
				}
				closesocket(clientSock);
				break;
			}
			else if (msg == "!Help")
			{
				std::string helpMsg = "\n1./quit, disconnects you.\n2./help opens help.\n";
				send(clientSock, helpMsg.c_str(), helpMsg.size(), 0);
			}
			std::string finalmsg = "[" + username + "] > " + msg + "\n";
			std::cout << finalmsg;
			sendData(clientSock, finalmsg);
		}
		else {
			std::cout << "\nClient [" << username << "] disconnected or error.\n";
			{
				std::lock_guard<std::mutex> lock(clientsMutex);
				clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
			}
			closesocket(clientSock);
			break;
		}
	}
}

int ServerClass::StartServer()
{
	running = true;

	std::string ipaddres = "127.0.0.1";
	std::string portBuff = "";

	std::cout << "please specify your ip addres: ";
	std::getline(std::cin, ipaddres);
	std::cout << "\nplease specify your port: ";
	std::getline(std::cin, portBuff);

	port = static_cast<WORD>(std::stoi(portBuff));

	erStat = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (erStat != 0) {
		std::cout << "Error WinSock version initializaion #";
		std::cout << WSAGetLastError();
		return 1;
	}
	else
		std::cout << "WinSock initialization is DONE!\n";

	ServSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServSocket == INVALID_SOCKET)
	{
		std::cout << "Error when initializing server socket. Closing";
		closesocket(ServSocket);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Initializing server socket is DONE!\n";

	in_addr ip_to_num {};
	erStat = inet_pton(AF_INET, ipaddres.c_str(), &ip_to_num);
	if (erStat <= 0)
	{
		std::cout << "Error in IP translation to numeric format";
		closesocket(ServSocket);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Succesfully transfered IP to byte format!\n";

	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(port);
	servInfo.sin_addr = ip_to_num;

	erStat = bind(ServSocket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (erStat != 0)
	{
		std::cout << "Error socket binding to server info. Error #";
		std::cout << WSAGetLastError();
		closesocket(ServSocket);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Binding socket to Server info done!\n";

	erStat = listen(ServSocket, SOMAXCONN);

	if (erStat != 0)
	{
		std::cout << "Can`t start to listen to. Error #";
		std::cout << WSAGetLastError();
		closesocket(ServSocket);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "listening...\n";

	while (true) {
		sockaddr_in clientInfo{};
		int clientInfoSize = sizeof(clientInfo);

		SOCKET clientSock = accept(ServSocket, (sockaddr*)&clientInfo, &clientInfoSize);
		if (clientSock == INVALID_SOCKET) {
			std::cout << "Client accept failed\n";
			continue;
		}

		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
		std::cout << "Client connected: " << clientIP << "\n";

		// Receive username
		std::vector<char> ClientUser(32);
		int bytes = recv(clientSock, ClientUser.data(), ClientUser.size() - 1, 0);
		std::string username = (bytes > 0) ? std::string(ClientUser.data(), bytes) : "UNDENTIFIED";

		{
			std::lock_guard<std::mutex> lock(clientsMutex);
			clients.push_back(clientSock);
		}
		
		std::thread(&ServerClass::recieveData, this, clientSock, username).detach();

	}

	closesocket(ServSocket);
	WSACleanup();
	return 0;
}
