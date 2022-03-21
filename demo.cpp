#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include  <ctype.h>
#include <getopt.h>
#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include <iostream>  
#include <time.h>
#include<opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define NSCALE 16 
#define FACENUM	5

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; } 

#define KEYVALLEN 100
 
/*	 删除左边的空格   */
char * l_trim(char * szOutput, const char *szInput)
{
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	for   (NULL; *szInput != '\0' && isspace(*szInput); ++szInput){
		;
	}
	return strcpy(szOutput, szInput);
}
 
/*	 删除右边的空格   */
char *r_trim(char *szOutput, const char *szInput)
{
	char *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	strcpy(szOutput, szInput);
	for(p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p){
		;
	}
	*(++p) = '\0';
	return szOutput;
}
 
/*	 删除两边的空格   */
char * a_trim(char * szOutput, const char * szInput)
{
	char *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	l_trim(szOutput, szInput);
	for(p = szOutput + strlen(szOutput) - 1;p >= szOutput && isspace(*p); --p){
		;
	}
	*(++p) = '\0';
	return szOutput;
}

int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal )
{
	char appname[32],keyname[32];
	char *buf,*c;
	char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
	FILE *fp;
	int found=0; /* 1 AppName 2 KeyName */
	if( (fp=fopen( profile,"r" ))==NULL ){
		printf( "openfile [%s] error [%s]\n",profile,strerror(errno) );
		return(-1);
	}
	fseek( fp, 0, SEEK_SET );
	memset( appname, 0, sizeof(appname) );
	sprintf( appname,"[%s]", AppName );
	while( !feof(fp) && fgets( buf_i, KEYVALLEN, fp )!=NULL ){
		l_trim(buf_o, buf_i);
		if( strlen(buf_o) <= 0 ){
			continue;
		}
		buf = NULL;
		buf = buf_o;
		if( found == 0 ){
			if( buf[0] != '[' ) {
					continue;
			}else if( strncmp(buf,appname,strlen(appname))==0 ){
					found = 1;
					continue;
			}
		}else if( found == 1 ){
			if( buf[0] == '#' ){
					continue;
			}else if ( buf[0] == '[' ) {
					break;
			}else{
				if( (c = (char*)strchr(buf, '=')) == NULL ){
					continue;
				}
				memset( keyname, 0, sizeof(keyname) );
				sscanf( buf, "%[^=|^ |^\t]", keyname );
				if( strcmp(keyname, KeyName) == 0 ){
					sscanf( ++c, "%[^\n]", KeyVal );
					char *KeyVal_o = (char *)malloc(strlen(KeyVal) + 1);
					if(KeyVal_o != NULL){
					memset(KeyVal_o, 0, sizeof(KeyVal_o));
					a_trim(KeyVal_o, KeyVal);
					if(KeyVal_o && strlen(KeyVal_o) > 0)
					strcpy(KeyVal, KeyVal_o);
					free(KeyVal_o);
					KeyVal_o = NULL;
					}
					found = 2;
					break;
				}else{
					continue;
				}
			}
		}
	}
	fclose( fp );
	if( found == 2 ){
		return(0);
	}
	
	else{
		return(-1);
	}
	
}

//时间戳转换为日期格式
void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
{
	time_t tTimeStamp = atoll(timeStamp);
	struct tm* pTm = gmtime(&tTimeStamp);
	strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
}

