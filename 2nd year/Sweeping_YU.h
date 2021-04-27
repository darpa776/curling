#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <stdlib.h>
using namespace std;
/*
*PATH_Prediction.h
*/
class Prediction {
public:
	//double x_robot_1[2000] = { 0 };
	//double x_robot_2[2000] = { 0 };
	//double y_robot_1[2000] = { 0 };
	//double y_robot_2[2000] = { 0 };
	//int i = 1;//the number of trajectory point
	int i = 0;

	float a = 0.5;
	int w = 0;//a is distance between closer robot and stone, b is distance between farther robot and stone

	double m1;
	double m2;
	double m3;



	double distance_R1[4000] = { 0 };
	//double distance_R2[1000] = { 0 };
	double angle_R1[4000] = { 0 };
	//double angle_R2[1000] = { 0 };

	double j_x_stone_plan[4000] = { 0 };
	double j_y_stone_plan[4000] = { 0 };//This would be the parameter from stone trajectory
	double j_v_stone_plan[4000] = { 0 };
	double x_robot_plan_1[4000] = { 0 };
	// double x_robot_plan_2[2000] = { 0 };
	double y_robot_plan_1[4000] = { 0 };
	//double y_robot_plan_2[2000] = { 0 };

	double x_stone_predict[4000] = { 0 };
	double y_stone_predict[4000] = { 0 };
//	double v_stone_predict[4000] = { 0 };
	double x_robot_predict_1[4000] = { 0 };
	//double x_robot_predict_2[2000] = { 0 };
	double y_robot_predict_1[4000] = { 0 };
	//double y_robot_predict_2[2000] = { 0 };

	double x_stone_real[4000] = { 0 };
	double y_stone_real[4000] = { 0 };
	double v_stone_real[4000] = { 0 };
	double v_stone_real_y[4000] = { 0 };
	double x_robot_real_1[4000] = { 0 };
	//double x_robot_real_2[2000] = { 0 };
	double y_robot_real_1[4000] = { 0 };
	//double y_robot_real_2[2000] = { 0 };

	float x_robot_ideal_1[4000] = { 0 };
	float y_robot_ideal_1[4000] = { 0 };

	void get_trajectory_plan();
	void get_real_stone_traj();
	int robot_trajectory(int);
	int plan_distance_angle(int);
	int plan_distance_angle_two(int, int);
	void velocity_cal(int);
	int path_prediction(int);
	int ideal_robot_trajectory(int, int);
	void move_plan(int, int);
	void real_formation(int);
	int ideal_formation(int);
	void move(int);
	int predict_formation(int);

	/////////////////////////////////용규/////////////////////////////////////////////////
	typedef struct sensor {
		double x, y;
		double rad;
		int xy;//거리
	}sensor;
	sensor data[1000];
	sensor Group[100];


	double dist=0.5;
	double ang=80;

	int sum = 0;
	int check = 0;
	int check_Count = 0;
	int checkmid = 0;
	int flag = 1;

	double stone_dist = 0, stone_rad = 0;
	double find_stone();

};
/*
*Laser.h
*/
/*
*Dynamics.h
*/
class Dynamics {
public:
	double minValX = 99999, minValY = 99999;
	double robot_speed[2000] = { 0 };// vel = 0; //로봇 속도
	double maxValX = 0, maxValY = 0;
	///////////////////다이나믹스///////////////////
	double theta1, theta2, theta3, theta4, theta1_1, theta2_1, theta3_1, theta4_1, theta4_2;
};
/*
*Kalman.h
*/
#define term 1.0/15.0 
class Kalman {
public:
	double dt = term;
	int i = 0, j = 0;//the number of trajectory point
	double pos_x = 0, pos_y = 0, vel = 0;;//This is the parameter of kalman
	int flagF = 0;
	float sum_error_kalman_x = 0, sum_error_kalman_y = 0;// error_sum of kalman

														 /*for test*/
	double x_stone_plan[2000] = { 0 };//This would be the parameter from stone trajectory
	double y_stone_plan[2000] = { 0 };
	double stone_vel[2000] = { 0 };

	struct Point
	{
		float frame_n, t, x, y;   // frame_n  프레임수, t = 실제시간, x = x좌표, y = y좌표
								  // 원점 : 백라인 중심, x좌표: 오른쪽 +, 단위 mm
								  // 1프레임 = 15hz
		Point(double _frame_n, double _t, double _x, double _y) { frame_n = _frame_n; t = _t; x = _x; y = _y; };
	};

	double get_trajectory();
	double kalman(double, double, double, double, double);
	void kalmantest(double, double);
};
/*
* matrix.h
*/
#ifndef __MATRIX_H__
#define __MATRIX_H__

class Matrix {
public:
	Matrix(int, int);
	Matrix(int, int, double, double, double, double, double, double, double, double, double);
	Matrix(double[][3], int, int);
	Matrix();
	~Matrix();
	Matrix(const Matrix&);
	Matrix& operator=(const Matrix&);
	inline double& operator()(int x, int y) { return p[x][y]; }


	Matrix& operator+=(const Matrix&);
	Matrix& operator-=(const Matrix&);
	Matrix& operator*=(const Matrix&);
	Matrix& operator*=(double);
	Matrix& operator/=(double);
	//Matrix  operator^(int);

	//friend std::ostream& operator<<(std::ostream&, const Matrix&);
	//friend std::istream& operator>>(std::istream&, Matrix&);


	//void swapRows(int, int);
	Matrix transpose();
	Matrix show();
	double value(int, int);
	Matrix save(FILE *fp);

	/*
	static Matrix createIdentity(int);
	static Matrix solve(Matrix, Matrix);
	static Matrix bandSolve(Matrix, Matrix, int);

	// functions on vectors
	static double dotProduct(Matrix, Matrix);

	// functions on augmented matrices
	static Matrix augment(Matrix, Matrix);
	Matrix gaussianEliminate();
	Matrix rowReduceFromGaussian();
	void readSolutionsFromRREF(std::ostream& os);
	Matrix inverse();*/

private:
	int rows_, cols_;
	double **p;

	void allocSpace();
	//Matrix expHelper(const Matrix&, int);
};

Matrix operator+(const Matrix&, const Matrix&);
Matrix operator-(const Matrix&, const Matrix&);
Matrix operator*(const Matrix&, const Matrix&);
Matrix operator*(const Matrix&, double);
Matrix operator*(double, const Matrix&);
Matrix operator/(const Matrix&, double);

#endif
