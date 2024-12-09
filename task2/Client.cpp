#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

void sendMessages(SOCKET ConnectSocket) {
   char sendbuf[DEFAULT_BUFLEN];
   while (true) {
      std::cin.getline(sendbuf, DEFAULT_BUFLEN);
      if (send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0) == SOCKET_ERROR) {
         std::cerr << "send failed" << std::endl;
         break;
      }
   }
}

void receiveMessages(SOCKET ConnectSocket) {
   char recvbuf[DEFAULT_BUFLEN];
   int recvbuflen = DEFAULT_BUFLEN;
   while (true) {
      int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
      if (iResult > 0) {
         recvbuf[iResult] = '\0';
         std::cout << "Server: " << recvbuf << std::endl;
      }
      else {
         std::cerr << "recv failed" << std::endl;
         break;
      }
   }
}

int main() {
   WSADATA wsaData;
   SOCKET ConnectSocket = INVALID_SOCKET;
   struct addrinfo* result = NULL, * ptr = NULL, hints;

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      std::cerr << "WSAStartup failed" << std::endl;
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   if (getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result) != 0) {
      std::cerr << "getaddrinfo failed" << std::endl;
      WSACleanup();
      return 1;
   }

   for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET) continue;

      if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
         closesocket(ConnectSocket);
         ConnectSocket = INVALID_SOCKET;
         continue;
      }
      break;
   }

   freeaddrinfo(result);
   if (ConnectSocket == INVALID_SOCKET) {
      std::cerr << "Unable to connect to server" << std::endl;
      WSACleanup();
      return 1;
   }

   std::thread sender(sendMessages, ConnectSocket);
   std::thread receiver(receiveMessages, ConnectSocket);

   sender.join();
   receiver.join();

   closesocket(ConnectSocket);
   WSACleanup();
   return 0;
}
