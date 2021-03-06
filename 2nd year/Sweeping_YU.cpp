// Sweeping_YU.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#include "Sweeping_YU.h"

float theta= 80;//다른 세타 하나더 만들어야 함
//theta is angle between y-axis and direction in which the laser sensor looks at the stone's center
float stone_radius = 0.1;
float robot_radius= 0.4;
float pi= 3.14159265358979323846;
float freq =1 / 30;
float w1 = 1;
float w2 = 0.25; 

string r = "C:\\Users\\jaehyeon\\Desktop\\컬링\\trajectory\\trajectory\\180201\\trial_001_18_02_01-10_NEAR.avi.txt";    // read txt -영상팀(v없는 것)trajectory test file
char kw[] = "C:\\Users\\jaehyeon\\Desktop\\어치\\system_model.txt";									  //write kalman system model
char tw[] = "C:\\Users\\jaehyeon\\Desktop\\어치\\system_model_000_18_02_07-10_NEAR.txt";        //write test
char pr[] = "1.txt";																			  // 주형이가 읽은 trajectory test file
char lr[] = "stone.txt";																		  //장용이 읽은 Laser value
																								  /*
																								  알고리즘만 돌아가고 주형이쪽 값에서 Dgist쪽으로 보내주면 되는거잖아
																								  그럼 YU_tab에 Thread 하나더 만들어서 코드 넣고 packet에 RIO에 각도랑 속도 보내는 패킷에 값넣으면 되는데 --> 주형이한테 물어보기
																								  */
																								  /*
																								  *test.cpp
																								  */
void main()//main문 // void main()으로 사용하여 테스트해보기 / void test()함수로도 사용 가능
{
	Prediction test_Pre; // Path prediction class object 생성
	//Laser test_Las; // Laser class object 생성
	Kalman test_kal_dyna;

	test_Pre.get_trajectory_plan();
	test_Pre.ideal_robot_trajectory(0, test_Pre.w);
	test_Pre.plan_distance_angle_two(0,test_Pre.w);
	//test_Pre.move_plan(0, test_Pre.i);
	test_Pre.get_real_stone_traj();
	//test_Pre.find_stone();

	int compare_index = 0;//아래 소스 실행을 위해....이것도 고쳐야함..
	while (1)
		//test_Pre.j_x_stone_plan[compare_index] != test_Pre.x_stone_real[compare_index] || test_Pre.j_y_stone_plan[compare_index] != test_Pre.y_stone_real[compare_index]
		//경로로 완전히 복귀했다고 해도..plan path를 기준으로 진동할 수 있으므로 1로 해도 될 듯
	{//실제로는 실시간 패킷으로 받은 친구들을 비교함
		if (compare_index == test_Pre.w)
		{
			break;
		}
		
	
		test_Pre.find_stone();
		test_Pre.velocity_cal(compare_index);
		test_Pre.real_formation(compare_index);
		test_Pre.ideal_formation(compare_index);

		test_Pre.path_prediction(compare_index);
		//
		//predict velocity?
		test_Pre.predict_formation(compare_index);
		test_Pre.plan_distance_angle(compare_index);//이때의 distance와
		//move(compare_index);//이때의 angle값을 넘겨주어야 함
		compare_index++;

		cout << "compare_index: " << compare_index << endl;
		//cout << "x_stone_predict:" << "  " << x_stone_predict[compare_index]
		//	<< "y_stone_predict:"<<y_stone_predict[compare_index] << endl;
	}
	//cout << compare_index << endl;
	//printf("%d",compare_index);
	//while문이랑 compare_index소스를 수정해야함. 실제상황에서 실시간으로 동일한 값으로 올 수 있기 때문에
	//현재 코드에서는 항상 다른값이 온다고 가정하고 반복문을 돌리는 것임



	//test_kal_dyna.kalmantest(test_Pre.stone_dist, test_Pre.stone_rad); //merge function _ kalman and dynamics 
}
/*
*PATH_Prediction.cpp
*/
void Prediction::get_trajectory_plan()
{
	//int num = 0;
	//int i = 1;
	FILE *stream;
	int file_state = 0;
	float x;
	float y;
	float v;
	fopen_s(&stream, pr, "r");

	if (stream == NULL)
	{
		printf("파일 열기 실패\n");
		file_state = fclose(stream);
		if (file_state == EOF)
			printf("파일 닫기 실패\n");
		//return 0;

	}
	while (fscanf_s(stream, "%f%f%f", &x, &y, &v) != EOF)
	{

		j_x_stone_plan[w] = x;
		j_y_stone_plan[w] = y;
		j_v_stone_plan[w] = v;
		if (j_x_stone_plan[w] == j_x_stone_plan[w - 1] || j_y_stone_plan[w] == j_y_stone_plan[w - 1])
		{
			break;
		}
		w++;
	}
	cout << w << endl;
	fclose(stream);
}