//图像颜色格式转换
int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen)
{
	offscreen.u32PixelArrayFormat = (unsigned int)format;
	offscreen.i32Width = width;
	offscreen.i32Height = height;
	
	switch (offscreen.u32PixelArrayFormat)
	{
	case ASVL_PAF_RGB24_B8G8R8:
		offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
		offscreen.ppu8Plane[0] = imgData;
		break;
	case ASVL_PAF_I420:
		offscreen.pi32Pitch[0] = width;
		offscreen.pi32Pitch[1] = width >> 1;
		offscreen.pi32Pitch[2] = width >> 1;
		offscreen.ppu8Plane[0] = imgData;
		offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
		offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
		break;
	case ASVL_PAF_NV12:
	case ASVL_PAF_NV21:
		offscreen.pi32Pitch[0] = offscreen.i32Width;
		offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
		offscreen.ppu8Plane[0] = imgData;
		offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
		break;
	case ASVL_PAF_YUYV:
	case ASVL_PAF_DEPTH_U16:
		offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
		offscreen.ppu8Plane[0] = imgData;
		break;
	case ASVL_PAF_GRAY:
		offscreen.pi32Pitch[0] = offscreen.i32Width;
		offscreen.ppu8Plane[0] = imgData;
		break;
	default:
		return 0;
	}
	return 1;
}
//opencv颜色空间转换 图像数据结构引入了步长pi32Pitch的概念。通过引入图像步长能够有效的避免高字节对齐的问题
int ColorSpaceConversionIpl(MInt32 format, IplImage* img, ASVLOFFSCREEN& offscreen)
{
    switch (format)        //原始图像颜色格式
    {
    case ASVL_PAF_I420:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0] >> 1;
        offscreen.pi32Pitch[2] = offscreen.pi32Pitch[0] >> 1;
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height * offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height * offscreen.pi32Pitch[0] * 5 / 4;
        break;
    case ASVL_PAF_YUYV:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        break;
    case ASVL_PAF_NV12:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
        break;
    case ASVL_PAF_NV21:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
        break;
    case ASVL_PAF_RGB24_B8G8R8:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        break;
    case ASVL_PAF_DEPTH_U16:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        break;
    case ASVL_PAF_GRAY:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img->width;
        offscreen.i32Height = img->height;
        offscreen.pi32Pitch[0] = img->widthStep;
        offscreen.ppu8Plane[0] = (MUInt8*)img->imageData;
        break;
    default:
        return 0;
    }
    return 1;
}

int ColorSpaceConversionMat(MInt32 format, cv::Mat img, ASVLOFFSCREEN& offscreen)
{
    switch (format)   //原始图像颜色格式
    {
    case ASVL_PAF_I420:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0] >> 1;
        offscreen.pi32Pitch[2] = offscreen.pi32Pitch[0] >> 1;
        offscreen.ppu8Plane[0] = img.data;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height * offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height * offscreen.pi32Pitch[0] * 5 / 4;
        break;
    case ASVL_PAF_YUYV:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.ppu8Plane[0] = img.data;;
        break;
    case ASVL_PAF_NV12:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[0] = img.data;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
        break;
    case ASVL_PAF_NV21:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[0] = img.data;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
        break;
    case ASVL_PAF_RGB24_B8G8R8:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.ppu8Plane[0] = img.data;
        break;
    case ASVL_PAF_DEPTH_U16:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.ppu8Plane[0] = img.data;
        break;
    case ASVL_PAF_GRAY:
        offscreen.u32PixelArrayFormat = (unsigned int)format;
        offscreen.i32Width = img.cols;
        offscreen.i32Height = img.rows;
        offscreen.pi32Pitch[0] = img.step;
        offscreen.ppu8Plane[0] = img.data;
        break;
    default:
        return 0;
    }
    return 1;
}

//opencv方式裁剪图片
void CutIplImage(IplImage* src, IplImage* dst, int x, int y)
{
    CvSize size = cvSize(dst->width, dst->height);//区域大小
    cvSetImageROI(src, cvRect(x, y, size.width, size.height));//设置源图像ROI
    cvCopy(src, dst); //复制图像
    cvResetImageROI(src);//源图像用完后，清空ROI
}
void cvSetImageROI(IplImage* image){
	
}

//帮助信息
int usage(){
	printf("Usage: facialengine [OPTION]... [FILE]...\n");
	printf("\t -a, --active \t Activation the sdk \n");
	printf("\t -v, --version \t SDK version  \n");
	printf("\t -d, --detect \t Detect photos, there is an absolute path behind it  \n");
}
//激活
int activation (char* appid, char* appkey){
	MRESULT res = ASFActivation(appid, appkey);
	if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res){
		printf("{\"error_code\":%d,\"message\":\"Activation failed\"}\n", res);
	}else{
		printf("{\"error_code\":0,\"message\":\"Activation succeed\"}\n", res);
	}
	
}

