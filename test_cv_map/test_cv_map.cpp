// test_cv_map.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <time.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace cv;
using namespace std;

int equ_h = 512, equ_w = 1024;
int Hd = 320, Wd = 320;
int THETA = 0, PHI = 0, FOV = 90;

Mat map_x = Mat::zeros(Hd, Wd, CV_32FC1);
Mat map_y = Mat::zeros(Hd, Wd, CV_32FC1);
Mat Arr3d = Mat::zeros(Hd, Wd, CV_32FC3);
Mat R, Rx, Ry, Rz;

typedef Point3_<float> Pixel;

void proc_px(Pixel& pixel) {
	Mat XYZ = (cv::Mat_<float>(3, 1) << pixel.x, pixel.y, pixel.z);
	XYZ = R * XYZ;
	pixel.x = XYZ.at<float>(0, 0);
	pixel.y = XYZ.at<float>(1, 0);
	pixel.z = XYZ.at<float>(2, 0);
}

struct Operator {
	void operator()(Pixel& pixel, const int* position) const {
		proc_px(pixel);
	}
};

void tic(double& t) { t = (double)getTickCount(); }
double toc(double& t) {	return ((double)getTickCount() - t) / getTickFrequency() * 1000; }

int main()
{
	double t;
	float x_ = 0, y_ = 0, z_ = 0, r;
	float wFOV, hFOV, w_len, h_len;
	float beta, gamma;

	Mat frame, outFrame;

	frame = cv::imread("P_1024.jpg");

	wFOV = FOV;
	hFOV = Hd * 1.0 / Wd * wFOV;

	w_len = 2 * tan(wFOV * CV_PI / 360.0);
	h_len = 2 * tan(hFOV * CV_PI / 360.0);

	for (int x{ 0 }; x < Wd; x++) {
		for (int y{ 0 }; y < Hd; y++) {
			x_ = 1;
			y_ = (x * 1.0 / Wd - 0.5) * w_len;
			z_ = (y * 1.0 / Hd - 0.5) * h_len;

			r = sqrt(x_ * x_ + y_ * y_ + z_ * z_);

			Arr3d.at<Vec3f>(y, x)[0] = x_ / r;
			Arr3d.at<Vec3f>(y, x)[1] = y_ / r;
			Arr3d.at<Vec3f>(y, x)[2] = z_ / r;
		}
	}

	//cout << Arr3d << "\n\n";

	while (1) {
		tic(t);

		map_x = cv::Scalar(0, 0, 0);
		map_y = cv::Scalar(0, 0, 0);

		x_ = 0, y_ = 0, z_ = 0;

		beta = -PHI * CV_PI / 180.0;
		gamma = THETA * CV_PI / 180.0;
		Rx = (Mat_<float>(3, 3) << 1, 0, 0, 0, cos(0), -1 * sin(0), 0, sin(0), cos(0));
		Ry = (Mat_<float>(3, 3) << cos(beta), 0, -1 * sin(beta), 0, 1, 0, sin(beta), 0, cos(beta));
		Rz = (Mat_<float>(3, 3) << cos(gamma), -1 * sin(gamma), 0, sin(gamma), cos(gamma), 0, 0, 0, 1);
		R = Rz * Ry * Rx;

		//cout << R << "\n\n";

		Arr3d.forEach<Pixel>(Operator());

		float lon, lat;
		for (size_t x{ 0 }; x < Wd; x++) {
			for (size_t y{ 0 }; y < Hd; y++) {
				x_ = Arr3d.at<Vec3f>(y, x)[0];
				y_ = Arr3d.at<Vec3f>(y, x)[1];
				z_ = Arr3d.at<Vec3f>(y, x)[2];

				lon = (std::asin(z_) / CV_PI + 0.5) * equ_h;
				lat = (std::atan2(y_, x_ + 0.01) / (2 * CV_PI) + 0.5) * equ_w;

				map_x.at<float>(y, x) = lat;
				map_y.at<float>(y, x) = lon;
			}
		}

		cv::remap(frame, outFrame, map_x, map_y, 0, 2);
		cout << "time: " << toc(t) << " ms" << endl;

		cv::imshow("persp", outFrame);
		int key = cv::waitKeyEx(0);
		if (key == 2490368) {  //up
			PHI = PHI + 10;
			if (PHI >= 90) PHI = 90;
		}
		else if (key == 2621440) {  //down
			PHI = PHI - 10;
			if (PHI <= -90) PHI = -90;
		}
		else if (key == 2424832) {  //left
			THETA = THETA - 10;
		}
		else if (key == 2555904) {  //reight
			THETA = THETA + 10;
		}
		else if (key == 27 || key == 32) {  //ESC Space
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
