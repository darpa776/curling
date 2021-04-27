//#include "stdafx.h"
#include "Packet.h"

// 패킷은 앞에 2바이트를 헤더(0xAA, 0x00)으로 사용합니다.
// 헤더 뒤에 1바이트를 패킷의 종류를 구분하는 용도로 사용합니다.
// 이 뒤에 데이터에 따라 가변적으로 패킷의 길이를 설정합니다.
// Rio로 보내는 패킷의 편의상 float형 2개의 고정 길이로 사용합니다.

void SendPacketStoneCnt(SOCKET s, int cnt, Point* posArr, bool* isRedArr) {
	char* data = new char[3 + 1];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : STONE_CNT
	data[idx++] = STONE_CNT;

	// Set Data : Stone cnt
	data[idx++] = (char)cnt;

	send(s, data, idx, 0);

	for (int i = 0; i < cnt; i++) {
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
		SendPacketStoneInfo(s, posArr[i], isRedArr[i]);
	}

	delete data;
}

void SendPacketStoneInfo(SOCKET s, Point pos, bool isRed) {
	char* data = new char[3 + sizeof(pos) + sizeof(isRed)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : STONE_INFO
	data[idx++] = STONE_INFO;

	// Set Data : Stone Info
	char * temp = (char*)&pos.x;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];
	temp = (char*)&pos.y;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	data[idx++] = isRed;

	send(s, data, idx, 0);

	delete data;
}

void SendPacketRobotInfo(SOCKET s, float angle, Point pos, float hogDist, float hogOffs) {
	unsigned char checkSum = 0;
	char* data = new char[3 + sizeof(angle) + sizeof(pos) + sizeof(hogDist) + sizeof(hogOffs) + sizeof(checkSum)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : ROBOT_INFO
	data[idx++] = ROBOT_INFO;

	// Set Data : Robot Info
	char * temp = (char*)&angle;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&pos.x;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];
	temp = (char*)&pos.y;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&hogDist;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&hogOffs;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	for (int l = 0; l < idx; l++)
		checkSum |= data[l];

	data[idx++] = checkSum;

	send(s, data, idx, 0);

	delete data;
}

void SendPacketStoneInfoAck(SOCKET s, char type) {
	char data[4];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : FLAG
	data[idx++] = STONE_INFO_ACK;

	// 0:premode,	1: robot info,	2 : call stone info,	3: stone info,	4: release
	data[idx++] = type;

	send(s, data, idx, 0);
}

void SendPacketRelease(SOCKET s, float speed, float angle, bool isRight, Point pos, Point tar_pos) {
	char* data = new char[3 + sizeof(speed) + sizeof(angle) + sizeof(isRight) + sizeof(pos) + sizeof(tar_pos)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RELEASE
	data[idx++] = RELEASE;

	// Set Data : speed, angle, isRight, position, target
	char * temp = (char*)&speed;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&angle;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	data[idx++] = isRight;

	temp = (char*)&pos.x;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];
	temp = (char*)&pos.y;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&tar_pos.x;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];
	temp = (char*)&tar_pos.y;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketThrowFlag(SOCKET s) {
	char data[3];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : FLAG
	data[idx++] = FLAG;

	send(s, data, idx, 0);
}