int Prediction::ideal_robot_trajectory(int c, int d)
{
	//int num=0;
	for (int j = c; j < d; j++)
	{
		x_robot_plan_1[j] = j_x_stone_plan[j] - (stone_radius + a + robot_radius)*sin(theta);
		//x_robot_plan_2[j] = x_stone_plan[j] - (stone_radius + b + robot_radius)*sin(theta);
		y_robot_plan_1[j] = j_y_stone_plan[j] + (stone_radius + a + robot_radius)*cos(theta);
		//y_robot_plan_2[j] = y_stone_plan[j] + (stone_radius + b + robot_radius)*cos(theta);
	}
	return 0;
}

int Prediction::plan_distance_angle_two(int a, int b)
{
	for (int j = a; j < b; j++)
	{
		distance_R1[j] = sqrt(pow((x_robot_plan_1[j + 1] - x_robot_plan_1[j]), 2.0) + pow((y_robot_plan_1[j + 1] - y_robot_plan_1[j]), 2.0));
		//distance_R2[j] = sqrt(pow((x_robot_plan_2[j + 1] - x_robot_plan_2[j]), 2.0) + pow((y_robot_plan_2[j + 1] - y_robot_plan_2[j]), 2.0));
		angle_R1[j] = atan((x_robot_plan_1[j + 1] - x_robot_plan_1[j]) / (y_robot_plan_1[j + 1] - y_robot_plan_1[j]));
		//angle_R2[j] = atan((x_robot_plan_2[j + 1] - x_robot_plan_2[j]) / (y_robot_plan_2[j + 1] - y_robot_plan_2[j]));
		//cout << "distance_R1:" << "  " << distance_R1[j] << "  " << "angle_R1:" << "  " << angle_R1[j] << endl;
		//cout << "distance_R2:" << "  " << distance_R2[j] << "  " << "angle_R2:" << "  " << angle_R2[j] << endl;
	}
	// atan((gag * pie)/180.0);
	return 0;
}

void Prediction::move_plan(int a, int b)
{
	for (int j = a; j < b; j++)
	{
		angle_R1[j] -= angle_R1[j - 1];
		printf("angle_R1: %lf", angle_R1[j]);
	}
}
void Prediction::get_real_stone_traj()
{
	int real_index = 0;
	FILE *stream;
	int file_state = 0;
	float x;
	float y;
	float v;
	fopen_s(&stream, "1.txt", "r");

	if (stream == NULL)
	{
		printf("파일 열기 실패\n");
		file_state = fclose(stream);
		if (file_state == EOF)
			printf("파일 닫기 실패\n");
		//return 0;

	}
	while (fscanf_s(stream, "%f%f%f", &x, &y, &v) != EOF)
	{
		//printf("%f ", x);
		//printf("%f ", y);
		//printf("%f \n", v);
		x_stone_real[real_index] = x;
		y_stone_real[real_index] = y;
		v_stone_real[real_index] = v;
		if (x_stone_real[real_index] == x_stone_real[real_index - 1] || y_stone_real[real_index] == y_stone_real[real_index - 1])//파일상 예외처리
		{
			break;
		}
		//	cout << "x_stone_real:" << "  " << x_stone_real[real_index] << "   " << "y_stone_real:" << "  " << y_stone_real[real_index] << "  " << "v_stone_real:" << "  " << v_stone_real[real_index] << endl;
		real_index++;
	}
	cout << real_index << endl;
	fclose(stream);
}






/*
*Laser.cpp
*/
double Prediction::find_stone()
{
	FILE *fp;
	fp = fopen(lr, "r");
	flag = 0;

	int yong = 0;
	while (!feof(fp))
	{


		fscanf(fp, "%d %lf %lf %lf", &data[yong].xy, &data[yong].rad, &data[yong].x, &data[yong].y);

		if (abs(data[check].xy - data[yong].xy) <= 100 && check != 0)// check!=0->밑의 조건문 만족했었고 
		{
			if (yong - check > 20 && abs(data[yong].xy - data[check].xy) <= 50 && data[yong].xy <= 1200)//20개정도 차이 나면 ,스톤은 특정 범위안에서 약간의 오차만 가지고 이동하니까 범위 밖은 빼버림
			{                                                        //왜 20개냐면 레이저센서 나온값에서 대부분 점찍히는게 15개~25개사이길래
				check_Count = yong - check;
				for (int i = 0; i < check_Count; ++i)
					Group[i] = data[check + i];
				//printf("찾음");
				//printf(" %d %lf\n", Group[check_Count / 2].xy, Group[check_Count / 2].rad);// 스톤 그룹의 중간값 거리와 rad각도
				


				//dist = Group[check_Count / 2].xy / 1000.0;
				//ang = Group[check_Count / 2].rad;
				cout << "ang: "<< ang <<"   "<< "dist: "<<dist << endl;
				check = 0;
				yong = 0;
				check_Count = 0;
			}
		}
		//printf("%d  %lf \n", Group[check].xy, Group[check].rad);
		if (abs(data[yong].xy - data[yong - 1].xy) >= 700 && data[yong].xy >= 200)//거리차이가 700보다 크면 70cm
		{
			if (abs(data[yong].xy - data[yong - 4].xy) >= 400 && abs(data[yong].xy - data[yong - 3].xy) >= 400)//거리차이가 계속 다른 4번째 뒤에 친구랑도 나면 //에러값 거르기
				check = yong;
		}
		yong++;
		flag = 1;
	}
	//for (int i = 0; i < check_Count; ++i)
	//printf(" %d %lf\n", Group[check_Count/2].xy,Group[check_Count/2].rad);
	fclose(fp);
	return 0;

}

