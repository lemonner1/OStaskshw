#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFFER_SIZE 512

int main(int argc, char** argv) {
   WSADATA wsaData;
   SOCKET clientSocket = INVALID_SOCKET;
   sockaddr_in clientAddr;
   char recvBuffer[BUFFER_SIZE];
   int result;

   result = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (result != 0) {
      std::cerr << "WSAStartup failed with error: " << result << std::endl;
      return 1;
   }

   clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (clientSocket < 0) {
      std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
      return 1;
   }

   ip_mreq multicastGroup;
   multicastGroup.imr_multiaddr.s_addr = inet_addr("230.168.11.11");
   multicastGroup.imr_interface.s_addr = INADDR_ANY;

   ZeroMemory(&clientAddr, sizeof(clientAddr));
   clientAddr.sin_family = AF_INET;
   clientAddr.sin_port = htons(12345);
   clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

   u_int reuseFlag = 1;
   if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseFlag, sizeof(reuseFlag)) < 0) {
      std::cerr << "Failed to set SO_REUSEADDR: " << WSAGetLastError() << std::endl;
      closesocket(clientSocket);
      WSACleanup();
      return 1;
   }

   if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
      std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
      closesocket(clientSocket);
      WSACleanup();
      return 1;
   }

   if (setsockopt(clientSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multicastGroup, sizeof(multicastGroup)) == SOCKET_ERROR) {
      std::cerr << "Failed to join multicast group: " << WSAGetLastError() << std::endl;
      closesocket(clientSocket);
      WSACleanup();
      return 1;
   }

   while (true) {
      result = recvfrom(clientSocket, recvBuffer, BUFFER_SIZE, 0, NULL, NULL);
      if (result > 0) {
         recvBuffer[result] = '\0';
         std::cout << "Message received: " << recvBuffer << std::endl;
      }
      else {
         std::cerr << "Receive error: " << WSAGetLastError() << std::endl;
      }
   }

   closesocket(clientSocket);
   WSACleanup();
   return 0;
}