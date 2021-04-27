// TAB_YU.cpp : implementation file

#include "stdafx.h"
#include "NTcurling.h"
#include "TAB_YU.h"

#define SERVER_PORT_NUM      8000		// Server에 연결할 때 사용할 Port number(8000)
//#define SERVER_IP        "192.168.0.13"	// Server에 연결할 때 사용할 IP address
//#define SERVER_IP        "192.168.0.11"	// Server에 연결할 때 사용할 IP address
#define SERVER_IP        "127.0.0.1"	// Server에 연결할 때 사용할 IP address
//#define SERVER_IP        "172.20.10.4"	// Server에 연결할 때 사용할 IP address

#define PORT_NUM      10200		// TX, Rio와 연결할 때 사용할 Port number(10200)
#define BLOG_SIZE       5		// 백로그 사이즈(5)
#define MAX_MSG_LEN 256			// 패킷의 최대 길이(256)
#define PI 3.14159265359

SOCKET  sock_base[FD_SETSIZE];	// 연결된 client 소켓을 관리할 SOCKET 배열
HANDLE hev_base[FD_SETSIZE];	// 각 client 소켓의 네트워크 이벤트를 감지할 이벤트 객체 배열
HANDLE server_hev;

int cnt;						// 연결된 client 수 관리
bool isThrower = true;			// Thrower와 Skip mode를 구분
bool isPremode = true;			// 연습모드와 경기모드를 구분

int vs_sock = -1;		// TX(vision) client의 소켓의 index를 저장
int test_sock = -1;

int robot_id = 1;		// Thrower인지 Skip인지 구분할 robot_id를 관리

int m_iCount = 0;

float hog_dist_trans = 500;	// 송신할 hogline 까지의 거리(default: 500)
float recv_speed, recv_angle, recv_reset, emp;	// 수신할 데이터(속도, 각도, Reset 신호, rio용 empty)
float speed, angle, speed_p, angle_p, speed_w, angle_w = 0;
bool isReady = false;		// ready 신호(robot info call 수신 시 set, release end 시 reset)
bool isDriveSt = false;		// drive start 신호
bool isRioRst = false;		// Rio reset 신호

SOCKADDR_IN servaddr = { 0 };