void Prediction::velocity_cal(int index)
{
	cout << "x_stone_plan:" << "  " << j_x_stone_plan[index] << "   " //<< "x_robot_real_2:" << "  " << x_robot_real_2[j] << "  "
		<< "y_stone_plan:" << "  " << j_y_stone_plan[index] << endl; //<< "  " << "y_robot_real_2:" << "  " << y_robot_real_2[j] << endl;
	cout << "x_robot_plan:" << "  " << x_robot_plan_1[index] << "   " //<< "x_robot_real_2:" << "  " << x_robot_real_2[j] << "  "
		<< "y_robot_plan:" << "  " << y_robot_plan_1[index] << endl; //<< "  " << "y_robot_real_2:" << "  " << y_robot_real_2[j] << endl;
	cout << "x_stone_real:" << "  " << x_stone_real[index] << "   " //<< "x_robot_real_2:" << "  " << x_robot_real_2[j] << "  "
		<< "y_stone_real:" << "  " << y_stone_real[index] << endl; //<< "  " << "y_robot_real_2:" << "  " << y_robot_real_2[j] << endl;


	v_stone_real_y[index] = v_stone_real[index] * tan(ang);//stone의 움직이는 각도로 넣을것(angle)
																  //	index++;
	//cout << "v_stone_real_y: " << v_stone_real_y[index] << endl;
}
void Prediction::real_formation(int ju)
{
	x_robot_real_1[ju] = x_stone_real[ju] - sin(ang)*(stone_radius + dist + robot_radius);
	y_robot_real_1[ju] = y_stone_real[ju] + cos(ang)*(stone_radius + dist + robot_radius);

	//x_robot_predict_1[ju + 1] = x_robot_ideal_1[ju] * +;
}

int Prediction::ideal_formation(int c)
{
	//현재 스톤위치에서 바람직한 로봇의 formation 위치 계산
	int j = c;
	x_robot_ideal_1[j] = x_stone_real[j] - (stone_radius + a + robot_radius)*sin(theta);
	//x_robot_real_2[j] = x_stone_real[j] - (stone_radius + b + robot_radius)*sin(theta);
	y_robot_ideal_1[j] = y_stone_real[j] + (stone_radius + a + robot_radius)*cos(theta);
	//y_robot_real_2[j] = y_stone_real[j] + (stone_radius + b + robot_radius)*cos(theta);
	cout << "x_robot_real_1:" << "  " << x_robot_real_1[j] << "   " //<< "x_robot_real_2:" << "  " << x_robot_real_2[j] << "  "
		<< "y_robot_real_1:" << "  " << y_robot_real_1[j] << endl; //<< "  " << "y_robot_real_2:" << "  " << y_robot_real_2[j] << endl;
	cout << "x_robot_ideal_1:" << "  " << x_robot_ideal_1[j] << "   " //<< "x_robot_real_2:" << "  " << x_robot_real_2[j] << "  "
		<< "y_robot_ideal_1:" << "  " << y_robot_ideal_1[j] << endl; //<< "  " << "y_robot_real_2:" << "  " << y_robot_real_2[j] << endl;

	return 0;
}

