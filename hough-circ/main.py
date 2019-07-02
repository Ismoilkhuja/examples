#!python
# -*- coding: utf-8 -*-
__version__ = "$Revision: 1.8 $"

import sys
import cv2
import numpy as np

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
def main():
    img = cv2.imread("data/pla.jpg", cv2.IMREAD_COLOR)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    gray = cv2.medianBlur(gray, 5)
    cv2.imwrite("result/01-blur.jpg",gray)

    edges = cv2.Canny(gray,50,200)
    cv2.imwrite("result/02-edge.jpg",edges)

    circles = cv2.HoughCircles(edges, cv2.HOUGH_GRADIENT, 1, 120, param1=100, param2=30, minRadius=0, maxRadius=0)

    if circles is None: return

    res = np.zeros(img.shape,dtype=np.uint8)
    for circle in circles:
        for x,y,r in circle:
            cv2.circle(img,(x,y),r, (0,255,0),2) # draw the outer circle
            cv2.circle(img,(x,y),2,(0,0,255),3) # draw the center of the circle
            cv2.circle(res,(x,y),r, (0,255,0),2) # draw the outer circle


    cv2.imwrite("result/03-houghCircles.png", res)
    cv2.imwrite("result/04-res.jpg", img)



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
if __name__ == "__main__":
    print("OpenCV ",cv2.__version__)
    sys.exit(main())


