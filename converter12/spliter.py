# This code accept 12-bit packed raw image, convert to 16-bit
# and split into 3 bands and save as tiff
# 2019 Insightrobotics

import os
import sys
import numpy as np
from PIL import Image
import cv2
import subprocess
import time

width = 7920
height = 6004

inp_folder = os.path.join("D:\\", "DSC", "take2")
out_folder = os.path.join("z:\\", "take2_splited_xxx")

if not os.path.exists(out_folder):
    os.mkdir(out_folder)

for n in range(3):
    path = os.path.join(out_folder, "ch%d"%n)
    if not os.path.exists(path):
        os.mkdir(path)

flist = sorted(os.listdir(inp_folder))

for fname in flist:

    fullname = os.path.join(inp_folder, fname)
    print "converting to 16 bit ", fname
    os.system("conv12.exe " + fullname + " " + fname)

    fid = open(fname, "rb")
    data16 = fid.read()
    fid.close()

    os.system("del " + fname)

    npa = np.frombuffer(data16, dtype=np.uint16)
    npa = npa.reshape([height, width])

    npa_ch = []
    npa_ch.append(npa[100:1850, :])
    npa_ch.append(npa[2050:3800, :])
    npa_ch.append(npa[4000:5750, :])

    print "writing tiff"
    # Most of the CPU time of this code goes to writing file
    for n in range(3):
        fname_no_ext = os.path.splitext(fname)[0]
        out_fname = os.path.join(out_folder, "ch%d"%n, 
                fname_no_ext + ".tiff")
        cv2.imwrite(out_fname, npa_ch[n])   