int Prediction::path_prediction(int index)
{
	if (index < 5)//robot출발후 얼마안된 시간이므로 전략 경로 대로 갈것이라고 생각함
	{
		x_stone_predict[index] = j_x_stone_plan[index + 1];
		y_stone_predict[index] = j_y_stone_plan[index + 1];
		cout  << "x_stone_predict: " << x_stone_predict[index] << "  " << "y_stone_predict: " << y_stone_predict[index] << "  " << "가중치 고려 전  " << endl;
		return 0;
	}
	else
	{
		m1 = (x_stone_real[index - 2] - x_stone_real[index - 3]) / (y_stone_real[index - 2] - y_stone_real[index - 3]);
		m2 = (x_stone_real[index - 1] - x_stone_real[index - 2]) / (y_stone_real[index - 1] - y_stone_real[index - 2]);
		m3 = 2 * m2 - m1;//m2와 m1의 비율만큼 m3을 설정함
						 //m3 = (x_robot_plan_1[k] - x_robot_plan_1[k - 1]) / (y_robot_plan_1[k] - y_robot_plan_1[k - 1]);
		y_stone_predict[index] = y_stone_real[index - 1] + v_stone_real_y[index - 1] * freq;
		x_stone_predict[index] = w1 * x_stone_real[index - 1] + w2 * v_stone_real_y[index - 1] * freq / m3;
	

		cout<< "x_stone_predict: " << x_stone_predict[index] << "  " << "y_stone_predict: " << y_stone_predict[index] << "  " << "가중치 고려 전  " << endl;

		
		x_stone_predict[index] =0.7* x_stone_predict[index]+ 0.2*j_x_stone_plan[index+1]+ 0.1*x_stone_real[index];
		y_stone_predict[index]=0.7* y_stone_predict[index] + 0.2*j_y_stone_plan[index+1] + 0.1*y_stone_real[index];


		//y_robot_predict_1[index] = 0.5*x_robot_ideal_1[index] + 0.5*x_robot_real_1[index];
		//x_robot_predict_1[index] = 0.5*y_robot_ideal_1[index] + 0.5*y_robot_real_1[index];
	}
	//변위는 x가 비교적 일정한 반면 y값의 변화가 크므로 y값을 가지고 x값을 알아낸다.

	//m1=(x_robot_plan_1[index] - x_robot_plan_1[index - 1]) / (y_robot_plan_1[index] - y_robot_plan_1[index - 1]);

	cout << "x_stone_predict: " << x_stone_predict[index] << "  " << "y_stone_predict: " << y_stone_predict[index] << "  " << "가중치 고려 후  " << endl;
	 //robot2는 하지 않았음
	return 0;
}
int Prediction::predict_formation(int c)
{
	//현재 스톤위치에서 바람직한 로봇의 formation 위치 계산
	int j = c;
	x_robot_predict_1[j] = x_stone_predict[j] - (stone_radius + a + robot_radius)*sin(theta);
	//x_robot_real_2[j] = x_stone_real[j] - (stone_radius + b + robot_radius)*sin(theta);
	y_robot_predict_1[j] = y_stone_predict[j] + (stone_radius + a + robot_radius)*cos(theta);
	//y_robot_real_2[j] = y_stone_real[j] + (stone_radius + b + robot_radius)*cos(theta);
	
	cout << "x_robot_predict_1: " << x_robot_predict_1[j] << "  "//<< "   " << "x_robot_predict_2:" << "  " << x_robot_predict_2[index] << "  "
		<< "y_robot_predict_1: " << y_robot_predict_1[j] << endl;//<< "  " << "y_robot_predict_2:" << "  " << y_robot_predict_2[index] << endl;
	return 0;
}
void Prediction::move(int ju)
{
	angle_R1[ju] -= angle_R1[ju - 1];
	printf("angle_R1: %lf", angle_R1[ju]);
}

int Prediction::plan_distance_angle(int c)
{
	int j = c;
	distance_R1[j] = sqrt(pow((x_robot_predict_1[j] - x_robot_real_1[j]), 2.0) + pow((y_robot_predict_1[j] - y_robot_real_1[j]), 2.0));
	//distance_R2[j] = sqrt(pow((x_robot_predict_2[j + 1] - x_robot_real_2[j]), 2.0) + pow((y_robot_predict_2[j + 1] - y_robot_real_2[j]), 2.0));
	angle_R1[j] = atan((x_robot_predict_1[j] - x_robot_real_1[j]) / (y_robot_predict_1[j] - y_robot_real_1[j]));
	//angle_R2[j] = atan((x_robot_predict_2[j + 1] - x_robot_real_2[j]) / (y_robot_predict_2[j + 1] - y_robot_real_2[j]));
	cout << "distance_R1:" << "  " << distance_R1[j] << "  " << "angle_R1:" << "  " << angle_R1[j] << endl;
	//cout << "distance_R2:" << "  " << distance_R2[j] << "  " << "angle_R2:" << "  " << angle_R2[j] << endl;
	// atan((gag * pie)/180.0);
	return 0;
}
/*
int Prediction::robot_trajectory(int c)//좌표들이 변화가 없는것처럼 보이지만 단위가 맞지않아 변화되는 양이 매우 적기때문.
{
	int j = c;
	x_robot_real_1[j] = x_stone_real[j] + (stone_radius + a + robot_radius)*sin(theta);
	//x_robot_real_2[j] = x_stone_real[j] - (stone_radius + b + robot_radius)*sin(theta);
	y_robot_real_1[j] = y_stone_real[j] + (stone_radius + a + robot_radius)*cos(theta);
	//y_robot_real_2[j] = y_stone_real[j] + (stone_radius + b + robot_radius)*cos(theta);
	//cout << "x_robot_1:" << "  " << x_robot_1[j] << "   " << "x_robot_2:" << "  " << x_robot_2[j] << "  " << "y_robot_1:" << "  " << y_robot_1[j] << "  " << "y_robot_2:" << "  " << y_robot_2[j] << endl;
	//cout << num << endl;
	//num++;


	return 0;
}
*/