/*** 파일 입출력용 ***/
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

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	// mfc에 표시할 멤버 변수들 초기화
	m_bRelease = false;
	m_bReleaseEnd = false;
	m_bEmergency = false;
	m_fTestSpeed = 0;
	m_fTestAngle = 0;
	m_bSetCam = false;
	m_recvRelease = false;
	//void RioThreadPoint(void *param);

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//winsock 초기화

	clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// server에 접속할 소켓 생성[clientSock]
	if (clientSock == -1)
	{
		AfxMessageBox("Connect Fail!", MB_ICONSTOP);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servaddr.sin_port = htons(SERVER_PORT_NUM);

	int re = 0;
	re = connect(clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server에 연결 요청
	while (re == -1) {
		re = connect(clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server에 연결 요청
	}

	server_hev = WSACreateEvent();

	WSAEventSelect(clientSock, server_hev, FD_READ | FD_CLOSE);	// 이벤트 설정

	serverSock = SetTCPServer(PORT_NUM, BLOG_SIZE);		// 대기 소켓 설정
	if (serverSock == -1) {
		AfxMessageBox("대기 소켓 오류!", MB_ICONSTOP);
		return FALSE;
	}


	//AfxBeginThread(SendThreadPoint, (void *)this);
	AfxBeginThread(RecvThreadPoint, (void *)this);	// Server로 부터 데이터를 받는 Thread 실행
													//AfxBeginThread(SendThreadPoint, (void *)this);
	AfxBeginThread(EventLoop, (void *)this);	// Rio와 Tx보드로 부터 들어오는 이벤트를 처리하는 Thread 실행
												//_beginthread(SendThreadPoint, 0, (void *)clientSock);
												//_beginthread(RecvThreadPoint, 0, (void *)clientSock);
												/*** 파일 입출력용 ***/
												//int tmp1, tmp2;
	fp = fopen("mode.txt", "rt");
	if (fp) {
		fscanf(fp, "%f %d", &m_pre, &isThrower);
		fclose(fp);
	}

	SetTimer(4, 10, NULL);		// 10ms 마다 Timer 호출 설정(변수 표시용)
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

/* 테스트로 사용한 송신용 Thread
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

// Rio Process를 처리할 Thread
UINT CTAB_YU::RioThreadPoint(LPVOID param) {
	CTAB_YU *p = (CTAB_YU *)param;
	for (int i = 1; i < cnt; i++)	// Rio로 주행 준비 신호 송신
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
	Sleep(100);						// 100ms 대기
	for (int i = 1; i < cnt; i++)	// Rio로 속도 프로파일 송신
	{	//if (i != vs_sock)
		SendPacketSpeedProf(sock_base[i], recv_angle, recv_speed);
		Sleep(10);
		//SendPacketCRLF(sock_base[i]);
	}
	Sleep(100);
	for (int i = 1; i < cnt; i++)	// Rio, Tx로 주행 시작 신호 송신
	{
		SendPacketDrivestart(sock_base[i], 1, emp);
		Sleep(10);
		//SendPacketCRLF(sock_base[i]);
	}
	SendPacketDrivestart(p->clientSock, 1, emp);
	isDriveSt = true;				// Drive start Set
	p->m_bRelease = true;			// NTCurlingDlg에 주행 시작을 알리기 위해 m_bRelease 변수 Set

	return 0;
}

// Server로 부터 패킷을 받는 Thread
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
			if (recv(sock, msg, MAX_MSG_LEN, 0)>0)	// 패킷 수신
			{
				switch (RecvClassify(msg))	// 패킷 종류 분류
				{
				case PRE_MODE:	// 주행 준비 신호일 경우
					RecvPacketPremode(msg, p->m_pre, p->m_mode);

					if (p->m_pre)
						isPremode = p->m_pre;
					else
						isPremode = false;

					if (p->m_mode == robot_id) {	// 수신한 robo id가 일치할 경우
						isThrower = true;		// Thrower로 인식
						p->m_bSetCam = false;	// 카메라 폴대를 접기 위해 m_bSetCam을 reset

						for (int i = 1; i < cnt; i++)	// Rio로 rio reset 신호 송신
							if (i != vs_sock)
								SendPacketRioreset(sock_base[i], recv_reset, emp);

					}
					else {
						isThrower = false;		// Skip으로 인식
						p->m_bSetCam = true;	// 카메라 폴대를 펴기 위해 m_bSetCam을 set
					}

					/*** 파일 입출력용 ***/
					fp = fopen("mode.txt", "wt");
					if (fp) {
						fprintf(fp, "%f %d", p->m_pre, isThrower);
						fclose(fp);
					}

					// Rx,Rio 양쪽 모두 주행 준비 신호 송신
					for (int i = 1; i < cnt; i++)
					{
						SendPacketPremode(sock_base[i], p->m_pre, p->m_mode);
						Sleep(10);
						//SendPacketCRLF(sock_base[i]);
					}
					//send(sock_base[i], msg, 11, 0);
					SendPacketStoneInfoAck(p->clientSock,0);
					break;

				case CALL_STONE_INFO:	// 스톤 정보 요청일 경우
					if (!isThrower) {
						for (int i = 1; i < cnt; i++)	// Skip이면 Tx로 스톤 정보 요청 송신
							if (i == vs_sock || i == test_sock)
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					SendPacketStoneInfoAck(p->clientSock,2);
					break;

				case CALL_ROBOT_INFO:	// 로봇 정보 요청일 경우
					if (isThrower) {
						for (int i = 1; i < cnt; i++)	// Thrower면 Tx로 로봇 정보 요청 송신
							if (i == vs_sock || i == test_sock)
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					break;

				case ROBOT_MODE:	// 로봇 모드일 경우
					RecvPacketThrowmode(msg, p->m_mode);
					if (p->m_mode == robot_id) {	// 수신한 robo id가 일치할 경우
						isThrower = true;		// Thrower로 인식
						p->m_bSetCam = false;	// 카메라 폴대를 접기 위해 m_bSetCam을 reset

												//	for (int i = 1; i < cnt; i++)	// Rio로 rio reset 신호 송신
												//		if (i != vs_sock)
												//			SendPacketRioreset(sock_base[i], recv_reset, emp);

					}
					else {
						isThrower = false;		// Skip으로 인식
						p->m_bSetCam = true;	// 카메라 폴대를 펴기 위해 m_bSetCam을 set
					}


					for (int i = 1; i < cnt; i++)	// Tx로 로봇 모드 송신
						if (i == vs_sock || i == test_sock)
							send(sock_base[i], msg, MAX_MSG_LEN, 0);
					break;


				case RELEASE:	// Release할 속도, 각도, 컬 방향, 위치 등의 정보일 경우
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

						/*for (int i = 1; i < cnt; i++)	// Thrower면 Tx로 Release 정보 송신
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
					else if (isPremode)	// Skip이지만 연습모드일 경우
					{
						RecvPacketRelease(msg, recv_speed, recv_angle, isRight, pos, tar_pos);
						p->m_fReleaseSpeed = recv_speed;
						p->m_fReleaseAngle = recv_angle;
						p->m_strCurl = (isRight) ? "Right" : "Left";
						p->m_strReleasePos.Format("%.2f, %.2f", pos.x, pos.y);
						p->m_strReleaseTarPos.Format("%.2f, %.2f", tar_pos.x, tar_pos.y);
						//p->m_bRelease = true;
						/*for (int i = 1; i < cnt; i++)	// Thrower면 Tx로 Release 정보 송신
						if (i == vs_sock || i == test_sock)
						send(sock_base[i], msg, MAX_MSG_LEN, 0);*/
					}
					SendPacketStoneInfoAck(p->clientSock, 4);
					break;

				case FLAG:	// Throw Flag인 경우
					if (!isThrower) {	// 
						for (int i = 1; i < cnt; i++)
							if (i == vs_sock || i == test_sock)	// Skip이면 Tx로 Throw Flag 송신
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					break;
				case STONE_INFO_ACK:	// 스톤 정보 수신 완료인 경우
					if (!isThrower) {
						for (int i = 1; i < cnt; i++)
							if (i == vs_sock || i == test_sock)	// Skip이면 Tx로 스톤 정보 수신 완료 신호 송신
								send(sock_base[i], msg, MAX_MSG_LEN, 0);
					}
					//AfxMessageBox("Release KU!", MB_ICONSTOP);
					//SendPacketStoneInfoAck(p->clientSock,3);
					break;

				case EMERGENCY:		// Emergency 신호인 경우
					for (int i = 1; i < cnt; i++)	// Tx로 Emergency 신호 송신
						if (i == vs_sock || i == test_sock)
							SendPacketEMERGENCY(sock_base[i], 0, 0);
					for (int i = 1; i < cnt; i++)	// Rio로 Emergency 신호 송신
						if (i != vs_sock)
						{
							SendPacketpredrive(sock_base[i], 5, 0);
							Sleep(10);
							//SendPacketCRLF(sock_base[i]);
						}
					p->m_bEmergency = true;
					break;
				case RESTART:	// Restart 신호인 경우
					p->m_bEmergency = true;
					for (int i = 1; i < cnt; i++)	// Tx로 Emergency 신호 송신
						if (i == vs_sock || i == test_sock)
							SendPacketRestart(sock_base[i], 0, 0);

					for (int i = 1; i < cnt; i++)
						if (i != vs_sock)
						{
							SendPacketpredrive(sock_base[i], 1, 0);
							Sleep(10);
							//SendPacketCRLF(sock_base[i]);
						}

					//isThrower = true;			// Thrower와 Skip mode를 구분
					//isPremode = true;			// 연습모드와 경기모드를 구분

					isReady = false;		// ready 신호(robot info call 수신 시 set, release end 시 reset)
					isDriveSt = false;		// drive start 신호
					isRioRst = false;		// Rio reset 신호

					p->m_fHogOffset = 0;

					p->m_bRelease = false;
					p->m_bReleaseEnd = false;
					p->m_bSetCam = false;

					break;

					/* 테스트로 사용했던 패킷
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
				case MY_TURN:	// My turn 신호인 경우
					RecvPacketMyturn(msg, is_myTurn);
					for (int i = 1; i < cnt; i++) {
						if (i == vs_sock || i == test_sock) {	// Tx로
																//SendPacketPremode(sock_base[i], p->m_pre, p->m_mode);		// 현재 모드 송신
																//Sleep(400);										// 패킷 충돌 방지용 delay 400ms
																//SendPacketThrowmode(sock_base[i], p->m_mode);	// 현재 Thrower 로봇 id 송신
																//Sleep(400);										// 패킷 충돌 방지용 delay 400ms
							SendPacketMyturn(sock_base[i], is_myTurn);		// My turn 신호 송신
						}
					}
					/*** 파일 입출력용 ***/
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
			p->clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// server에 접속할 소켓 생성[clientSock]
			int re = -1;
			while (re == -1) {
				re = connect(p->clientSock, (struct sockaddr *)&servaddr, sizeof(servaddr));	//Server에 연결 요청
			}
			sock = p->clientSock;
			server_hev = WSACreateEvent();
			WSAEventSelect(p->clientSock, server_hev, FD_READ | FD_CLOSE);
			break;		// 연결 종료일 경우
		}
	}


	closesocket(sock);

	return 0;
}

// Tx, Rio와 송신한 SBC의 Socket 설정
SOCKET CTAB_YU::SetTCPServer(short pnum, int blog)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// 소켓 생성
	if (sock == -1) { return -1; }

	SOCKADDR_IN servaddr = { 0 };// 소켓 주소
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT_NUM);

	int re = 0;
	re = bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));// 소켓 주소와 네트워크 인터페이스 결합
	if (re == -1) { return -1; }

	re = listen(sock, blog);// 백 로그 큐 설정
	if (re == -1) { return -1; }
	return sock;
}

