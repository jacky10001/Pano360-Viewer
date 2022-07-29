// e2p-cpp.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

cv::Mat frame;
cv::Mat outFrame;

int mx1 = 0, my1 = 0, mx2 = 0, my2 = 0, m_sta = 0;
int THETA = 120;
int PHI = 0;
float FOV = 90;
int Hd = 320, Wd = 320;
cv::Mat map_x = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat map_y = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat R, XYZ, Rx, Ry, Rz;

int equ_h;
int equ_w;
float equ_cx;
float equ_cy;

float wFOV;
float hFOV;

float c_x;
float c_y;

float w_len, h_len;

float x_ = 0, y_ = 0, z_ = 0, r;

cv::Mat X_Arr = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat Y_Arr = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat Z_Arr = cv::Mat::zeros(Hd, Wd, CV_32FC1);

int main()
{
	frame = cv::imread("images/P_1024.jpg");

	equ_h = frame.rows;
	equ_w = frame.cols;
	equ_cx = equ_w / 2.0;
	equ_cy = equ_h / 2.0;

	wFOV = FOV;
	hFOV = Hd * 1.0 / Wd * wFOV;

	c_x = Wd / 2.0;
	c_y = Hd / 2.0;

	w_len = 2 * std::tan(wFOV * CV_PI / 360.0);
	h_len = 2 * std::tan(hFOV * CV_PI / 360.0);

	for (size_t x{ 0 }; x < Wd; x++) {
		for (size_t y{ 0 }; y < Hd; y++) {
			x_ = 1;
			y_ = (x * 1.0 / Wd - 0.5) * w_len;
			z_ = (y * 1.0 / Hd - 0.5) * h_len;

			r = std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);

			X_Arr.at<float>(y, x) = x_ / r;
			Y_Arr.at<float>(y, x) = y_ / r;
			Z_Arr.at<float>(y, x) = z_ / r;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------
	clock_t t1, t2;
	double t_cost;

	map_x = cv::Scalar(0, 0, 0);
	map_y = cv::Scalar(0, 0, 0);

	x_ = 0, y_ = 0, z_ = 0;
	XYZ = (cv::Mat_<float>(3, 1) << x_, y_, z_);

	float beta = -PHI * CV_PI / 180.0;
	float gamma = THETA * CV_PI / 180.0;

	Rx = (cv::Mat_<float>(3, 3) << 1, 0, 0, 0, std::cos(0), -1 * std::sin(0), 0, std::sin(0), std::cos(0));
	Ry = (cv::Mat_<float>(3, 3) << std::cos(beta), 0, -1 * std::sin(beta), 0, 1, 0, std::sin(beta), 0, std::cos(beta));
	Rz = (cv::Mat_<float>(3, 3) << std::cos(gamma), -1 * std::sin(gamma), 0, std::sin(gamma), std::cos(gamma), 0, 0, 0, 1);
	R = Rz * Ry * Rx;

	//std::cout << Rx << "\n";
	//std::cout << Rx.size() << "\n";

	//std::cout << Ry << "\n";
	//std::cout << Ry.size() << "\n";

	//std::cout << Rz << "\n";
	//std::cout << Rz.size() << "\n";

	//std::cout << R << "\n";
	//std::cout << R.size() << "\n";

	float lat, lon;

	t1 = clock();
	for (size_t x{ 0 }; x < Wd; x++) {
		for (size_t y{ 0 }; y < Hd; y++) {
			XYZ.at<float>(0, 0) = X_Arr.at<float>(y, x);
			XYZ.at<float>(1, 0) = Y_Arr.at<float>(y, x);
			XYZ.at<float>(2, 0) = Z_Arr.at<float>(y, x);

			//std::cout << XYZ << "\n";
			XYZ = R * XYZ;
			//std::cout << XYZ << "\n";
			x_ = XYZ.at<float>(0, 0);
			y_ = XYZ.at<float>(1, 0);
			z_ = XYZ.at<float>(2, 0);

			lon = (std::asin(z_) / CV_PI + 0.5) * equ_h;
			lat = (std::atan2(y_, x_ + 0.01) / (2 * CV_PI) + 0.5) * equ_w;

			map_x.at<float>(y, x) = lat;
			map_y.at<float>(y, x) = lon;
		}
	}

	t2 = clock();
	t_cost = ((double)(t2 - t1)) / CLOCKS_PER_SEC;
	printf("\n\ntime: %f\n\n", t_cost);

	cv::remap(frame, outFrame, map_x, map_y, 0, 2);
	//--------------------------------------------------------------------------------------------------------------------

	cv::imshow("persp", outFrame);
	cv::waitKey(0);
	cv::destroyAllWindows();
	return 0;
}


// 執行程式: Ctrl + F5 或 [偵錯] > [啟動但不偵錯] 功能表
// 偵錯程式: F5 或 [偵錯] > [啟動偵錯] 功能表

// 開始使用的提示: 
//   1. 使用 [方案總管] 視窗，新增/管理檔案
//   2. 使用 [Team Explorer] 視窗，連線到原始檔控制
//   3. 使用 [輸出] 視窗，參閱組建輸出與其他訊息
//   4. 使用 [錯誤清單] 視窗，檢視錯誤
//   5. 前往 [專案] > [新增項目]，建立新的程式碼檔案，或是前往 [專案] > [新增現有項目]，將現有程式碼檔案新增至專案
//   6. 之後要再次開啟此專案時，請前往 [檔案] > [開啟] > [專案]，然後選取 .sln 檔案
