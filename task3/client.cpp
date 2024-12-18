#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 512
#define SERVER_PORT "80"

void handleSend(SOCKET socket) {
   char buffer[BUFFER_SIZE];
   int result;

   while (true) {
      std::cin.getline(buffer, BUFFER_SIZE);

      result = send(socket, buffer, (int)strlen(buffer), 0);
      if (result == SOCKET_ERROR) {
         std::cerr << "Error sending message: " << WSAGetLastError() << "\n";
         closesocket(socket);
         WSACleanup();
         return;
      }
   }
}

void handleReceive(SOCKET socket) {
   char buffer[BUFFER_SIZE];
   int result;

   while (true) {
      result = recv(socket, buffer, BUFFER_SIZE, 0);
      if (result > 0) {
         buffer[result] = '\0';
         std::cout << "Server: " << buffer << "\n";
      }
      else if (result == 0) {
         std::cout << "Connection closed by server.\n";
         break;
      }
      else {
         std::cerr << "Error receiving data: " << WSAGetLastError() << "\n";
         break;
      }
   }
}

int main() {
   WSADATA wsaData;
   SOCKET clientSocket = INVALID_SOCKET;
   struct addrinfo* addrResult = nullptr, hints{};
   int result;

   result = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (result != 0) {
      std::cerr << "WSAStartup failed with error: " << result << "\n";
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   result = getaddrinfo("127.0.0.1", SERVER_PORT, &hints, &addrResult);
   if (result != 0) {
      std::cerr << "getaddrinfo failed: " << result << "\n";
      WSACleanup();
      return 1;
   }

   clientSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
   if (clientSocket == INVALID_SOCKET) {
      std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
      freeaddrinfo(addrResult);
      WSACleanup();
      return 1;
   }

   result = connect(clientSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
   freeaddrinfo(addrResult);
   if (result == SOCKET_ERROR) {
      std::cerr << "Connection failed: " << WSAGetLastError() << "\n";
      closesocket(clientSocket);
      WSACleanup();
      return 1;
   }

   std::thread sendThread(handleSend, clientSocket);
   std::thread receiveThread(handleReceive, clientSocket);
   sendThread.join();
   receiveThread.join();

   closesocket(clientSocket);
   WSACleanup();
   return 0;
}