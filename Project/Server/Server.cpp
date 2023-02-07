#define WIN32_LEAN_AND_MEAN
#define BROADCAST_IP "192.168.1.255"
#define PORT 50000
#define TIMEOUT 10

#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

void broadcastHandler()
{
	SOCKET udpSocket = INVALID_SOCKET;
	SOCKADDR_IN addr;

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
		return;

	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, "1", sizeof(char) < 0))
	{
		closesocket(udpSocket);
		return;
	}

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(udpSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(udpSocket);
		return;
	}

	while (true)
	{
		char recvbuf[22];
		int len = sizeof(addr);
		recvfrom(udpSocket, recvbuf, sizeof(recvbuf), NULL, (sockaddr*)&addr, &len);
		sendto(udpSocket, "I'm server!", 12, NULL, (sockaddr*)&addr, sizeof(addr));
	}
}

void sendMsg(SOCKET s, string msg)
{
	int msgSize = msg.size();

	send(s, (char*)&msgSize, sizeof(int), NULL);
	send(s, msg.c_str(), msgSize, NULL);
}

char* recvMsg(SOCKET s)
{
	char* msg = nullptr;
	int msgSize = 0;

	if (recv(s, (char*)&msgSize, sizeof(int), NULL) <= 0)
		s = INVALID_SOCKET;

	msg = new char[msgSize + 1];
	msg[msgSize] = '\0';
	if (recv(s, msg, msgSize, NULL) <= 0)
		s = INVALID_SOCKET;

	return msg;
}

