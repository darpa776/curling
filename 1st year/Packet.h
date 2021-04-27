#pragma once
#include "common.h"

// char[4] => float 변환용 union
union PacketCvt {
	char ch[4];
	float f;
};
union PacketCvtd {
	char ch[8];
	double d;
};

// 위치 정보를 저장할 Point 구조체
class Point {
public:
	float x;
	float y;

	Point(float x = 0, float y = 0) {
		this->x = x;
		this->y = y;
	}
};

// 패킷 3번째 바이트에 들어가 패킷의 종류를 구분할 24(0~23)개의 enum
enum {
	STONE_CNT, STONE_INFO, STONE_INFO_ACK, ROBOT_INFO, RELEASE, FLAG, SPEED_PROF, POS_PROF, RESET, MODE,
	RUN, RESET_KU, START, ROBOT_MODE, PRE_MODE, MY_TURN, CALL_STONE_INFO, CALL_ROBOT_INFO, EMERGENCY, RESTART,
	CALIB_DATA, TARGET_ANGLE, CALIB_ANGLE, HOG_DIST, RIO_ENCODER, INFO_TIME, INFO_RESULT
};

// 받은 msg의 Type을 반환
int RecvClassify(char* msg);

// SendFunc : 전송할 값들을 매개변수로 사용

// [고대(비젼) > 고대(전략)] 스톤 갯수
void SendPacketStoneCnt(SOCKET s, int cnt, Point* p, bool* isRedArr);

// [고대(비젼) > 고대(전략)]스톤의 위치
void SendPacketStoneInfo(SOCKET s, Point pos, bool isRed);

// [고대(비젼) > DGIST] 투구 중 로봇 각도 위치
void SendPacketRobotInfo(SOCKET s, float angle, Point pos, float hogDist, float hogOffs);

// [고대(전략) > 고대(비젼)] 스톤 정보 응답 (0:premode,	1:robot info,	2:call stone info,	3:stone info,	4:release)
void SendPacketStoneInfoAck(SOCKET s, char type);

// [고대(전략) > 영남대] 릴리즈 속력, 각도, 컬방향, (위치, 목표 지점)
void SendPacketRelease(SOCKET s, float speed, float angle, bool isRight, Point pos, Point tar_pos);

// [영남대 > 고대(비전)] 투구 플래그
void SendPacketThrowFlag(SOCKET s);

// [영남대 > DGIST] 투구 플래그
void SendPacketRioThrowFlag(SOCKET s);

// [영남대 > DGIST] 투구 중 로봇 각도, 속도 프로파일
void SendPacketSpeedProf(SOCKET s, float angle, float Speed);

// [영남대 > DGIST] 위치 프로파일
void SendPacketPositionProf(SOCKET s, float posx, float posy);

// [영남대 > DGIST] RIO Reset
void SendPacketRioreset(SOCKET s, float reset, float emp);

// [영남대 > DGIST] 주행모드
void SendPacketRunningmode(SOCKET s, float mode, float emp);

// [영남대 > DGIST] 주행준비
void SendPacketpredrive(SOCKET s, float Preparation, float emp);

// [영남대 > DGIST] 주행시작
void SendPacketDrivestart(SOCKET s, float start, float emp);

// [영남대 > DGIST] KU_RESET
void SendPacketKUReset(SOCKET s, float reset, float emp);

// [고대(전략) > 영남대] 투구로봇 ID
void SendPacketThrowmode(SOCKET s, float number);

// [고대(전략) > 영남대] 연습모드
void SendPacketPremode(SOCKET s, float pre, float number);

// [고대(전략) > 영남대] 로봇 투구 차례
void SendPacketMyturn(SOCKET s, bool turn);

// [고대(전략) > 고대(비젼)] 스톤 정보 호출
void SendPacketCallstoneinfo(SOCKET s);

// [영남대 > 고대(비젼)] 로봇 정보 호출
void SendPacketCallrobotinfo(SOCKET s, bool isDist);

