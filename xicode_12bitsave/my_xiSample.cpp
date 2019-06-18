// xiSample.cpp : program that captures 10 images

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <ncurses.h>
#include <memory.h>
#include <m3api/xiapi_dng_store.h>

using namespace std;


#include <m3api/xiApi.h> // Linux, OSX
#include <memory.h>

#define HandleResult(res,place) if (res!=XI_OK) {printf("Error after %s (%d)\n",place,res);goto finish;}

int _tmain(int argc, _TCHAR* argv[])
{

    int expos_t;
    expos_t = 10000;

    int NIMAGES;
    NIMAGES = 1;

    int rad_num;
    int gain;

    int my_bit_depth = 12;


    expos_t = stoi(argv[2]);
    NIMAGES = stoi(argv[4]);
    rad_num = stoi(argv[6]);
    gain = stoi(argv[8]);

    XI_IMG img;
    memset(&img, 0, sizeof(img));
    img.size = sizeof(XI_IMG);

    HANDLE xiH = NULL;
    XI_RETURN stat = XI_OK;

	printf("Opening camera...\n");
	stat = xiOpenDevice(0, &xiH);
	HandleResult(stat,"xiOpenDevice");

	stat = xiSetParamInt(xiH, XI_PRM_EXPOSURE, expos_t);
	HandleResult(stat,"xiSetParam (exposure set)");

    int read;
    xiGetParamInt(xiH, XI_PRM_EXPOSURE, &read);
    printf("EXPOS: %d\n\r", read);

	stat = xiSetParamInt(xiH, XI_PRM_GAIN, gain);
    HandleResult(stat,"xiSetParam (gain)");

    xiGetParamInt(xiH, XI_PRM_GAIN, &read);
    printf("GAIN: %d\n\r", read);

	//stat = xiSetParamInt(xiH, XI_PRM_ACQ_TIMING_MODE, 
	//		XI_ACQ_TIMING_MODE_FRAME_RATE_LIMIT);
	//stat = xiSetParamInt(xiH, XI_PRM_FRAMERATE, 20);


    stat = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT,        XI_FRM_TRANSPORT_DATA);
    stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH,    my_bit_depth);
    stat = xiSetParamInt(xiH, XI_PRM_SENSOR_DATA_BIT_DEPTH,    my_bit_depth);
    stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
    stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
    stat = xiSetParamInt(xiH, XI_PRM_BUFFER_POLICY,            XI_BP_SAFE);

    int ava_bw;
    xiGetParamInt(xiH, XI_PRM_AVAILABLE_BANDWIDTH, &ava_bw);

    stat = xiSetParamInt(xiH, XI_PRM_LIMIT_BANDWIDTH, ava_bw * 0.94);


	printf("Starting acquisition...\n");
	stat = xiStartAcquisition(xiH);
	HandleResult(stat,"xiStartAcquisition");

    int currcnt;
    currcnt = 0;
    int losscnt;
    losscnt = 0;
    int last_ts;
    last_ts = 0;

    int last_tsSec;
    last_tsSec = 0;

    char fname[256];

    //initscr();
    //cbreak();
    //noecho();
    //nodelay(stdscr, TRUE);

	for (int images=0; images < NIMAGES; images++)
	{
		stat = xiGetImage(xiH, 5000, &img);

                if (img.acq_nframe != (currcnt + 1))
                {
                    losscnt += 1;
                    //printf("loss count:%d\n\r", losscnt);
                    currcnt = img.acq_nframe;
                }
                currcnt = img.acq_nframe;

                XI_DNG_METADATA metadata;
                xidngFillMetadataFromCameraParams(xiH, &metadata);

                sprintf(fname, "/home/ins/DSC/ima_%05d_%08d.dng", rad_num, images);

                //stat = xidngStore(fname, &img, &metadata);
                //HandleResult(stat,"xidngStore");

                //sprintf(fname, "/home/ins/DSC/ima_%05d_%08d.raw", rad_num, images);
                sprintf(fname, "/media/xxx/6451d127-e708-42e4-be6f-95a6a729832a/home/ins/DSC/ima_%05d_%08d.raw", rad_num, images);

                FILE* fid = fopen(fname, "wb");
                fwrite(img.bp, 1, img.bp_size, fid);
                fclose(fid);


                int diff;
                if (img.tsSec > last_tsSec){
                    diff = img.tsUSec + 1000000 - last_ts;
                }
                else{
                    diff = img.tsUSec - last_ts;
                }


                //mvprintw(0, 0, "NFRAME: % 6d  ", img.acq_nframe);
                //mvprintw(1, 0, "DIFF: % 6.1f mS  ", (diff/1000.0));
                //mvprintw(2, 0, "N LOSS: % 6d", losscnt);

                printf("DIFF: % 6.1f mS", (diff/1000.0));
                printf("  LOSS: % 6d\n\r", losscnt);

                last_ts = img.tsUSec;
                last_tsSec = img.tsSec;

		HandleResult(stat,"xiGetImage");
        int ch;
        //ch = getch();
        //if (ch == 'q'){
        //    break;
        //}
    }

    //endwin();

	printf("Stopping acquisition...\n");
	xiStopAcquisition(xiH);
	xiCloseDevice(xiH);
finish:
    printf("loss cnt:%d\n\r", losscnt);
	return 0;
}



/*
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
'wb_blue','wb_green', 'wb_red', 
]

*/


