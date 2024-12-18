#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

int main() {
   WSADATA wsaData;
   SOCKET serverSocket = INVALID_SOCKET;
   sockaddr_in serverAddr;
   int result;

   result = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (result != 0) {
      std::cerr << "WSAStartup failed with error: " << result << std::endl;
      return 1;
   }

   serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (serverSocket < 0) {
      std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
      return 1;
   }

   ZeroMemory(&serverAddr, sizeof(serverAddr));
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(12345);
   serverAddr.sin_addr.s_addr = inet_addr("230.168.11.11");

   u_int ttl = 1;
   if (setsockopt(serverSocket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR) {
      std::cerr << "Failed to set IP_MULTICAST_TTL: " << WSAGetLastError() << std::endl;
      closesocket(serverSocket);
      WSACleanup();
      return 1;
   }

   const char* message = "Hello, clients!";
   while (true) {
      sendto(serverSocket, message, strlen(message), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
      std::cout << "Message sent: " << message << std::endl;
      Sleep(1000);
   }

   closesocket(serverSocket);
   WSACleanup();
   return 0;
}