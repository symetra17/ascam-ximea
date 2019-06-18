from ximea import xiapi
import cv2
import time
import os
import screeninfo
import random
from subprocess import call
from cv2 import namedWindow, setWindowProperty
from cv2 import WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN
import traceback
from evdev import InputDevice
from select import select

using_gamepad = False
full_screen = False

def save_usr_settings():
    fid = open("my_setting.cfg", "w")
    fid.write("%d "%(int(usr_expos)))
    fid.write("%.1f "%(usr_gain))
    fid.write("%.1f "%(usr_apert))
    fid.close()

def load_usr_settings():
  try:
    fid = open("my_setting.cfg", "r")
    plist = fid.readline().split(" ")
    usr_expos = eval(plist[0])
    usr_gain = eval(plist[1])
    usr_apert = eval(plist[2])
    fid.close()
  except:
    # we will adjust this value by hotkey
    usr_expos = 50000  # 10ms
    usr_apert = 3.5
    usr_gain = 0.0
    print traceback.format_exc()
  return usr_expos, usr_gain, usr_apert


if using_gamepad:
    gamepad = InputDevice('/dev/input/event4')

usr_expos, usr_gain, usr_apert = load_usr_settings()

cam = xiapi.Camera()
cam.open_device()


cam.set_gpo_selector('XI_GPO_PORT3')
cam.set_gpo_mode('XI_GPO_EXPOSURE_ACTIVE')


#cam.set_gpo_mode('XI_GPO_FRAME_ACTIVE')
#cam.set_gpo_mode('XI_GPO_FRAME_ACTIVE_NEG')
#cam.set_gpo_mode('XI_GPO_EXPOSURE_ACTIVE')
#cam.set_gpo_mode('XI_GPO_EXPOSURE_ACTIVE_NEG')


cam.enable_lens_mode()

cam.set_exposure(int(usr_expos))
cam.set_gain(usr_gain)
time.sleep(0.5)
cam.set_lens_aperture_value(usr_apert)

# full resolution is too slow, so turn on downsample 
# on camera
#cam.set_downsampling('XI_DWN_2x2')


if full_screen:
    namedWindow("ximea", WND_PROP_FULLSCREEN)
    setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN)
screen = screeninfo.get_monitors()[0]


losscnt = 0
currentcnt = 0

img = xiapi.Image()

cam.start_acquisition()

scr_shot_cnt = 0

font = cv2.FONT_HERSHEY_SIMPLEX

while True:

    ta = time.time()

    cam.get_image(img)

    #if img.acq_nframe != (currentcnt + 1):
    #    losscnt += 1
    #    print "loss cnt:", losscnt
    #currentcnt = img.acq_nframe

    #npa = img.get_image_data_numpy()
    #npa2 = cv2.resize(npa, (screen.width, screen.height))
    #
    ##show acquired image with time since the beginning of acquisition
    #text = "%4.2f ms" % (float(usr_expos)/1000)
    #cv2.putText(
    #    npa2, text, (10, 50), font, 2, (255, 255, 255), 2)
    #
    #text = "f%.1f"%(usr_apert)
    #cv2.putText(
    #    npa2, text, (350, 50), font, 2, (255, 255, 255), 2)
    #
    #text = '{:d}dB'.format(int(usr_gain))
    #cv2.putText(
    #    npa2, text, (500, 50), font, 2, (255, 255, 255), 2)
    #
    #cv2.imshow('ximea', npa2)
    #
    #key = cv2.waitKeyEx(5)
    #
    #if using_gamepad:
    #    r, w, x = select([gamepad], [], [], 0.001)
    #    if r != []:
    #        for event in gamepad.read():
    #            print(event)
    #            if event.code == 17 and event.value == -1:
    #                key = 65362
    #            if event.code == 17 and event.value == 1:
    #                key = 65364        
    #            if event.code == 315 and event.value == 0:
    #                key = 10
    #
    #if key != -1:
    #    print 'key ', int(key)
    #
    #if key == 27:
    #    break
    #
    #if key == 65362:
    #    usr_expos *= 1.1
    #    cam.set_exposure(int(usr_expos))
    #if key == 65364:
    #    usr_expos /= 1.1
    #    cam.set_exposure(int(usr_expos))
    #
    #if key == 65363:  # right arrow ->
    #    usr_apert = usr_apert * 1.1
    #    cam.set_lens_aperture_value(usr_apert)
    #    usr_apert = cam.get_lens_aperture_value()
    #
    #if key == 65361:  # left arrow <-
    #    usr_apert = usr_apert / 1.1
    #    cam.set_lens_aperture_value(usr_apert)
    #    usr_apert = cam.get_lens_aperture_value()
    #
    #if key == 65470:            # F1
    #    usr_gain = 0.0
    #    cam.set_gain(usr_gain)
    #if key == 65471:            # F2
    #    usr_gain = 6.0
    #    cam.set_gain(usr_gain)
    #if key == 65472:            # F3
    #    usr_gain = 12.0
    #    cam.set_gain(usr_gain)
    #
    #if key == 32:  # space bar
    #    cv2.imwrite("scr_shot_%04d.tiff"%(scr_shot_cnt), npa)
    #    scr_shot_cnt += 1
    #
    #if key == 10:              # enter key
    #
    #    cv2.destroyAllWindows()
    #    
    #    cam.stop_acquisition()
    #    cam.close_device()
    #    usr_expos_str = "%d"%(int(usr_expos))
    #    
    #    save_usr_settings()
    #
    #    
    #    call(["./my_xiSample", 
    #      "expo_ms", usr_expos_str, 
    #      "nimg", "6000",
    #      "random_num",  "%05d"%random.randint(1, 99999), 
    #      "gain", "%.0f"%usr_gain ])
    #    
    #
    #    quit()
    #
    #    cam.open_device()
    #    cam.set_downsampling('XI_DWN_2x2')
    #    cam.start_acquisition()
    #    namedWindow("ximea", WND_PROP_FULLSCREEN)
    #    setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN)


    tb = time.time()
    print '%.1f' % ((tb-ta) * (1000)), 'ms'

save_usr_settings()
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