/*
*Dynamics.cpp
*/
/*
*Kalman.cpp
*/
void Kalman::kalmantest(double dist, double rad)
{
	Kalman test_Kal; // Kalman class object 생성

	Dynamics test_dy; //Dynamics class object 생성
	test_Kal.get_trajectory();

	/*file write*/
	int FLAG1;
	FILE *fp;
	fp = fopen(tw, "w"); // system model test 결과 text파일로 저장

	if (fp == NULL)
	{
		perror("\nFile open failed-write func");
		FLAG1 = 0;
	}

	/*Inisial set*/
	//float dt = 1/30.0; //영상 처리팀이 넘겨주는 데이터의 시간텀 or 데이터 넘어와서 걸리는 시간 -->term

	for (int k = 0; k < test_Kal.i; k++)//데이터 개수 만큼 포문 돌려서 시뮬레이션
	{
		//fprintf(fp, "test code : %d \n", i); // file opening test code
		test_Kal.kalman(test_Kal.x_stone_plan[k], test_Kal.y_stone_plan[k], test_Kal.stone_vel[k], test_Kal.dt, test_Kal.stone_vel[k + 1]);//kalman(double x, double y, double v, double dt, double z_v);


		printf("Ideal position x : %6.3f \n", test_Kal.x_stone_plan[k + 1]); // present position of real situation
		printf("Ideal position y : %6.3f \n", test_Kal.y_stone_plan[k + 1]);
		printf("Kalman position x : %6.3f [diff:%.3f]\n", test_Kal.pos_x, fabs(test_Kal.x_stone_plan[k + 1] - test_Kal.pos_x)); // present position of kalman
		printf("Kalman position y : %6.3f [diff:%.3f]\n", test_Kal.pos_y, fabs(test_Kal.y_stone_plan[k + 1] - test_Kal.pos_y));
		printf("error of x_position if using kalman filter: %f%%\n", (fabs(test_Kal.x_stone_plan[k + 1] - test_Kal.pos_x)) / fabs(test_Kal.x_stone_plan[k + 1]) * 100);
		printf("error of y_position if using kalman filter: %f%%\n", (fabs(test_Kal.y_stone_plan[k + 1] - test_Kal.pos_y)) / fabs(test_Kal.y_stone_plan[k + 1]) * 100);

		fprintf(fp, "Ideal position x : %6.3f \n", test_Kal.x_stone_plan[k + 1]);
		fprintf(fp, "Ideal position y : %6.3f \n", test_Kal.y_stone_plan[k + 1]);
		fprintf(fp, "Kalman position x : %6.3f [diff:%.3f]\n", test_Kal.pos_x, fabs(test_Kal.x_stone_plan[k + 1] - test_Kal.pos_x));
		fprintf(fp, "Kalman position y : %6.3f [diff:%.3f]\n", test_Kal.pos_y, fabs(test_Kal.y_stone_plan[k + 1] - test_Kal.pos_y));
		fprintf(fp, "error of x_position if using kalman filter: %f%%\n", (fabs(test_Kal.x_stone_plan[k + 1] - test_Kal.pos_x)) / fabs(test_Kal.x_stone_plan[k + 1]) * 100);
		fprintf(fp, "error of y_position if using kalman filter: %f%%\n", (fabs(test_Kal.y_stone_plan[k + 1] - test_Kal.pos_y)) / fabs(test_Kal.y_stone_plan[k + 1]) * 100);

		test_Kal.sum_error_kalman_x += fabs(test_Kal.x_stone_plan[k + 1] - test_Kal.pos_x); //error sum
		test_Kal.sum_error_kalman_y += fabs(test_Kal.y_stone_plan[k + 1] - test_Kal.pos_y);

		//printf("\n\n\n--------------------------------------------------------");
		//printf("%f   %f", dist, rad); // test _ dist and rad

		double angle = rad;// 스톤과 로봇사이의 각도
		double a = 0, b = 0, x_1 = 0, y_1 = 0, x_2 = 0, y_2 = 0;
		double z = 0; //지면과 축 사이의 높이 차이

		double delta_z, delta_r;
		double chang;//쁘사이
		double l1 = 3.5, l2 = 7, l3 = 7;// 링크 길이
		int distance = dist;// 스톤과 로봇사이의 거리

		a = test_Kal.pos_x + distance * sin((90 - angle)*pi / 180); // 스위핑 로봇의 x좌표 		
		b = test_Kal.pos_y + distance * cos((90 - angle)*pi / 180); // 스위핑 로봇의 y좌표 

		x_1 = test_Kal.pos_x + 8;	// 스위핑 지점x
		y_1 = test_Kal.pos_y + 1;	// 스위핑 지점y

		x_2 = a - x_1;  // 로봇에서 스톤까지 오차;
		y_2 = b - y_1;

		delta_z = z - l1;
		delta_r = sqrt((x_2*x_2) + (y_2*y_2)); //y_2

		test_dy.theta1 = atan2(x_2, y_2);
		test_dy.theta1_1 = (test_dy.theta1 * 180 / pi);//theta1 각도

		test_dy.theta3 = acos(((delta_z*delta_z) + (delta_r*delta_r) - (l2*l2) - (l3*l3)) / (2 * l2*l3));
		test_dy.theta3_1 = (test_dy.theta3 * 180 / pi); //theta3 각도

		chang = asin((l3*sin(pi - test_dy.theta3)) / sqrt((delta_z*delta_z) + (delta_r*delta_r))); //(180 - (180 - theta3)) / 2;

		if (test_dy.theta3 > 0)
		{
			test_dy.theta2 = pi / 2 - atan2(delta_z, delta_r) - chang;
			test_dy.theta2_1 = (test_dy.theta2 * 180 / pi);//theta2 각도
		}
		else if (test_dy.theta3 < 0)
		{
			test_dy.theta2 = pi / 2 - atan2(delta_z, delta_r) + chang;
			test_dy.theta2_1 = (test_dy.theta2 * 180 / pi);//theta2 각도
		}
		if (test_dy.theta3 > 90)
		{
			test_dy.theta4 = (test_dy.theta3 * 180 / pi) - 90;   // 540 - (theta2 * 180 / pi) - (180 - (theta3 * 180 / pi)) - 90 - 90;
																 //theta4_1 = 2048 - 11.375 * theta4;
		}
		else if (test_dy.theta3 <= 90)
		{
			test_dy.theta4 = 90 - (test_dy.theta3 * 180 / pi);   // 540 - (theta2 * 180 /pi) - (180 - (theta3 * 180 / pi)) - 90 - 90;
																 //theta4_1 = 2048 - 11.375 * theta4;
		}
		printf("theta1 = %lf , theta2 = %lf, theta3 = %lf, theta4 = %lf\n", test_dy.theta1_1, test_dy.theta2_1, test_dy.theta3_1, test_dy.theta4);



	}
	if (test_dy.minValY < test_Kal.pos_x < test_dy.maxValY && test_dy.minValX < test_Kal.pos_y < test_dy.maxValX)
	{
		if (test_Kal.stone_vel[test_Kal.j] < test_dy.robot_speed[test_Kal.j])
		{
			cout << "no sweeping" << endl;
		}
		else if (test_Kal.stone_vel[test_Kal.j] >= test_dy.robot_speed[test_Kal.j])
		{
			if (test_Kal.x_stone_plan[test_Kal.j] < test_Kal.pos_x && test_Kal.x_stone_plan[test_Kal.j] > test_Kal.pos_x)
			{
				cout << "sweeping" << endl;
				if (test_Kal.y_stone_plan[test_Kal.j] < test_Kal.pos_y && test_Kal.y_stone_plan[test_Kal.j] >test_Kal.pos_y)
				{
					cout << "sweeping" << endl;
				}
				else if (test_Kal.y_stone_plan[test_Kal.j] == test_Kal.pos_y)
				{
					cout << "no sweeping" << endl;
				}
			}
			else if (test_Kal.x_stone_plan[test_Kal.j] == test_Kal.pos_x)
			{
				cout << "no sweeping" << endl;
				if (test_Kal.y_stone_plan[test_Kal.j] < test_Kal.pos_y && test_Kal.y_stone_plan[test_Kal.j] > test_Kal.pos_y)
				{
					cout << "sweeping" << endl;
				}
				else if (test_Kal.y_stone_plan[test_Kal.j] == test_Kal.pos_y)
				{
					cout << "no sweeping" << endl;
				}
			}
		}
	}

	printf("Total error sum(no%% just diff) of x_position if using kalman filter: %f\n", test_Kal.sum_error_kalman_x);
	printf("Total error sum(no%% just diff) of y_position if using kalman filter: %f\n", test_Kal.sum_error_kalman_y);
	//printf("Total error sum(%%) of x_position if using kalman filter: %f\n", per_error_kal_x);
	//printf("Total error sum(%%) of y_position if using kalman filter: %f\n", per_error_kal_y);
	//printf("Reduction in error of x_position: %d%% \n", 100 - (int)((sum_error_kalman_x / sum_error_measure) * 100));
	//printf("Reduction in error of y_position: %d%% \n", 100 - (int)((sum_error_kalman_y / sum_error_measure) * 100));

	fprintf(fp, "Total error sum(no%% just diff) of x_position if using kalman filter: %f\n", test_Kal.sum_error_kalman_x);
	fprintf(fp, "Total error sum(no%% just diff) of y_position if using kalman filter: %f\n", test_Kal.sum_error_kalman_y);
	//fprintf(fp,"Total error sum(%%) of x_position if using kalman filter: %f\n", per_error_kal_x);
	//fprintf(fp,"Total error sum(%%) of y_position if using kalman filter: %f\n", per_error_kal_y);
	//fprintf(fp,"Reduction in error of x_position: %d%% \n", 100 - (int)((sum_error_kalman_x / sum_error_measure) * 100));
	//fprintf(fp,"Reduction in error of y_position: %d%% \n", 100 - (int)((sum_error_kalman_y / sum_error_measure) * 100)); 

	fclose(fp);
}
double Kalman::get_trajectory()
{
	std::ifstream ifs;
	std::vector<Point> points;

	ifs.open(r);

	if (!ifs) {   //파일이 제대로 열렸는지 체크
		cerr << "File open failed-read func";
		return 1;
	}
	while (ifs.good())
	{
		int frame_n, t, x, y;
		ifs >> frame_n >> t >> x >> y;

		points.push_back(Point(frame_n, t, x, y));

		if (x != -1 || y != -1)
		{
			x_stone_plan[i] = x;
			y_stone_plan[i] = y;
			//i++;
			if (x_stone_plan[i] != x_stone_plan[i - 1] || y_stone_plan[i] != y_stone_plan[i - 1])
			{
				i++;
			}

		}

	}
	for (int j = 0; j < i; j++)
	{
		double d = sqrt(pow((x_stone_plan[j + 1] - x_stone_plan[j]), 2.0) + pow(y_stone_plan[j + 1] - y_stone_plan[j], 2.0));
		stone_vel[j + 1] = d * term; //▲
									 //cout << x_stone_plan[j] << "  " << y_stone_plan[j] << "  " << endl;
	}
	//cout << i<<endl;
	/*for (int j = 0; j < i; j++)
	{
	cout << x_stone_plan[j] << "  " << y_stone_plan[j] << "  " << stone_vel[j] << endl;
	}*/
	ifs.close();
	return 0;
}
double Kalman::kalman(double x, double y, double v, double dt, double z_v)
{
	/*
	x, y : 현재(실시간)의 전 위치
	v    : 현재(실시간)의 전 위치
	dt   : 속도 받는 텀
	z_v  : 현재 속도
	*/
	Matrix A(3, 3, 1, 0, dt, 0, 1, dt, 0, 0, 1);//◈

	Matrix H(1, 3, 0, 0, 1, 0, 0, 0, 0, 0, 0);  //◈

	Matrix Q(3, 3, 1, 0, 0, 0.5, 0, 0, 0, 0, 3);//◈ // @가장 적절한 값
	Matrix R(1, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0);  //◈ // @가장 적절한 값

	Matrix X(1, 3, x, y, v, 0, 0, 0, 0, 0, 0);  //책이랑 transpose 차이

	double p = 0.0000000001;                    //◈ // @작을수록 좋음
	Matrix P(3, 3, p, 0, 0, 0, p, 0, 0, 0, p);  //◈
	Matrix Pp(3, 3);
	Matrix Xp(3, 1);


	if (flagF == 0)
	{
		/*file write*/
		FILE *kfp;
		kfp = fopen(kw, "w");

		if (kfp == NULL)
		{
			perror("\nFile open failed-write func");
		}

		fprintf(kfp, "A\n");
		A.save(kfp);
		fprintf(kfp, "\nH\n");
		H.save(kfp);
		fprintf(kfp, "\nQ\n");
		Q.save(kfp);
		fprintf(kfp, "\nR\n");
		R.save(kfp);
		fprintf(kfp, "\nP\n");
		P.save(kfp);

		fclose(kfp);
		flagF = 1;
	}

	/*1.추정값*/
	Xp = A * X.transpose();                             //cout << x << "  " << y << "  " << v<< endl;
														//Xp.show(); cout << endl;
	Pp = (A * P * A.transpose()) + Q;                   //Pp.show(); cout << endl;
														/*2.칼만이득 예측값*/
	Matrix K(3, 1);
	Matrix inv(1, 1);
	inv = (H * Pp * H.transpose()) + R;                 //inv.show(); cout << endl;
	K = Pp * H.transpose()*inv;                         //K.show(); cout << endl;

	Matrix Z(1, 1, z_v, 0, 0, 0, 0, 0, 0, 0, 0);//책이랑 transpose 차이

												/*3.추정값 계산*/
	Matrix XT(3, 1);
	XT = Xp + (K * (Z - (H*Xp)));                       //XT.show(); cout << endl;
	X = XT.transpose();                                 //X.show(); cout << endl;

														/*4.오차 공분산 계산*/
	P = Pp - (K * H * Pp);                              // P.show(); cout << endl;
														//X.show();
	pos_x = X.value(0, 0);
	pos_y = X.value(0, 1);
	vel = X.value(0, 2);
	// X.show(); //cout << endl;
	//cout << pos_x << "  " << pos_y << "  " << vel << endl;
	//cout << "------------------------------------------------------------" << endl;
	return 0;
}
/*
* matrix.cpp
*/
#include <stdexcept>
#define EPS 1e-10

