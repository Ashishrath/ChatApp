#include <iostream>
#include <WinSock2.h>				// boiler plate
#include <WS2tcpip.h>				// for InetPton
#include <tchar.h>					// for _T
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")	// boiler plate

bool Initialize() {					// initialization boiler plate
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, std::vector<SOCKET> &clients) {
	// Send and receive messages
	char buffer[4096];

	while (1) {
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived <= 0) {
			std::cout << "Client disconnected." << std::endl;
			break;
		}
		std::string message(buffer, bytesReceived);
		std::cout << "Message from client: " << message << std::endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
	}

	closesocket(clientSocket);
}

int main() {
	if (!Initialize()) {			// initialization
		std::cout << "Winsock initialization failed." << std::endl;
		return 1;
	}
	std::cout << "Server Program" << std::endl;

	// Creating Socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);	// AF_INET for IPv4, AF_INET6 for IPv6

	if (listenSocket == INVALID_SOCKET) {
		std::cout << "Socket Creation Failed." << std::endl;
		return 1;
	}

	// Create Address Structure 
	int port = 2000;
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	// Convert IP addr (0.0.0.0) put it inside the sin_family in binary format
	if (InetPton(AF_INET, _T("0.0.0.0"), &serverAddr.sin_addr) != 1) {
		std::cout << "Setting Address Structure Failed." << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Bind
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Bind Failed." << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Listen 
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen Failed." << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server is listening on port " << port << std::endl;

	std::vector<SOCKET> clients;

	while (1) {
		// Accept
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			std::cout << "Invalid client socket." << std::endl;
		}

		clients.push_back(clientSocket);
		std::thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}

	closesocket(listenSocket);

	WSACleanup();					// finalalization (cleanup)
	return 0;
}