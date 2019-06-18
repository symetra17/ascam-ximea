from ximea import xiapi
import cv2
import time
import os
import screeninfo
import random
from subprocess import call
from cv2 import namedWindow, setWindowProperty
from cv2 import WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN


# we will adjust this value by hotkey
usr_expos = 1.0

cam = xiapi.Camera()
cam.open_device()


#cam.set_exposure(int(usr_expos))

usr_expos = cam.get_exposure()

#cam.set_gain(6)


usr_gain = cam.get_gain()

# full resolution is too slow, so turn on downsample 
# on camera
cam.set_downsampling('XI_DWN_2x2')


namedWindow("ximea", WND_PROP_FULLSCREEN)
setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN)
screen = screeninfo.get_monitors()[0]


losscnt = 0
currentcnt = 0

img = xiapi.Image()

cam.start_acquisition()

loop_result = 'just quit'

while True:

    cam.get_image(img)

    if img.acq_nframe != (currentcnt + 1):
        losscnt += 1
        print "loss cnt:", losscnt
    currentcnt = img.acq_nframe

    #print "acq_n:", img.acq_nframe

    ta = time.time()
    npa = img.get_image_data_numpy()
    npa = cv2.resize(npa, (screen.width, screen.height))

    #show acquired image with time since the beginning of acquisition
    font = cv2.FONT_HERSHEY_SIMPLEX
    text = '{:4.2f}ms'.format(usr_expos/1000)
    cv2.putText(
        npa, text, (10, 50), font, 2, (255, 255, 255), 2
        )
    text = '{:d}dB'.format(int(usr_gain))
    cv2.putText(
        npa, text, (10, 150), font, 2, (255, 255, 255), 2
        )

    cv2.imshow('ximea', npa)

    key = cv2.waitKeyEx(2)

    if key != -1:
        print 'key ', int(key)

    if key == 27:
        break

    if key == 65362:
        usr_expos *= 1.1
        cam.set_exposure(int(usr_expos))
    if key == 65364:
        usr_expos /= 1.1
        cam.set_exposure(int(usr_expos))

    if key == 65470:            # F1
        usr_gain = 0.0
        cam.set_gain(usr_gain)
    if key == 65471:            # F2
        usr_gain = 6.0
        cam.set_gain(usr_gain)
    if key == 65472:            # F3
        usr_gain = 12.0
        cam.set_gain(usr_gain)

    if key == 10:              # enter key
        loop_result = 'start save'
        cv2.destroyAllWindows()
        #break

        cam.stop_acquisition()
        cam.close_device()
        usr_expos_str = "%d"%(int(usr_expos))
        call(["/home/ins/my_xiSample", 
          "expo_ms", usr_expos_str, 
          "nimg", "6000",
          "random_num",  "%05d"%random.randint(1, 99999), 
          "gain", "%.0f"%usr_gain ])
        cam.open_device()
        cam.set_downsampling('XI_DWN_2x2')
        cam.start_acquisition()
        namedWindow("ximea", WND_PROP_FULLSCREEN)
        setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN)


    tb = time.time()
    #print '%.1f' % ((tb-ta) * (1000)), 'ms'


cv2.destroyAllWindows()
cam.stop_acquisition()
cam.close_device()

     


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


