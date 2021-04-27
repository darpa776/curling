#include "stdafx.h"
#include <iostream> 
#include <windows.h>
#include <time.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp> 
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "tracker_opencv.h"
#include <fstream>


#define PI 3.1416
#define WINSIZE 3      //optical flow를 수행할 윈도우의 사이즈.  여기서는 7*7
#define MAX_COUNT 100  //flow 할 피처 포인트 몇개나 사용하는가
#define MAX_SIZE 1000

using namespace cv;
using namespace std;

char inputString[MAX_SIZE];
int count_mouse_click = 0;
double pre_pointX[4], pre_pointY[4];
double pre_newpointX[4], pre_newpointY[4];
double width;
double height;
int caculate_start = 0;

void CallBackFunc(int event, int x, int y, int flags, void* param)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		cout << count_mouse_click << "번째 클릭 좌표 = (" << x << ", " << y << ")" << endl;
		pre_pointX[count_mouse_click] = x;
		pre_pointY[count_mouse_click] = y;

		count_mouse_click++;
	}

	if (count_mouse_click == 4 && caculate_start == 0)
	{
		caculate_start = 1;

		cout << "#######################################################" << endl;
		cout << "H계산에 필요한 4개의 점을 모두 클릭했습니다." << endl << endl;


		width = ((pre_pointX[1] - pre_pointX[0]) + (pre_pointX[3] - pre_pointX[2]))*0.5;
		height = ((pre_pointY[2] - pre_pointY[0]) + (pre_pointY[3] - pre_pointY[1]))*0.5;

		pre_newpointX[0] = pre_pointX[3] - width;
		pre_newpointX[1] = pre_pointX[3];
		pre_newpointX[2] = pre_newpointX[0];
		pre_newpointX[3] = pre_newpointX[1];

		pre_newpointY[0] = pre_pointY[3] - height;
		pre_newpointY[1] = pre_newpointY[0];
		pre_newpointY[2] = pre_pointY[3];
		pre_newpointY[3] = pre_newpointY[2];

		for (int i = 0; i < 4; i++)
			cout << pre_newpointX[i] << " " << pre_newpointY[i] << endl;

		vector<Point2f> pts_src;
		vector<Point2f> pts_dst;

		for (int i = 0; i < 4; i++) {
			pts_src.push_back(Point2f(pre_pointX[i], pre_pointY[i]));
			pts_dst.push_back(Point2f(pre_newpointX[i], pre_newpointY[i]));
		}

		Mat h222 = findHomography(pts_src, pts_dst);

		cout << pts_src << endl;
		cout << pts_dst << endl;
		cout << h222 << endl;
	}
}

struct CallbackParam
{
	Mat frame;
	Point pt1, pt2;
	Rect roi;
	bool drag;
	bool updated;
};
CallbackParam param;

void onMouse(int event, int x, int y, int flags, void* param)
{
	CallbackParam *p = (CallbackParam *)param;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		p->pt1.x = x;
		p->pt1.y = y;
		p->pt2 = p->pt1;
		p->drag = true;
	}
	if (event == CV_EVENT_LBUTTONUP)
	{
		int w = x - p->pt1.x;
		int h = y - p->pt1.y;

		p->roi.x = p->pt1.x;
		p->roi.y = p->pt1.y;
		p->roi.width = w;
		p->roi.height = h;
		p->drag = false;

		if (w >= 10 && h >= 10)
		{
			p->updated = true;
		}
	}
	if (p->drag && event == CV_EVENT_MOUSEMOVE)
	{
		if (p->pt2.x != x || p->pt2.y != y)
		{
			Mat img = p->frame.clone();
			p->pt2.x = x;
			p->pt2.y = y;
			rectangle(img, p->pt1, p->pt2, Scalar(0, 255, 0), 1);
			imshow("image", img);
		}
	}
}

