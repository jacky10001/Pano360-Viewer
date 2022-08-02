// test_lut.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#define _CRT_SECURE_NO_WARNINGS

#define PI 3.1415926535897932
#define PI2 6.2831853071795864

#include <time.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace cv;
using namespace std;

#define SIZE 128
float sin_tablePI2[SIZE + 1];
float cos_tablePI2[SIZE + 1];
float tan_tablePI2[SIZE + 1];
float asin_tablePI2[SIZE + 1];
static float sin_unit;
static float cos_unit;
static float tan_unit;
static float asin_unit;
void sin_init()
{
	int i;
	sin_unit = PI2 / SIZE;
	for (i = 1; i <= SIZE; ++i)
		sin_tablePI2[i] = sin(i * PI2 / (SIZE + 1.0));
}
float sinL(float x)
{
	unsigned i;

	x = fmod(x, PI2);
	if (x < 0.0) x += PI2; // 確保為正 
	i = x / sin_unit;       // 算區間

	// 一次插值
	return sin_tablePI2[i] + \
		(x - i * sin_unit) * (sin_tablePI2[i + 1] - sin_tablePI2[i]) / sin_unit;
	// return sin_tablePI2[i]; // 不做內插
}
void cos_init()
{
	int i;
	cos_unit = PI2 / SIZE;
	for (i = 0; i < SIZE; ++i)
		cos_tablePI2[i] = cos(i * PI2 / (SIZE + 1.0));
}
float cosL(float x)
{
	unsigned i;

	x = fmod(x, PI2);
	if (x < 0.0) x += PI2; // 確保為正 
	i = x / cos_unit;       // 算區間

	// 一次插值
	return cos_tablePI2[i] + \
		(x - i * cos_unit) * (cos_tablePI2[i + 1] - cos_tablePI2[i]) / cos_unit;
	// return cos_tablePI2[i]; // 不做內插
}
void tan_init()
{
	int i;
	tan_unit = PI2 / SIZE;
	for (i = 1; i <= SIZE; ++i)
		tan_tablePI2[i] = tan(i * PI2 / (SIZE + 1.0));
}
float tanL(float x)
{
	unsigned i;

	x = fmod(x, PI2);
	if (x < 0.0) x += PI2; // 確保為正 
	i = x / tan_unit;       // 算區間

	// 一次插值
	return tan_tablePI2[i] + \
		(x - i * tan_unit) * (tan_tablePI2[i + 1] - tan_tablePI2[i]) / tan_unit;
	// return tan_tablePI2[i]; // 不做內插
}
void asin_init()
{
	int i;
	asin_unit = 1 / SIZE;
	for (i = 1; i <= SIZE; ++i)
		asin_tablePI2[i] = asin(i / (SIZE + 1.0));
}
float asinL(float x)
{
	unsigned i;

	//x = fmod(x, 360);
	if (x < 0.0) x += 1; // 確保為正 
	i = x / asin_unit;       // 算區間

	// 一次插值
	return asin_tablePI2[i] + \
		(x - i * asin_unit) * (asin_tablePI2[i + 1] - asin_tablePI2[i]) / asin_unit;
	// return asin_tablePI2[i]; // 不做內插
}

Mat frame, outFrame;

int THETA = 0, PHI = 0, FOV = 90;
int Hd = 320, Wd = 320;
Mat map_x = Mat::zeros(Hd, Wd, CV_32FC1);
Mat map_y = Mat::zeros(Hd, Wd, CV_32FC1);
Mat R, XYZ, Rx, Ry, Rz;

int equ_h, equ_w;
float wFOV, hFOV;
float w_len, h_len;
float x_ = 0, y_ = 0, z_ = 0, r;

