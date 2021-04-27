#include "stdafx.h"
#include <iostream>  
#include <windows.h>
#include <opencv2/highgui.hpp>  
#include <opencv2/imgproc.hpp> 
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define _CRT_SECURE_NO_WARNINGS

using namespace cv;
using namespace std;

int count_mouse_click = 0;
double pointX[4], pointY[4];
int caculate_start = 0;
Mat img_gray;

//https://helloacm.com/cc-function-to-compute-the-bilinear-interpolation/
float BilinearInterpolation(float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2, float x, float y)
{
	float x2x1, y2y1, x2x, y2y, yy1, xx1;
	x2x1 = x2 - x1;
	y2y1 = y2 - y1;
	x2x = x2 - x;
	y2y = y2 - y;
	yy1 = y - y1;
	xx1 = x - x1;
	return 1.0 / (x2x1 * y2y1) * (
		q11 * x2x * y2y +
		q21 * xx1 * y2y +
		q12 * x2x * yy1 +
		q22 * xx1 * yy1
		);
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		cout << count_mouse_click << "번째 왼쪽 마우스 버튼 클릭.. 좌표 = (" << x << ", " << y << ")" << endl;
		pointX[count_mouse_click] = x;
		pointY[count_mouse_click] = y;

		count_mouse_click++;
	}

	if (count_mouse_click == 4 && caculate_start == 0)
	{
		caculate_start = 1;

		cout << "#######################################################" << endl;
		cout << "H계산에 필요한 4개의 점을 모두 클릭했습니다." << endl << endl;


		double width = ((pointX[1] - pointX[0]) + (pointX[3] - pointX[2]))*0.5;
		double height = ((pointY[2] - pointY[0]) + (pointY[3] - pointY[1]))*0.5;

		double newpointX[4] = { pointX[3] - width, pointX[3], pointX[3] - width, pointX[3] };
		double newpointY[4] = { pointY[3] - height, pointY[3] - height, pointY[3], pointY[3] };

		for (int i = 0; i < 4; i++)
			cout << newpointX[i] << " " << newpointY[i] << endl;

		//inverse mapping
		Mat img_result = Mat::zeros(img_gray.size(), CV_8UC1);

		rectangle(img_result, Point(newpointX[0], newpointY[0]), Point(newpointX[3], newpointY[3]), Scalar(255), 1);
		imshow("img_gray image2", img_result);


		vector<Point2f> pts_src;
		vector<Point2f> pts_dst;

		for (int i = 0; i < 4; i++) {
			pts_src.push_back(Point2f(pointX[i], pointY[i]));
			pts_dst.push_back(Point2f(newpointX[i], newpointY[i]));
		}
		Mat h222 = findHomography(pts_src, pts_dst);

		cout << pts_src << endl;
		cout << pts_dst << endl;
		cout << h222 << endl;

		//nomalized DLT 알고리즘 적용.. 1/2  시작
		//여기에서는 원본과 결과 이미지가 동일 크기인경우임.
		//home.deib.polimi.it/caglioti/StimaParametriGeometrici.ppt
		int image_width = img_gray.cols;
		int image_height = img_gray.rows;

		Mat T_norm_old = Mat::zeros(3, 3, CV_64FC1);
		T_norm_old.at<double>(0, 0) = image_width + image_height;
		T_norm_old.at<double>(1, 1) = image_width + image_height;
		T_norm_old.at<double>(0, 2) = image_width * 0.5;
		T_norm_old.at<double>(1, 2) = image_height * 0.5;
		T_norm_old.at<double>(2, 2) = 1;

		Mat T_norm_new = Mat::zeros(3, 3, CV_64FC1);
		T_norm_new.at<double>(0, 0) = image_width + image_height;
		T_norm_new.at<double>(1, 1) = image_width + image_height;
		T_norm_new.at<double>(0, 2) = image_width * 0.5;
		T_norm_new.at<double>(1, 2) = image_height * 0.5;
		T_norm_new.at<double>(2, 2) = 1;

		for (int i = 0; i < 4; i++)
		{
			pointX[i] = (image_width + image_height)*pointX[i] + image_width*0.5;
			pointY[i] = (image_width + image_height)*pointY[i] + image_height*0.5;

			newpointX[i] = (image_width + image_height)*newpointX[i] + image_width*0.5;
			newpointY[i] = (image_width + image_height)*newpointY[i] + image_height*0.5;
		}
		///////////////////nomalized DLT 알고리즘 적용.. 1/2 끝

		double data[8][9] = {
			{ -1 * pointX[0], -1 * pointY[0], -1, 0, 0, 0,     pointX[0] * newpointX[0], pointY[0] * newpointX[0], newpointX[0] },
			{ 0, 0, 0, -1 * pointX[0], -1 * pointY[0], -1,   pointX[0] * newpointY[0], pointY[0] * newpointY[0], newpointY[0] },
			{ -1 * pointX[1], -1 * pointY[1], -1, 0, 0, 0,pointX[1] * newpointX[1], pointY[1] * newpointX[1], newpointX[1] },
			{ 0, 0, 0, -1 * pointX[1], -1 * pointY[1], -1,pointX[1] * newpointY[1], pointY[1] * newpointY[1], newpointY[1] },
			{ -1 * pointX[2], -1 * pointY[2], -1, 0, 0, 0,pointX[2] * newpointX[2], pointY[2] * newpointX[2], newpointX[2] },
			{ 0, 0, 0, -1 * pointX[2], -1 * pointY[2], -1,pointX[2] * newpointY[2], pointY[2] * newpointY[2], newpointY[2] },
			{ -1 * pointX[3], -1 * pointY[3], -1, 0, 0, 0,pointX[3] * newpointX[3], pointY[3] * newpointX[3], newpointX[3] },
			{ 0, 0, 0, -1 * pointX[3], -1 * pointY[3], -1,pointX[3] * newpointY[3], pointY[3] * newpointY[3], newpointY[3] },
		};
		Mat A(8, 9, CV_64FC1, data);
		cout << "Matrix A" << endl;
		cout << A << endl;

		Mat d, u, vt, v;
		SVD::compute(A, d, u, vt, SVD::FULL_UV);
		transpose(vt, v);

		cout << "Matrix V" << endl;
		cout << v << endl;

		Mat h(3, 3, CV_64FC1);

		//마지막 컬럼값을 H로 취한다. 
		int lrow = 0;
		int lcols = v.cols - 1;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				h.at<double>(i, j) = v.at<double>(lrow, lcols);
				lrow++;
			}
		}

		//h_33을 1로 만든다.
		double dw = h.at<double>(2, 2);
		h = h / dw;

		//nomalized DLT 알고리즘 적용.. 2/2 시작
		Mat T_norm_new_invert = T_norm_new.inv();
		h = T_norm_new_invert*h*T_norm_old;
		/////////////nomalized DLT 알고리즘 적용.. 2/2  끝

		for (int y = 0; y<img_result.rows; y++)
		{
			for (int x = 0; x<img_result.cols; x++)
			{
				double data[3] = { x, y,1 };
				Mat oldpoint(3, 1, CV_64FC1);
				Mat newpoint(3, 1, CV_64FC1, data);

				Mat h2 = h.inv();
				oldpoint = h2*newpoint;

				int oldX, oldY;

				oldX = (int)((oldpoint.at<double>(0, 0)) / (oldpoint.at<double>(2, 0)) + 0.5);
				oldY = (int)((oldpoint.at<double>(1, 0)) / (oldpoint.at<double>(2, 0)) + 0.5);


				if ((oldX >= 0 && oldY >= 0) && (oldX < img_result.cols && oldY < img_result.rows))
					img_result.at<uchar>(y, x) = img_gray.at<uchar>(oldY, oldX);
			}
		}

		//보간법 적용
		Mat img_result2 = Mat::zeros(img_gray.size(), CV_8UC1);

		for (int y = 1; y<img_result.rows - 1; y++)
		{
			for (int x = 1; x < img_result.cols - 1; x++)
			{
				int q11 = img_result.at<uchar>(y - 1, x - 1);
				int q12 = img_result.at<uchar>(y + 1, x - 1);
				int q21 = img_result.at<uchar>(y + 1, x + 1);
				int q22 = img_result.at<uchar>(y - 1, x + 1);

				if (img_result.at<uchar>(y, x) == 0)
				{
					int p = BilinearInterpolation(q11, q12, q21, q22, x - 1, x + 1, y - 1, y + 1, x, y);
					if (p > 255) p = 255;
					if (p < 0)  p = 0;

					img_result2.at<uchar>(y, x) = p;
				}
				else img_result2.at<uchar>(y, x) = img_result.at<uchar>(y, x);

			}
		}

		//rectangle(img_result, Point(newpointX[0], newpointY[0]), Point(newpointX[3], newpointY[3]), Scalar(255), 1);
		imshow("my result image2", img_result2);

		//Size size(600, 600);
		//Mat im_dst = Mat::zeros(size, CV_8UC1);
		//warpPerspective(img_gray, im_dst, h222, size);

		//imshow("opencv result image3", im_dst);
	}
}

