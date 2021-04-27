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
#define WINSIZE 3      //optical flow�� ������ �������� ������.  ���⼭�� 7*7
#define MAX_COUNT 100  //flow �� ��ó ����Ʈ ��� ����ϴ°�
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
		cout << count_mouse_click << "��° Ŭ�� ��ǥ = (" << x << ", " << y << ")" << endl;
		pre_pointX[count_mouse_click] = x;
		pre_pointY[count_mouse_click] = y;

		count_mouse_click++;
	}

	if (count_mouse_click == 4 && caculate_start == 0)
	{
		caculate_start = 1;

		cout << "#######################################################" << endl;
		cout << "H��꿡 �ʿ��� 4���� ���� ��� Ŭ���߽��ϴ�." << endl << endl;


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

	cout << "ROI ���� ��ǥ ���" << endl;
	cout << "�𼭸� 1 ��ǥ: " << param.pt1.x << " " << param.pt1.y << endl;
	cout << "�𼭸� 2 ��ǥ: " << param.pt2.x << " " << param.pt2.y << endl;

}



int main()
{
	ofstream outFile("��ǥ.txt");


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

	VideoCapture cap(return_path); // �о�� ���� ������ ����
	if (!cap.isOpened())  // ���� ������ ���� �� �����ߴٸ�, ���α׷��� �����Ѵ�.
	{
		cout << "Cannot open the video file" << endl;
		return -1;
	}


	Mat   src_img, dst_img, intrinsic, distortion;
	// (2) �Ű� ���� �������ε�
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


	bool bSuccess = cap.read(src_img); // �����κ��� �� �������� �о�´�.
	if (!bSuccess) // �� �������� �о���� �� �����ߴٸ�, ����(while��)�� ����������.
	{
		cout << "This is end of the video." << endl;
		//break;
	}

	// (1) ���� ����̵Ǵ� �̹��� �ҷ� ����
	//cap.read(src_img);
	//	dst_img = src_img;


	// (3) �ְ� ����
	undistort(src_img, dst_img, intrinsic, distortion, noArray());

	// (4) �̹����� ǥ�� Ű�� ���� �� ����



	if (first_image_setting == 0)
	{
		first_image_setting = 1;
		namedWindow("setting", 0);
		//rectangle(dst_img, param.pt1, param.pt2, Scalar(0, 255, 0), 1);
		imshow("setting", dst_img);
		setMouseCallback("setting", CallBackFunc, NULL);

		cout << "���� �� - ������ �� - ���� �Ʒ�- ������ �Ʒ� ������ Ŭ��" << endl;
		waitKey(0);

		ROI_setting(dst_img);


	}

	//waitKey(1);

	IplImage* src_color = &IplImage(dst_img);
	IplImage* src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	IplImage* pre_src_color = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);  //t-1�� �̹��� ������
	IplImage* pre_src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);  //t-1�� �̹��� ������
	IplImage* result_view = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);




	//������ ����Ǵ°�  �׳� ������ �ſ� ũ�� ����  
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

	int use_harris = 0; // use_harris : 0�̸� shi_Tomasi ������� ���, 0�� �ƴ϶�� Harris corner����� ���
						//�̹������� �ڳʸ� ������
	cvGoodFeaturesToTrack(mask, ROI_eig_image, ROI_temp_image, corners, &corner_count, 0.1, 0.1, NULL, 3, use_harris, 0.04);
	////�����ȼ��� �����Ͽ� ��Ȯ�� �����ȼ� ��ġ�� �����س�
	cvFindCornerSubPix(mask, corners, corner_count, cvSize(WINSIZE, WINSIZE), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

	// Ư¡�� ��ǥ ���߱�
	for (int i = 0; i < corner_count; i++) {
		corners[i].x = corners[i].x + param.pt1.x;
		corners[i].y = corners[i].y + param.pt1.y;
	}

	// Ư¡�� ǥ���ϱ�
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

	//���� �� ó�� ���� ���� ���� ���� �������� ����
	IplImage* pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	IplImage* prev_pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
	cvCopy(src_grey, pre_src_grey);
	cvCopy(pyramid, prev_pyramid);

	//���� ã�� �� ��ǥ ����
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
	width = src_color->width;    //�ҽ� �̹��� ����ü���� �̹��� �� �ҷ�����
	height = src_color->height;   //�ҽ� �̹��� ����ü���� �̹��� ���� �ҷ�����

	while (1)
	{
		//time_t start = clock();
		cvCopy(src_color, result_view);

		cvCvtColor(src_color, src_grey, CV_BGR2GRAY);

		if (pre_num_of_pts > 0) {
			float feature_errors[500];
			cvCalcOpticalFlowPyrLK(pre_src_grey, src_grey, prev_pyramid, pyramid, pre_feature_points, corners, corner_count, cvSize(WINSIZE, WINSIZE), 3, status, feature_errors, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
			flags = CV_LKFLOW_PYR_A_READY;


			int valid_pt_R = 0;  //valid�� ���� ���� count

			for (int i = 0; i < corner_count; ++i)
			{  //�� point�� valid ����
				if (status[i] != 0) ++valid_pt_R;

				if (feature_errors[i] > 20) {
					//feature_found[i]���� 0�� ������ �Ǹ� �������� �߰����� ����
					//feature_errors[i] ���� �����Ӱ� ���������� ������ �Ÿ��� 550�� ������ ���ܷ� ó��
					printf("Error is %f\n", feature_errors[i]);
					continue;
				}
			}


			//draw result, ȭ��ǥ ������� �׸��� ���� ������ �Ÿ��� 3�� ���̷� ȭ��ǥ �׷���
			for (int i = 0; i < corner_count; i++) {
				if (status[i] == 0) continue; //valid �� ���� �ƴϸ� �Ѿ
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

				q.x = (int)(p.x - 3 * arrow_length * cos(angle));  //3�� ���̷� ȭ��ǥ �׷���
				q.y = (int)(p.y - 3 * arrow_length * sin(angle));

				//������������ ȭ��ǥ�� �׸��� �ʴ´�.
				if ((arrow_length < 3) | (arrow_length > 40)) continue; //ȭ��ǥ ���̰� �ʹ�ª�ų� ��� �ȱ׸���.

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
		//���� �� ó�� ���� ���ݿ��� ����
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);
		//���� ã�� �� ��ǥ ����
		for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
		pre_num_of_pts = corner_count;
		//cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //�̷� ���� ����.
		//cvPutText(result_view, s_output_result, cvPoint(width - 150, height - 20), &font, cvScalar(0, 255, 0));   //cvPoint�� ���� ���� ��ġ ����(uv)

		// Ʈ��ŷ ������
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

		// Ʈ��ŷ ���׸���
		for (int i = 0; i < tracking_cou - 1; i++)

		{

			//cvLine(result_view, cvPoint(tracking_points[i].x, tracking_points[i].y), cvPoint(tracking_points[i + 1].x, tracking_points[i + 1].y), CV_RGB(0, 255, 0), 1, 4, 0);
		}

		cvNamedWindow("newimage", CV_WINDOW_AUTOSIZE);
		cvMoveWindow("newimage", 0, 0);
		cvShowImage("newimage", result_view);
		//if (waitKey(100) == 27) break; // 100ms -> �ƽ�Ű�ڵ� �� 27 ��, ESC ������ ���� - break

		////��ī��-ī���� �˰���
		//char feature_found[MAX_CORNERS];
		//float feature_errors[MAX_CORNERS];
		//CvSize pyr_sz = cvSize(imgA->width + 8, imgB->height / 3);

		//IplImage* pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
		//IplImage* pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

		//CvPoint2D32f* cornersB = new CvPoint2D32f[MAX_CORNERS];

		////������ �ڳ�(cornerA)�� ������ -> �̵��� ������ ��ġ�� cornerB�� ����ȴ�.
		//cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, cornersA, cornersB, corner_count,
		// cvSize(WINSIZE, WINSIZE), 5, feature_found, feature_errors,
		// cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3), 0);

		//for (int i = 0; i<corner_count; i++) {
		// if (feature_found[i] == 0 || feature_errors[i] > 550) {
		//  //feature_found[i]���� 0�� ������ �Ǹ� �������� �߰����� ����
		//  //feature_errors[i] ���� �����Ӱ� ���������� ������ �Ÿ��� 550�� ������ ���ܷ� ó��
		//  printf("Error is %f\n", feature_errors[i]);
		//  continue;
		// }
		//cvShowImage("OpticalFlow", src_color);
		//if (waitKey(33) == 27) break;




		bool bSuccess = cap.read(src_img); // �����κ��� �� �������� �о�´�.
		if (!bSuccess) // �� �������� �о���� �� �����ߴٸ�, ����(while��)�� ����������.
		{
			cout << "This is end of the video." << endl;
			break;
		}

		// (1) ���� ����̵Ǵ� �̹��� �ҷ� ����
		//cap.read(src_img);
	//	dst_img = src_img;


		// (3) �ְ� ����
		undistort(src_img, dst_img, intrinsic, distortion, noArray());

		// (4) �̹����� ǥ�� Ű�� ���� �� ����



		if (first_image_setting == 0)
		{
			first_image_setting = 1;
			namedWindow("setting", 0);
			//rectangle(dst_img, param.pt1, param.pt2, Scalar(0, 255, 0), 1);
			imshow("setting", dst_img);
			setMouseCallback("setting", CallBackFunc, NULL);

			cout << "���� �� - ������ �� - ���� �Ʒ�- ������ �Ʒ� ������ Ŭ��" << endl;
			waitKey(0);

			ROI_setting(dst_img);


		}

		waitKey(1);

		IplImage* src_color = &IplImage(dst_img);
		IplImage* src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		IplImage* pre_src_color = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);  //t-1�� �̹��� ������
		IplImage* pre_src_grey = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);  //t-1�� �̹��� ������
		IplImage* result_view = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 3);




		//������ ����Ǵ°�  �׳� ������ �ſ� ũ�� ����  
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

		int use_harris = 0; // use_harris : 0�̸� shi_Tomasi ������� ���, 0�� �ƴ϶�� Harris corner����� ���
							//�̹������� �ڳʸ� ������
		cvGoodFeaturesToTrack(mask, ROI_eig_image, ROI_temp_image, corners, &corner_count, 0.1, 0.1, NULL, 3, use_harris, 0.04);
		////�����ȼ��� �����Ͽ� ��Ȯ�� �����ȼ� ��ġ�� �����س�
		cvFindCornerSubPix(mask, corners, corner_count, cvSize(WINSIZE, WINSIZE), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

		// Ư¡�� ��ǥ ���߱�
		for (int i = 0; i < corner_count; i++) {
			corners[i].x = corners[i].x + param.pt1.x;
			corners[i].y = corners[i].y + param.pt1.y;
		}

		// Ư¡�� ǥ���ϱ�
		int features_cx = 0;
		int features_cy = 0;
		for (int i = 0; i < corner_count; ++i) {
			//Ư¡�� �ε����� ��ǥ ���

			cvCircle(result_view, cvPointFrom32f(corners[i]), 1, CV_RGB(0, 255, 0), -1, 4, 0);
			features_cx = corners[i].x + features_cx;
			features_cy = corners[i].y + features_cy;
		}
		features_cx = features_cx / corner_count;
		features_cy = features_cy / corner_count;
		cvCircle(result_view, cvPoint(features_cx, features_cy), 3, CV_RGB(0, 0, 255), -1, 4, 0);
		tracking_points[tracking_cou].x = features_cx;
		tracking_points[tracking_cou].y = features_cy;
		//�����ǥ���



		tracking_cou++;




		//���� �� ó�� ���� ���� ���� ���� �������� ����
		IplImage* pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		IplImage* prev_pyramid = cvCreateImage(cvGetSize(src_color), IPL_DEPTH_8U, 1);
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);

		//���� ã�� �� ��ǥ ����
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
		width = src_color->width;    //�ҽ� �̹��� ����ü���� �̹��� �� �ҷ�����
		height = src_color->height;   //�ҽ� �̹��� ����ü���� �̹��� ���� �ҷ�����


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cvCopy(src_color, result_view);

		cvCvtColor(src_color, src_grey, CV_BGR2GRAY);

		if (pre_num_of_pts > 0) {
			float feature_errors[500];
			cvCalcOpticalFlowPyrLK(pre_src_grey, src_grey, prev_pyramid, pyramid, pre_feature_points, corners, corner_count, cvSize(WINSIZE, WINSIZE), 3, status, feature_errors, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
			flags = OPTFLOW_USE_INITIAL_FLOW;


			int valid_pt_R = 0;  //valid�� ���� ���� count

			for (int i = 0; i < corner_count; ++i)
			{  //�� point�� valid ����
				if (status[i] != 0) ++valid_pt_R;

				if (feature_errors[i] > 50) {
					//feature_found[i]���� 0�� ������ �Ǹ� �������� �߰����� ����
					//feature_errors[i] ���� �����Ӱ� ���������� ������ �Ÿ��� 550�� ������ ���ܷ� ó��
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


		//draw result, ȭ��ǥ ������� �׸��� ���� ������ �Ÿ��� 3�� ���̷� ȭ��ǥ �׷���
		for (int i = 0; i < corner_count; i++) {
			if (status[i] == 0) continue; //valid �� ���� �ƴϸ� �Ѿ
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

			q.x = (int)(p.x - 3 * arrow_length * cos(angle));  //3�� ���̷� ȭ��ǥ �׷���
			q.y = (int)(p.y - 3 * arrow_length * sin(angle));

			//������������ ȭ��ǥ�� �׸��� �ʴ´�.
			if ((arrow_length < 3) | (arrow_length > 40)) continue; //ȭ��ǥ ���̰� �ʹ�ª�ų� ��� �ȱ׸���.

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
		//���� �� ó�� ���� ���ݿ��� ����
		cvCopy(src_grey, pre_src_grey);
		cvCopy(pyramid, prev_pyramid);
		//���� ã�� �� ��ǥ ����
		for (int i = 0; i < corner_count; ++i) pre_feature_points[i] = corners[i];
		pre_num_of_pts = corner_count;
		//cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //�̷� ���� ����.
		//cvPutText(result_view, s_output_result, cvPoint(width - 150, height - 20), &font, cvScalar(0, 255, 0));   //cvPoint�� ���� ���� ��ġ ����(uv)

		// Ʈ��ŷ ������
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
		//�����ǥ���
		cout << tracking_points[tracking_cou].x << " " << tracking_points[tracking_cou].y << endl;

		outFile << tracking_points[tracking_cou].x << " " << tracking_points[tracking_cou].y << endl;

		//cout << "���� ������" << endl;
		tracking_cou++;

		// Ʈ��ŷ ���׸���
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
		//if (waitKey(100) == 27) break; // 100ms -> �ƽ�Ű�ڵ� �� 27 ��, ESC ������ ���� - break

									   ////��ī��-ī���� �˰���
									   //char feature_found[MAX_CORNERS];
									   //float feature_errors[MAX_CORNERS];
									   //CvSize pyr_sz = cvSize(imgA->width + 8, imgB->height / 3);

									   //IplImage* pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
									   //IplImage* pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

									   //CvPoint2D32f* cornersB = new CvPoint2D32f[MAX_CORNERS];

									   ////������ �ڳ�(cornerA)�� ������ -> �̵��� ������ ��ġ�� cornerB�� ����ȴ�.
									   //cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, cornersA, cornersB, corner_count,
									   // cvSize(WINSIZE, WINSIZE), 5, feature_found, feature_errors,
									   // cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3), 0);

									   //for (int i = 0; i<corner_count; i++) {
									   // if (feature_found[i] == 0 || feature_errors[i] > 550) {
									   //  //feature_found[i]���� 0�� ������ �Ǹ� �������� �߰����� ����
									   //  //feature_errors[i] ���� �����Ӱ� ���������� ������ �Ÿ��� 550�� ������ ���ܷ� ó��
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
