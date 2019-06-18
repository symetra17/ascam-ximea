#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <glob.h>

using namespace cv;

int main(int argc, char *argv[]){

	unsigned int width = atoi(argv[3]);
	unsigned int height = atoi(argv[4]);
    unsigned char* rimg = new unsigned char[(width*height*3)/2];

    FILE* fid = fopen(argv[1], "rb");
	if (fid == NULL){
		printf("Could not open input file\n");
		return -1;
	}
    
	int nbytes = (width*height*3)/2;
	fread(rimg, nbytes, 1, fid);
    fclose(fid);
	
    unsigned short* oimg = new unsigned short[width*height*2];
    unsigned short p0;
    unsigned short p1;
	int optr = 0;
    for (int m=0; m<(width*height*3)/2; m+=3){

        p0 = rimg[m+1] & 0x0f;
		p0 = p0 << 8;
		p0 += rimg[m+0];

        p1 = rimg[m+2];
		p1 = p1 << 4;
		p1 += ((rimg[m+1] & 0xf0)>>4);

        oimg[optr] = p1;
        oimg[optr+1] = p0;
		optr += 2;
    }

    Mat newImg = Mat(height, width, CV_16UC1, oimg);

    imwrite("output16.tiff", newImg);

    newImg = newImg/16;

    Mat newnewImg;
    newImg.convertTo(newnewImg, CV_8UC1);
    namedWindow("converter", WINDOW_OPENGL);
    imshow("converter", newnewImg);
    waitKey(0);


    FILE* ofid = fopen(argv[2], "wb");
    fwrite(oimg, width*height, 2, ofid);
    fclose(ofid);

    return 0;

}


