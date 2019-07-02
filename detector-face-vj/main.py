#!python
# -*- coding: utf-8 -*-
__version__ = "$Revision: 1.9 $"
import sys
import cv2
import numpy as np


def main():

    cascPath = "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml" 
    face_cascade = cv2.CascadeClassifier(cascPath)

    cascPath = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml" 
    eyes_cascade = cv2.CascadeClassifier(cascPath)

    video_capture = cv2.VideoCapture(0)
    video_capture.set(3,640)
    video_capture.set(4,480)
    s = (320,240)

    #s = (320,180)
    #video_capture = cv2.VideoCapture('data/1.mp4')

    ret, frame = video_capture.read() 
    if(not ret):
        print("[E] video_capture.read")
        return

    h,w,_ = frame.shape
    print(w,h)

    f = (w/s[0],h/s[1])


    print("[i] press ESC to exit...")
    while ret:
        gray = cv2.cvtColor( frame, cv2.COLOR_BGR2GRAY)
        gray_small = cv2.resize(gray,s )
        
        faces = face_cascade.detectMultiScale(gray_small, scaleFactor=1.2, minNeighbors=5)

        for (x,y,w,h) in faces:
            x,w = int(x*f[0]), int(w*f[0])
            y,h = int(y*f[1]), int(h*f[1]) 
            cv2.rectangle(frame,(x,y),(x+w,y+h),(255,0,0),2)

            roi_frame = frame[y:y+h, x:x+w]
            roi_gray = gray[y:y+h, x:x+w]
            eyes = eyes_cascade.detectMultiScale(roi_gray)
            for (ex,ey,ew,eh) in eyes:
                r = int(max(ew/2.0,eh/2.0))
                cx,cy = ex+r,ey+r
                cv2.circle(roi_frame, (cx,cy),r,(0,255,0),2)

        cv2.imshow("ocv-examples src", frame)

        k = cv2.waitKey(1) & 0xFF 
        if ( k == ord('q') ) or (k == 27) :
            break

        ret, frame = video_capture.read() # Capture frame-by-frame

    video_capture.release()
    cv2.destroyAllWindows()



if __name__ == "__main__":
    print(cv2.__version__)
    sys.exit(main())


