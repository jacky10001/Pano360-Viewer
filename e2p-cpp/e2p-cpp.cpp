// e2p-cpp.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

cv::Mat frame, outFrame;

int THETA = 0, PHI = 0, FOV = 90;
int Hd = 320, Wd = 320;
cv::Mat map_x = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat map_y = cv::Mat::zeros(Hd, Wd, CV_32FC1);
cv::Mat R, XYZ, Rx, Ry, Rz;

int equ_h, equ_w;
float wFOV, hFOV;
float w_len, h_len;
float x_ = 0, y_ = 0, z_ = 0, r;

int main()
{
	frame = cv::imread("images/P_1024.jpg");

	equ_h = frame.rows;
	equ_w = frame.cols;

	wFOV = FOV;
	hFOV = Hd * 1.0 / Wd * wFOV;

	w_len = 2 * std::tan(wFOV * CV_PI / 360.0);
	h_len = 2 * std::tan(hFOV * CV_PI / 360.0);

	//--------------------------------------------------------------------------------------------------------------------
	clock_t t1, t2;
	double t_cost;

	float beta, gamma;
	float lat, lon;
	while (1) {
		map_x = cv::Scalar(0, 0, 0);
		map_y = cv::Scalar(0, 0, 0);

		x_ = 0, y_ = 0, z_ = 0;
		XYZ = (cv::Mat_<float>(3, 1) << x_, y_, z_);

		beta = -PHI * CV_PI / 180.0;
		gamma = THETA * CV_PI / 180.0;
		std::cout << beta << " " << gamma << "\n\n";
		Rx = (cv::Mat_<float>(3, 3) << 1, 0, 0, 0, std::cos(0), -1 * std::sin(0), 0, std::sin(0), std::cos(0));
		Ry = (cv::Mat_<float>(3, 3) << std::cos(beta), 0, -1 * std::sin(beta), 0, 1, 0, std::sin(beta), 0, std::cos(beta));
		Rz = (cv::Mat_<float>(3, 3) << std::cos(gamma), -1 * std::sin(gamma), 0, std::sin(gamma), std::cos(gamma), 0, 0, 0, 1);
		R = Rz * Ry * Rx;

		std::cout << Rx << "\n\n";
		std::cout << Ry << "\n\n";
		std::cout << Rz << "\n\n";
		std::cout << R << "\n\n";

		t1 = clock();
		for (size_t x{ 0 }; x < Wd; x++) {
			for (size_t y{ 0 }; y < Hd; y++) {
				x_ = 1;
				y_ = (x * 1.0 / Wd - 0.5) * w_len;
				z_ = (y * 1.0 / Hd - 0.5) * h_len;

				r = std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);

				XYZ.at<float>(0, 0) = x_ / r;
				XYZ.at<float>(1, 0) = y_ / r;
				XYZ.at<float>(2, 0) = z_ / r;

				//std::cout << "\n\n------------------------------------------\n";
				//std::cout << XYZ << "\n-----------------------\n";
				XYZ = R * XYZ;
				//std::cout << XYZ << "\n";
				//std::cout << "\n------------------------------------------\n\n";

				x_ = XYZ.at<float>(0, 0);
				y_ = XYZ.at<float>(1, 0);
				z_ = XYZ.at<float>(2, 0);

				lon = (std::asin(z_) / CV_PI + 0.5) * equ_h;
				lat = (std::atan2(y_, x_ + 0.01) / (2 * CV_PI) + 0.5) * equ_w;

				map_x.at<float>(y, x) = lat;
				map_y.at<float>(y, x) = lon;
			}
		}
		cv::remap(frame, outFrame, map_x, map_y, 0, 2);

		t2 = clock();
		t_cost = ((double)(t2 - t1)) / CLOCKS_PER_SEC;
		//--------------------------------------------------------------------------------------------------------------------

		cv::imshow("persp", outFrame);
		int key = cv::waitKeyEx(0);
		std::cout << "key: " << key << "\n";
		std::cout << "time: " << t_cost << "\n";
		if (key == 2490368) {  //up
			PHI = PHI + 10;
			if (PHI >= 90) PHI = 90;
		}
		if (key == 2621440) {  //down
			PHI = PHI - 10;
			if (PHI <= -90) PHI = -90;
		}
		if (key == 2424832) {  //left
			THETA = THETA - 10;
		}
		if (key == 2555904) {  //reight
			THETA = THETA + 10;
		}
		if (key == 27 || key == 32) {  //ESC Space
			break;
		}
	}
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
