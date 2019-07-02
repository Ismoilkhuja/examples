
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include "opencv2/videoio/videoio_c.h"

#define ESC_KEY 27

#define VIDEO_FRAME_WIDTH  320
#define VIDEO_FRAME_HEIGHT 240

#define WIN_SRC   "ocv-example original"
#define WIN_CMAP  "ocv-example colors map"
#define WIN_RES   "ocv-example result"
#define WIN_CTL   "ocv-example control"

IplImage *frame_c3[3] ;
IplImage *frame_c1[3];
IplImage *mask[3];

CvVideoWriter* writer;

CvCapture* capture;

int t_max[3]={ 180,256,256 }; // ограничение сверху  H S V
int t_min[3]={  0, 0, 0 }; // ограничение снизу
 
int iswriter=0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int init(int argc, char* argv[]);
int run(char* argv[]);
void done();

int color_filter(IplImage *src,IplImage *dst);
int histogr_eq(IplImage* src,IplImage* dst) ;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int main(int argc, char* argv[]) {
   if(!init(argc,argv)) { run(argv); }
   done();
   return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void done(){
   int i;

   cvDestroyAllWindows();
   cvReleaseCapture(&capture);

   for(i=0;i<3;i++) cvReleaseImage(&frame_c3[i]);
   for(i=0;i<3;i++) cvReleaseImage(&frame_c1[i]);
   for(i=0;i<3;i++) cvReleaseImage(&mask[i]);

   fprintf(stdout,"\n");
   fprintf(stdout,"%i\t%i\t%i\n",t_min[0],t_min[1],t_min[2]); 
   fprintf(stdout,"%i\t%i\t%i\n",t_max[0],t_max[1],t_max[2]); 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int init(int argc, char* argv[]){
   int w=VIDEO_FRAME_WIDTH;
   int h=VIDEO_FRAME_HEIGHT;
   int i,codec;

   // инициализируем камеру
   capture = cvCreateCameraCapture(0); 
   if(!capture) { fprintf(stderr,"[E] cvCreateCameraCapture\n "); return 1;  }
   // устанавливаем параметры картинки
   cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, VIDEO_FRAME_WIDTH );
   cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, VIDEO_FRAME_HEIGHT );

   // получаем кадр 
   frame_c3[0]=cvQueryFrame( capture );
   if(!frame_c3[0]) { fprintf(stderr,"[E] cvQueryFrame error\n "); return 2;   }

   // проверяем параметры картинки
   w=frame_c3[0]->width;
   h=frame_c3[0]->height;
   fprintf(stdout,"[i] %ix%i \n",w,h);

   // создаём буфер для цветовой карты
   frame_c3[1] = cvCloneImage(frame_c3[0]);
   if(!frame_c3[1]){ fprintf(stderr,"[E] cvCloneImage\n"); return 1; }

   // создаём буфер для результата
   frame_c3[2] = cvCloneImage(frame_c3[0]);
   if(!frame_c3[2]){ fprintf(stderr,"[E] cvCloneImage\n"); return 1; }

   // отдельные каналы
   for(i=0;i<3;i++) {
	  frame_c1[i]=cvCreateImage( cvGetSize(frame_c3[0]), frame_c3[0]->depth, 1 );
	  if(!frame_c1[i]){ fprintf(stderr,"[E] cvCreateImage\n"); return 1; }
   }

   // маска 
   for(i=0;i<3;i++) {
	  mask[i]=cvCreateImage( cvGetSize(frame_c3[0]), frame_c3[0]->depth, 1 );
	  if(!mask[i]){ fprintf(stderr,"[E] cvCreateImage\n"); return 1; }
   }

   iswriter=(argc>1);
   if(iswriter) { 
	  // codec =  CV_FOURCC('D','I','V','3'); // DivX MPEG-4 codec
	  // codec =  CV_FOURCC('M','P','4','2'); // MPEG-4 codec
	  // codec =  CV_FOURCC('P','I','M','1'); // MPEG-1 codec
	  // codec =  CV_FOURCC('I','2','6','3'); // ITU H.263 codec
	  // codec =  CV_FOURCC('M','P','E','G'); // MPEG-1 codec
	  codec = CV_FOURCC('D','I','V','X'); // DivX codec 
	  writer = cvCreateVideoWriter( argv[1], codec, 20, cvGetSize(frame_c3[0]),1 );
	  if(!writer) { fprintf( stderr, "[E] cvCreateVideoWriter: %s\n",argv[1]); return 5; }
	  fprintf(stdout,"[i] recode video: %s\n ",argv[1]); 
   } else {
	  fprintf(stdout,"[W] recode video off\n");
   } 

   // открываем и размещаем окна
   cvNamedWindow(WIN_SRC, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_SRC, 10,20);
   
   cvNamedWindow(WIN_CMAP, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_CMAP, 10+10+w,20);

   cvNamedWindow(WIN_RES, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_RES, 10+(10+w)*2,20);

   cvNamedWindow(WIN_CTL, CV_WINDOW_NORMAL); 
   cvResizeWindow(WIN_CTL, 500, 220);
   cvMoveWindow(WIN_CTL, 10,20+(20+h));

   cvCreateTrackbar("max H: ", WIN_CTL, &t_max[0], 180, NULL);
   cvCreateTrackbar("max S: ", WIN_CTL, &t_max[1], 256, NULL);
   cvCreateTrackbar("max V: ", WIN_CTL, &t_max[2], 256, NULL); // (CvTrackbarCallback)update_tb

   cvCreateTrackbar("min H: ", WIN_CTL, &t_min[0], 180, NULL);
   cvCreateTrackbar("min S: ", WIN_CTL, &t_min[1], 256, NULL);
   cvCreateTrackbar("min V: ", WIN_CTL, &t_min[2], 256, NULL);

   fprintf(stdout,"[i] press ESC to exit...\n");

   return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int run(char* argv[]) { 
      for(;;) {
	  // захватываем кадр
	  frame_c3[0]=cvQueryFrame( capture );
	  if(!frame_c3[0]) { fprintf(stderr,"[E] cvQueryFrame error\n "); break;   }
	  cvShowImage( WIN_SRC, frame_c3[0] ); // отображаем исходное изображение 

	  color_filter(frame_c3[0],frame_c3[2]); // отфильтровываем лишние точки по цвету 

	  cvShowImage( WIN_RES, frame_c3[2]); // отображаем результат
	  
 	  if(iswriter){ cvWriteFrame(writer, frame_c3[2]); } // пишем видео

	  if( cvWaitKey(10) == ESC_KEY ) break;
   }
   return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int color_filter(IplImage *src,IplImage *dst) {
  
   //cvCopy(src,dst,NULL);
   histogr_eq(src,frame_c3[1]);
   //cvSmooth(frame_c3[1],frame_c3[1],CV_MEDIAN, 21, 0, 0, 0 ) ; 
   cvSmooth(frame_c3[1],frame_c3[1],CV_GAUSSIAN,0,0,5,0);  
	  
   cvShowImage( WIN_CMAP, frame_c3[1]); // отображаем карту цветов

   cvCvtColor( frame_c3[1],frame_c3[1], CV_BGR2HSV ); // конвертируем карту цветов в HSV

   cvSplit(  frame_c3[1], frame_c1[0], frame_c1[1], frame_c1[2], NULL ); // разрезаем карту цветов на каналы 
  
   // фильтруем фон по порогам
   cvThreshold( frame_c1[0], mask[0], (double)t_min[0]-1.0, 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[0], mask[1], (double)t_max[0], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[0],1);

   cvThreshold( frame_c1[1], mask[0], (double)t_min[1]-1.0, 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[1], mask[1], (double)t_max[1], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[1],1);

   cvThreshold( frame_c1[2], mask[0], (double)t_min[2]-1.0, 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[2], mask[1], (double)t_max[2], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[2],1);

   cvMul(frame_c1[0],frame_c1[1],mask[2],1);
   cvMul(frame_c1[2],mask[2], mask[2],1);

   cvNot(mask[2],mask[2]);
   cvSubS(mask[2],cvScalarAll(254),mask[2],NULL);

   // формируем картинку результата
   cvCvtColor(  frame_c3[1],  frame_c3[1], CV_HSV2BGR ); 
   cvCopy( src, dst, NULL ) ; 
   cvSet( dst, CV_RGB(0,0,0), mask[2] ) ;

   return 0; 
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int histogr_eq(IplImage* src,IplImage* dst) {

   cvSplit( src, frame_c1[0], frame_c1[1], frame_c1[2], NULL );
   cvEqualizeHist(frame_c1[0],frame_c1[0]);
   cvEqualizeHist(frame_c1[1],frame_c1[1]);
   cvEqualizeHist(frame_c1[2],frame_c1[2]);
   cvMerge( frame_c1[0], frame_c1[1], frame_c1[2], NULL, dst );

   return 0;
}

