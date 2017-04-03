#include <stdio.h>
#include <WinSock2.h>
#include <tchar.h>
#include <winsock.h>
#include <windows.h>

#define DIR_LEN BUFSIZ
#define MAX_PACKETLEN 512
#define PORT 1337

#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma warning(disable : 4996)

int _tmain(int argc, char* argv[]){
	int port = 0;
	if (argc < 2)
	{
		printf("Usage : loader.exe target\nExample : loader.exe target.exe");
		return 0;
	}
	port = PORT;

	WSADATA wsaData;
	int status;
	int SockLen;
	int Readn, Writen;
	SOCKET Winsock;
	SOCKET EndpointSocket, ClientSocket;
	struct sockaddr_in SockInfo, ClientSockInfo;
	char ReadBuffer[MAX_PACKETLEN];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("error\r\n");
		return 0;
	}

	EndpointSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	if (EndpointSocket == INVALID_SOCKET)
		return 1;

	ZeroMemory(&SockInfo, sizeof(struct sockaddr_in));

	SockInfo.sin_family = AF_INET;
	SockInfo.sin_port = htons(port);
	SockInfo.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	status = bind(EndpointSocket, (struct sockaddr*)&SockInfo, sizeof(struct sockaddr_in));
	if (status == SOCKET_ERROR)
	{
		printf("Bind Error\n");
		return 0;
	}
	if (SOCKET_ERROR == listen(EndpointSocket, 50000))
	{
		printf("listen Error\n");
		return 0;
	}
	while (1)
	{
		ZeroMemory(&ClientSockInfo, sizeof(struct sockaddr_in));
		SockLen = sizeof(struct sockaddr_in);
		ClientSocket = accept(EndpointSocket, (struct sockaddr*)&ClientSockInfo, &SockLen);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("Accept Error\n");
			closesocket(EndpointSocket);
			WSACleanup();
			return 1;
		}
		char *ip = inet_ntoa(ClientSockInfo.sin_addr);
		printf("Accept Client (%s:%d)\n", ip, ClientSockInfo.sin_port);
		STARTUPINFO si = { 0, };
		PROCESS_INFORMATION pi;

		BOOL state;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = (HANDLE)ClientSocket;
		si.hStdOutput = (HANDLE)ClientSocket;
		si.hStdError = (HANDLE)ClientSocket;

		state = CreateProcess(
			NULL,
			(LPWSTR)argv[1],
			NULL, NULL,
			TRUE,
			0,
			NULL, NULL,
			&si,
			&pi
			);

		if (state == NULL)
		{
			printf("cannot create process...");
		}
		closesocket(ClientSocket);
	}
	closesocket(EndpointSocket);
	WSACleanup();
	return 0;
}
