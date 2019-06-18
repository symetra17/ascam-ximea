#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

int main(){

    unsigned char* rimg = new unsigned char[(7920*6004*3)/2];

    FILE* fid = fopen("/media/ins/red_ssd/my_image/ima_00000_00000039.raw", "rb");
    if (fid==NULL){
        printf("Could not open file.\n");
        exit(0);
    }
    int nbytes = (7920*6004*3)/2;
    for (int n=0; n<nbytes; n++){
        rimg[n] = fgetc(fid);
    }
    fclose(fid);

    unsigned char* oimg = new unsigned char[7920*6004];

    unsigned short p0;
    unsigned short p1;

    for (int m=0; m<(7920*6004); m++){
        p0 = rimg[m+1] & 0x0f;
        p1 = 0;

        p0 = p0<<4;
        p0 += rimg[m];

        oimg[m] = p0;
        oimg[m+1] = p1;
    }

    FILE* ofid = fopen("out.tiff", "wb");
    fwrite(oimg, 7920*6004, 2, ofid);
    fclose(ofid);

    Mat newImg = Mat(/*height*/6004, /*width*/7920, CV_16UC1, oimg);
    newImg = newImg/512;

    Mat newnewImg;
    newImg.convertTo(newnewImg, CV_8UC1);
    imshow("", newnewImg);
    waitKey(0);
    //imwrite("out.png", newImg);
    return 0;

}


