#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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

   int N=20; 

   Mat foreground = Mat::zeros(frame.size(), CV_32FC3);
   Mat background = Mat::zeros(frame.size(), CV_32FC3);

   namedWindow(WIN_NAME_SRC);
   moveWindow(WIN_NAME_SRC, 10,30);

   cout<<"[i] собираем средний фон..." << endl;
   for(int i=0;i<N;i++){
	  if(!cap.read(frame)) { cerr << "[E] cannot read a frame from video stream" << endl; break; }
	  imshow("ocv-examples src",frame); 
	  accumulate(frame, background);
	  waitKey(10);
   }
   background/=N;


   namedWindow(WIN_NAME_RES);
   moveWindow(WIN_NAME_RES, 10+(w+10),30);
   Mat mask;
   Mat frame_res;
   cout<<endl<<"[i] запускаем детектор" << endl; 
   for(;;){
	  if(!cap.read(frame)) { cerr << "[E] cannot read a frame from video stream" << endl; break; }
	  imshow(WIN_NAME_SRC,frame); // показать исходный кадр

	  // вычитаем усреднённый фон
	  frame.convertTo(foreground, CV_32FC3);
	  absdiff(foreground, background, foreground); 

	  // строим маску переднего плана
	  cvtColor(foreground, foreground, CV_BGR2GRAY);
	  threshold(foreground, foreground, 70, 255, CV_THRESH_BINARY);
	  foreground.convertTo(mask, CV_8U);

	  // накладываем маску на исходный кадр
	  frame_res=frame;
	  add(frame,Scalar(-100,+100,-100),frame_res,mask);
	  imshow(WIN_NAME_RES,frame_res); // отображаем результат

	  wri.write(frame_res); 


	  waitKey(10);
   }



   return 0;
}