// 감지할 네트워크 이벤트 설정용 함수
HANDLE CTAB_YU::AddNetworkEvent(SOCKET sock, long net_event)
{
	HANDLE hev = WSACreateEvent();

	sock_base[cnt] = sock;	// 소켓 관리를 위해 sock_base에 등록
	hev_base[cnt] = hev;	// 네트워크 이벤트 감지를 위해 hev_base에 등록
	cnt++;					// 사용 중인 client 소켓 수 증가

	WSAEventSelect(sock, hev, net_event);	// 이벤트 설정
	return hev;
}

// Tx, Rio에 의해 일어나는 네트워크 이벤트 설정용 Thread
UINT CTAB_YU::EventLoop(LPVOID param)
{
	CTAB_YU *p = (CTAB_YU *)param;
	p->AddNetworkEvent(p->serverSock, FD_ACCEPT);	// Tx, Rio에서의 연결 요청 감지 설정

	while (true)
	{
		int index = WSAWaitForMultipleEvents(cnt, hev_base, false, INFINITE, false);
		WSANETWORKEVENTS net_events;
		WSAEnumNetworkEvents(sock_base[index], hev_base[index], &net_events);
		switch (net_events.lNetworkEvents)
		{
		case FD_ACCEPT: p->AcceptProc(index); break;	// 연결 요청일 경우
		case FD_READ:
			p->ReadProc(index);							// 패킷 수신일 경우
			break;
		case FD_CLOSE: p->CloseProc(index); break;		// 연결 종료일 경우
		}
	}
	closesocket(p->serverSock);//소켓 닫기
	closesocket(p->clientSock);//소켓 닫기  
	WSACleanup();//윈속 해제화 
}

