#include <stdio.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <stdlib.h>
#include <iostream>
using namespace std;

int h_bins = 16, s_bins = 8;//ֱ��ͼ������
int hist_size[] = { h_bins, s_bins };
float total = 0;
float hist1[128];
float hist2[128];
IplImage* CurrentKeyFrame = NULL;

//������ɫֱ��ͼ����һ��ͼƬתΪһά��ɫֱ��ͼ����
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

	/** ����ͼ��ת����HSV��ɫ�ռ� */
	cvCvtColor(src, hsv, CV_BGR2HSV);
	cvCvtPixToPlane(hsv, h_plane, s_plane, v_plane, 0);

	/** ����ֱ��ͼ����ά, ÿ��ά���Ͼ��� */
	CvHistogram * hist = cvCreateHist(2, hist_size, CV_HIST_ARRAY, ranges, 1);

	/** ����H,S����ƽ������ͳ��ֱ��ͼ */
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


//�Ƚ�ֱ��ͼ��totalΪ���Ĳ��
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
	//����IplImageָ��
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


	//��������
	cvNamedWindow("video", 1);
	//ʹ������������
	cvMoveWindow("video", 30, 0);


	//����Ƶ�ļ�
	if (!(pCapture = cvCaptureFromFile("video.mp4")))
	{
		fprintf(stderr, "Error: Can not open video file video.mp4\n");
		return -2;
	}

	//��֡��ȡ��Ƶ
	while (pFrame = cvQueryFrame(pCapture))
	{
		nFrmNum++;

		itoa(nFrmNum, PicName, 10);
		//sprintf(filename,   "%s%c",   PicName,   pngfile);
		strcat(PicName, pngfile);
		
		//����ǵ�һ֡����Ҫ�����ڴ棬����ʼ��
		if (nFrmNum == 1)
		{
			pBkImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
			cvSaveImage(PicName, pFrame);//��ͼ��д���ļ�
			CurrentKeyFrame = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 3);
			cvCopy(pFrame, CurrentKeyFrame, NULL);
		}
		else
		{
			//cvSaveImage(PicName, pFrame );//��ͼ��д���ļ�
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

			//��ʾͼ��
			cvShowImage("video", pFrame);
			if (cvWaitKey(200) >= 0)
				break;
		}
	}

	//ע������
	cvDestroyWindow("video");
	cvDestroyWindow("background");
	cvDestroyWindow("foreground");

	//�ͷ�ͼ��;���
	cvReleaseImage(&pFrImg);
	cvReleaseImage(&pBkImg);

	cvReleaseMat(&pFrameMat);
	cvReleaseMat(&pFrMat);
	cvReleaseMat(&pBkMat);

	cvReleaseCapture(&pCapture);

	return 0;
}