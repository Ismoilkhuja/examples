
#include "opencv2/videoio/videoio_c.h"
#include <stdio.h>
#include <cv.h>
#include <highgui.h>

#define ESC_KEY 27

#define VIDEO_FRAME_WIDTH  640
#define VIDEO_FRAME_HEIGHT 480

// #define VIDEO_FRAME_WIDTH  320
// #define VIDEO_FRAME_HEIGHT 240

// #define VIDEO_FRAME_WIDTH  160
// #define VIDEO_FRAME_HEIGHT 120

#define MIN_CONTOUR (VIDEO_FRAME_HEIGHT/20.0)

#define WIN_SRC   "ocv-example original"
#define WIN_CMAP  "ocv-example colors map"
#define WIN_RES   "ocv-example result"

IplImage *frame_c3[2] ;
IplImage *frame_c1[3];
IplImage *mask[3];

CvCapture* capture;
CvMemStorage* storage;
CvVideoWriter* writer;

int iswriter=0;

IplConvKernel* kern;

int t_max[3];
int t_min[3];



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int init(int argc, char* argv[]);
int run(char* argv[]);
void done();

int color_filter(IplImage *src,IplImage *dst);
int histogr_eq(IplImage* src,IplImage* dst);
int find_circle() ;

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
   cvReleaseMemStorage(&storage);
   cvReleaseVideoWriter(&writer);
   
   cvReleaseStructuringElement( &kern );

   for(i=0;i<2;i++) cvReleaseImage(&frame_c3[i]);
   for(i=0;i<3;i++) cvReleaseImage(&frame_c1[i]);
   for(i=0;i<3;i++) cvReleaseImage(&mask[i]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int init(int argc, char* argv[]){
   int frame_w;
   int frame_h;
   int codec;
   int r=3;
   int i;

   if(argc<7) {
	  fprintf(stderr,"[E] bad arguments\n\t%s h0 s0 v0  h1 s1 v1 [out.avi]",argv[0]);
	  return 1;
   }

   t_min[0]=atoi(argv[1]);
   t_min[1]=atoi(argv[2]);
   t_min[2]=atoi(argv[3]);

   t_max[0]=atoi(argv[4]);
   t_max[1]=atoi(argv[5]);
   t_max[2]=atoi(argv[6]);
  
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
   frame_w=frame_c3[0]->width;
   frame_h=frame_c3[0]->height;
   fprintf(stdout,"[i] %ix%i \n",frame_w,frame_h);


   // создаём буфер для цветовой карты
   frame_c3[1] = cvCloneImage(frame_c3[0]);
   if(!frame_c3[1]){ fprintf(stderr,"[E] cvCloneImage\n"); return 1; }

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

   storage = cvCreateMemStorage(0);
   if(!storage){ fprintf(stderr,"[E] cvCreateMemStorage\n"); return 1; }

   // kern = cvCreateStructuringElementEx(r*2+1, r*2+1, r, r, CV_SHAPE_ELLIPSE,NULL);
   kern = cvCreateStructuringElementEx(r*2+1, r*2+1, r, r, CV_SHAPE_RECT,NULL);
   if(!kern){ fprintf( stderr, "[E] cvCreateStructuringElementEx\n"); return 5; }

   iswriter=(argc>7);
   if(iswriter) { 
	  // codec =  CV_FOURCC('D','I','V','3'); // DivX MPEG-4 codec
	  // codec =  CV_FOURCC('M','P','4','2'); // MPEG-4 codec
	  // codec =  CV_FOURCC('P','I','M','1'); // MPEG-1 codec
	  // codec =  CV_FOURCC('I','2','6','3'); // ITU H.263 codec
	  // codec =  CV_FOURCC('M','P','E','G'); // MPEG-1 codec
	  codec = CV_FOURCC('D','I','V','X'); // DivX codec 
	  writer = cvCreateVideoWriter( argv[7], codec, 20, cvGetSize(frame_c3[0]),1 );
	  if(!writer) { fprintf( stderr, "[E] cvCreateVideoWriter: %s\n",argv[7]); return 5; }
	  fprintf(stdout,"[i] recode video: %s\n ",argv[7]); 
   } else {
	  fprintf(stdout,"[W] recode video off\n");
   } 

   // открываем и размещаем окна
   cvNamedWindow(WIN_SRC, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_SRC, 10,20);
   
   cvNamedWindow(WIN_CMAP, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_CMAP, 10+(10+frame_w),20);

   cvNamedWindow(WIN_RES, CV_WINDOW_AUTOSIZE ); 
   cvMoveWindow(WIN_RES, 10,20+(20+frame_h));

   fprintf(stdout,"[i] press ESC to exit...\n");



   return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int run(char* argv[]) { 

   for(;;) {
	  // захватываем кадр
	  frame_c3[0]=cvQueryFrame( capture );
	  if(!frame_c3[0]) { fprintf(stderr,"[E] cvQueryFrame error\n "); break;   }

	  histogr_eq(frame_c3[0],frame_c3[1]); 
	  color_filter(frame_c3[1],frame_c3[1]); // отфильтровываем лишние точки по цвету 

	  cvCvtColor( frame_c3[1], frame_c1[0], CV_BGR2GRAY );
	  
	  cvThreshold( frame_c1[0], frame_c1[0], 1.0, 256.0, CV_THRESH_BINARY ); 
	  //cvMorphologyEx( frame_c1[0], frame_c1[0], NULL, kern, CV_MOP_DILATE, 1 );
	  //cvMorphologyEx( frame_c1[0], frame_c1[0], NULL, kern, CV_MOP_CLOSE, 1 );
	  cvMorphologyEx( frame_c1[0], frame_c1[0], NULL, kern, CV_MOP_OPEN , 1 );

	  cvCanny( frame_c1[0],frame_c1[0], 100,200,3);

	  find_circle();
	  
	  cvShowImage( WIN_RES, frame_c1[0] ); 
	  cvShowImage( WIN_SRC, frame_c3[0]); 

 	  if(iswriter){ cvWriteFrame(writer, frame_c3[0]); } // пишем видео

	  if( cvWaitKey(10) == ESC_KEY ) break;
   }
   return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int color_filter(IplImage *src,IplImage *dst) {
 
   // карта цветов
   //cvSmooth(src,dst,CV_MEDIAN,21,0,0,0) ; // сглаживание основного буфера (карта цветов)
   cvSmooth(src,dst,CV_GAUSSIAN,0,0,9,0);  
   cvShowImage(WIN_CMAP,dst); // отображаем карту цветов

   cvCvtColor( dst, dst, CV_BGR2HSV ); // конвертируем карту цветов в HSV

   cvSplit( dst, frame_c1[0], frame_c1[1], frame_c1[2], NULL ); // разрезаем карту цветов на каналы 
  
   // фильтруем фон по порогам
   cvThreshold( frame_c1[0], mask[0], (double)t_min[0], 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[0], mask[1], (double)t_max[0], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[0],1);

   cvThreshold( frame_c1[1], mask[0], (double)t_min[1], 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[1], mask[1], (double)t_max[1], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[1],1);

   cvThreshold( frame_c1[2], mask[0], (double)t_min[2], 1.0, CV_THRESH_BINARY ); 
   cvThreshold( frame_c1[2], mask[1], (double)t_max[2], 1.0, CV_THRESH_BINARY_INV ); 
   cvMul(mask[0],mask[1],frame_c1[2],1);

   // объединяем маски трёх каналов
   cvMul(frame_c1[0],frame_c1[1],mask[2],1);
   cvMul(frame_c1[2],mask[2], mask[2],1);

   // инвертируем маску
   cvNot(mask[2],mask[2]);
   cvSubS(mask[2],cvScalarAll(254),mask[2],NULL);

   // формируем картинку результата
   cvCvtColor( dst, dst, CV_HSV2BGR ); 
   cvCopy(src,dst,0) ; 
   cvSet(dst,CV_RGB(0,0,0),mask[2]) ;

   return 0; 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int find_circle() {
   CvSeq* contours=NULL;
   CvSeq* h_next=0;
   CvPoint2D32f p;
   float radius; // радиус объекта на картинке
   CvSeq* c;

   cvClearMemStorage(storage);
   cvFindContours(frame_c1[0],storage,&contours,sizeof(CvContour),CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0) );

   if(!contours) { return 2;}

   // поиск контура максимального размера 
   for( c=contours; c!=NULL; c=c->h_next ) {
	  if(c!=contours) { if (h_next->total>=c->total) { h_next->h_next=h_next->h_next->h_next; continue; } }
	  h_next=c;
   }

   if( h_next->total< MIN_CONTOUR ) { return 1; } // нет объекта - слишком маленькие контуры

   //cvDrawContours( frame_c3[0], h_next, CV_RGB(255,0,255), CV_RGB(128,128,0),2, 2, CV_AA, cvPoint(0,0) ) ;
   cvMinEnclosingCircle(h_next,&p,&radius) ; //Минимальная окружность

   cvCircle( frame_c3[0], cvPoint(p.x,p.y) , 2, CV_RGB(0,255,0), 2, 8, 0 );
   cvCircle( frame_c3[0], cvPoint(p.x,p.y) , radius, CV_RGB(255,0,0), 2, 8, 0 );

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