int main(int argc, char** argv)
{
	TCHAR szFilePath[MAX_PATH] = { 0, };
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = sizeof(szFilePath);
	ofn.lpstrFilter = NULL;
	//ofn.lpstrFilter = _T("Avi Files(*.avi)\0*.avi\0All Files (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (::GetOpenFileName(&ofn) == false) return 0;

	TCHAR *return_path = ofn.lpstrFile;
	cout << return_path << endl;
	VideoCapture cap(return_path); // 읽어올 비디오 파일을 연다
	if (!cap.isOpened())  // 비디오 파일을 여는 데 실패했다면, 프로그램을 종료한다.
	{
		cout << "Cannot open the video file" << endl;
		return -1;
	}

	double fps = cap.get(CV_CAP_PROP_FPS); // 비디오를 초당 몇 프레임 읽어올 것인지 설정한다

	cout << "Frame per seconds : " << fps << endl;

	//namedWindow("MyVideo", CV_WINDOW_AUTOSIZE);
	// 'MyVideo'라는 이름의 윈도우(창)을 생성한다.

    char savefile[200]; // 이미지 파일 이름을 200자 이내로 제한하기 위한 char 변수 선언
	int i=0;
	while(1)
	{
		Mat frame;
		bool bSuccess= cap.read(frame); // 비디오로부터 새 프레임을 읽어온다.
		if (!bSuccess) // 새 프레임을 읽어오는 데 실패했다면, 루프(while문)를 빠져나간다.
		{
			cout << "Cannot read the frame from video file, This is end of the video." << endl;
			break;
		}
		imshow("image", frame); // 이미지를 출력한다.

		sprintf_s(savefile, "image%d.png", i++);
		imwrite(savefile, frame); // img를 파일로 저장한다.
		waitKey(30); // 100ms의 딜레이를 발생시킨다. 즉, 1초에 10장의 이미지를 저장하게 된다. 수정함.
		if (waitKey(30) == 27) // 영상이 재생되는 시간(30초) 동안 'esc' 키를 기다린다.
							   //30초 내에 입력되는 경우 루프를 빠져나간다.
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}

	}
	Mat img_original;

	//이미지파일을 로드하여 image에 저장  
	img_original = imread("image0.png", IMREAD_COLOR);
	if (img_original.empty())
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//그레이스케일 이미지로 변환  
	cvtColor(img_original, img_gray, COLOR_BGR2GRAY);

	//윈도우 생성  
	namedWindow("gray image", WINDOW_AUTOSIZE);
	//namedWindow("result image", WINDOW_AUTOSIZE);


	//윈도우에 출력  
	imshow("gray image", img_gray);

	//윈도우에 콜백함수를 등록
	setMouseCallback("gray image", CallBackFunc, NULL);

	cout << "왼쪽 위 - 오른쪽 위 - 왼쪽 아래, 오른쪽 아래 순으로 클릭해주세요" << endl;

	//키보드 입력이 될때까지 대기  
	waitKey(0);
	return 0;
}