#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

std::vector<std::thread> clientThreads;
std::mutex clientMutex;

void handleClient(SOCKET ClientSocket) {
   char recvbuf[DEFAULT_BUFLEN];
   int recvbuflen = DEFAULT_BUFLEN;
   int iResult, iSendResult;

   do {
      iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
      if (iResult > 0) {
         std::cout << "Bytes received: " << iResult << std::endl;

         iSendResult = send(ClientSocket, recvbuf, iResult, 0);
         if (iSendResult == SOCKET_ERROR) {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
            break;
         }
         std::cout << "Bytes sent: " << iSendResult << std::endl;
      }
      else if (iResult == 0) {
         std::cout << "Connection closing..." << std::endl;
      }
      else {
         std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
         break;
      }
   } while (iResult > 0);

   closesocket(ClientSocket);
}

int main() {
   WSADATA wsaData;
   SOCKET ListenSocket = INVALID_SOCKET;

   struct addrinfo* result = NULL;
   struct addrinfo hints;

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      std::cerr << "WSAStartup failed" << std::endl;
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
      std::cerr << "getaddrinfo failed" << std::endl;
      WSACleanup();
      return 1;
   }

   ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
   if (ListenSocket == INVALID_SOCKET) {
      std::cerr << "Socket failed" << std::endl;
      freeaddrinfo(result);
      WSACleanup();
      return 1;
   }

   if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
      std::cerr << "Bind failed" << std::endl;
      freeaddrinfo(result);
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   freeaddrinfo(result);

   if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
      std::cerr << "Listen failed" << std::endl;
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   while (true) {
      SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
      if (ClientSocket == INVALID_SOCKET) {
         std::cerr << "Accept failed" << std::endl;
         break; 
      }

      std::lock_guard<std::mutex> lock(clientMutex);
      clientThreads.emplace_back(handleClient, ClientSocket);
   }

   for (auto& thread : clientThreads) {
      if (thread.joinable()) {
         thread.join();
      }
   }

   closesocket(ListenSocket);
   WSACleanup();
   return 0;
}
