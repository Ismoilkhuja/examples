#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/bgsegm.hpp"

using namespace cv;
using namespace std;

int main() {
   const string WIN_NAME_SRC="ocv-examples src";
   const string WIN_NAME_RES="ocv-examples res";
   const string out="result/out.avi";
   

   Mat frame;
   VideoCapture cap(0);
   if (!cap.isOpened()) { cerr << "[E] cannot open the video cam" << endl; return 1; }
   if(!cap.read(frame)) { cerr << "[E] cannot read a frame from video stream" << endl; return 3; }

   int w = frame.cols; 
   int h = frame.rows; 
   cout << "[i] frame size : " << w << " x " << h << endl;

   VideoWriter wri(out, CV_FOURCC('M', 'P', 'E', 'G'), 20, Size(w,h), true); 
   if ( !wri.isOpened() ) { cerr << "[E] failed to write the video:" << out << endl; return 2; }



   Ptr<BackgroundSubtractor> mog = bgsegm::createBackgroundSubtractorMOG();

   
   namedWindow(WIN_NAME_SRC);
   moveWindow(WIN_NAME_SRC, 10,30);

   namedWindow(WIN_NAME_RES);
   moveWindow(WIN_NAME_RES, 10+(w+10),30);

   Mat frame_res;
   Mat foreground = Mat::zeros(frame.size(), CV_32FC3);
   Mat mask = Mat::zeros(frame.size(), CV_8U);
  
   for(;;) {
	  if (!cap.read(frame)) break; 
	  imshow(WIN_NAME_SRC,frame); 
	  mog->apply(frame,foreground,0.01); 

	  threshold(foreground, foreground, 70, 255, CV_THRESH_BINARY);
	  foreground.convertTo(mask, CV_8U);

	  frame_res=frame;
	  add(frame,Scalar(-100,+100,-100),frame_res,mask);
	  imshow(WIN_NAME_RES,frame_res);

	   wri.write(frame_res); 

	waitKey(10);
   }

   return 0;

}