// Tx, Rio로 부터의 연결 요청 처리 프로세스
void CTAB_YU::AcceptProc(int index)
{
	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	SOCKET dosock = accept(sock_base[0], (SOCKADDR *)&cliaddr, &len);
	LPCSTR temp;

	if (cnt == FD_SETSIZE)	// client를 더이상 연결할 수 없는 경우
	{
		temp = "FULL!";
		//temp.Format(_T("FULL! %s:%d can't connect!", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));
		AfxMessageBox(temp, MB_ICONSTOP);	// FUll Message Box 출력
		closesocket(dosock);
		return;
	}

	AddNetworkEvent(dosock, FD_READ | FD_CLOSE);	// dsock(Tx or Rio) 네트워크 수신 및 연결 종료 감지 설정 및 등록
	temp = "connect";
	//temp.Format(_T("%s:%d connect", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));
	//AfxMessageBox(temp, MB_ICONINFORMATION);	// connect Message Box 출력

	Sleep(100);
	send(sock_base[cnt - 1], "vs", 11, 0);	// Tx 확인용 vs 메세지 송신(Tx일 경우 echo로 수신, Rio일 경우 echo 없음)
	Sleep(10);
	send(sock_base[cnt - 1], "test", 11, 0);	// 위와 동일하게 Test용 프로그램인지 확인용 test 메세지 송신
}

