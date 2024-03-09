#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void sendMessage(SOCKET sock) {
	std::cout << "Enter your name: " << std::endl;
	std::string name;
	getline(std::cin, name);
	std::string message;

	while (1) {
		getline(std::cin, message);
		std::string msgToSend = name + ": " + message;
		int bytesSend = send(sock, msgToSend.c_str(), msgToSend.length(), 0);

		if (bytesSend == SOCKET_ERROR) {
			std::cout << "Message sending failed." << std::endl;
			break;
		}

		if (message == "quit") {
			std::cout << "Stopping the application." << std::endl;
			break;
		}
	}

	closesocket(sock);
	WSACleanup();
}

void receiveMessage(SOCKET sock) {
	char buffer[4096];
	int receiveLength;
	std::string receivedMsg = "";
	while (1) {
		receiveLength = recv(sock, buffer, sizeof(buffer), 0);
		if (receiveLength <= 0) {
			std::cout << "Disconnected from server." << std::endl;
			break;
		}
		else {
			receivedMsg = std::string(buffer, receiveLength);
			std::cout << receivedMsg << std::endl;
		}
	}

	closesocket(sock);
	WSACleanup();
}

int main() {

	if (!Initialize()) {
		std::cout << "Initialization of client side winsock failed." << std::endl;
	}

	// Creating socket
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET) {
		std::cout << "Invalid socket." << std::endl;
		return 1;
	}

	int port = 2000;
	std::string serverAddress = "127.0.0.1";
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	inet_pton(AF_INET, serverAddress.c_str(), &(serverAddr.sin_addr));

	if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Cannot connect to the server." << std::endl;
		std::cout << ": " << WSAGetLastError();
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	std::cout << "Successfully connected to server." << std::endl;

	std::thread senderThread(sendMessage, sock);
	std::thread receiverThread(receiveMessage, sock);

	senderThread.join();
	receiverThread.join();

	return 0;
}