#include <iostream>
#include <string>

#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "Bullet.h"
#include "Obstacle.h"
#include "Common.h"
#include "packet.h"
#include <random>
#include <vector>
#include <queue>


#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;

string gameMode = "Menumode";



class Client
{
public:
	char name[20];
	int ID;

	Client()
	{
		*name = NULL;
		ID = -1;
	}
};

queue<Client> waitClientList;
HANDLE hMatchingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE hGameStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
bool gameOver = false;

// ���� ID�ο�
int makeID();
// ���� ������ �ޱ�
void receiveGameData(SOCKET s);
// ���� ������ ����
void sendGameData(SOCKET s);
// ���� ��� ����
void sendResult(SOCKET s, int result);
// �ʱ�ȭ�Լ�
void initialization();


DWORD WINAPI networkThread(LPVOID arg)
{
	SOCKET clientSock = (SOCKET)arg;
	int retval;
	Client client;

	// �г��� �ޱ�
	retval = recv(clientSock, client.name, sizeof(client.name), 0);
	if (retval == SOCKET_ERROR) err_display("receive - clientName");

	// ID �Ҵ�
	retval = send(clientSock, (char*)makeID(), sizeof(client.ID), 0);
	if (retval == SOCKET_ERROR) err_display("send - clientID");

	while (true)
	{
		// ��Ī ��ȣ ����
		unsigned short start;
		retval = recv(clientSock, (char*)&start, sizeof(start), 0);
		if (retval == SOCKET_ERROR) err_display("receive - c_playetPacket");
		if (start != GAMESTART) err_display("receive - start");

		// ť�� ����
		waitClientList.push(client);

		// ���� ���� �� ���� ���
		WaitForSingleObject(hGameStartEvent, INFINITE);

		// ���� ���� ��ȣ ����
		s_UIPacket gameStart(GAMESTART);

		retval = send(clientSock, (char*)&gameStart, sizeof(gameStart), 0);
		if (retval == SOCKET_ERROR) err_display("send - s_UIPacket(gameStart)");

		// initPacket ����
		s_initPacket init;

		retval = send(clientSock, (char*)&init, sizeof(init), 0);
		if (retval == SOCKET_ERROR) err_display("send - initPacket");

		// ���� ����
		while (!gameOver)
		{
			// ���� ������ �ޱ�
			receiveGameData(clientSock);

			// ���� ������ ����
			sendGameData(clientSock);
		}
		// ���� ��� ����
		sendResult(clientSock, 0);
	}
}

int makeID()
{
	return waitClientList.size() + 1;
}

void receiveGameData(SOCKET s)
{
	int retval;

	// c_playerPacket �ޱ�
	c_playerPacket playerPacket;
	retval = recv(s, (char*)&playerPacket, sizeof(playerPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_playetPacket");

	// c_bulletPacket �ޱ�
	c_bulletPacket bulletPacket;
	retval = recv(s, (char*)&bulletPacket, sizeof(bulletPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_bulletPacket");

	// c_inputPacket �ޱ�
	c_inputPacket inputPacket;
	retval = recv(s, (char*)&inputPacket, sizeof(inputPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_inputPacket");
}

void sendGameData(SOCKET s)
{
	int retval;

	s_enemyPacket enemyPacket;
	retval = send(s, (char*)&enemyPacket, sizeof(enemyPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - enemyPacket");

	s_itemPacket itemPacket;
	retval = send(s, (char*)&itemPacket, sizeof(itemPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - itemPacket");

	s_bulletPacket bulletPacket;
	retval = send(s, (char*)&bulletPacket, sizeof(bulletPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - bulletPacket");

	s_obstaclePacket obstaclePacket;
	retval = send(s, (char*)&obstaclePacket, sizeof(obstaclePacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - obstaclePacket");

	s_UIPacket UIPacket;
	retval = send(s, (char*)&UIPacket, sizeof(UIPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - UIPacket");

	s_playerPacket playerPacket;
	retval = send(s, (char*)&playerPacket, sizeof(playerPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - playerPacket");
}

void sendResult(SOCKET s, int result)
{
	int retval;

	s_UIPacket UIPacket(result);
	retval = send(s, (char*)&UIPacket, sizeof(UIPacket), 0);
	if (retval == SOCKET_ERROR) err_display("send - UIPacket");
}


DWORD WINAPI gameThread(LPVOID arg) {
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	printf("[�˸�] ���� �ʱ�ȭ ����\n");

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	// bind
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	// listen
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	// ������ ��� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// ��Ʈ��ũ������ ����
		hThread = CreateThread(NULL, 0, networkThread, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }


		// ���ӽ����� ����
		if (waitClientList.size() == 3) {
			gameMode = "PlayMode";
			initialization();
			hThread = CreateThread(NULL, 0, gameThread, (LPVOID)client_sock, 0, NULL);
			SetEvent(hGameStartEvent);
			if (hThread == NULL) { closesocket(client_sock); }
			else { CloseHandle(hThread); }
			for (int i = 0; i < 3; ++i) {
				waitClientList.pop();
			}

		}
	}

	// ���� �ݱ�
	closesocket(listen_sock);


	// ���� ����
	WSACleanup();
	return 0;
}

