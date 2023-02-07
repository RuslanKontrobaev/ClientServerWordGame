#define WIN32_LEAN_AND_MEAN
#define BROADCAST_IP "192.168.1.255"
#define PORT 50000
#define TIMEOUT 1

#include <iostream>
#include <winsock2.h>
#include <string>
#include <map>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

bool ready2recvTimeout(int sock, int seconds)
{
	fd_set fd{};
	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	timeval tv{};
	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	return (select(sock + 1, &fd, NULL, NULL, &tv) == 1);
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

	recv(s, (char*)&msgSize, sizeof(int), NULL);

	msg = new char[msgSize + 1];
	msg[msgSize] = '\0';
	recv(s, msg, msgSize, NULL);

	return msg;
}

int main(int argc, char* argv[]) {
	WSAData wsaData;
	SOCKADDR_IN addr;
	SOCKET udpSocket = INVALID_SOCKET, tcpSocket = INVALID_SOCKET;
	map<int, string> serverIp;

	char headerText[] = R"(
	  /$$$$$$  /$$       /$$$$$$ /$$$$$$$$ /$$   /$$ /$$$$$$$$
	 /$$__  $$| $$      |_  $$_/| $$_____/| $$$ | $$|__  $$__/
	| $$  \__/| $$        | $$  | $$      | $$$$| $$   | $$   
	| $$      | $$        | $$  | $$$$$   | $$ $$ $$   | $$   
	| $$      | $$        | $$  | $$__/   | $$  $$$$   | $$   
	| $$    $$| $$        | $$  | $$      | $$\  $$$   | $$   
	|  $$$$$$/| $$$$$$$$ /$$$$$$| $$$$$$$$| $$ \  $$   | $$   
	 \______/ |________/|______/|________/|__/  \__/   |__/)";

	cout << headerText << endl << endl << endl;

	cout << "Initialize WinSock...";
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		cout << "Failed" << endl;
		return 1;
	}
	cout << "Ok!" << endl;

	cout << "Creation UDP socket...";
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		cout << "Failed with error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 2;
	}
	cout << "Ok!" << endl;

	cout << "Setting broadcast option...";
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, "1", sizeof(char) < 0))
	{
		cout << "Failed" << endl;
		closesocket(udpSocket);
		WSACleanup();
		return 3;
	}
	cout << "Ok!" << endl;

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

	cout << "Sending broadcast message...";
	for (auto i = 0; i <= 4; i++)
	{
		if (sendto(udpSocket, "Looking for a server!", 22, NULL, (sockaddr*)&addr, sizeof(addr)) < 0)
		{
			if (i == 4)
			{
				cout << "Failed after " << i + 1 << " trying" << endl;
				closesocket(udpSocket);
				WSACleanup();
				return 4;
			}
			continue;
		}
		break;
	}
	cout << "Ok!" << endl;

	cout << "Searching servers..." << endl;
	int position = 1;
	while (true)
	{
		if (ready2recvTimeout(udpSocket, TIMEOUT))
		{
			char recvbuf[22];
			int sockAddrLen = sizeof(addr);
			if (recvfrom(udpSocket, recvbuf, 12, NULL, (sockaddr*)&addr, &sockAddrLen) < 0)
			{
				cout << "Failed to receive message from servers" << endl;
				continue;
			}
			serverIp.insert( {position++, inet_ntoa(addr.sin_addr) } );
		}
		else break;
	}

	if (serverIp.begin() == serverIp.end())
	{
		cout << "Servers not found" << endl;
		closesocket(udpSocket);
		WSACleanup();
		return 5;
	}
	
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "SERVER\tIP" << endl;
	for (auto iter = serverIp.begin(); iter != serverIp.end(); ++iter)
		cout << iter->first << "\t" << iter->second << endl;
	cout << "--------------------------------------------------------------------------------" << endl << endl;

	cout << "Closing UDP socket...";
	if (closesocket(udpSocket) == SOCKET_ERROR)
	{
		cout << "Failed with error: " << WSAGetLastError() << endl;
		return 6;
	}
	cout << "Ok!" << endl;

	string chosenIp;
	cout << "Please enter the number of the server which you want to connect..." << endl;
	while (true)
	{
		int num;
		cout << "Number: ";
		if (!(cin >> num))
		{
			cin.clear();
			cin.ignore(cin.rdbuf()->in_avail());
			cout << "Invalid number entered" << endl;
			continue;
		}
		if (num <= 0)
		{
			cout << "You are entered a negative or zero number!" << endl;
			continue;
		}
		else if (num > serverIp.size())
		{
			cout << "You are entered a number that is greater than the number of servers!" << endl;
			continue;
		}

		auto item = serverIp.find(num);
		/*cout << "You selected SERVER: " << item->first << ", IP: " << item->second << endl;*/
		chosenIp = item->second;
		break;
	}

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(chosenIp.c_str());

	cout << "Creation TCP socket...";
	tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcpSocket == INVALID_SOCKET)
	{
		cout << "Failed with error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 7;
	}
	cout << "Ok!" << endl;

	cout << "Connecting to server " << chosenIp << "...";
	if (connect(tcpSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cout << "Failed with error: " << WSAGetLastError() << endl;
		closesocket(tcpSocket);
		WSACleanup();
		return 8;
	}
	cout << "Ok!" << endl;

	//WaitForSingleObject(t, INFINITE);

	while (true)
	{
		char* msg = recvMsg(tcpSocket);

		if (string(msg) == "TTT")
		{
			cout << "Enter word: ";
			string msg;
			cin >> msg;
			//getline(cin, msg);
			sendMsg(tcpSocket, msg);
		}
		else
			cout << msg << endl;

		delete[] msg;
	}

	closesocket(tcpSocket);
	WSACleanup();
	return 0;
}