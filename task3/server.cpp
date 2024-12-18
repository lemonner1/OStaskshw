#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 512
#define SERVER_PORT "80"

std::vector<SOCKET> clientSockets;

void handleClient(SOCKET clientSocket) {
   char buffer[BUFFER_SIZE];
   int result;

   while (true) {
      result = recv(clientSocket, buffer, BUFFER_SIZE, 0);
      if (result > 0) {
         buffer[result] = '\0';
         std::cout << "Received: " << buffer << "\n";

         for (auto& socket : clientSockets) {
            if (socket != clientSocket) {
               send(socket, buffer, result, 0);
            }
         }
      }
      else if (result == 0) {
         std::cout << "Client disconnected.\n";
         break;
      }
      else {
         if (WSAGetLastError() == WSAEWOULDBLOCK) {
            Sleep(500);
            continue;
         }
         std::cerr << "Error receiving data: " << WSAGetLastError() << "\n";
         break;
      }
   }

   closesocket(clientSocket);
   clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
}

int main() {
   WSADATA wsaData;
   SOCKET serverSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
   struct addrinfo* addrResult = nullptr, hints{};
   int result;

   result = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (result != 0) {
      std::cerr << "WSAStartup failed: " << result << "\n";
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   result = getaddrinfo(nullptr, SERVER_PORT, &hints, &addrResult);
   if (result != 0) {
      std::cerr << "getaddrinfo failed: " << result << "\n";
      WSACleanup();
      return 1;
   }

   serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
   if (serverSocket == INVALID_SOCKET) {
      std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
      freeaddrinfo(addrResult);
      WSACleanup();
      return 1;
   }

   result = bind(serverSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
   freeaddrinfo(addrResult);
   if (result == SOCKET_ERROR) {
      std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
      closesocket(serverSocket);
      WSACleanup();
      return 1;
   }

   result = listen(serverSocket, SOMAXCONN);
   if (result == SOCKET_ERROR) {
      std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
      closesocket(serverSocket);
      WSACleanup();
      return 1;
   }

   u_long nonBlockingMode = 1;
   ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);

   while (true) {
      clientSocket = accept(serverSocket, nullptr, nullptr);
      if (clientSocket == INVALID_SOCKET) {
         if (WSAGetLastError() == WSAEWOULDBLOCK) {
            Sleep(500);
            continue;
         }
         std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
         break;
      }

      ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode);
      clientSockets.push_back(clientSocket);

      std::thread clientThread(handleClient, clientSocket);
      clientThread.detach();
   }

   closesocket(serverSocket);
   WSACleanup();
   return 0;
}