// Tx, Rio로 부터의 수신 처리 프로세스
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

	if (strcmp(msg, "vs") == 0) {	// vs를 송신한 client를 Tx로 인식하기 위해 vs_sock에서 관리
									//AfxMessageBox("connect_vs", MB_ICONINFORMATION);
		vs_sock = index;
	}
	else if (strcmp(msg, "test") == 0) {	// 위와 동일하게 test 프로그램을 관리
											//AfxMessageBox("connect_test", MB_ICONINFORMATION);
		test_sock = index;
	}

	switch (RecvClassify(msg))	// 수신한 패킷 분류
	{

	case STONE_CNT:		// 스톤 갯수인 경우
		if (!isThrower) {
			RecvPacketStoneCnt(msg, stCnt);
			//std::cout << "* Stone Info >> KU_Server *" << std::endl;
			//std::cout << " cnt : " << stCnt << std::endl;
			//i = 1;
			send(clientSock, msg, MAX_MSG_LEN, 0);	// Skip이면 server로 송신
		}
		break;
	case STONE_INFO:	// 스톤 정보인 경우
		if (!isThrower) {
			RecvPacketStoneInfo(msg, pos, isColor);
			//std::cout << " stone" << i++ << " pos : (" << pos.x << ", " << pos.y << ")" << std::endl;
			send(clientSock, msg, MAX_MSG_LEN, 0);	// Skip이면 server로 송신
		}
		break;


	case ROBOT_INFO:	// 로봇 정보(각도&위치 or Hogline까지 거리)인 경우
		if (isThrower) {// Thrower이면
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

			m_bSetCam = false;		// 카메라 폴대 접음

			SendPacketStoneInfoAck(clientSock,1);

			if (m_fHogOffset == 0)	// Drive Start하지 않은 경우(각도&위치 정보인 경우)
			{
				SendPacketHogdistance(clientSock, hog_dist_trans);	// Server로 호그라인 default 거리 전송
				Sleep(10);											// 10ms 대기
				SendPacketPositionProf(clientSock, pos.x, pos.y);	// server로 위치 정보 송신
				isReady = true;
				for (int i = 1; i < cnt; i++) {
					//SendPacketPositionProf(sock_base[i], pos.x, pos.y);
					if (i != vs_sock) {								// Rio로 angle 정보와 calibration angle 신호 송신
																	//SendPacketpredrive(sock_base[i], 1, 0);
																	//Sleep(20);
						SendPacketTargetangle(sock_base[i], angle, 0.0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
						Sleep(20);
						SendPacketCalibAngle(sock_base[i], 1, 0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
						//Sleep(10000);								// 20s 대기
						//isReady = true;							// reset_flag set
						//AfxBeginThread(RioThreadPoint, (void *)this);	// Rio process thread 시작
					}
				}
			}

			Dist = 500 + recv_speed*0.1;	// 거리 계산
			if ( m_fHogOffset < 0) {	// 계산된 거리 보다 측정 거리가 작은 경우
				isRioRst = false;	// 변수 초기화
				isDriveSt = false;
				isReady = false;
				m_bReleaseEnd = true;	// 스톤 Throw(Release가 끝남)를 알리는 변수 set
				SendPacketThrowFlag(clientSock);	// Throw flag, server로 송신
				for (int i = 1; i < cnt; i++)		// Rio로 Throw flag 송신
					SendPacketRioThrowFlag(sock_base[i]);
				for (int i = 1; i < cnt; i++)	// Rio로 Drive start 종료 송신
					if (i != vs_sock)
					{
						SendPacketDrivestart(sock_base[i], 0, 0);
						Sleep(20);
						//SendPacketCRLF(sock_base[i]);
					}
				Sleep(5000);
				for (int i = 1; i < cnt; i++)		// Rio로 대기모드(5) 송신
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

	//case RIO_ENCODER:	// 추가 수정 171210

	//	RecvPacketRioencoder(msg, Rio_encoder, emp);
	//	m_fRioEncoder = Rio_encoder;

	//	AfxMessageBox(Rio_encoder, MB_ICONSTOP);

	//	if (m_fRioangle < m_fRioEncoder)
	//	{
	//		isRioRst = false;	// 변수 초기화
	//		isDriveSt = false;
	//		isReady = false;
	//		m_bReleaseEnd = true;	// 스톤 Throw(Release가 끝남)를 알리는 변수 set
	//		SendPacketThrowFlag(clientSock);	// Throw flag, server로 송신
	//		for (int i = 1; i < cnt; i++)
	//		{// Rio로 Throw flag 송신
	//			SendPacketRioThrowFlag(sock_base[i]);
	//			Sleep(20);
	//			//SendPacketCRLF(sock_base[i]);
	//		}
	//		for (int i = 1; i < cnt; i++)	// Rio로 Drive start 종료 송신
	//			if (i != vs_sock)
	//			{
	//				SendPacketDrivestart(sock_base[i], 0, 0);
	//				Sleep(20);
	//				//SendPacketCRLF(sock_base[i]);
	//			}
	//		Sleep(5000);
	//		for (int i = 1; i < cnt; i++)		// Rio로 대기모드(5) 송신
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


// Tx, Rio로 부터의 연결 종료 프로세스
void CTAB_YU::CloseProc(int index)
{
	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	getpeername(sock_base[index], (SOCKADDR *)&cliaddr, &len);
	//printf("[%s:%d] disconnect\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

	closesocket(sock_base[index]);	// 소켓 닫기
	WSACloseEvent(hev_base[index]);	// 이벤트 감지 종료

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

// 변수 표시용(업데이트) Timer
void CTAB_YU::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 4)
	{
		UpdateData(FALSE);
		/*** 파일 입출력용 ***/
		if (isThrower)
			GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Thrower"));
		else
			GetDlgItem(IDC_BTMODE)->SetWindowText(_T("Skip"));

	}
	CDialog::OnTimer(nIDEvent);
}

// Thrower <-> Skip 전환 테스트용 버튼
void CTAB_YU::OnBnClickedBtmode()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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

// 모드 송신 (Joystic 조작용)
void CTAB_YU::SendMode(float prep) {
	if (prep != 0) {	// 입력 모드가 0이 아니면 Rio로 모드 송신
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
	}	// 입력 모드가 0이면 Rio로 Drive 종료 신호 송신
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

// 주행 값 송신 (Joystic 조작용)
void CTAB_YU::TestMove(bool cst) {
	if (!cst) // 입력의 변경이 일어난 경우 Rio로 속도 프로파일 송신
		for (int i = 1; i<cnt; i++) {
			if (i != vs_sock) {
				SendPacketSpeedProf(sock_base[i], m_fTestAngle, m_fTestSpeed);
				Sleep(20);
				//SendPacketCRLF(sock_base[i]);
			}
		}
	for (int i = 1; i < cnt; i++)	// Rio로 Drive start 신호 송신
		if (i != vs_sock)
		{
			SendPacketDrivestart(sock_base[i], 1, 0);
			Sleep(20);
			//SendPacketCRLF(sock_base[i]);
		}
}

// 각도&위치 정보 요청 (Joystic 조작용)
void CTAB_YU::CallAngle() {
	for (int i = 1; i<cnt; i++)	// Tx로 각도&위치 정보 요청
		if (i == vs_sock || i == test_sock)
			SendPacketCallrobotinfo(sock_base[i], 0);
}

void CTAB_YU::SendAngle() {
	for (int i = 1; i<cnt; i++)	// Tx로 각도&위치 정보 요청
		if (i != vs_sock)
			SendPacketTargetangle(sock_base[i], 1.5, 0);
}

void CTAB_YU::SendCalib() {
	for (int i = 1; i<cnt; i++)	// Tx로 각도&위치 정보 요청
		if (i != vs_sock)
			SendPacketCalibAngle(sock_base[i], 1, 0);
}

void CTAB_YU::SendEmergency() {
	for (int i = 1; i<cnt; i++)	// Tx로 각도&위치 정보 요청
		if (i != vs_sock)
			SendPacketDrivestart(sock_base[i], 0, 0);


	for (int i = 1; i<cnt; i++)	// Tx로 각도&위치 정보 요청
		if (i == vs_sock || i == test_sock)
			SendPacketEMERGENCY(sock_base[i], 0, 0);
}