int main()
{
	clock_t t1, t2;
	double t_cost;
	float beta, gamma;
	float lat, lon;

	sin_init();
	cos_init();
	tan_init();
	asin_init();

	cout << sinL(0 * PI / 180) << "\n\n";
	cout << sinL(90 * PI / 180) << "\n\n";
	cout << cosL(0 * PI / 180) << "\n\n";
	cout << cosL(90 * PI / 180) << "\n\n";
	cout << asinL(0) << "\n\n";
	cout << asinL(1) << "\n\n";

	for (int a = -1; a <= 1; a++) {
		cout << asin(a) << " ";
	}

	//return 0;

	beta = -PHI * CV_PI / 180.0;
	gamma = THETA * CV_PI / 180.0;
	Rx = (Mat_<float>(3, 3) << 1, 0, 0, 0, cosL(0), -1 * sinL(0), 0, sinL(0), cosL(0));
	Ry = (Mat_<float>(3, 3) << cosL(beta), 0, -1 * sinL(beta), 0, 1, 0, sinL(beta), 0, cosL(beta));
	Rz = (Mat_<float>(3, 3) << cosL(gamma), -1 * sinL(gamma), 0, sinL(gamma), cosL(gamma), 0, 0, 0, 1);
	R = Rz * Ry * Rx;

	//cout << Rx << "\n\n";
	//cout << Ry << "\n\n";
	//cout << Rz << "\n\n";
	//cout << R << "\n\n";

	//return 0;

	frame = imread("P_1024.jpg");

	equ_h = frame.rows;
	equ_w = frame.cols;

	wFOV = FOV;
	hFOV = Hd * 1.0 / Wd * wFOV;

	w_len = 2 * tanL(wFOV * CV_PI / 360.0);
	h_len = 2 * tanL(hFOV * CV_PI / 360.0);

	//--------------------------------------------------------------------------------------------------------------------
	while (1) {
		map_x = Scalar(0, 0, 0);
		map_y = Scalar(0, 0, 0);

		x_ = 0, y_ = 0, z_ = 0;
		XYZ = (Mat_<float>(3, 1) << x_, y_, z_);

		beta = -PHI * CV_PI / 180.0;
		gamma = THETA * CV_PI / 180.0;
		Rx = (Mat_<float>(3, 3) << 1, 0, 0, 0, cosL(0), -1 * sinL(0), 0, sinL(0), cosL(0));
		Ry = (Mat_<float>(3, 3) << cosL(beta), 0, -1 * sinL(beta), 0, 1, 0, sinL(beta), 0, cosL(beta));
		Rz = (Mat_<float>(3, 3) << cosL(gamma), -1 * sinL(gamma), 0, sinL(gamma), cosL(gamma), 0, 0, 0, 1);
		R = Rz * Ry * Rx;

		//cout << Rx << "\n\n";
		//cout << Ry << "\n\n";
		//cout << Rz << "\n\n";
		//cout << R << "\n\n";

		t1 = clock();
		for (size_t x{ 0 }; x < Wd; x++) {
			for (size_t y{ 0 }; y < Hd; y++) {
				x_ = 1;
				y_ = (x * 1.0 / Wd - 0.5) * w_len;
				z_ = (y * 1.0 / Hd - 0.5) * h_len;

				r = sqrt(x_ * x_ + y_ * y_ + z_ * z_);

				XYZ.at<float>(0, 0) = x_ / r;
				XYZ.at<float>(1, 0) = y_ / r;
				XYZ.at<float>(2, 0) = z_ / r;

				//cout << "\n\n------------------------------------------\n";
				//cout << XYZ << "\n-----------------------\n";
				XYZ = R * XYZ;
				//cout << XYZ << "\n";
				//cout << "\n------------------------------------------\n\n";

				x_ = XYZ.at<float>(0, 0);
				y_ = XYZ.at<float>(1, 0);
				z_ = XYZ.at<float>(2, 0);

				lon = (asin(z_) / CV_PI + 0.5) * equ_h;
				lat = (atan2(y_, x_ + 0.01) / (2 * CV_PI) + 0.5) * equ_w;

				map_x.at<float>(y, x) = lat;
				map_y.at<float>(y, x) = lon;
			}
		}
		remap(frame, outFrame, map_x, map_y, 0, 2);

		t2 = clock();
		t_cost = ((double)(t2 - t1)) / CLOCKS_PER_SEC;
		//--------------------------------------------------------------------------------------------------------------------

		imshow("persp", outFrame);
		int key = waitKeyEx(0);
		cout << "key: " << key << "\n";
		cout << "time: " << t_cost << "\n";
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
	destroyAllWindows();
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