// [고대(전략) > 영남대] 긴급 정지
void SendPacketEMERGENCY(SOCKET s, float emp1, float emp2);

// [고대(전략) > 영남대] 재시작
void SendPacketRestart(SOCKET s, float emp1, float emp2);

// [고대(전략) > 영남대] 캘리브레이션
void SendPacketCalibdata(SOCKET s);

// [영남대 > DIGST] 목표 각도 전송
void SendPacketTargetangle(SOCKET s, float t_angle, float emp);

// [영남대 > DIGST] 각도 보정 전송
void SendPacketCalibAngle(SOCKET s, float d1, float d2);

// [영남대 > 고대(전략)] 투구되는 호그라인 거리 전송
void SendPacketHogdistance(SOCKET s, float Hog_dist);

// [DGIST > 영남대] 투구되는 호그라인 거리 전송	// 추가 수정 171210
void SendPacketRioencoder(SOCKET s, float Rio_encoder, float emp);

void SendPacketCRLF(SOCKET s);

// [전략 > 영상]info time
void SendPacketInfoTime(SOCKET s, char * msg);

// [영상 > 전략]info result
void SendPacketInfoResult(SOCKET s, float release, float arrive, Point pos);

// RecvFunc : 값을 입력받을 변수를 매개변수로 사용

// [고대(비젼) > 고대(전략)] 스톤 갯수
void RecvPacketStoneCnt(char* msg, int &cnt);

// [고대(비젼) > 고대(전략)] 스톤의 위치
int RecvPacketStoneInfo(char* msg, Point &pos, bool &isRed);

// [고대(비젼) > DGIST] 투구 중 로봇 각도 위치
void RecvPacketRobotInfo(char* msg, float &angle, Point &pos, float &hogDist, float &hogOffs);

// [고대(전략) > 영남대] 릴리즈 속력, 각도, 컬방향, (위치, 목표 지점)
void RecvPacketRelease(char* msg, float &speed, float &angle, bool &isRight, Point &pos, Point &tar_pos);

// [영남대 > 고대(비젼)] 지금 받은 packet이 투구 플래그인지 반환
bool isThrowFlag(char* msg);

// [영남대 > DGIST] 투구 중 로봇 각도, 속도 프로파일
void RecvPacketSpeedProf(char* msg, float &angle, float &speed);

// [DGIST > 영남대] 각속도 프로파일
void RecvPacketPositionProf(char* msg, float &posx, float &posy);

// [고대(전략) > 영남대] RESET_KU
void RecvPacketRioReset(char* msg, float &reset, float &emp);

// [고대(전략) > 영남대] 투구로봇 ID 전송
void RecvPacketThrowmode(char* msg, float &mode);

// [고대(전략) > 영남대] 연습모드, 경기모드 선택
void RecvPacketPremode(char* msg, float &pre, float &mode);

// [고대(전략) > 영남대] 로봇 투구 차례인지 선택
void RecvPacketMyturn(char* msg, bool &turn);

// [고대(전략) > 고대(비젼)] 스톤 정보 요청
bool RecvPacketCallstoneinfo(char* msg);

// [영남대 > 고대(비젼)] 로봇 정보 요청
bool RecvPacketCallrobotinfo(char* msg);

// [고대(전략) > 영남대] 비상 정지
bool RecvPacketEmergency(char* msg);

// [고대(전략) > 영남대] 재시작
bool RecvPacketRestart(char* msg);

// [고대(전략) > 영남대] Calibration data로 사용
bool RecvPacketCalibdata(char* msg);

// [영남대 > 고대(전략)] 투구되는 호그라인 거리 전송
void RecvPacketHogdistance(char* msg, float &Hog_dist);

// [DGIST > 영남대] 투구되는 호그라인 거리 전송	// 추가 수정 171210
void RecvPacketRioencoder(char* msg, double &Rio_encoder, double &emp);

// info time
char *RecvPacketInfoTime(char* msg);

// info result
void RecvPacketInfoResult(char* msg, float &release, float &arrive, Point &pos);

// 패킷응답
char RecvPacketAcK(char* msg);