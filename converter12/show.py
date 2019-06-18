import os
import sys
import numpy as np
from PIL import Image
import cv2
import bitstring

npa = np.load("xxx.npy")
npa = cv2.resize(npa, None, fx=0.21, fy=0.16, 
    interpolation=cv2.INTER_AREA)
cv2.imshow("", npa*128)
cv2.waitKey(0)

