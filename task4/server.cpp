#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 512
#define PORT 12345
#pragma warning(disable : 4996)

int main() {
   WSADATA wsaData;
   SOCKET udpSocket = INVALID_SOCKET;
   sockaddr_in serverAddr = {}, clientAddr = {};
   char recvBuffer[BUFFER_SIZE];
   int recvResult;

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      printf("WSAStartup failed: %d\n", WSAGetLastError());
      return 1;
   }

   udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
   if (udpSocket == INVALID_SOCKET) {
      printf("Socket creation failed: %d\n", WSAGetLastError());
      WSACleanup();
      return 1;
   }

   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(PORT);
   serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

   if (bind(udpSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) != 0) {
      printf("Bind failed: %d\n", WSAGetLastError());
      closesocket(udpSocket);
      WSACleanup();
      return 1;
   }

   printf("Server is running on port %d...\n", PORT);

   int clientAddrLen = sizeof(clientAddr);

   recvResult = recvfrom(udpSocket, recvBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddr, &clientAddrLen);
   if (recvResult > 0) {
      recvBuffer[recvResult] = '\0';
      printf("Received: %s\n", recvBuffer);

      sendto(udpSocket, recvBuffer, recvResult, 0, (SOCKADDR*)&clientAddr, clientAddrLen);
   }
   else {
      printf("Receive failed: %d\n", WSAGetLastError());
   }

   closesocket(udpSocket);
   WSACleanup();

   return 0;
}
