// TAB_YU.cpp : implementation file

#include "stdafx.h"
#include "NTcurling.h"
#include "TAB_YU.h"

#define SERVER_PORT_NUM      8000		// Server�� ������ �� ����� Port number(8000)
//#define SERVER_IP        "192.168.0.13"	// Server�� ������ �� ����� IP address
//#define SERVER_IP        "192.168.0.11"	// Server�� ������ �� ����� IP address
#define SERVER_IP        "127.0.0.1"	// Server�� ������ �� ����� IP address
//#define SERVER_IP        "172.20.10.4"	// Server�� ������ �� ����� IP address

#define PORT_NUM      10200		// TX, Rio�� ������ �� ����� Port number(10200)
#define BLOG_SIZE       5		// ��α� ������(5)
#define MAX_MSG_LEN 256			// ��Ŷ�� �ִ� ����(256)
#define PI 3.14159265359

SOCKET  sock_base[FD_SETSIZE];	// ����� client ������ ������ SOCKET �迭
HANDLE hev_base[FD_SETSIZE];	// �� client ������ ��Ʈ��ũ �̺�Ʈ�� ������ �̺�Ʈ ��ü �迭
HANDLE server_hev;

int cnt;						// ����� client �� ����
bool isThrower = true;			// Thrower�� Skip mode�� ����
bool isPremode = true;			// �������� ����带 ����

int vs_sock = -1;		// TX(vision) client�� ������ index�� ����
int test_sock = -1;

int robot_id = 1;		// Thrower���� Skip���� ������ robot_id�� ����

int m_iCount = 0;

float hog_dist_trans = 500;	// �۽��� hogline ������ �Ÿ�(default: 500)
float recv_speed, recv_angle, recv_reset, emp;	// ������ ������(�ӵ�, ����, Reset ��ȣ, rio�� empty)
float speed, angle, speed_p, angle_p, speed_w, angle_w = 0;
bool isReady = false;		// ready ��ȣ(robot info call ���� �� set, release end �� reset)
bool isDriveSt = false;		// drive start ��ȣ
bool isRioRst = false;		// Rio reset ��ȣ

SOCKADDR_IN servaddr = { 0 };

/*** ���� ����¿� ***/
FILE * fp;

// CTAB_YU dialog

IMPLEMENT_DYNAMIC(CTAB_YU, CDialog)

CTAB_YU::CTAB_YU(CWnd* pParent /*=NULL*/)
	: CDialog(CTAB_YU::IDD, pParent)
	, m_fReleaseSpeed(0)
	, m_fReleaseAngle(0)
	, m_strCurl(_T(""))
	, m_strReleasePos(_T(""))
	, m_strReleaseTarPos(_T(""))
	, m_fAccel(0)
	, m_strAccelPos(_T(""))
	, m_fRobotAngle(0)
	, m_strRobotPos(_T(""))
	, m_fHogDist(0)
	, m_fHogOffset(0)
	, m_fRioEncoder(0)
	, m_fRioangle(0)
{

}

CTAB_YU::~CTAB_YU()
{
}

void CTAB_YU::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SPEED, m_fReleaseSpeed);
	DDX_Text(pDX, IDC_ANGLE, m_fReleaseAngle);
	DDX_Text(pDX, IDC_CURL, m_strCurl);
	DDX_Text(pDX, IDC_POS, m_strReleasePos);
	DDX_Text(pDX, IDC_TARPOS, m_strReleaseTarPos);
	DDX_Text(pDX, IDC_ACCEL2, m_fRioEncoder);
	DDX_Text(pDX, IDC_ACCELPOS, m_strAccelPos);
	DDX_Text(pDX, IDC_RANGLE, m_fRobotAngle);
	DDX_Text(pDX, IDC_RPOS, m_strRobotPos);
	DDX_Text(pDX, IDC_HOGDIST, m_fHogDist);
	DDX_Text(pDX, IDC_HOGOFFSET, m_fHogOffset);
}


BEGIN_MESSAGE_MAP(CTAB_YU, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTMODE, &CTAB_YU::OnBnClickedBtmode)
	ON_EN_CHANGE(IDC_SPEED, &CTAB_YU::OnEnChangeSpeed)
	ON_EN_CHANGE(IDC_ANGLE, &CTAB_YU::OnEnChangeAngle)
END_MESSAGE_MAP()


// CTAB_YU message handlers