using std::ostream;  using std::istream;  using std::endl;
using std::domain_error;

Matrix::Matrix(int rows, int cols, double a, double b, double c, double d, double e, double f, double g, double h, double i) : rows_(rows), cols_(cols) //initial value
{
	if ((b == 0) && (c == 0) && (d == 0) && (e == 0) && (f == 0) && (g == 0) && (h == 0) && (i == 0))
	{
		int rows = 1; int cols = 1;
		allocSpace();
		p[0][0] = a;
	}
	else if ((d == 0) && (e == 0) && (f == 0) && (g == 0) && (h == 0) && (i == 0))
	{
		int rows = 1; int cols = 3;
		allocSpace();
		p[0][0] = a; p[0][1] = b; p[0][2] = c;
	}
	else
	{
		int rows = 3; int cols = 3;
		allocSpace();
		p[0][0] = a; p[0][1] = b; p[0][2] = c;
		p[1][0] = d; p[1][1] = e; p[1][2] = f;
		p[2][0] = g; p[2][1] = h; p[2][2] = i;
	}
}
Matrix Matrix::show()
{
	for (int i = 0; i < rows_; i++)
	{
		for (int j = 0; j < cols_; j++)
		{
			printf("%f ", p[i][j]);
		}
		printf("\n");
	}
	return *this;
}
double Matrix::value(int i, int j)
{
	return p[i][j];

}
Matrix Matrix::save(FILE *fp)
{
	for (int i = 0; i < rows_; i++)
	{
		for (int j = 0; j < cols_; j++)
		{
			fprintf(fp, "%f ", p[i][j]);
		}
		fprintf(fp, "\n");
	}
	return *this;
}
/* PUBLIC MEMBER FUNCTIONS
********************************/

