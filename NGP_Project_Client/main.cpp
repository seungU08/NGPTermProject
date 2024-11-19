#include "Common.h"
#include "Packet.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000

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

// ����
bool gameOver = false;

// ���� ������ ����
void sendGameData(SOCKET s);

// ���� ������ �ޱ�
void receiveGameData(SOCKET s);

// ���� ��� �ޱ�
void receiveResult(SOCKET s);

int main()
{
	size_t retval{};

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect");

	// �г��� �Է� �ޱ� (���� �ʿ�)
	Client client;


	// �г��� ����
	retval = send(sock, client.name, sizeof(client.name), 0);
	if (retval == SOCKET_ERROR) err_display("send - clientName");

	// ID �Ҵ� �ޱ�
	retval = recv(sock, (char*)client.ID, sizeof(client.ID), 0);
	if (retval == SOCKET_ERROR) err_display("receive - clientID");

	while (true)
	{
		// ���� ���� �̺�Ʈ ���� (�ۼ� �ʿ�)

		// ���� ��Ī ��ȣ ����
		unsigned short matchingStart = GAMESTART;
		retval = send(sock, (char*)matchingStart, sizeof(matchingStart), 0);
		if (retval == SOCKET_ERROR) err_display("send - matchingStart");

		// ���� ���� ��ȣ ����
		s_UIPacket gameStart;

		retval = recv(sock, (char*)&gameStart, sizeof(gameStart), 0);
		if (retval == SOCKET_ERROR) err_display("receive - s_UIPacket(gameStart)");
		if (gameStart.s_UIType != GAMESTART) err_display("receive - s_UIPacket(gameStart)");

		// initPacket ����
		s_initPacket init;

		retval = send(sock, (char*)&init, sizeof(init), 0);
		if (retval == SOCKET_ERROR) err_display("receive - initPacket");

		while (!gameOver)
		{
			// ���� ������ ����
			sendGameData(sock);

			// ���� ������ �ޱ�
			receiveGameData(sock);
		}

		receiveResult(sock);
	}

	closesocket(sock);

	WSACleanup();
	return 0;

}

void sendGameData(SOCKET s)
{
	int retval;

	// c_playerPacket ����
	c_playerPacket playerPacket;
	// ���� ���� �ʿ�
	retval = send(s, (char*)&playerPacket, sizeof(playerPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_playetPacket");

	// c_bulletPacket ����
	c_bulletPacket bulletPacket;
	// ���� ���� �ʿ�
	retval = send(s, (char*)&bulletPacket, sizeof(bulletPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_bulletPacket");

	// c_inputPacket ����
	c_inputPacket inputPacket;
	// ���� ���� �ʿ�
	retval = send(s, (char*)&inputPacket, sizeof(inputPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - c_inputPacket");
}

void receiveGameData(SOCKET s)
{
	int retval;

	s_enemyPacket enemyPacket;
	retval = recv(s, (char*)&enemyPacket, sizeof(enemyPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - enemyPacket");

	s_itemPacket itemPacket;
	retval = recv(s, (char*)&itemPacket, sizeof(itemPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - itemPacket");

	s_bulletPacket bulletPacket;
	retval = recv(s, (char*)&bulletPacket, sizeof(bulletPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - bulletPacket");

	s_obstaclePacket obstaclePacket;
	retval = recv(s, (char*)&obstaclePacket, sizeof(obstaclePacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - obstaclePacket");

	s_playerPacket playerPacket;
	retval = recv(s, (char*)&playerPacket, sizeof(playerPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - playerPacket");
}

void receiveResult(SOCKET s)
{
	int retval;

	s_UIPacket UIPacket;
	retval = recv(s, (char*)&UIPacket, sizeof(UIPacket), 0);
	if (retval == SOCKET_ERROR) err_display("receive - UIPacket");
}