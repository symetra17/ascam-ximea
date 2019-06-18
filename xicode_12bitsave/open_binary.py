from ximea import xiapi
import cv2
import time
import os
import numpy as np
import PIL
from PIL import Image

inp_folder = "/home/ins/DSC"
out_folder = "/home/ins/nam_seng_wai_tiff"

#/media/ins/Seagate Backup Plus Drive/nams_seung_wai_2019"


flist = sorted(os.listdir(inp_folder))
pic_index = 0

for pic_index in range(len(flist)):

  fullpath = os.path.join(inp_folder, flist[pic_index])

  print fullpath

  fid = open(fullpath, 'rb')
  raw_buf = fid.read()
  npa = np.frombuffer(raw_buf, dtype=np.uint8)
  npa = npa.reshape([6004,7920])
  fid.close()

  
  image = Image.fromarray(npa.astype('uint8'))
  fn = os.path.splitext(flist[pic_index])[0]
  image.save(os.path.join(out_folder, fn + ".tiff"))

  npa = cv2.resize(npa, None, fx=0.15, fy=0.15)
  cv2.imshow('XiCAM image', npa)
  key = cv2.waitKeyEx(1)


  #if key == 65363:
  #  if pic_index < (len(flist) - 1):
  #      pic_index += 1

  #if key == 65361:
  #  if pic_index > 0:
  #      pic_index -= 1

  if key == 27:  # esc key
    cv2.destroyAllWindows()
    break;


