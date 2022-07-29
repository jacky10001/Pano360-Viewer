// test_cv_foreach.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
//#include "pch.h"
#include <opencv2/opencv.hpp>

// Use cv and std namespaces
using namespace cv;
using namespace std;

// Define a pixel 定義Pixel結構
typedef Point3_<uint8_t> Pixel;

/**
 * @brief tic is called to start timer 開始函數運行時間計算
 *
 * @param t
 */
void tic(double& t)
{
	t = (double)getTickCount();
}

/**
 * @brief toc is called to end timer 結束函數運行時間計算
 *
 * @param t
 * @return double 返回值運行時間ms
 */
double toc(double& t)
{
	return ((double)getTickCount() - t) / getTickFrequency() * 1000;
}

/**
 * @brief 閾值分割
 *
 * @param pixel
 */
void complicatedThreshold(Pixel& pixel)
{
	//x,y,z分別代表三個通道的值
	if (pow(double(pixel.x) / 10, 2.5) > 100)
	{
		pixel.x = 255;
		pixel.y = 255;
		pixel.z = 255;
	}
	else
	{
		pixel.x = 0;
		pixel.y = 0;
		pixel.z = 0;
	}
}

/**
 * @brief Parallel execution with function object. 並行處理函數結構體
 *
 */
struct Operator
{
	//處理函數
	void operator()(Pixel& pixel, const int* position) const
	{
		// Perform a simple threshold operation
		complicatedThreshold(pixel);
	}
};

int main()
{
	// Read image 讀圖
	Mat image = imread("butterfly.jpg");

	// Scale image 30x 將圖像擴大爲30倍，長寬都變大30倍
	resize(image, image, Size(), 30, 30);

	// Print image size 打印圖像尺寸
	cout << "Image size " << image.size() << endl;

	// Number of trials 測試次數
	int numTrials = 5;

	// Print number of trials 測試次數
	cout << "Number of trials : " << numTrials << endl;

	// Make two copies 圖像複製
	Mat image1 = image.clone();
	Mat image2 = image.clone();
	Mat image3 = image.clone();

	// Start timer 時間函數,單位爲ms
	double t;
	//開始計算時間
	tic(t);

	//循環測試numTrials次
	for (int n = 0; n < numTrials; n++)
	{
		// Naive pixel access at方法直接讀取數據
		// Loop over all rows 遍歷行
		for (int r = 0; r < image.rows; r++)
		{
			// Loop over all columns 遍歷列
			for (int c = 0; c < image.cols; c++)
			{
				// Obtain pixel at (r, c) 直接訪問像素數據
				Pixel pixel = image.at<Pixel>(r, c);
				// Apply complicatedTreshold 閾值分割
				complicatedThreshold(pixel);
				// Put result back 保存結果
				image.at<Pixel>(r, c) = pixel;
			}
		}
	}
	//計算函數執行時間
	cout << "Naive way: " << toc(t) << endl;

	// Start timer
	tic(t);

	// image1 is guaranteed to be continous, but
	// if you are curious uncomment the line below
	//需要判斷圖像連續存儲，1表示圖像連續，0不連續
	//cout << "Image 1 is continous : " << image1.isContinuous() << endl;

	//通過指針訪問像素點，類似YUV圖像處理，前提圖像存儲是連續的
	for (int n = 0; n < numTrials; n++)
	{
		// Get pointer to first pixel
		//初始指針
		Pixel* pixel = image1.ptr<Pixel>(0, 0);

		// Mat objects created using the create method are stored
		// in one continous memory block.
		// 訪問像素點位置
		const Pixel* endPixel = pixel + image1.cols * image1.rows;

		// Loop over all pixels
		for (; pixel != endPixel; pixel++)
		{
			complicatedThreshold(*pixel);
		}
	}
	cout << "Pointer Arithmetic " << toc(t) << endl;

	tic(t);
	//forEach遍歷像素
	for (int n = 0; n < numTrials; n++)
	{
		image2.forEach<Pixel>(Operator());
	}
	cout << "forEach : " << toc(t) << endl;

	//C++版本
	cout << __cplusplus << endl;

	//使用C++11 lambda特性
	tic(t);
	for (int n = 0; n < numTrials; n++)
	{
		// Parallel execution using C++11 lambda.
		image3.forEach<Pixel>([](Pixel& pixel, const int* position) -> void {
			complicatedThreshold(pixel);
			});
	}
	cout << "forEach C++11 : " << toc(t) << endl;

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