void ROI_setting(Mat frame)
{
	//tracker_opencv tracker;

	namedWindow("image", WINDOW_AUTOSIZE);
	imshow("image", frame);


	setMouseCallback("image", onMouse, &param);

	param.frame = frame;
	param.drag = false;
	param.updated = false;

	while (1)
	{
		// image display
		imshow("image", frame);


		// user input
		if (waitKey() == 27)
		{
			destroyAllWindows();
			break;	// ESC Key (exit)
		}

	}

	cout << "ROI 영역 좌표 출력" << endl;
	cout << "모서리 1 좌표: " << param.pt1.x << " " << param.pt1.y << endl;
	cout << "모서리 2 좌표: " << param.pt2.x << " " << param.pt2.y << endl;

}



int main()
{
	ofstream outFile("좌표.txt");


	TCHAR szFilePath[MAX_PATH] = { 0 };
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = sizeof(szFilePath);
	ofn.lpstrFilter = NULL;
	//ofn.lpstrFilter = _T("Avi Files(*.avi)\0*.avi\0All Files   IplImage* src_grey(*.*)\0*.*\0");
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


	Mat   src_img, dst_img, intrinsic, distortion;
	// (2) 매개 변수 파일을로드
	intrinsic = Mat::zeros(3, 3, CV_64FC1);
	intrinsic.at<double>(0, 0) = 1181.232811;
	intrinsic.at<double>(1, 1) = 1177.908305;
	intrinsic.at<double>(0, 2) = 697.524395;
	intrinsic.at<double>(1, 2) = 319.947064;
	intrinsic.at<double>(2, 2) = 1;


	distortion = Mat::zeros(1, 4, CV_64FC1);
	distortion.at<double>(0, 0) = -0.077163;
	distortion.at<double>(0, 1) = 0.111523;
	distortion.at<double>(0, 2) = -0.004409;
	distortion.at<double>(0, 3) = -0.008375;

	int first_image_setting = 0;


	bool bSuccess = cap.read(src_img); // 비디오로부터 새 프레임을 읽어온다.
	if (!bSuccess) // 새 프레임을 읽어오는 데 실패했다면, 루프(while문)를 빠져나간다.
	{
		cout << "This is end of the video." << endl;
		//break;
	}

	// (1) 보정 대상이되는 이미지 불러 오기
	//cap.read(src_img);
	//	dst_img = src_img;


	// (3) 왜곡 보정
	undistort(src_img, dst_img, intrinsic, distortion, noArray());

	// (4) 이미지를 표시 키를 누를 때 종료



	if (first_image_setting == 0)
	{
		first_image_setting = 1;
		namedWindow("setting", 0);
		//rectangle(dst_img, param.pt1, param.pt2, Scalar(0, 255, 0), 1);
		imshow("setting", dst_img);
		setMouseCallback("setting", CallBackFunc, NULL);

		cout << "왼쪽 위 - 오른쪽 위 - 왼쪽 아래- 오른쪽 아래 순으로 클릭" << endl;
		waitKey(0);

		ROI_setting(dst_img);


	}

	//waitKey(1);

	IplImage* src_color = &IplImage(dst_img);
	IplImage* src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	IplImage* pre_src_color = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);  //t-1의 이미지 프레임
	IplImage* pre_src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);  //t-1의 이미지 프레임
	IplImage* result_view = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);




	//점들이 저장되는곳  그냥 간단히 매우 크게 지정  
	CvPoint2D32f* corners = new CvPoint2D32f[500];
	int corner_count = 500;
	CvPoint2D32f tracking_points[5000];
	int tracking_cou = 0;



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




	//goodfeature to track 
	cvCvtColor(src_color, src_grey, CV_BGR2GRAY);
	IplImage* mask = (IplImage*)cvClone(src_grey);
	cvSetImageROI(mask, CvRect(param.pt1.x, param.pt1.y, width, height));

	IplImage* ROI_eig_image = cvCreateImage(cvSize(mask->width, mask->height), IPL_DEPTH_32F, 1);
	IplImage* ROI_temp_image = cvCreateImage(cvSize(mask->width, mask->height), IPL_DEPTH_32F, 1);

	int use_harris = 0; // use_harris : 0이면 shi_Tomasi 방법으로 계산, 0이 아니라면 Harris corner방법을 사용
						//이미지에서 코너를 추출함
	cvGoodFeaturesToTrack(mask, ROI_eig_image, ROI_temp_image, corners, &corner_count, 0.1, 0.1, NULL, 3, use_harris, 0.04);
	////서브픽셀을 검출하여 정확한 서브픽셀 위치를 산출해냄
	cvFindCornerSubPix(mask, corners, corner_count, cvSize(WINSIZE, WINSIZE), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

	// 특징점 좌표 맞추기
	for (int i = 0; i < corner_count; i++) {
		corners[i].x = corners[i].x + param.pt1.x;
		corners[i].y = corners[i].y + param.pt1.y;
	}

	// 특징점 표시하기
	int features_cx = 0;
	int features_cy = 0;
	for (int i = 0; i < corner_count; ++i) {
		cvCircle(result_view, cvPointFrom32f(corners[i]), 1, CV_RGB(0, 255, 0), -1, 4, 0);
		features_cx = corners[i].x + features_cx;
		features_cy = corners[i].y + features_cy;
	}
	features_cx = features_cx / corner_count;
	features_cy = features_cy / corner_count;
	cvCircle(result_view, cvPoint(features_cx, features_cy), 3, CV_RGB(0, 0, 255), -1, 4, 0);
	tracking_points[tracking_cou].x = features_cx;
	tracking_points[tracking_cou].y = features_cy;
	tracking_cou++;

	//다음 씬 처리 위해 지금 영상 이전 영상으로 저장
	IplImage* pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	IplImage* prev_pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	cvCopy(src_grey, pre_src_grey);
	cvCopy(pyramid, prev_pyramid);

	//현재 찾은 점 좌표 저장
	int pre_num_of_pts;
	CvPoint2D32f pre_feature_points[500];
	for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
	pre_num_of_pts = corner_count;

	//cvSetImageROI(gray, CvRect(MP_x, MP_y, MP_X - MP_x, MP_Y - MP_y));
	//cvShowImage("ROI", gray);
	char* status = 0;
	status = new char[500];
	int flags = 0;

	int width, height;
	width = src_color->width;    //소스 이미지 구조체에서 이미지 폭 불러오기
	height = src_color->height;   //소스 이미지 구조체에서 이미지 높이 불러오기

	while (1)
	{
		//time_t start = clock();
		cvCopy(src_color, result_view);

		cvCvtColor(src_color, src_grey, CV_BGR2GRAY);

		if (pre_num_of_pts > 0) {
			float feature_errors[500];
			cvCalcOpticalFlowPyrLK(pre_src_grey, src_grey, prev_pyramid, pyramid, pre_feature_points, corners, corner_count, cvSize(WINSIZE, WINSIZE), 3, status, feature_errors, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
			flags = CV_LKFLOW_PYR_A_READY;


			int valid_pt_R = 0;  //valid한 점의 개수 count

			for (int i = 0; i < corner_count; ++i)
			{  //그 point가 valid 한지
				if (status[i] != 0) ++valid_pt_R;

				if (feature_errors[i] > 20) {
					//feature_found[i]값이 0이 리턴이 되면 대응점을 발견하지 못함
					//feature_errors[i] 현재 프레임과 이전프레임 사이의 거리가 550이 넘으면 예외로 처리
					printf("Error is %f\n", feature_errors[i]);
					continue;
				}
			}


			//draw result, 화살표 모양으로 그리고 실제 움직인 거리의 3배 길이로 화살표 그려줌
			for (int i = 0; i < corner_count; i++) {
				if (status[i] == 0) continue; //valid 한 점이 아니면 넘어감
				int line_thickness = 1;
				CvScalar line_color = CV_RGB(255, 0, 0);
				CvPoint p, q;
				p.x = (int)pre_feature_points[i].x;
				p.y = (int)pre_feature_points[i].y;
				q.x = (int)corners[i].x;
				q.y = (int)corners[i].y;

				double angle;
				angle = atan2((double)p.y - q.y, (double)p.x - q.x);
				double arrow_length;
				arrow_length = sqrt(pow((float)(p.y - q.y), 2) + pow((float)(p.x - q.x), 2));

				q.x = (int)(p.x - 3 * arrow_length * cos(angle));  //3배 길이로 화살표 그려줌
				q.y = (int)(p.y - 3 * arrow_length * sin(angle));

				//일정길이이하 화살표는 그리지 않는다.
				if ((arrow_length < 3) | (arrow_length > 40)) continue; //화살표 길이가 너무짧거나 길면 안그린다.

																		//draw arrow
				cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);
				p.x = (int)(q.x + 5 * cos(angle + PI / 4));
				if (p.x >= width) p.x = width - 1;
				else if (p.x < 0) p.x = 0;

				p.y = (int)(q.y + 5 * sin(angle + PI / 4));
				if (p.y >= height) p.y = height - 1;
				else if (p.y < 0)  p.y = 0;

				cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);

				p.x = (int)(q.x + 5 * cos(angle - PI / 4));
				if (p.x >= width) p.x = width - 1;
				else if (p.x < 0) p.x = 0;
				p.y = (int)(q.y + 5 * sin(angle - PI / 4));
				if (p.y > height) p.y = height - 1;
				else if (p.y < 0) p.y = 0;
				cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);
			}
		}
		//printf("max_count_R : %d\n", corner_count);

		for (int i = 0; i < corner_count; ++i) cvCircle(result_view, cvPointFrom32f(corners[i]), 1, CV_RGB(0, 255, 0), -1, 4, 0);
		//다음 씬 처리 위해 지금영상 저장
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);
		//현재 찾은 점 좌표 저장
		for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
		pre_num_of_pts = corner_count;
		//cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //이런 저런 설정.
		//cvPutText(result_view, s_output_result, cvPoint(width - 150, height - 20), &font, cvScalar(0, 255, 0));   //cvPoint로 글자 시작 위치 설정(uv)

		// 트래킹 선누적
		for (int i = 0; i < corner_count; ++i) {
			features_cx = corners[i].x + features_cx;
			features_cy = corners[i].y + features_cy;
		}
		features_cx = features_cx / corner_count;
		features_cy = features_cy / corner_count;
		//cvCircle(result_view, cvPoint(features_cx, features_cy), 3, CV_RGB(0, 0, 255), -1, 4, 0);
		tracking_points[tracking_cou].x = features_cx;
		tracking_points[tracking_cou].y = features_cy;
		tracking_cou++;

		// 트래킹 선그리기
		for (int i = 0; i < tracking_cou - 1; i++)

		{

			//cvLine(result_view, cvPoint(tracking_points[i].x, tracking_points[i].y), cvPoint(tracking_points[i + 1].x, tracking_points[i + 1].y), CV_RGB(0, 255, 0), 1, 4, 0);
		}

		cvNamedWindow("newimage", CV_WINDOW_AUTOSIZE);
		cvMoveWindow("newimage", 0, 0);
		cvShowImage("newimage", result_view);
		//if (waitKey(100) == 27) break; // 100ms -> 아스키코드 값 27 즉, ESC 누르면 실행 - break

		////루카스-카나데 알고리즘
		//char feature_found[MAX_CORNERS];
		//float feature_errors[MAX_CORNERS];
		//CvSize pyr_sz = cvSize(imgA->width + 8, imgB->height / 3);

		//IplImage* pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
		//IplImage* pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

		//CvPoint2D32f* cornersB = new CvPoint2D32f[MAX_CORNERS];

		////추출한 코너(cornerA)를 추적함 -> 이동한 점들의 위치는 cornerB에 저장된다.
		//cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, cornersA, cornersB, corner_count,
		// cvSize(WINSIZE, WINSIZE), 5, feature_found, feature_errors,
		// cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3), 0);

		//for (int i = 0; i<corner_count; i++) {
		// if (feature_found[i] == 0 || feature_errors[i] > 550) {
		//  //feature_found[i]값이 0이 리턴이 되면 대응점을 발견하지 못함
		//  //feature_errors[i] 현재 프레임과 이전프레임 사이의 거리가 550이 넘으면 예외로 처리
		//  printf("Error is %f\n", feature_errors[i]);
		//  continue;
		// }
		//cvShowImage("OpticalFlow", src_color);
		//if (waitKey(33) == 27) break;




		bool bSuccess = cap.read(src_img); // 비디오로부터 새 프레임을 읽어온다.
		if (!bSuccess) // 새 프레임을 읽어오는 데 실패했다면, 루프(while문)를 빠져나간다.
		{
			cout << "This is end of the video." << endl;
			break;
		}

		// (1) 보정 대상이되는 이미지 불러 오기
		//cap.read(src_img);
	//	dst_img = src_img;


		// (3) 왜곡 보정
		undistort(src_img, dst_img, intrinsic, distortion, noArray());

		// (4) 이미지를 표시 키를 누를 때 종료



		if (first_image_setting == 0)
		{
			first_image_setting = 1;
			namedWindow("setting", 0);
			//rectangle(dst_img, param.pt1, param.pt2, Scalar(0, 255, 0), 1);
			imshow("setting", dst_img);
			setMouseCallback("setting", CallBackFunc, NULL);

			cout << "왼쪽 위 - 오른쪽 위 - 왼쪽 아래- 오른쪽 아래 순으로 클릭" << endl;
			waitKey(0);

			ROI_setting(dst_img);


		}

		waitKey(1);

		IplImage* src_color = &IplImage(dst_img);
		IplImage* src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		IplImage* pre_src_color = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);  //t-1의 이미지 프레임
		IplImage* pre_src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);  //t-1의 이미지 프레임
		IplImage* result_view = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);




		//점들이 저장되는곳  그냥 간단히 매우 크게 지정  
		CvPoint2D32f* corners = new CvPoint2D32f[500];
		int corner_count = 500;
		CvPoint2D32f tracking_points[5000];
		int tracking_cou = 0;



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




		//goodfeature to track 
		cvCvtColor(src_color, src_grey, CV_BGR2GRAY);
		IplImage* mask = (IplImage*)cvClone(src_grey);
		cvSetImageROI(mask, CvRect(param.pt1.x, param.pt1.y, width, height));

		IplImage* ROI_eig_image = cvCreateImage(cvSize(mask->width, mask->height), IPL_DEPTH_32F, 1);
		IplImage* ROI_temp_image = cvCreateImage(cvSize(mask->width, mask->height), IPL_DEPTH_32F, 1);

		int use_harris = 0; // use_harris : 0이면 shi_Tomasi 방법으로 계산, 0이 아니라면 Harris corner방법을 사용
							//이미지에서 코너를 추출함
		cvGoodFeaturesToTrack(mask, ROI_eig_image, ROI_temp_image, corners, &corner_count, 0.1, 0.1, NULL, 3, use_harris, 0.04);
		////서브픽셀을 검출하여 정확한 서브픽셀 위치를 산출해냄
		cvFindCornerSubPix(mask, corners, corner_count, cvSize(WINSIZE, WINSIZE), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

		// 특징점 좌표 맞추기
		for (int i = 0; i < corner_count; i++) {
			corners[i].x = corners[i].x + param.pt1.x;
			corners[i].y = corners[i].y + param.pt1.y;
		}

		// 특징점 표시하기
		int features_cx = 0;
		int features_cy = 0;
		for (int i = 0; i < corner_count; ++i) {
			//특징점 인덱스와 좌표 출력

			cvCircle(result_view, cvPointFrom32f(corners[i]), 1, CV_RGB(0, 255, 0), -1, 4, 0);
			features_cx = corners[i].x + features_cx;
			features_cy = corners[i].y + features_cy;
		}
		features_cx = features_cx / corner_count;
		features_cy = features_cy / corner_count;
		cvCircle(result_view, cvPoint(features_cx, features_cy), 3, CV_RGB(0, 0, 255), -1, 4, 0);
		tracking_points[tracking_cou].x = features_cx;
		tracking_points[tracking_cou].y = features_cy;
		//평균좌표출력



		tracking_cou++;




		//다음 씬 처리 위해 지금 영상 이전 영상으로 저장
		IplImage* pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		IplImage* prev_pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);

		//현재 찾은 점 좌표 저장
		int pre_num_of_pts;
		CvPoint2D32f pre_feature_points[500];
		for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
		pre_num_of_pts = corner_count;

		//cvSetImageROI(gray, CvRect(MP_x, MP_y, MP_X - MP_x, MP_Y - MP_y));
		//cvShowImage("ROI", gray);
		char* status = 0;
		status = new char[500];
		int flags = 0;

		int width, height;
		width = src_color->width;    //소스 이미지 구조체에서 이미지 폭 불러오기
		height = src_color->height;   //소스 이미지 구조체에서 이미지 높이 불러오기


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cvCopy(src_color, result_view);

		cvCvtColor(src_color, src_grey, CV_BGR2GRAY);

		if (pre_num_of_pts > 0) {
			float feature_errors[500];
			cvCalcOpticalFlowPyrLK(pre_src_grey, src_grey, prev_pyramid, pyramid, pre_feature_points, corners, corner_count, cvSize(WINSIZE, WINSIZE), 3, status, feature_errors, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
			flags = OPTFLOW_USE_INITIAL_FLOW;


			int valid_pt_R = 0;  //valid한 점의 개수 count

			for (int i = 0; i < corner_count; ++i)
			{  //그 point가 valid 한지
				if (status[i] != 0) ++valid_pt_R;

				if (feature_errors[i] > 50) {
					//feature_found[i]값이 0이 리턴이 되면 대응점을 발견하지 못함
					//feature_errors[i] 현재 프레임과 이전프레임 사이의 거리가 550이 넘으면 예외로 처리
					printf("Error is %f\n", feature_errors[i]);
					corners[i] = NULL;
					pre_feature_points[i] = NULL;
					status[i] = NULL;
					--valid_pt_R;
				}

				if (abs(pre_feature_points[i].x - corners[i].x) > 30)
				{
					corners[i] = NULL;
					pre_feature_points[i] = NULL;
					status[i] = NULL;
					--valid_pt_R;
				}
				if (corners[i].y - pre_feature_points[i].y > 30)
				{
					corners[i] = NULL;
					pre_feature_points[i] = NULL;
					status[i] = NULL;
					--valid_pt_R;
				}

				continue;
			}


		}


		//draw result, 화살표 모양으로 그리고 실제 움직인 거리의 3배 길이로 화살표 그려줌
		for (int i = 0; i < corner_count; i++) {
			if (status[i] == 0) continue; //valid 한 점이 아니면 넘어감
			int line_thickness = 1;
			CvScalar line_color = CV_RGB(255, 0, 0);
			CvPoint p, q;
			p.x = (int)pre_feature_points[i].x;
			p.y = (int)pre_feature_points[i].y;
			q.x = (int)corners[i].x;
			q.y = (int)corners[i].y;

			double angle;
			angle = atan2((double)p.y - q.y, (double)p.x - q.x);
			double arrow_length;
			arrow_length = sqrt(pow((float)(p.y - q.y), 2) + pow((float)(p.x - q.x), 2));

			q.x = (int)(p.x - 3 * arrow_length * cos(angle));  //3배 길이로 화살표 그려줌
			q.y = (int)(p.y - 3 * arrow_length * sin(angle));

			//일정길이이하 화살표는 그리지 않는다.
			if ((arrow_length < 3) | (arrow_length > 40)) continue; //화살표 길이가 너무짧거나 길면 안그린다.

																	//draw arrow
			cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);
			p.x = (int)(q.x + 5 * cos(angle + PI / 4));
			if (p.x >= width) p.x = width - 1;
			else if (p.x < 0) p.x = 0;

			p.y = (int)(q.y + 5 * sin(angle + PI / 4));
			if (p.y >= height) p.y = height - 1;
			else if (p.y < 0)  p.y = 0;

			cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);

			p.x = (int)(q.x + 5 * cos(angle - PI / 4));
			if (p.x >= width) p.x = width - 1;
			else if (p.x < 0) p.x = 0;
			p.y = (int)(q.y + 5 * sin(angle - PI / 4));
			if (p.y > height) p.y = height - 1;
			else if (p.y < 0) p.y = 0;
			cvLine(result_view, p, q, line_color, line_thickness, CV_AA, 0);
		}

		//printf("max_count_R : %d\n", corner_count);

		for (int i = 0; i < corner_count; ++i) cvCircle(result_view, cvPointFrom32f(corners[i]), 1, CV_RGB(0, 255, 0), -1, 4, 0);
		//다음 씬 처리 위해 지금영상 저장
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);
		//현재 찾은 점 좌표 저장
		for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
		pre_num_of_pts = corner_count;
		//cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //이런 저런 설정.
		//cvPutText(result_view, s_output_result, cvPoint(width - 150, height - 20), &font, cvScalar(0, 255, 0));   //cvPoint로 글자 시작 위치 설정(uv)

		// 트래킹 선누적
		for (int i = 0; i < corner_count; ++i) {
			features_cx = corners[i].x + features_cx;
			features_cy = corners[i].y + features_cy;
			//	cout << i << endl;
			//	cout << corners[i].x << endl;
			//	cout << corners[i].y << endl;
		}
		features_cx = features_cx / corner_count;
		features_cy = features_cy / corner_count;
		//cvCircle(result_view, cvPoint(features_cx, features_cy), 3, CV_RGB(0, 0, 255), -1, 4, 0);
		tracking_points[tracking_cou].x = features_cx;
		tracking_points[tracking_cou].y = features_cy;
		//평균좌표출력
		cout << tracking_points[tracking_cou].x << " " << tracking_points[tracking_cou].y << endl;

		outFile << tracking_points[tracking_cou].x << " " << tracking_points[tracking_cou].y << endl;

		//cout << "다음 프레임" << endl;
		tracking_cou++;

		// 트래킹 선그리기
		for (int i = 0; i < tracking_cou - 1; i++)

		{

			//cvLine(result_view, cvPoint(tracking_points[i].x, tracking_points[i].y), cvPoint(tracking_points[i + 1].x, tracking_points[i + 1].y), CV_RGB(0, 255, 0), 1, 4, 0);
		}

		cvNamedWindow("newimage", CV_WINDOW_AUTOSIZE);
		cvMoveWindow("newimage", 0, 0);
		cvShowImage("newimage", result_view);



		//time_t end = clock();
		//float gap = (float)(end - start) / CLOCKS_PER_SEC;
		//printf("%f\n", gap);
		//if (waitKey(100) == 27) break; // 100ms -> 아스키코드 값 27 즉, ESC 누르면 실행 - break

									   ////루카스-카나데 알고리즘
									   //char feature_found[MAX_CORNERS];
									   //float feature_errors[MAX_CORNERS];
									   //CvSize pyr_sz = cvSize(imgA->width + 8, imgB->height / 3);

									   //IplImage* pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
									   //IplImage* pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

									   //CvPoint2D32f* cornersB = new CvPoint2D32f[MAX_CORNERS];

									   ////추출한 코너(cornerA)를 추적함 -> 이동한 점들의 위치는 cornerB에 저장된다.
									   //cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, cornersA, cornersB, corner_count,
									   // cvSize(WINSIZE, WINSIZE), 5, feature_found, feature_errors,
									   // cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3), 0);

									   //for (int i = 0; i<corner_count; i++) {
									   // if (feature_found[i] == 0 || feature_errors[i] > 550) {
									   //  //feature_found[i]값이 0이 리턴이 되면 대응점을 발견하지 못함
									   //  //feature_errors[i] 현재 프레임과 이전프레임 사이의 거리가 550이 넘으면 예외로 처리
									   //  printf("Error is %f\n", feature_errors[i]);
									   //  continue;
									   // }
									   //cvShowImage("OpticalFlow", src_color);
			   //if (waitKey(33) == 27) break;
		}
outFile.close();
destroyAllWindows();
return 0;

}
