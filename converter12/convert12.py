from ximea import xiapi
import cv2
import time
import os

import random
from subprocess import call
from cv2 import namedWindow, setWindowProperty
from cv2 import WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN
import traceback


fid = open("pack12.raw", "rb")
aa = fid.read()
fid.close()

img = xiapi.Image()
img.bp = 
img.bp[0] = aa[0]
img.bp[1] = aa[1]

#npa = img.get_image_data_numpy()


fields_in_images = ['AbsoluteOffsetX', 'AbsoluteOffsetY', 
'DownsamplingX', 'DownsamplingY', 'GPI_level', 
'__class__', '__ctypes_from_outparam__', '__delattr__', 
'__dict__', '__doc__', '__format__', '__getattribute__', 
'__hash__', '__init__', '__module__', '__new__', '__reduce__', 
'__reduce_ex__', '__repr__', '__setattr__', '__setstate__', 
'__sizeof__', '__str__', '__subclasshook__', '__weakref__', 
'_b_base_', '_b_needsfree_', '_fields_', '_objects', 
'acq_nframe', 
'black_level', 
'bp', 'bp_size', 
'data_saturation', 
'exposure_sub_times_us[5]', 
'exposure_time_us', 
'flags', 
'frm', 
'gain_db', 
'get_bytes_per_pixel', 
'get_image_data_numpy', 
'get_image_data_raw', 
'height', 'width'
'image_user_data', 
'img_desc', 
'nframe', 
'padding_x', 
'size', 
'transport_frm', 
'tsSec', 'tsUSec', 
'wb_blue', 'wb_green', 'wb_red', 
]