Matrix::Matrix(int rows, int cols) : rows_(rows), cols_(cols)
{
	allocSpace();
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] = 0;
		}
	}
}
Matrix::Matrix(double a[][3], int rows, int cols) : rows_(rows), cols_(cols)
{
	allocSpace();
	for (int i = 0; i < rows_; ++i)
	{
		for (int j = 0; j < cols_; ++j)
		{
			p[i][j] = a[i][j];
			//cout << p[i][j]<<endl;
		}
	}

}


Matrix::Matrix() : rows_(1), cols_(1)
{
	allocSpace();
	p[0][0] = 0;
}

Matrix::~Matrix()
{
	for (int i = 0; i < rows_; ++i) {
		delete[] p[i];
	}
	delete[] p;
}

Matrix::Matrix(const Matrix& m) : rows_(m.rows_), cols_(m.cols_)
{
	allocSpace();
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] = m.p[i][j];
		}
	}
}

Matrix& Matrix::operator=(const Matrix& m)
{
	if (this == &m) {
		return *this;
	}

	if (rows_ != m.rows_ || cols_ != m.cols_) {
		for (int i = 0; i < rows_; ++i) {
			delete[] p[i];
		}
		delete[] p;

		rows_ = m.rows_;
		cols_ = m.cols_;
		allocSpace();
	}

	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] = m.p[i][j];
		}
	}
	return *this;
}

