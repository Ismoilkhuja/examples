#!python
# -*- coding: utf-8 -*-
__version__ = "$Revision: 1.8 $"
import sys
import os
import numpy as np
import cv2
from scipy.ndimage import maximum_filter


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
def find_templ( img, img_tpl ):
    # размер шаблона
    h,w = img_tpl.shape

    # строим карту совпадений с шаблоном
    match_map = cv2.matchTemplate( img, img_tpl, cv2.TM_CCOEFF_NORMED)

    max_match_map = np.max(match_map) # значение карты для области максимально близкой к шаблону
    print(max_match_map)
    if(max_match_map < 0.71): # совпадения не обнаружены 
        return []

    a = 0.7 # коэффициент "похожести", 0 - все, 1 - точное совпадение 

    # отрезаем карту по порогу 
    match_map = (match_map >= max_match_map * a  ) * match_map  

    # выделяем на карте локальные максимумы
    match_map_max = maximum_filter(match_map, size=min(w,h) ) 
    # т.е. области наиболее близкие к шаблону
    match_map = np.where( (match_map==match_map_max), match_map, 0) 

    # координаты локальных максимумов
    ii = np.nonzero(match_map)
    rr = tuple(zip(*ii))

    res = [ [ c[1], c[0], w, h ] for c in rr ]
   
    return res


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
# рисуем рамки найденных совпадений
def draw_frames(img,coord):
    res = img.copy()
    for c in coord:
        top_left = (c[0],c[1])
        bottom_right = (c[0] + c[2], c[1] + c[3])
        cv2.rectangle(res, top_left, bottom_right, color=(0,0,255), thickness=5 )
    return res 

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
def main():
  
    f = "data/2310024.jpg"
    bu = "data/bu/"
     
    templ = [ os.path.join(bu, b) for b in os.listdir(bu) if os.path.isfile(os.path.join(bu, b)) ]

    img = cv2.imread(f,cv2.IMREAD_GRAYSCALE)

    for t in templ:
        print(t)
        img_tpl = cv2.imread(t,cv2.IMREAD_GRAYSCALE)
        coord = find_templ( img, img_tpl )
        img_res =  cv2.cvtColor(img, cv2.COLOR_GRAY2BGR) 
        img_res = draw_frames(img_res,coord)
        tn = os.path.splitext(os.path.basename(t) ) [0]
        cv2.imwrite("result/res-%s.jpg"%tn, img_res)
        for c in coord:
            print(c)
        print(len(coord))
        print("- - - - - - - - - - - - - - -" )


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
if __name__ == "__main__":
    print("OpenCV ",cv2.__version__)
    sys.exit(main())

