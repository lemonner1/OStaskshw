#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

int __cdecl main(int argc, char** argv)
{
   WSADATA wsaData;
   SOCKET ConnectSocket = INVALID_SOCKET;
   struct addrinfo* result = NULL, * ptr = NULL, hints;
   char recvbuf[DEFAULT_BUFLEN];
   int iResult;
   int recvbuflen = DEFAULT_BUFLEN;

   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0) {
      printf("WSAStartup failed with error: %d\n", iResult);
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   iResult = getaddrinfo("pmk.tversu.ru", "80", &hints, &result);
   if (iResult != 0) {
      printf("getaddrinfo failed with error: %d\n", iResult);
      WSACleanup();
      return 1;
   }

   for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET) {
         printf("socket failed with error: %ld\n", WSAGetLastError());
         WSACleanup();
         return 1;
      }

      iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
      if (iResult == SOCKET_ERROR) {
         closesocket(ConnectSocket);
         ConnectSocket = INVALID_SOCKET;
         continue;
      }
      break;
   }

   freeaddrinfo(result);

   if (ConnectSocket == INVALID_SOCKET) {
      printf("Unable to connect to server!\n");
      WSACleanup();
      return 1;
   }

   const char* sendbuf =
      "GET / HTTP/1.1\r\n"
      "Host: pmk.tversu.ru\r\n"
      "Connection: close\r\n"
      "\r\n";

   iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
   if (iResult == SOCKET_ERROR) {
      printf("send failed with error: %d\n", WSAGetLastError());
      closesocket(ConnectSocket);
      WSACleanup();
      return 1;
   }

   printf("HTTP request sent\n");

   do {
      iResult = recv(ConnectSocket, recvbuf, recvbuflen - 1, 0);
      if (iResult > 0) {
         recvbuf[iResult] = '\0'; 
         printf("%s", recvbuf);
      }
      else if (iResult == 0) {
         printf("Connection closed\n");
      }
      else {
         printf("recv failed with error: %d\n", WSAGetLastError());
      }
   } while (iResult > 0);

   closesocket(ConnectSocket);
   WSACleanup();

   return 0;
}