void SendPacketRioThrowFlag(SOCKET s) {
	float emp = 0;
	char* data = new char[3 + sizeof(emp) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : FLAG
	data[idx++] = FLAG;

	// Set Data : emp;
	char * temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}


void SendPacketSpeedProf(SOCKET s, float angle, float speed) {
	char* data = new char[3 + sizeof(angle) + sizeof(speed)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : SPEED_PROF
	data[idx++] = SPEED_PROF;

	// Set Data : Angle & Speed Profil
	char * temp = (char*)&angle;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&speed;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketPositionProf(SOCKET s, float posx, float posy) {
	char* data = new char[3 + sizeof(posx) + sizeof(posy)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : ACCEL_PROF
	data[idx++] = POS_PROF;

	// Set Data : pos
	char * temp = (char*)&posx;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];
	temp = (char*)&posy;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}


// [DGIST > 영남대] RIO Reset
void SendPacketRioreset(SOCKET s, float reset, float emp)
{
	char* data = new char[3 + sizeof(reset) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RESET
	data[idx++] = RESET;

	// Set Data : reseet
	char * temp = (char*)&reset;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

// [DGIST > 영남대] 주행모드
void SendPacketRunningmode(SOCKET s, float mode, float emp)
{
	char* data = new char[3 + sizeof(mode) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : MODE
	data[idx++] = MODE;

	// Set Data : mode
	char * temp = (char*)&mode;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

// [DGIST > 영남대] 주행준비
void SendPacketpredrive(SOCKET s, float Preparation, float emp)
{
	char* data = new char[3 + sizeof(Preparation) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RUN
	data[idx++] = RUN;

	// Set Data : Preparration
	char * temp = (char*)&Preparation;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

// [DGIST > 영남대] 주행시작
void SendPacketDrivestart(SOCKET s, float start, float emp)
{
	char* data = new char[3 + sizeof(start) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : START
	data[idx++] = START;

	// Set Data : strat
	char * temp = (char*)&start;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketKUReset(SOCKET s, float reset, float emp)
{
	char* data = new char[3 + sizeof(reset) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RESET_KU
	data[idx++] = RESET_KU;

	// Set Data : reset
	char * temp = (char*)&reset;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketThrowmode(SOCKET s, float number)
{
	char* data = new char[3 + sizeof(number)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : ROBOT_MODE
	data[idx++] = ROBOT_MODE;

	// Set Data : Number
	char * temp = (char*)&number;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketPremode(SOCKET s, float pre, float number)
{
	char* data = new char[3 + sizeof(pre) + sizeof(number)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : PRE_MODE
	data[idx++] = PRE_MODE;

	// Set Data : pre
	char * temp = (char*)&pre;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	temp = (char*)&number;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketMyturn(SOCKET s, bool turn)
{
	char* data = new char[3 + sizeof(turn)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : MY_TURN
	data[idx++] = MY_TURN;

	// Set Data : turn
	data[idx++] = turn;

	send(s, data, idx, 0);

	delete data;
}

void SendPacketCallstoneinfo(SOCKET s) {
	char data[3];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : CALL_STONE_INFO
	data[idx++] = CALL_STONE_INFO;

	send(s, data, idx, 0);
}

void SendPacketCallrobotinfo(SOCKET s, bool isDist) {
	char data[3 + sizeof(isDist)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : CALL_ROBOT_INFO
	data[idx++] = CALL_ROBOT_INFO;

	// Set Data : isDist
	data[idx++] = isDist;

	send(s, data, idx, 0);
}

void SendPacketEMERGENCY(SOCKET s, float emp1, float emp2) {
	char data[3 + sizeof(emp1) + sizeof(emp2)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : EMERGENCY
	data[idx++] = EMERGENCY;

	// Set Data : emp1
	char * temp = (char*)&emp1;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	// Set Data : emp2
	temp = (char*)&emp2;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);
}

void SendPacketRestart(SOCKET s, float emp1, float emp2) {
	char data[3 + sizeof(emp1) + sizeof(emp2)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RESTART
	data[idx++] = RESTART;

	// Set Data : emp1
	char * temp = (char*)&emp1;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	// Set Data : emp2
	temp = (char*)&emp2;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);
}

void SendPacketCalibdata(SOCKET s) {
	char data[3];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : CALIB_DATA
	data[idx++] = CALIB_DATA;

	send(s, data, idx, 0);
}

void SendPacketTargetangle(SOCKET s, float t_angle, float emp)
{
	char* data = new char[3 + sizeof(t_angle) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : ROBOT_MODE
	data[idx++] = TARGET_ANGLE;

	// Set Data : Number
	char * temp = (char*)&t_angle;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	// Set Data : emp
	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketCalibAngle(SOCKET s, float d1, float d2)
{
	char* data = new char[3 + sizeof(d1) + sizeof(d2)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : CALIB_ANGLE
	data[idx++] = CALIB_ANGLE;

	// Set Data : d1
	char * temp = (char*)&d1;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	// Set Data : d2
	temp = (char*)&d2;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketHogdistance(SOCKET s, float Hog_dist)
{
	char* data = new char[3 + sizeof(Hog_dist)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : HOG_DIST
	data[idx++] = HOG_DIST;

	// Set Data : Hog_dist
	char * temp = (char*)&Hog_dist;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}

void SendPacketRioencoder(SOCKET s, float Rio_encoder, float emp)	// 추가 수정 171210
{
	char* data = new char[3 + sizeof(Rio_encoder) + sizeof(emp)];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : RIO_ENCODER
	data[idx++] = RIO_ENCODER;

	// Set Data : Rio_encoder
	char * temp = (char*)&Rio_encoder;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];

	// Set Data : emp
	temp = (char*)&emp;
	data[idx++] = temp[3];
	data[idx++] = temp[2];
	data[idx++] = temp[1];
	data[idx++] = temp[0];
	send(s, data, idx, 0);

	delete data;
}

void SendPacketCRLF(SOCKET s)
{
	char * data = new char[2];
	int idx = 0;

	data[idx++] = '\r';
	data[idx++] = '\n';

	send(s, data, idx, 0);

	delete data;
}

// info time
void SendPacketInfoTime(SOCKET s, char * msg) {
	char data[256];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : INFO_TIME
	data[idx++] = INFO_TIME;

	// Set Data : data
	for (int i = 0; data[idx++] = msg[i]; i++);

	data[idx++] = '\0';

	send(s, data, idx, 0);
}

// info result
void SendPacketInfoResult(SOCKET s, float release, float arrive, Point pos) {
	char* data = new char[3 + sizeof(release) + sizeof(arrive) + sizeof(pos) + 3];
	int idx = 0;

	// Header : 0xAA, 0x00 설정
	data[idx++] = 0xAA;
	data[idx++] = 0x00;

	// Packet Type : INFO_RESULT
	data[idx++] = INFO_RESULT;

	// Set Data : release
	char * temp = (char*)&release;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	data[idx++] = '%';

	temp = (char*)&arrive;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	data[idx++] = '%';

	temp = (char*)&pos.x;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	data[idx++] = '%';

	temp = (char*)&pos.y;
	data[idx++] = temp[0];
	data[idx++] = temp[1];
	data[idx++] = temp[2];
	data[idx++] = temp[3];

	send(s, data, idx, 0);

	delete data;
}

int RecvClassify(char* msg) {
	if ((unsigned char)msg[0] != 0xAA || (unsigned char)msg[1] != 0x00)
		return -1;
	if (msg[2] < STONE_CNT)
		return -1;
	return msg[2];
}

void RecvPacketStoneCnt(char* msg, int &cnt) {
	cnt = msg[3];
}

int RecvPacketStoneInfo(char* msg, Point &pos, bool &isRed) {
	PacketCvt tmp;
	int idx = 3;
	unsigned char checkSum = 0;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.x = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.y = tmp.f;

	isRed = msg[idx++];

	for (int l = 0; l < idx; l++)
		checkSum |= msg[l];

	if (checkSum == msg[idx])
		return -1;
	else
		return 0;
}

void RecvPacketRobotInfo(char* msg, float &angle, Point &pos, float &hogDist, float &hogOff) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	angle = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.x = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.y = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	hogDist = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	hogOff = tmp.f;
}

void RecvPacketRelease(char* msg, float &speed, float &angle, bool &isRight, Point &pos, Point &tar_pos) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	speed = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	angle = tmp.f;

	isRight = msg[idx++];

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.x = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.y = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	tar_pos.x = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	tar_pos.y = tmp.f;
}

bool isThrowFlag(char* msg) {
	return (msg[2] == FLAG) ? true : false;
}

void RecvPacketSpeedProf(char* msg, float &angle, float &speed) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	angle = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	speed = tmp.f;
}

void RecvPacketAccelProf(char* msg, Point &pos, float &accel) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.x = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.y = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	accel = tmp.f;
}

//전략에서 받을 Reset
void RecvPacketRioReset(char* msg, float &reset, float &emp) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	reset = tmp.f;
	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	emp = tmp.f;
}

void RecvPacketThrowmode(char* msg, float &mode) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	mode = tmp.f;
}

void RecvPacketPremode(char* msg, float &pre, float &mode) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pre = tmp.f;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	mode = tmp.f;
}

void RecvPacketMyturn(char* msg, bool &turn) {
	turn = msg[3];
}

bool RecvPacketCallstoneinfo(char* msg) {
	return (msg[2] == CALL_STONE_INFO) ? true : false;
}

bool RecvPacketCallrobotinfo(char* msg) {
	return (msg[2] == CALL_ROBOT_INFO) ? true : false;
}

bool RecvPacketEmergency(char* msg) {
	return (msg[2] == EMERGENCY) ? true : false;
}

bool RecvPacketRestart(char* msg) {
	return (msg[2] == RESTART) ? true : false;
}

bool RecvPacketCalibdata(char* msg) {
	return (msg[2] == CALIB_DATA) ? true : false;
}

void RecvPacketHogdistance(char* msg, float &Hog_dist) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	Hog_dist = tmp.f;

}

void RecvPacketRioencoder(char* msg, double &Rio_encoder, double &emp)	// 추가 수정 171210
{
	PacketCvtd tmp;
	int idx = 3;

	for (int i = 0; i < 8; i++)
		tmp.ch[7 - i] = msg[idx++];
	Rio_encoder = tmp.d;

}

// info time
char *RecvPacketInfoTime(char* msg) {
	return &msg[3];
}

// info result
void RecvPacketInfoResult(char* msg, float &release, float &arrive, Point &pos) {
	PacketCvt tmp;
	int idx = 3;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	release = tmp.f;

	idx++;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	arrive = tmp.f;

	idx++;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.x = tmp.f;

	idx++;

	for (int i = 0; i < 4; i++)
		tmp.ch[i] = msg[idx++];
	pos.y = tmp.f;
}


// 패킷응답
char RecvPacketAcK(char* msg) {
	if (msg[3] == STONE_INFO_ACK)
		return msg[4];
	return -1;
}