Matrix& Matrix::operator+=(const Matrix& m)
{
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] += m.p[i][j];
		}
	}
	return *this;
}

Matrix& Matrix::operator-=(const Matrix& m)
{
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] -= m.p[i][j];
		}
	}
	return *this;
}

Matrix& Matrix::operator*=(const Matrix& m)
{
	Matrix temp(rows_, m.cols_);
	for (int i = 0; i < temp.rows_; ++i) {
		for (int j = 0; j < temp.cols_; ++j) {
			for (int k = 0; k < cols_; ++k) {
				temp.p[i][j] += (p[i][k] * m.p[k][j]);
			}
		}
	}
	return (*this = temp);
}

Matrix& Matrix::operator*=(double num)
{
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] *= num;
		}
	}
	return *this;
}

Matrix& Matrix::operator/=(double num)
{
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			p[i][j] /= num;
		}
	}
	return *this;
}
/*
Matrix Matrix::operator^(int num)
{
Matrix temp(*this);
return expHelper(temp, num);
}

void Matrix::swapRows(int r1, int r2)
{
double *temp = p[r1];
p[r1] = p[r2];
p[r2] = temp;
}
*/
Matrix Matrix::transpose()
{
	Matrix ret(cols_, rows_);
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < cols_; ++j) {
			ret.p[j][i] = p[i][j];
		}
	}
	/*for (int i = 0; i < rows_; ++i) {
	for (int j = 0; j < cols_; ++j) {
	cout << ret.p[i][j] << endl;
	}
	}*/
	return ret;
}

/* PRIVATE HELPER FUNCTIONS
********************************/

void Matrix::allocSpace()
{
	p = new double*[rows_];
	for (int i = 0; i < rows_; ++i) {
		p[i] = new double[cols_];
	}
}
/*
Matrix Matrix::expHelper(const Matrix& m, int num)
{
if (num == 0) {
return createIdentity(m.rows_);
} else if (num == 1) {
return m;
} else if (num % 2 == 0) {  // num is even
return expHelper(m * m, num/2);
} else {                    // num is odd
return m * expHelper(m * m, (num-1)/2);
}
}
*/
/* NON-MEMBER FUNCTIONS
********************************/

Matrix operator+(const Matrix& m1, const Matrix& m2)
{
	Matrix temp(m1);
	return (temp += m2);
}

Matrix operator-(const Matrix& m1, const Matrix& m2)
{
	Matrix temp(m1);
	return (temp -= m2);
}

Matrix operator*(const Matrix& m1, const Matrix& m2)
{
	Matrix temp(m1);
	return (temp *= m2);
}

Matrix operator*(const Matrix& m, double num)
{
	Matrix temp(m);
	return (temp *= num);
}

Matrix operator*(double num, const Matrix& m)
{
	return (m * num);
}

Matrix operator/(const Matrix& m, double num)
{
	Matrix temp(m);
	return (temp /= num);
}
