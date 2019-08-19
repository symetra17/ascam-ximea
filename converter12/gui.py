import cv2
from tkinter import *
from tkinter.filedialog import askopenfilename
import os
file_path = os.path.dirname(os.path.abspath(__file__)) 
import subprocess

def openimage():
    fname = ''
    fname = askopenfilename(filetypes=(
        ('XIMEA RAW files', '*.raw'),
        ))
    if len(fname) == 0:
        return
    
    iwidth = int(width_box.get("1.0",END))
    iheight = int(height_box.get("1.0",END))

    path=os.path.join(file_path,'conv12')
    print(path+" "+ fname)
    subprocess.Popen([path+" "+ fname + ' output.raw %d %d'%(iwidth,iheight)], 
            shell=True)


master = Tk()
master.title('Packed 12 bit image converter for XIMEA camera')
master.geometry("500x400") #Width x Height

Label(master,text='Width').pack()
width_box = Text(master, height=1)
width_box.pack(padx=5)

Label(master,text='Height').pack()
height_box = Text(master, height=1)
height_box.pack(padx=5)

btn0 = Button(master, text="Select file", command=openimage, 
            height=1, 
            font=('Helvetica', '20'))
btn0.pack(pady=(10,10),padx=(10,10), fill=X)

master.mainloop()
