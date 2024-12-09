#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFFER_SIZE 512
#define SERVER_PORT 12345

int main() {
   WSADATA wsaData;
   SOCKET clientSocket = INVALID_SOCKET;
   sockaddr_in serverAddr = {};
   char sendBuffer[BUFFER_SIZE];
   char recvBuffer[BUFFER_SIZE];
   int sendResult, recvResult;

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      printf("WSAStartup failed: %d\n", WSAGetLastError());
      return 1;
   }

   clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
   if (clientSocket == INVALID_SOCKET) {
      printf("Socket creation failed: %d\n", WSAGetLastError());
      WSACleanup();
      return 1;
   }

   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(SERVER_PORT);
   serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

   printf("Enter message to send: ");
   std::cin.getline(sendBuffer, BUFFER_SIZE);

   sendResult = sendto(clientSocket, sendBuffer, strlen(sendBuffer), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
   if (sendResult == SOCKET_ERROR) {
      printf("Send failed: %d\n", WSAGetLastError());
      closesocket(clientSocket);
      WSACleanup();
      return 1;
   }

   printf("Message sent: %s\n", sendBuffer);

   int serverAddrLen = sizeof(serverAddr);

   recvResult = recvfrom(clientSocket, recvBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddr, &serverAddrLen);
   if (recvResult > 0) {
      recvBuffer[recvResult] = '\0';
      printf("Server replied: %s\n", recvBuffer);
   }
   else {
      printf("Receive failed: %d\n", WSAGetLastError());
   }

   closesocket(clientSocket);
   WSACleanup();

   return 0;
}