//获取版本信息
int getVersion (char* appid, char* appkey){
	MRESULT res = MOK;
	ASF_ActiveFileInfo activeFileInfo = { 0 };
	res = ASFGetActiveFileInfo(&activeFileInfo);
	//SDK版本信息
	const ASF_VERSION version = ASFGetVersion();
	if (res != MOK){
		printf("{\"error_code\":%d,\"message\":\"Get Active file failed\", \"version\":\"%s\", \"BuildDate\":\"%s\"}\n", res,version.Version,version.BuildDate);
	}
	else{
		//这里仅获取了有效期时间，还需要其他信息直接打印即可
		char startDateTime[32];
		timestampToTime(activeFileInfo.startTime, startDateTime, 32);
		//printf("startTime: %s\n", startDateTime);
		char endDateTime[32];
		timestampToTime(activeFileInfo.endTime, endDateTime, 32);
		//printf("endTime: %s\n", endDateTime);
		printf("{\"error_code\":0,\"message\":\"Successful\",\"startTime:\":\"%s\",\"endTime:\":\"%s\", \"version\":\"%s\", \"BuildDate\":\"%s\"}\n", startDateTime,endDateTime,version.Version,version.BuildDate);
	}
	
	
}
//照片人脸检测
int detect(char* appid, char* appkey, char* path){
	IplImage* originalImg  = cvLoadImage(path);
	//图像裁剪，宽度做四字节对齐，若能保证图像是四字节对齐这步可以不用做
	IplImage* img = cvCreateImage(cvSize(originalImg->width - originalImg->width % 4, originalImg->height), IPL_DEPTH_8U, originalImg->nChannels);
	CutIplImage(originalImg, img, 0, 0);
	
	MRESULT res = ASFOnlineActivation(appid, appkey);
	//初始化引擎
	MHandle handle = NULL;
	MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
	res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, NSCALE, FACENUM, mask, &handle);
	if (res != MOK){
		printf("{\"error_code\":%d,\"message\":\"Engine initialization failed\"}\n", res);
	}else{
		/* int Width1 = 640;
		int Height1 = 480; 
		int w = 255;
		int h = 384; 
		int Format1 = ASVL_PAF_RGB24_B8G8R8;
		MUInt8* imageData = (MUInt8*)malloc(h*w*3/2);
		FILE* fp1 = fopen(path, "rb");
		
		if (fp1){
			fread(imageData, 1, h*w*3/2, fp1);	//读NV21裸数据
			fclose(fp1);
			ASVLOFFSCREEN offscreen = { 0 };
			ColorSpaceConversion(w, h, Format1, imageData, offscreen);
			
			ASF_MultiFaceInfo detectedFaces = { 0 };
			ASF_SingleFaceInfo SingleDetectedFaces = { 0 };
			ASF_FaceFeature feature = { 0 };
			ASF_FaceFeature copyfeature = { 0 };
			res = ASFDetectFacesEx(handle, &offscreen, &detectedFaces);
			if (res != MOK && detectedFaces.faceNum > 0)
			{
				printf("%s ASFDetectFaces 1 fail: %d\n", path, res);
			}
			else
			{
				SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
				SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
				SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
				SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
				SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
				
				// 单人脸特征提取
				res = ASFFaceFeatureExtractEx(handle, &offscreen, &SingleDetectedFaces, &feature);
				if (res != MOK)
				{
					printf("%s ASFFaceFeatureExtractEx 1 fail: %d\n", path, res);
				}
				else
				{
					//拷贝feature，否则第二次进行特征提取，会覆盖第一次特征提取的数据，导致比对的结果为1
					copyfeature.featureSize = feature.featureSize;
					copyfeature.feature = (MByte *)malloc(feature.featureSize);
					memset(copyfeature.feature, 0, feature.featureSize);
					memcpy(copyfeature.feature, feature.feature, feature.featureSize);
					
					
					//设置活体置信度 SDK内部默认值为 IR：0.7  RGB：0.5（无特殊需要，可以不设置）
					ASF_LivenessThreshold threshold = { 0 };
					threshold.thresholdmodel_BGR = 0.5;
					threshold.thresholdmodel_IR = 0.7;
					res = ASFSetLivenessParam(handle, &threshold);
					if (res != MOK){
						printf("{\"error_code\":%d,\"message\":\"Invalid liveness detection parameter\"} \n", res);
					}
					printf("{\"error_code\":0,\"message\":\"Successful\",\"faceNum\":%d,\"RGB\":%f, \"nIR\":%f} \n", detectedFaces.faceNum, threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR);
				}
			}
		}else{
			printf("{\"error_code\":86021,\"message\":\"Failed to read image\"} \n");
		}
		*/
		
		IplImage* originalImg = cvLoadImage(path);    

		//图像裁剪，宽度做四字节对齐，若能保证图像是四字节对齐这步可以不用做
		IplImage* img = cvCreateImage(cvSize(originalImg->width - originalImg->width % 4, originalImg->height), IPL_DEPTH_8U, originalImg->nChannels);
		CutIplImage(originalImg, img, 0, 0);

		//图像数据以结构体形式传入，对更高精度的图像兼容性更好
		ASF_MultiFaceInfo detectedFaces = { 0 };
		ASVLOFFSCREEN offscreen = { 0 };
		ASF_SingleFaceInfo SingleDetectedFaces = { 0 };
		ASF_FaceFeature feature = { 0 };
		ASF_FaceFeature copyfeature = { 0 };
		//IplImage 转 ASVLOFFSCREEN
		ColorSpaceConversionIpl(ASVL_PAF_RGB24_B8G8R8, img, offscreen);
		if (img){
			res = ASFDetectFacesEx(handle, &offscreen, &detectedFaces);
			if (MOK != res){
				printf("{\"error_code\":%d,\"message\":\"Engine detect faces failed for %s\"}\n", res, path);
			}
			else{
				SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
				SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
				SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
				SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
				SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
				
				// 单人脸特征提取
				res = ASFFaceFeatureExtractEx(handle, &offscreen, &SingleDetectedFaces, &feature);
				if (res != MOK)
				{
					printf("%s ASFFaceFeatureExtractEx 1 fail: %d\n", path, res);
				}
				else
				{
					//拷贝feature，否则第二次进行特征提取，会覆盖第一次特征提取的数据，导致比对的结果为1
					copyfeature.featureSize = feature.featureSize;
					copyfeature.feature = (MByte *)malloc(feature.featureSize);
					memset(copyfeature.feature, 0, feature.featureSize);
					memcpy(copyfeature.feature, feature.feature, feature.featureSize);
					//设置活体置信度 SDK内部默认值为 IR：0.7  RGB：0.5（无特殊需要，可以不设置）
					ASF_LivenessThreshold threshold = { 0 };
					threshold.thresholdmodel_BGR = 0.5;
					threshold.thresholdmodel_IR = 0.7;
					res = ASFSetLivenessParam(handle, &threshold);
					if (res != MOK){
						printf("{\"error_code\":%d,\"message\":\"Invalid liveness detection parameter\"} \n", res);
					}
					printf("{\"error_code\":0,\"message\":\"Successful\",\"faceNum\":%d,\"RGB\":%f, \"nIR\":%f, \"rect\":[%d,%d,%d,%d]} \n", detectedFaces.faceNum, threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR,SingleDetectedFaces.faceRect.left,SingleDetectedFaces.faceRect.top,SingleDetectedFaces.faceRect.right,SingleDetectedFaces.faceRect.bottom);
				}
			}
			
			//释放图像内存，这里只是做人脸检测，若还需要做特征提取等处理，图像数据没必要释放这么早
			cvReleaseImage(&img);
		}
		cvReleaseImage(&originalImg);
	}
	
	
}


int main(int argc, char *argv[])
{
		char appid[50];
		char appkey[50];
		GetProfileString("./config.ini", "arcface", "appid", appid);
		GetProfileString("./config.ini", "arcface", "appkey", appkey);
		
		char *optstr = "avhd:";
		int pkey = 0;
		struct option opts[] = {
			{"active", 0, NULL, 'a'},
			{"version", 0, NULL, 'v'},
			{"help", 0, NULL, 'h'},
			{"detect", 1, NULL, 'd'},
			{0, 0, 0, 0}
		};
		
		int opt;
		while((opt = getopt_long(argc, argv, optstr, opts, &pkey)) != -1){
			switch(opt) {
				case 'a':
					activation(appid,appkey);
					break;
				case 'v':
					getVersion(appid,appkey);
					break;
				case 'h':
					usage();
					break;
				case 'd':
					detect(appid,appkey,optarg);
					break;
				default:
					usage();
			}
		}
		
		return 0;
}