int main(int argc, char* argv[]) {
	WSAData wsaData;
	SOCKADDR_IN addr;
	SOCKET tcpListenSocket = INVALID_SOCKET, tcpClientSocket = INVALID_SOCKET;
	SOCKET playerSocket[3]{};

	char headerText[] = R"(
	  /$$$$$$  /$$$$$$$$ /$$$$$$$  /$$    /$$ /$$$$$$$$ /$$$$$$$ 
	 /$$__  $$| $$_____/| $$__  $$| $$   | $$| $$_____/| $$__  $$
	| $$  \__/| $$      | $$  \ $$| $$   | $$| $$      | $$  \ $$
	|  $$$$$$ | $$$$$   | $$$$$$$/|  $$ / $$/| $$$$$   | $$$$$$$/
	 \____  $$| $$__/   | $$__  $$ \  $$ $$/ | $$__/   | $$__  $$
	 /$$  \ $$| $$      | $$  \ $$  \  $$$/  | $$      | $$  \ $$
	|  $$$$$$/| $$$$$$$$| $$  | $$   \  $/   | $$$$$$$$| $$  | $$
	 \______/ |________/|__/  |__/    \_/    |________/|__/  |__/)";

	cout << headerText << endl << endl << endl;

	cout << "Initialize WinSock...";
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		cout << "Failed" << endl;
		return 1;
	}
	cout << "Ok!" << endl;

	DWORD tId;
	HANDLE t = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)broadcastHandler, NULL, NULL, &tId);
	if (t == NULL)
	{
		cout << "Thread creation failed" << endl;
		return 2;
	}
		
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	cout << "Creation TCP listen socket...";
	tcpListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcpListenSocket == INVALID_SOCKET)
	{
		cout << "Failed with error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 3;
	}
	cout << "Ok!" << endl;

	cout << "Binding TCP listen socket...";
	if (bind(tcpListenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		cout << "Failed with error: " << WSAGetLastError() << endl;
		closesocket(tcpListenSocket);
		WSACleanup();
		return 4;
	}
	cout << "Ok!" << endl;

	cout << "Setting TCP listen socket status...";
	if (listen(tcpListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Failed with error: " << WSAGetLastError() << endl;
		closesocket(tcpListenSocket);
		WSACleanup();
		return 5;
	}
	cout << "Ok!" << endl;

	cout << "Waiting for the players..." << endl;
	for (auto i = 0; i < 3; i++)
	{
		int len = sizeof(addr);
		tcpClientSocket = accept(tcpListenSocket, (SOCKADDR*)&addr, &len);
		if (tcpClientSocket == INVALID_SOCKET)
		{
			cout << "Accepting socket failed" << endl;
			closesocket(tcpListenSocket);
			WSACleanup();
			return 10;
		}

		playerSocket[i] = tcpClientSocket;
		if (i == 0)
		{
			string msg = "Waiting for the 2nd player...";
			sendMsg(playerSocket[0], msg);
			cout << "1st";
		}
		else if (i == 1)
		{
			string msg = "Waiting for the 3rd player...";
			sendMsg(playerSocket[0], msg);
			sendMsg(playerSocket[1], msg);
			cout << "2nd";
		}
		else if (i == 2)
			cout << "3rd";

		cout << " player connected, IP: " << inet_ntoa(addr.sin_addr) << ", PORT: " << ntohs(addr.sin_port) << endl;
	}

	closesocket(tcpListenSocket);

	for (auto i = 0; i < 3; i++)
		sendMsg(playerSocket[i], "\nRULES: The players take turns. The point is that each next word begins with the letter that ended the previous word. Good luck!\n");
		
	cout << "The game has started..." << endl;

	int whoMoveNow = 0;
	string moveMsg = "TTT"; // Code to move player
	char lastLetter = 0;

	while (true)
	{
		if (whoMoveNow > 2)
			whoMoveNow = 0;

		string waitMsg = "Waiting turn player " + to_string(whoMoveNow + 1) + "...";

		switch (whoMoveNow)
		{
		case 0:
		{
			sendMsg(playerSocket[0], moveMsg);

			sendMsg(playerSocket[1], waitMsg);
			sendMsg(playerSocket[2], waitMsg);

			char* msg = recvMsg(playerSocket[0]);

			while (strlen(msg) < 2)
			{
				string notice = "The word must contain at least two letters";
				sendMsg(playerSocket[0], notice);
				sendMsg(playerSocket[0], moveMsg);
				msg = recvMsg(playerSocket[0]);
			}

			if (lastLetter == 0)
			{
				string answMsg = "Player 1 wrote: " + string(msg);
				cout << answMsg << endl;

				sendMsg(playerSocket[1], answMsg);
				sendMsg(playerSocket[2], answMsg);
				lastLetter = tolower(msg[strlen(msg) - 1]);
			}
			else
			{
				while (tolower(msg[0]) != lastLetter)
				{
					string notice = "Wrong word! Try again";
					sendMsg(playerSocket[0], notice);
					sendMsg(playerSocket[0], moveMsg);
					msg = recvMsg(playerSocket[0]);
				}

				string answMsg = "Player 1 wrote: " + string(msg);
				cout << answMsg << endl;

				sendMsg(playerSocket[1], answMsg);
				sendMsg(playerSocket[2], answMsg);

				lastLetter = tolower(msg[strlen(msg) - 1]);
			}
			delete[] msg;
			break;
		}
		case 1:
		{
			sendMsg(playerSocket[1], moveMsg);

			sendMsg(playerSocket[0], waitMsg);
			sendMsg(playerSocket[2], waitMsg);

			char* msg = recvMsg(playerSocket[1]);

			while (strlen(msg) < 2)
			{
				string notice = "The word must contain at least two letters";
				sendMsg(playerSocket[1], notice);
				sendMsg(playerSocket[1], moveMsg);
				msg = recvMsg(playerSocket[1]);
			}

			while (tolower(msg[0]) != lastLetter)
			{
				string notice = "Wrong word! Try again";
				sendMsg(playerSocket[1], notice);
				sendMsg(playerSocket[1], moveMsg);
				msg = recvMsg(playerSocket[1]);
			}

			string answMsg = "Player 2 wrote: " + string(msg);
			cout << answMsg << endl;

			sendMsg(playerSocket[0], answMsg);
			sendMsg(playerSocket[2], answMsg);

			lastLetter = tolower(msg[strlen(msg) - 1]);
			delete[] msg;
			break;
		}
		case 2:
		{
			sendMsg(playerSocket[2], moveMsg);

			sendMsg(playerSocket[0], waitMsg);
			sendMsg(playerSocket[1], waitMsg);

			char* msg = recvMsg(playerSocket[2]);

			while (strlen(msg) < 2)
			{
				string notice = "The word must contain at least two letters";
				sendMsg(playerSocket[2], notice);
				sendMsg(playerSocket[2], moveMsg);
				msg = recvMsg(playerSocket[2]);
			}

			while (tolower(msg[0]) != lastLetter)
			{
				string notice = "Wrong word! Try again";
				sendMsg(playerSocket[2], notice);
				sendMsg(playerSocket[2], moveMsg);
				msg = recvMsg(playerSocket[2]);
			}

			string answMsg = "Player 3 wrote: " + string(msg);
			cout << answMsg << endl;

			sendMsg(playerSocket[0], answMsg);
			sendMsg(playerSocket[1], answMsg);

			lastLetter = tolower(msg[strlen(msg) - 1]);
			delete[] msg;
			break;
		}
		}

		whoMoveNow++;
	}

	for (auto i = 0; i < 3; i++)
	{
		if (closesocket(playerSocket[i]) == SOCKET_ERROR)
		{
			cout << "Closing player socket failed with error: " << WSAGetLastError() << endl;
			return 6;
		}
	}

	WSACleanup();
	return 0;
}