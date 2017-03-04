#include <stdio.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <stdlib.h>
#include <iostream>
using namespace std;

int h_bins = 16, s_bins = 8;//直方图方块数
int hist_size[] = { h_bins, s_bins };
float total = 0;
float hist1[128];
float hist2[128];
IplImage* CurrentKeyFrame = NULL;

//创建颜色直方图，将一幅图片转为一维颜色直方图数组
void LoadHist(IplImage* frame, float collect[])
{
	IplImage* src = frame;
	IplImage* hsv = cvCreateImage(cvGetSize(src), 8, 3);
	IplImage* h_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* s_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* planes[] = { h_plane, s_plane };

	float h_ranges[] = { 0, 180 };
	float s_ranges[] = { 0, 255 };
	float* ranges[] = { h_ranges, s_ranges };

	/** 输入图像转换到HSV颜色空间 */
	cvCvtColor(src, hsv, CV_BGR2HSV);
	cvCvtPixToPlane(hsv, h_plane, s_plane, v_plane, 0);

	/** 创建直方图，二维, 每个维度上均分 */
	CvHistogram * hist = cvCreateHist(2, hist_size, CV_HIST_ARRAY, ranges, 1);

	/** 根据H,S两个平面数据统计直方图 */
	cvCalcHist(planes, hist, 0, 0);
	int count = 0;
	for (int h = 0; h < h_bins; h++)
	{

		for (int s = 0; s < s_bins; s++)
		{
			float bin_val = cvQueryHistValue_2D(hist, h, s);
			collect[count] = bin_val;
			count++;
			//cout<<bin_val<<endl;
		}

	}

}


//比较直方图，total为最后的差别
void HistCompare(float hista[], float histb[])
{
	for (int i = 0; i < h_bins * s_bins; i++)
	{
		if (hista[i] >= histb[i])
		{
			total = total + hista[i] - histb[i];
		}
		else
		{
			total = total - hista[i] + histb[i];
		}
	}
}

int main(int argc, char** argv)
{
	//声明IplImage指针
	IplImage* pFrame = NULL;
	IplImage* pFrImg = NULL;
	IplImage* pBkImg = NULL;

	CvMat* pFrameMat = NULL;
	CvMat* pFrMat = NULL;
	CvMat* pBkMat = NULL;

	CvCapture* pCapture = NULL;

	int nFrmNum = 0;
	char PicName[20];
	const char *pngfile = ".jpg";


	//创建窗口
	cvNamedWindow("video", 1);
	//使窗口有序排列
	cvMoveWindow("video", 30, 0);


	//打开视频文件
	if (!(pCapture = cvCaptureFromFile("video.mp4")))
	{
		fprintf(stderr, "Error: Can not open video file video.mp4\n");
		return -2;
	}

	//逐帧读取视频
	while (pFrame = cvQueryFrame(pCapture))
	{
		nFrmNum++;

		itoa(nFrmNum, PicName, 10);
		//sprintf(filename,   "%s%c",   PicName,   pngfile);
		strcat(PicName, pngfile);
		
		//如果是第一帧，需要申请内存，并初始化
		if (nFrmNum == 1)
		{
			pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
			cvSaveImage(PicName, pFrame);//把图像写入文件
			CurrentKeyFrame = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 3);
			cvCopy(pFrame, CurrentKeyFrame, NULL);
		}
		else
		{
			//cvSaveImage(PicName, pFrame );//把图像写入文件
			LoadHist(CurrentKeyFrame, hist1);
			LoadHist(pFrame, hist2);
			HistCompare(hist1, hist2);
			if (total >(1.2*pFrame->width * pFrame->height))
			{
				cout << total;
				total = 0;
				cvCopy(pFrame, CurrentKeyFrame, NULL);
				cvSaveImage(PicName, pFrame);
			}
			else
			{
				total = 0;
			}

			//显示图像
			cvShowImage("video", pFrame);
			if (cvWaitKey(200) >= 0)
				break;
		}
	}

	//注销窗口
	cvDestroyWindow("video");
	cvDestroyWindow("background");
	cvDestroyWindow("foreground");

	//释放图像和矩阵
	cvReleaseImage(&pFrImg);
	cvReleaseImage(&pBkImg);

	cvReleaseMat(&pFrameMat);
	cvReleaseMat(&pFrMat);
	cvReleaseMat(&pBkMat);

	cvReleaseCapture(&pCapture);

	return 0;
}