#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <stdio.h>
#include <string.h>
#include <windows.h>

using namespace std;
using namespace cv;

int ct = 0, c = 1;
char tipka, filename[100], filename2[100];

Mat frameL, frameR;
Mat grayL, grayR;
Mat resultL, resultR;
Mat disp, disp8;

VideoCapture capL, capR;

int thresh = 200, thresh2 = 200;
int brightness1 = 0, brightness2 = 0;
int contrast1 = 1, contrast2 = 1;

int max_brightness = 200;
int max_contrast = 5;
int max_thresh = 255;

int xL[50], xR[50], disparity[50];
int centerL[50], centerR[50];
vector<Point2f> corners;
vector<vector<Point> > contours;

Point CenterL, CenterR;

const char* source_window1 = "Source image1";
const char* corners_window1 = "Corners detected1";
const char* hsv_window1 = "HSV image1";

const char* source_window2 = "Source image2";
const char* corners_window2 = "Corners detected2";
const char* hsv_window2 = "HSV image2";

int maxCorners1 = 14;
int maxCorners2 = 14;
int maxTrackbar = 10;

void process(int maxCorners, Mat src, int contrast, int brightness, const char* window1, const char* window2, int index)
{
	Mat gray;
	Mat srcCopy;


	srcCopy = src.clone();
	srcCopy.convertTo(srcCopy, -1, contrast, brightness); //increase the contrast by 2
	
	cvtColor(srcCopy, gray, COLOR_BGR2GRAY);

	maxCorners = MAX(maxCorners, 1);
	double qualityLevel = 0.01;
	double minDistance = 10;
	int blockSize = 3, gradientSize = 3;
	bool useHarrisDetector = false;
	double k = 0.04;
	goodFeaturesToTrack(gray,
		corners,
		maxCorners,
		qualityLevel,
		minDistance,
		Mat(),
		blockSize,
		useHarrisDetector,
		k);
	int radius = 2;
	for (size_t i = 0; i < corners.size(); i++)
	{
		circle(srcCopy, corners[i], radius, Scalar(0, 0, 255), 2, 8, 0);
		if (index == 1) {
			xL[i] = (int)corners[i].x;
		}
		else if (index == 2) {
			xR[i] = (int)corners[i].x;
		}

		//fprintf(stderr, "pic[%2d] : [%2d] X[%3d] Y[%3d] \n", index, i, (int)corners[i].x, (int)corners[i].y)
	}

	namedWindow(window1);
	imshow(window1, srcCopy);
}

void process_moment(Mat src, int thresh, const char* window, int index)
{
	Mat canny_output;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(src, canny_output, thresh, thresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	/// Draw contours
	//Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	Mat drawing = src;
	for (int i = 0; i< contours.size(); i++)
	{
		drawContours(drawing, contours, i, Scalar(0, 0, 255), 2, 8, hierarchy, 0, Point());
		circle(drawing, mc[i], 4, Scalar(0, 0, 255), -1, 8, 0);
	}

	/// Show in a window
	namedWindow(window);
	imshow(window, drawing);

	/// Calculate the area with the moments 00 and compare with the result of the OpenCV function
	for (int i = 0; i< contours.size(); i++)
	{
		drawContours(drawing, contours, i, Scalar(0, 0, 255), 2, 8, hierarchy, 0, Point());
		circle(drawing, mc[i], 4, Scalar(0, 0, 255), -1, 8, 0);
		if (index == 1) {
			CenterL.x = (int)mc[i].x;
			CenterL.y = (int)mc[i].y;
		}
		else if (index == 2) {
			CenterR.x = (int)mc[i].x;
			CenterR.y = (int)mc[i].y;
		}
	}
}


int main(int argc, char** argv)
{
	Mat left_for_matcher, right_for_matcher;

	VideoCapture capL(0);
	VideoCapture capR(1);

	if (!(capL.isOpened() && capR.isOpened())) {
		cerr << "Unable to open the cameras " << endl;
		capL.release(); capR.release();
		return -1;
	}

	Size frameSize(320, 240);
	double fps = 15;

	capL.set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
	capL.set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);
	capL.set(CV_CAP_PROP_FPS, fps); //desired  FPS

	capR.set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
	capR.set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);
	capR.set(CV_CAP_PROP_FPS, fps); //desired  FPS


	for (;;) {
			
		capL >> frameL;
		capR >> frameR;

		cvtColor(frameL, grayL, CV_BGR2GRAY);
		cvtColor(frameR, grayR, CV_BGR2GRAY);
		
		
		//for corner object
		//namedWindow("control");
		//createTrackbar("maxCorners1 : ", "control", &maxCorners1, maxTrackbar);
		//createTrackbar("Brightness1: ", "control", &brightness1, max_brightness);
		//createTrackbar("Contrast1  : ", "control", &contrast1, max_contrast);
		//createTrackbar("maxCorners2 : ", "control", &maxCorners2, maxTrackbar);
		//createTrackbar("Brightness2: ", "control", &brightness2, max_brightness);
		//createTrackbar("Contrast2  : ", "control", &contrast2, max_contrast);

		//if (contrast1 < 0) contrast1 = 0;
		//if (contrast2 < 0) contrast2 = 0;

		//process(maxCorners1, frameL, contrast1, brightness1, source_window1, hsv_window1, 1);
		//process(maxCorners2, frameR, contrast2, brightness2, source_window2, hsv_window2, 2);

		//for (size_t i = 0; i < corners.size(); i++)
		//{
		//	disparity[i] = abs((int)xL[i] - (int)xR[i]);
		//	printf("Corner : %3d || XL : [%3d] || XR : [%3d] || disparity : [%3d] \n", i, xL[i], xR[i],disparity[i]);
		//}
		
		//for search contour ball
		process_moment(frameL, thresh, source_window1, 1);
		process_moment(frameR, thresh, source_window2, 2);

		Point CenterLeft(CenterL.x, CenterL.y);
		Point CenterRight(CenterR.x, CenterR.y);

		int disparity_c = abs((int)CenterL.x - (int)CenterR.x);
		printf("CenterLeft : %3d || CenterRight : %3d || disparity : [%3d]\n", CenterL.x, CenterR.x, disparity_c);

		//disparity map
		Size imagesize = frameL.size();
		Mat disparity_left = Mat(imagesize.height, imagesize.width, CV_16S);
		Mat disparity_right = Mat(imagesize.height, imagesize.width, CV_16S);
		Mat g1, g2, disp, disp8;

		cvtColor(frameL, g1, COLOR_BGR2GRAY);
		cvtColor(frameR, g2, COLOR_BGR2GRAY);

		Ptr<cv::StereoBM> sbm = cv::StereoBM::create(0, 21);

		sbm->setDisp12MaxDiff(1);
		sbm->setNumDisparities(112);
		sbm->setSpeckleRange(8);
		sbm->setSpeckleWindowSize(0);
		sbm->setUniquenessRatio(0);
		sbm->setTextureThreshold(507);
		sbm->setMinDisparity(-39);
		sbm->setPreFilterCap(61);
		sbm->setPreFilterSize(5);
		sbm->compute(g1, g2, disparity_left);

		normalize(disparity_left, disp8, 0, 255, CV_MINMAX, CV_8U);

		imshow("Disparity Map", disp8);

		if (waitKey(5) >= 0)
			break;

	}
	return 0;
	
}