BOOL CTAB_YU::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	// mfc�� ǥ���� ��� ������ �ʱ�ȭ
	m_bRelease = false;
	m_bReleaseEnd = false;
	m_bEmergency = false;
	m_fTestSpeed = 0;
	m_fTestAngle = 0;
	m_bSetCam = false;
	m_recvRelease = false;
	//void RioThreadPoint(void *param);

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//winsock �ʱ�ȭ

	clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// server�� ������ ���� ����[clientSock]
	if (clientSock == -1)
	{
		AfxMessageBox("Connect Fail!", MB_ICONSTOP);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servaddr.sin_port = htons(SERVER_PORT_NUM);

	int re = 0;
	re = connect(clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server�� ���� ��û
	while (re == -1) {
		re = connect(clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server�� ���� ��û
	}

	server_hev = WSACreateEvent();

	WSAEventSelect(clientSock, server_hev, FD_READ | FD_CLOSE);	// �̺�Ʈ ����

	serverSock = SetTCPServer(PORT_NUM, BLOG_SIZE);		// ��� ���� ����
	if (serverSock == -1) {
		AfxMessageBox("��� ���� ����!", MB_ICONSTOP);
		return FALSE;
	}


	//AfxBeginThread(SendThreadPoint, (void *)this);
	AfxBeginThread(RecvThreadPoint, (void *)this);	// Server�� ���� �����͸� �޴� Thread ����
													//AfxBeginThread(SendThreadPoint, (void *)this);
	AfxBeginThread(EventLoop, (void *)this);	// Rio�� Tx����� ���� ������ �̺�Ʈ�� ó���ϴ� Thread ����
												//_beginthread(SendThreadPoint, 0, (void *)clientSock);
												//_beginthread(RecvThreadPoint, 0, (void *)clientSock);
												/*** ���� ����¿� ***/
												//int tmp1, tmp2;
	fp = fopen("mode.txt", "rt");
	if (fp) {
		fscanf(fp, "%f %d", &m_pre, &isThrower);
		fclose(fp);
	}

	SetTimer(4, 10, NULL);		// 10ms ���� Timer ȣ�� ����(���� ǥ�ÿ�)
	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

/* �׽�Ʈ�� ����� �۽ſ� Thread
UINT CTAB_YU::SendThreadPoint(LPVOID param1) {
//	CTAB_YU *p = (CTAB_YU *)param1;
//	SOCKET sock = p->clientSock;
//	while (true)
//	{
//		if(ex_bRIO_ResetFlag)
//		{
//		for (int i = 1; i < cnt; i++)
//		{
//			SendPacketReset(sock_base[i], 0,0);
//			Sleep(10);
//			SendPacketReset(sock_base[i],1,0);
//		}
//		ex_bRIO_ResetFlag = false;
//		ex_bReadyFlag = true;
//	}
//
//	if(ex_bDriveReady)
//	{
//		for (int i = 1; i < cnt; i++)
//		{
//			SendPacketmode(sock_base[i],ex_bRIO_ModeFlag,0);
//		}
////		ex_bsendparam = true;
//	}
//
//	if(ex_bDriveStart)
//	{
//		for (int i = 1; i < cnt; i++)
//		{
//			SendPacketDrivestart(sock_base[i],6,0);
//			ex_bRelease = true;
//			ex_bRelease_time = (p->m_fReleaseSpeed);
//		}
//		ex_bDriveStart = false;
//	}
//	if(ex_bReleasefinish)
//	{
//		for (int i = 1; i < cnt; i++)
//		{
//			SendPacketmode(sock_base[i],5,0);
//			Sleep(100);
//			SendPacketDrivestart(sock_base[i],0,0);
//		}
//		ex_bReleasefinish = false;
//	}
//
//	if(ex_bRelease)
//	{
//		for(int i =2; i< ex_bRelease_time;i++)
//		{
//			Sleep(1000);
//		}
//		//m_tabYU.m_bReleaseEnd = true;
//		Release_trow = true;
//		if(otherclock)
//		for(int i =0; i< ex_bRelease_time + 25;i++)
//		{
//			Sleep(1000);
//		}
//		ex_bReleasefinish = true;
//m_tabYU.m_bReleaseEnd = true;
}

/*if (GetKeyState(VK_MENU) & 0x8000) {
if (isThrower) {
std::cout << " 1. Send Throw Flag\t2. Send Speed Profile" << std::endl;
int menu;
Point pos[16];
std::cin >> menu;
switch (menu) {
case 1:
std::cout << "* Send Throw Flag * " << std::endl;
SendPacketThrowFlag(sock);
break;
case 2:
std::cout << "* Send Speed Profile * " << std::endl;
std::cout << " angle : ";
int angle;
std::cin >> angle;

std::cout << " speed : ";
int speed;
std::cin >> speed;

for (int i = 1; i < cnt; i++)
{
SendPacketSpeedProf(sock_base[i], angle, speed);
}
break;
}
}
}
}*/
/*	if(ex_bDriveReadyDelay)
{
for (int i = 1; i < cnt; i++)
{
//			SendPacketSpeedProf(sock_base[i], angle, speed);
}
ex_bDriveReadyDelay = false;
}
return 0;
}*/

// Rio Process�� ó���� Thread
UINT CTAB_YU::RioThreadPoint(LPVOID param) {
	CTAB_YU *p = (CTAB_YU *)param;
	for (int i = 1; i < cnt; i++)	// Rio�� ���� �غ� ��ȣ �۽�
		if (i != vs_sock)
		{
			if (p->m_pre != 2)
			{
				SendPacketpredrive(sock_base[i], 1, emp);
				Sleep(20);
				//SendPacketCRLF(sock_base[i]);
			}
			else
			{
				SendPacketpredrive(sock_base[i], 2, emp);
				Sleep(10);
				//SendPacketCRLF(sock_base[i]);
			}
		}
	Sleep(100);						// 100ms ���
	for (int i = 1; i < cnt; i++)	// Rio�� �ӵ� �������� �۽�
	{	//if (i != vs_sock)
		SendPacketSpeedProf(sock_base[i], recv_angle, recv_speed);
		Sleep(10);
		//SendPacketCRLF(sock_base[i]);
	}
	Sleep(100);
	for (int i = 1; i < cnt; i++)	// Rio, Tx�� ���� ���� ��ȣ �۽�
	{
		SendPacketDrivestart(sock_base[i], 1, emp);
		Sleep(10);
		//SendPacketCRLF(sock_base[i]);
	}
	SendPacketDrivestart(p->clientSock, 1, emp);
	isDriveSt = true;				// Drive start Set
	p->m_bRelease = true;			// NTCurlingDlg�� ���� ������ �˸��� ���� m_bRelease ���� Set

	return 0;
}

// Server�� ���� ��Ŷ�� �޴� Thread
UINT CTAB_YU::RecvThreadPoint(LPVOID param)
{
	CTAB_YU *p = (CTAB_YU *)param;
	SOCKET sock = p->clientSock;
	char msg[MAX_MSG_LEN];
	float speed, angle;
	bool isRight, is_myTurn;
	Point pos, tar_pos;

	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);

	while (true)
	{
		WSANETWORKEVENTS net_events;
		WSAEnumNetworkEvents(p->clientSock, server_hev, &net_events);
		switch (net_events.lNetworkEvents)
		{
		case FD_READ:
			if (recv(sock, msg, MAX_MSG_LEN, 0)>0)	// ��Ŷ ����
			{
				switch (RecvClassify(msg))	// ��Ŷ ���� �з�
				{
				case PRE_MODE:	// ���� �غ� ��ȣ�� ���
					RecvPacketPremode(msg, p->m_pre, p->m_mode);

					if (p->m_pre)
						isPremode = p->m_pre;
					else
						isPremode = false;

					if (p->m_mode == robot_id) {	// ������ robo id�� ��ġ�� ���
						isThrower = true;		// Thrower�� �ν�
						p->m_bSetCam = false;	// ī�޶� ���븦 ���� ���� m_bSetCam�� reset

						for (int i = 1; i < cnt; i++)	// Rio�� rio reset ��ȣ �۽�
							if (i != vs_sock)
								SendPacketRioreset(sock_base[i], recv_reset, emp);

					}
					else {
						isThrower = false;		// Skip���� �ν�
						p->m_bSetCam = true;	// ī�޶� ���븦 ��� ���� m_bSetCam�� set
					}

					/*** ���� ����¿� ***/
					fp = fopen("mode.txt", "wt");
					if (fp) {
						fprintf(fp, "%f %d", p->m_pre, isThrower);
						fclose(fp);
					}

					// Rx,Rio ���� ��� ���� �غ� ��ȣ �۽�
					for (int i = 1; i < cnt; i++)
					{
						SendPacketPremode(sock_base[i], p->m_pre, p->m_mode);
						Sleep(10);
						//SendPacketCRLF(sock_base[i]);
					}
					//send(sock_base[i], msg, 11, 0);
					SendPacketStoneInfoAck(p->clientSock,0);
					break;

				case CALL_STONE_INFO:	// ���� ���� ��û�� ���
					if (!isThrower) {
						for (int i = 1; i < cnt; i++)	// Skip�̸� Tx�� ���� ���� ��û �۽�
							if (i == vs_sock || i == test_sock)
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					SendPacketStoneInfoAck(p->clientSock,2);
					break;

				case CALL_ROBOT_INFO:	// �κ� ���� ��û�� ���
					if (isThrower) {
						for (int i = 1; i < cnt; i++)	// Thrower�� Tx�� �κ� ���� ��û �۽�
							if (i == vs_sock || i == test_sock)
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					break;

				case ROBOT_MODE:	// �κ� ����� ���
					RecvPacketThrowmode(msg, p->m_mode);
					if (p->m_mode == robot_id) {	// ������ robo id�� ��ġ�� ���
						isThrower = true;		// Thrower�� �ν�
						p->m_bSetCam = false;	// ī�޶� ���븦 ���� ���� m_bSetCam�� reset

												//	for (int i = 1; i < cnt; i++)	// Rio�� rio reset ��ȣ �۽�
												//		if (i != vs_sock)
												//			SendPacketRioreset(sock_base[i], recv_reset, emp);

					}
					else {
						isThrower = false;		// Skip���� �ν�
						p->m_bSetCam = true;	// ī�޶� ���븦 ��� ���� m_bSetCam�� set
					}


					for (int i = 1; i < cnt; i++)	// Tx�� �κ� ��� �۽�
						if (i == vs_sock || i == test_sock)
							send(sock_base[i], msg, MAX_MSG_LEN, 0);
					break;


				case RELEASE:	// Release�� �ӵ�, ����, �� ����, ��ġ ���� ������ ���
					if (isThrower && isReady) {

						p->m_recvRelease = true;
						RecvPacketRelease(msg, recv_speed, recv_angle, isRight, pos, tar_pos);
						p->m_fReleaseSpeed = recv_speed;
						p->m_fReleaseAngle = recv_angle;
						p->m_strCurl = (isRight) ? "Right" : "Left";
						p->m_bCW = isRight;
						p->m_strReleasePos.Format("%.2f, %.2f", pos.x, pos.y);
						p->m_strReleaseTarPos.Format("%.2f, %.2f", tar_pos.x, tar_pos.y);
						//p->m_bRelease = true;

						/*for (int i = 1; i < cnt; i++)	// Thrower�� Tx�� Release ���� �۽�
						if (i == vs_sock || i == test_sock)
						send(sock_base[i], msg, MAX_MSG_LEN, 0);*/

						AfxBeginThread(RioThreadPoint, (void *)p);

						/*if(p->m_pre)
						{
						speed_p = speed;
						angle_p = angle;

						speed_w = pos.x - tar_pos.x;
						angle_w = pos.y - tar_pos.y;
						}*/

						//p->m_bSetCam = true;

						//std::cout << "* Release Info *" << std::endl;
						//std::cout << " speed : " << speed << std::endl;
						//std::cout << " angle : " << angle << std::endl;
						//std::cout << " curl : " << ((isRight) ? "Right" : "Left") << std::endl;
						//std::cout << " pos : (" << pos.x << ", " << pos.y << ")" << std::endl;
					}
					else if (isPremode)	// Skip������ ��������� ���
					{
						RecvPacketRelease(msg, recv_speed, recv_angle, isRight, pos, tar_pos);
						p->m_fReleaseSpeed = recv_speed;
						p->m_fReleaseAngle = recv_angle;
						p->m_strCurl = (isRight) ? "Right" : "Left";
						p->m_strReleasePos.Format("%.2f, %.2f", pos.x, pos.y);
						p->m_strReleaseTarPos.Format("%.2f, %.2f", tar_pos.x, tar_pos.y);
						//p->m_bRelease = true;
						/*for (int i = 1; i < cnt; i++)	// Thrower�� Tx�� Release ���� �۽�
						if (i == vs_sock || i == test_sock)
						send(sock_base[i], msg, MAX_MSG_LEN, 0);*/
					}
					SendPacketStoneInfoAck(p->clientSock, 4);
					break;

				case FLAG:	// Throw Flag�� ���
					if (!isThrower) {	// 
						for (int i = 1; i < cnt; i++)
							if (i == vs_sock || i == test_sock)	// Skip�̸� Tx�� Throw Flag �۽�
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					break;
				case STONE_INFO_ACK:	// ���� ���� ���� �Ϸ��� ���
					if (!isThrower) {
						for (int i = 1; i < cnt; i++)
							if (i == vs_sock || i == test_sock)	// Skip�̸� Tx�� ���� ���� ���� �Ϸ� ��ȣ �۽�
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					//AfxMessageBox("Release KU!", MB_ICONSTOP);
					//SendPacketStoneInfoAck(p->clientSock,3);
					break;

				case EMERGENCY:		// Emergency ��ȣ�� ���
					for (int i = 1; i < cnt; i++)	// Tx�� Emergency ��ȣ �۽�
						if (i == vs_sock || i == test_sock)
							SendPacketEMERGENCY(sock_base[i], 0, 0);
					for (int i = 1; i < cnt; i++)	// Rio�� Emergency ��ȣ �۽�
						if (i != vs_sock)
						{
							SendPacketpredrive(sock_base[i], 5, 0);
							Sleep(10);
							//SendPacketCRLF(sock_base[i]);
						}
					p->m_bEmergency = true;
					break;
				case RESTART:	// Restart ��ȣ�� ���
					p->m_bEmergency = true;
					for (int i = 1; i < cnt; i++)	// Tx�� Emergency ��ȣ �۽�
						if (i == vs_sock || i == test_sock)
							SendPacketRestart(sock_base[i], 0, 0);

					for (int i = 1; i < cnt; i++)
						if (i != vs_sock)
						{
							SendPacketpredrive(sock_base[i], 1, 0);
							Sleep(10);
							//SendPacketCRLF(sock_base[i]);
						}

					//isThrower = true;			// Thrower�� Skip mode�� ����
					//isPremode = true;			// �������� ����带 ����

					isReady = false;		// ready ��ȣ(robot info call ���� �� set, release end �� reset)
					isDriveSt = false;		// drive start ��ȣ
					isRioRst = false;		// Rio reset ��ȣ

					p->m_fHogOffset = 0;

					p->m_bRelease = false;
					p->m_bReleaseEnd = false;
					p->m_bSetCam = false;

					break;

					/* �׽�Ʈ�� ����ߴ� ��Ŷ
					case RESET_KU:
					if (isThrower)
					{
					RecvPacketRioReset(msg, recv_reset, emp);
					AfxMessageBox("Release KU!", MB_ICONSTOP);
					if (recv_reset) {
					reset_flag = true;
					AfxBeginThread(RioThreadPoint, (void *)param);
					}
					else
					reset_flag = false;
					}
					break;*/
				case MY_TURN:	// My turn ��ȣ�� ���
					RecvPacketMyturn(msg, is_myTurn);
					for (int i = 1; i < cnt; i++) {
						if (i == vs_sock || i == test_sock) {	// Tx��
																//SendPacketPremode(sock_base[i], p->m_pre, p->m_mode);		// ���� ��� �۽�
																//Sleep(400);										// ��Ŷ �浹 ������ delay 400ms
																//SendPacketThrowmode(sock_base[i], p->m_mode);	// ���� Thrower �κ� id �۽�
																//Sleep(400);										// ��Ŷ �浹 ������ delay 400ms
							SendPacketMyturn(sock_base[i], is_myTurn);		// My turn ��ȣ �۽�
						}
					}
					/*** ���� ����¿� ***/
					fp = fopen("mode.txt", "rt");
					if (fp) {
						fscanf(fp, "%f %d", &p->m_pre, &isThrower);
						fclose(fp);
					}
					//SendPacketStoneInfoAck(p->clientSock);
					break;

				case INFO_TIME:
					for (int i = 1; i < cnt; i++) {
						if (i == vs_sock || i == test_sock)
						{
							SendPacketInfoTime(sock_base[i], RecvPacketInfoTime(msg));
						}
					}
					break;
				}
			}
			break;
		case FD_CLOSE:
			closesocket(p->clientSock);
			p->clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// server�� ������ ���� ����[clientSock]
			int re = -1;
			while (re == -1) {
				re = connect(p->clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server�� ���� ��û
			}
			sock = p->clientSock;
			server_hev = WSACreateEvent();
			WSAEventSelect(p->clientSock, server_hev, FD_READ | FD_CLOSE);
			break;		// ���� ������ ���
		}
	}


	closesocket(sock);

	return 0;
}

// Tx, Rio�� �۽��� SBC�� Socket ����
SOCKET CTAB_YU::SetTCPServer(short pnum, int blog)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// ���� ����
	if (sock == -1) { return -1; }

	SOCKADDR_IN servaddr = { 0 };// ���� �ּ�
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT_NUM);

	int re = 0;
	re = bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));// ���� �ּҿ� ��Ʈ��ũ �������̽� ����
	if (re == -1) { return -1; }

	re = listen(sock, blog);// �� �α� ť ����
	if (re == -1) { return -1; }
	return sock;
}

// ������ ��Ʈ��ũ �̺�Ʈ ������ �Լ�
HANDLE CTAB_YU::AddNetworkEvent(SOCKET sock, long net_event)
{
	HANDLE hev = WSACreateEvent();

	sock_base[cnt] = sock;	// ���� ������ ���� sock_base�� ���
	hev_base[cnt] = hev;	// ��Ʈ��ũ �̺�Ʈ ������ ���� hev_base�� ���
	cnt++;					// ��� ���� client ���� �� ����

	WSAEventSelect(sock, hev, net_event);	// �̺�Ʈ ����
	return hev;
}

// Tx, Rio�� ���� �Ͼ�� ��Ʈ��ũ �̺�Ʈ ������ Thread
UINT CTAB_YU::EventLoop(LPVOID param)
{
	CTAB_YU *p = (CTAB_YU *)param;
	p->AddNetworkEvent(p->serverSock, FD_ACCEPT);	// Tx, Rio������ ���� ��û ���� ����

	while (true)
	{
		int index = WSAWaitForMultipleEvents(cnt, hev_base, false, INFINITE, false);
		WSANETWORKEVENTS net_events;
		WSAEnumNetworkEvents(sock_base[index], hev_base[index], &net_events);
		switch (net_events.lNetworkEvents)
		{
		case FD_ACCEPT: p->AcceptProc(index); break;	// ���� ��û�� ���
		case FD_READ:
			p->ReadProc(index);							// ��Ŷ ������ ���
			break;
		case FD_CLOSE: p->CloseProc(index); break;		// ���� ������ ���
		}
	}
	closesocket(p->serverSock);//���� �ݱ�
	closesocket(p->clientSock);//���� �ݱ�  
	WSACleanup();//���� ����ȭ 
}

// Tx, Rio�� ������ ���� ��û ó�� ���μ���
void CTAB_YU::AcceptProc(int index)
{
	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	SOCKET dosock = accept(sock_base[0], (SOCKADDR *)&cliaddr, &len);
	LPCSTR temp;

	if (cnt == FD_SETSIZE)	// client�� ���̻� ������ �� ���� ���
	{
		temp = "FULL!";
		//temp.Format(_T("FULL! %s:%d can't connect!", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));
		AfxMessageBox(temp, MB_ICONSTOP);	// FUll Message Box ���
		closesocket(dosock);
		return;
	}

	AddNetworkEvent(dosock, FD_READ | FD_CLOSE);	// dsock(Tx or Rio) ��Ʈ��ũ ���� �� ���� ���� ���� ���� �� ���
	temp = "connect";
	//temp.Format(_T("%s:%d connect", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));
	//AfxMessageBox(temp, MB_ICONINFORMATION);	// connect Message Box ���

	Sleep(100);
	send(sock_base[cnt - 1], "vs", 11, 0);	// Tx Ȯ�ο� vs �޼��� �۽�(Tx�� ��� echo�� ����, Rio�� ��� echo ����)
	Sleep(10);
	send(sock_base[cnt - 1], "test", 11, 0);	// ���� �����ϰ� Test�� ���α׷����� Ȯ�ο� test �޼��� �۽�
}

// Tx, Rio�� ������ ���� ó�� ���μ���
void CTAB_YU::ReadProc(int index)
{
	//CTAB_YU *p = (CTAB_YU *)param;
	char msg[MAX_MSG_LEN];
	recv(sock_base[index], msg, MAX_MSG_LEN, 0);

	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	getpeername(sock_base[index], (SOCKADDR *)&cliaddr, &len);

	//char smsg[MAX_MSG_LEN];
	//sprintf(smsg, "[%s:%d]:%s", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), msg);

	static int stCnt, i;
	float accel, hogDist, hogOffs, Dist;
	bool isRight, isColor;
	Point pos, tar_pos;
	double Rio_encoder, emp, robot_angle;
	float r_release, r_arrive;
	Point r_pos;

	if (strcmp(msg, "vs") == 0) {	// vs�� �۽��� client�� Tx�� �ν��ϱ� ���� vs_sock���� ����
									//AfxMessageBox("connect_vs", MB_ICONINFORMATION);
		vs_sock = index;
	}
	else if (strcmp(msg, "test") == 0) {	// ���� �����ϰ� test ���α׷��� ����
											//AfxMessageBox("connect_test", MB_ICONINFORMATION);
		test_sock = index;
	}

	switch (RecvClassify(msg))	// ������ ��Ŷ �з�
	{

	case STONE_CNT:		// ���� ������ ���
		if (!isThrower) {
			RecvPacketStoneCnt(msg, stCnt);
			//std::cout << "* Stone Info >> KU_Server *" << std::endl;
			//std::cout << " cnt : " << stCnt << std::endl;
			//i = 1;
			send(clientSock, msg, MAX_MSG_LEN, 0);	// Skip�̸� server�� �۽�
		}
		break;
	case STONE_INFO:	// ���� ������ ���
		if (!isThrower) {
			RecvPacketStoneInfo(msg, pos, isColor);
			//std::cout << " stone" << i++ << " pos : (" << pos.x << ", " << pos.y << ")" << std::endl;
			send(clientSock, msg, MAX_MSG_LEN, 0);	// Skip�̸� server�� �۽�
		}
		break;


	case ROBOT_INFO:	// �κ� ����(����&��ġ or Hogline���� �Ÿ�)�� ���
		if (isThrower) {// Thrower�̸�
			RecvPacketRobotInfo(msg, angle, pos, hogDist, hogOffs);
			m_fRobotAngle = angle;
			//m_strRobotPos.Format("%.2f, %.2f", pos.x, pos.y);
			m_fHogDist = hogDist;
			m_fHogOffset = hogOffs;
			//std::cout << "* Robot Info >> UNIST_NI Board *" << std::endl;
			//std::cout << " angle : " << angle << std::endl;
			//std::cout << " pos : (" << pos.x << ", " << pos.y << ")" << std::endl;
			//std::cout << " hogline distance : " << hogDist << std::endl;
			//std::cout << " hogline offset : " << hogOffs << std::endl;
			//for (int i = 1; i < cnt; i++)
			//{
			//	send(sock_base[i], msg, MAX_MSG_LEN, 0);
			//}

			//m_fRioangle = 10000 / cos(m_fRobotAngle*(PI / 180));

			m_bSetCam = false;		// ī�޶� ���� ����

			SendPacketStoneInfoAck(clientSock,1);

			if (m_fHogOffset == 0)	// Drive Start���� ���� ���(����&��ġ ������ ���)
			{
				SendPacketHogdistance(clientSock, hog_dist_trans);	// Server�� ȣ�׶��� default �Ÿ� ����
				Sleep(10);											// 10ms ���
				SendPacketPositionProf(clientSock, pos.x, pos.y);	// server�� ��ġ ���� �۽�
				isReady = true;
				for (int i = 1; i < cnt; i++) {
					//SendPacketPositionProf(sock_base[i], pos.x, pos.y);
					if (i != vs_sock) {								// Rio�� angle ������ calibration angle ��ȣ �۽�
																	//SendPacketpredrive(sock_base[i], 1, 0);
																	//Sleep(20);
						SendPacketTargetangle(sock_base[i], angle, 0.0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
						Sleep(20);
						SendPacketCalibAngle(sock_base[i], 1, 0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
						//Sleep(10000);								// 20s ���
						//isReady = true;							// reset_flag set
						//AfxBeginThread(RioThreadPoint, (void *)this);	// Rio process thread ����
					}
				}
			}

			Dist = 500 + recv_speed*0.1;	// �Ÿ� ���
			if ( m_fHogOffset < 0) {	// ���� �Ÿ� ���� ���� �Ÿ��� ���� ���
				isRioRst = false;	// ���� �ʱ�ȭ
				isDriveSt = false;
				isReady = false;
				m_bReleaseEnd = true;	// ���� Throw(Release�� ����)�� �˸��� ���� set
				SendPacketThrowFlag(clientSock);	// Throw flag, server�� �۽�
				for (int i = 1; i < cnt; i++)		// Rio�� Throw flag �۽�
					SendPacketRioThrowFlag(sock_base[i]);
				for (int i = 1; i < cnt; i++)	// Rio�� Drive start ���� �۽�
					if (i != vs_sock)
					{
						SendPacketDrivestart(sock_base[i], 0, 0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
					}
				Sleep(5000);
				for (int i = 1; i < cnt; i++)		// Rio�� �����(5) �۽�
					if (i != vs_sock)
					{
						SendPacketpredrive(sock_base[i], 5, 0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
					}
				otherclock = true;
			}
		}
		break;

	case INFO_RESULT:
		RecvPacketInfoResult(msg,r_release,r_arrive,r_pos);
		SendPacketInfoResult(clientSock,r_release,r_arrive,r_pos);
		break;

	//case RIO_ENCODER:	// �߰� ���� 171210

	//	RecvPacketRioencoder(msg, Rio_encoder, emp);
	//	m_fRioEncoder = Rio_encoder;

	//	AfxMessageBox(Rio_encoder, MB_ICONSTOP);

	//	if (m_fRioangle < m_fRioEncoder)
	//	{
	//		isRioRst = false;	// ���� �ʱ�ȭ
	//		isDriveSt = false;
	//		isReady = false;
	//		m_bReleaseEnd = true;	// ���� Throw(Release�� ����)�� �˸��� ���� set
	//		SendPacketThrowFlag(clientSock);	// Throw flag, server�� �۽�
	//		for (int i = 1; i < cnt; i++)
	//		{// Rio�� Throw flag �۽�
	//			SendPacketRioThrowFlag(sock_base[i]);
	//			Sleep(20);
	//			//SendPacketCRLF(sock_base[i]);
	//		}
	//		for (int i = 1; i < cnt; i++)	// Rio�� Drive start ���� �۽�
	//			if (i != vs_sock)
	//			{
	//				SendPacketDrivestart(sock_base[i], 0, 0);
	//				Sleep(20);
	//				//SendPacketCRLF(sock_base[i]);
	//			}
	//		Sleep(5000);
	//		for (int i = 1; i < cnt; i++)		// Rio�� �����(5) �۽�
	//			if (i != vs_sock)
	//			{
	//				SendPacketpredrive(sock_base[i], 5, 0);
	//				Sleep(20);
	//				//SendPacketCRLF(sock_base[i]);
	//			}
	//	}
	//	break;

		/*case ACCEL_PROF:
		if (isThrower) {
		RecvPacketAccelProf(msg, pos, accel);
		m_fAccel = angle;
		m_strAccelPos.Format("%.2f, %.2f", pos.x, pos.y);
		//std::cout << "* Accel Profile *" << std::endl;
		//std::cout << " pos : (" << pos.x << ", " << pos.y << ")" << std::endl;
		//std::cout << " accel : " << accel << std::endl;
		}
		break;*/
	}
}


// Tx, Rio�� ������ ���� ���� ���μ���
void CTAB_YU::CloseProc(int index)
{
	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	getpeername(sock_base[index], (SOCKADDR *)&cliaddr, &len);
	//printf("[%s:%d] disconnect\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

	closesocket(sock_base[index]);	// ���� �ݱ�
	WSACloseEvent(hev_base[index]);	// �̺�Ʈ ���� ����

	cnt--;
	sock_base[index] = sock_base[cnt];
	hev_base[index] = hev_base[cnt];

	char msg[MAX_MSG_LEN];
	sprintf(msg, "[%s:%d] disconnect\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	//for (int i = 1; i<cnt; i++)
	//{
	//	send(sock_base[i], msg, MAX_MSG_LEN, 0);
	//}
}

// ���� ǥ�ÿ�(������Ʈ) Timer
void CTAB_YU::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (nIDEvent == 4)
	{
		UpdateData(FALSE);
		/*** ���� ����¿� ***/
		if (isThrower)
			GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Thrower"));
		else
			GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Skip"));

	}
	CDialog::OnTimer(nIDEvent);
}

// Thrower <-> Skip ��ȯ �׽�Ʈ�� ��ư
void CTAB_YU::OnBnClickedBtmode()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	isThrower = !isThrower;
	if (isThrower)
		GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Thrower"));
	else
		GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Skip"));

}

void CTAB_YU::OnEnChangeSpeed()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CTAB_YU::OnEnChangeAngle()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

// ��� �۽� (Joystic ���ۿ�)
void CTAB_YU::SendMode(float prep) {
	if (prep != 0) {	// �Է� ��尡 0�� �ƴϸ� Rio�� ��� �۽�
		for (int i = 1; i < cnt; i++)
		{
			if (i != vs_sock)
			{
				SendPacketpredrive(sock_base[i], prep, 0);
				Sleep(20);
				//SendPacketCRLF(sock_base[i]);
			}
		}
		return;
	}	// �Է� ��尡 0�̸� Rio�� Drive ���� ��ȣ �۽�
	for (int i = 1; i < cnt; i++)
	{
		if (i != vs_sock)
		{
			SendPacketDrivestart(sock_base[i], 0, 0);
			Sleep(20);
			//SendPacketCRLF(sock_base[i]);
		}
	}
}

// ���� �� �۽� (Joystic ���ۿ�)
void CTAB_YU::TestMove(bool cst) {
	if (!cst) // �Է��� ������ �Ͼ ��� Rio�� �ӵ� �������� �۽�
		for (int i = 1; i<cnt; i++) {
			if (i != vs_sock) {
				SendPacketSpeedProf(sock_base[i], m_fTestAngle, m_fTestSpeed);
				Sleep(20);
				//SendPacketCRLF(sock_base[i]);
			}
		}
	for (int i = 1; i < cnt; i++)	// Rio�� Drive start ��ȣ �۽�
		if (i != vs_sock)
		{
			SendPacketDrivestart(sock_base[i], 1, 0);
			Sleep(20);
			//SendPacketCRLF(sock_base[i]);
		}
}

// ����&��ġ ���� ��û (Joystic ���ۿ�)
void CTAB_YU::CallAngle() {
	for (int i = 1; i<cnt; i++)	// Tx�� ����&��ġ ���� ��û
		if (i == vs_sock || i == test_sock)
			SendPacketCallrobotinfo(sock_base[i], 0);
}

void CTAB_YU::SendAngle() {
	for (int i = 1; i<cnt; i++)	// Tx�� ����&��ġ ���� ��û
		if (i != vs_sock)
			SendPacketTargetangle(sock_base[i], 1.5, 0);
}

void CTAB_YU::SendCalib() {
	for (int i = 1; i<cnt; i++)	// Tx�� ����&��ġ ���� ��û
		if (i != vs_sock)
			SendPacketCalibAngle(sock_base[i], 1, 0);
}

void CTAB_YU::SendEmergency() {
	for (int i = 1; i<cnt; i++)	// Tx�� ����&��ġ ���� ��û
		if (i != vs_sock)
			SendPacketDrivestart(sock_base[i], 0, 0);


	for (int i = 1; i<cnt; i++)	// Tx�� ����&��ġ ���� ��û
		if (i == vs_sock || i == test_sock)
			SendPacketEMERGENCY(sock_base[i], 0, 0);
}