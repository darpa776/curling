///////////////////////////////////////////////////////////////////////
// OpenCV tracking example.
// Written by darkpgmr (http://darkpgmr.tistory.com), 2013

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "tracker_opencv.h"

using namespace std;

tracker_opencv::tracker_opencv(void)
{
}

tracker_opencv::~tracker_opencv(void)
{
}

void tracker_opencv::init(Mat img, Rect rc)
{
	Mat mask = Mat::zeros(rc.height, rc.width, CV_8U);
	ellipse(mask, Point(rc.width/2, rc.height/2), Size(rc.width/2, rc.height/2), 0, 0, 360, 255, CV_FILLED);

	if(img.channels()<=1)
	{
		float vrange[] = {0,256};
		const float* phranges = vrange;
		Mat roi(img, rc);
		calcHist(&roi, 1, 0, mask, m_model, 1, &m_param.hist_bins, &phranges);
	}
	else if(m_param.color_model==CM_GRAY)
	{
		Mat gray;
		cvtColor(img, gray, CV_BGR2GRAY);

		float vrange[] = {0,256};
		const float* phranges = vrange;
		Mat roi(gray, rc);
		calcHist(&roi, 1, 0, mask, m_model, 1, &m_param.hist_bins, &phranges);
	}
	else if(m_param.color_model==CM_HUE)
	{
		Mat hsv;
		cvtColor(img, hsv, CV_BGR2HSV);

		float hrange[] = {0,180};
		const float* phranges = hrange;
		int channels[] = {0};
		Mat roi(hsv, rc);
		calcHist(&roi, 1, channels, mask, m_model, 1, &m_param.hist_bins, &phranges);
	}
	else if(m_param.color_model==CM_RGB)
	{
		float vrange[] = {0,255};
		const float* ranges[] = {vrange, vrange, vrange};	// B,G,R
		int channels[] = {0, 1, 2};
		int hist_sizes[] = {m_param.hist_bins, m_param.hist_bins, m_param.hist_bins};
		Mat roi(img, rc);
		calcHist(&roi, 1, channels, mask, m_model3d, 3, hist_sizes, ranges);
	}
	else if(m_param.color_model==CM_HSV)
	{
		Mat hsv;
		cvtColor(img, hsv, CV_BGR2HSV);

		float hrange[] = {0,180};
		float vrange[] = {0,255};
		const float* ranges[] = {hrange, vrange, vrange};	// hue, saturation, brightness
		int channels[] = {0, 1, 2};
		int hist_sizes[] = {m_param.hist_bins, m_param.hist_bins, m_param.hist_bins};
		Mat roi(hsv, rc);
		calcHist(&roi, 1, channels, mask, m_model3d, 3, hist_sizes, ranges);
	}

	m_rc = rc;
}

