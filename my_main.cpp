// Insightrobotics 2019
//
// This code connect to ximea camera and save the images to disk
// when testing this code, hyper threading is disabled, pulseaudio removed,
// looks like it shodws more consistent in performance
// press esc to start saving to nvme ssd


#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <m3api/xiApi.h>
#include <memory.h>
#include <stdlib.h>  
#include <unistd.h>
#include <termios.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define HandleResult(res,place) if (res!=XI_OK) {printf("Error after %s (%d)\n",place,res);goto finish;}

#include <chrono>
#include <sys/time.h>

#include <pthread.h>
# include <thread>
#include <mutex>
#include <sys/statvfs.h>

using namespace std;
using namespace cv;
using namespace std::chrono;

int graphical_target = 0;
int use_12bit = 1;
int go_full_screen = 0;
int my_fps = 20;

// this function get keyboard input
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 0;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}


void camera_param_init(HANDLE* xiH, int expos_t, float apert, int gain){

    XI_RETURN stat = XI_OK;

    //xiSetParamInt(0, XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF);
    //xiSetParamInt(0, XI_PRM_PROC_NUM_THREADS, 1);      // looks like it doesn't do anything

	auto sstat = xiOpenDevice(0, xiH);
    if (sstat!=0){
        exit(0);
    }

    stat = xiSetParamInt(*xiH, XI_PRM_LIMIT_BANDWIDTH , 1500*8);
    if (stat!=0) exit(0);
    xiSetParamInt(*xiH, XI_PRM_LIMIT_BANDWIDTH_MODE , XI_ON);
    if (stat!=0) exit(0);

	stat = xiSetParamInt(*xiH, XI_PRM_EXPOSURE, expos_t);
    if (stat!=0){
        printf("set XI_PRM_EXPOSURE failed.\n");
        exit(0);
    }

    xiSetParamInt(*xiH, XI_PRM_LENS_MODE, XI_ON);
    if (stat!=0){
        printf("set LENS_MODE failed.\n");
        exit(0);
    }
    sleep(1);

    // we like to do manual exposure at fixed frame rate
	stat = xiSetParamFloat(*xiH, XI_PRM_LENS_APERTURE_VALUE, apert);
    if (stat!=0){
        printf("set XI_PRM_LENS_APERTURE_VALUE failed.\n");
        exit(0);
    }

    int read;
    xiGetParamInt(*xiH, XI_PRM_EXPOSURE, &read);
    printf("EXPOS: %d\n\r", read);

	stat = xiSetParamInt(*xiH, XI_PRM_GAIN, gain);
    if (stat!=0){
        printf("set XI_PRM_GAIN failed.\n");
        exit(0);
    }

    xiGetParamInt(*xiH, XI_PRM_GAIN, &read);
    printf("GAIN: %d\n\r", read);

	stat = xiSetParamInt(*xiH, XI_PRM_ACQ_TIMING_MODE, 
			XI_ACQ_TIMING_MODE_FRAME_RATE_LIMIT);
	stat = xiSetParamInt(*xiH, XI_PRM_FRAMERATE, my_fps);
    if (stat!=0){
        printf("set XI_PRM_FRAMERATE not sucess\n");
        exit(0);
    }

    stat = xiSetParamInt(*xiH, XI_PRM_BUFFER_POLICY, XI_BP_SAFE);
    if (stat!=0){
        printf("set XI_PRM_BUFFER_POLICY %d\n", stat);
        exit(0);
    }
    
// XI_PRM_ACQ_TRANSPORT_BUFFER_SIZE no such parameter
// XI_PRM_ACQ_TRANSPORT_PACKET_SIZE no such parameter

    xiSetParamInt(*xiH, XI_PRM_ACQ_BUFFER_SIZE, 1000000000);
    if (stat!=0){
        exit(0);
    }
    
    xiSetParamInt(*xiH, XI_PRM_BUFFERS_QUEUE_SIZE, 8);
    if (stat!=0){
        exit(0);
    }

    if (use_12bit){
        // use 12 bit packed, that would cause data rate goes up 1.5X
        stat = xiSetParamInt(*xiH, XI_PRM_IMAGE_DATA_FORMAT,        XI_FRM_TRANSPORT_DATA);
        if (stat!=0) exit(0);
        stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH,    12);
        if (stat!=0) exit(0);
        stat = xiSetParamInt(*xiH, XI_PRM_SENSOR_DATA_BIT_DEPTH,    12);
        if (stat!=0) exit(0);
        stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
        if (stat!=0) exit(0);
        stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
        if (stat!=0) exit(0);
    }else{
        stat = xiSetParamInt(*xiH, XI_PRM_IMAGE_DATA_FORMAT,        XI_FRM_TRANSPORT_DATA);
        if (stat!=0) exit(0);
        stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH,    8);
        if (stat!=0) exit(0);
        //stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
        //stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
    }
}

std::mutex mtx[2];
XI_IMG img70mb[2];

struct arg_struct{
    XI_IMG* pxim;
    std::mutex* pmutx;
    int record_id;
};

int rec_state = 0;

int get_free_space_mb(std::string path){
    struct statvfs buf;
    statvfs(path.c_str(), &buf);
    return buf.f_bsize * buf.f_bavail/1048576;
}

void* save_disk_task(void* arg){

    int record_id = ((arg_struct*)arg)->record_id;
    char fname[256];
    int last_saved=0;
    int last_ts=0;
    int tdiff;
    int fuck_cnt = 0;

    XI_IMG* xim = ((arg_struct*) arg)->pxim;
    std::mutex* pmutx = ((arg_struct*) arg)->pmutx;

    std::string folder = "/media/ins/red_ssd/my_image/";

    while(1){

        if (!rec_state){
            usleep(10000);
            continue;
        }

        pmutx->lock();
        if (xim->acq_nframe > last_saved){
            
            if (get_free_space_mb(folder) < 100){
                printf("Storage is full.\n");
                rec_state = 0;
            }else{
                sprintf(fname, "%sima_%05d_%08d.raw", folder.c_str(), record_id, 
                    xim->acq_nframe);
                FILE* fid = fopen(fname, "wb");
                if (fid == NULL){
                    printf("Saving file failed.\n");
                    rec_state = 0;
                }else{
                    fwrite(xim->bp, 1, xim->bp_size, fid);
                    fclose(fid);
                    last_saved = xim->acq_nframe;
                    tdiff = xim->tsUSec-last_ts;
                    if (tdiff < 0){
                        tdiff += 1000000;
                    }                
                    if (tdiff/1000>100){
                        if (last_ts!=0){
                            fuck_cnt++;
                        }
                    }
                    last_ts = xim->tsUSec;                
                    printf("ACQ %d  TDIFF %d %d\n", xim->acq_nframe, 
                        tdiff, fuck_cnt);
                }
            }
        }
        pmutx->unlock();
        usleep(10000);
    }
}




int main(int argc, char* argv[]) {
    //std::string folderxx = "/media/ins/red_ssd/my_image/";
    //std::string folderxx = "/media/ins/red_ssd/";
    //printf("%d\n",get_free_space_mb(folderxx));
    //return 0;

    Mat mimage;
    mimage = Mat::zeros(6004, 7920, CV_8UC1);

    Mat smallimg;
    //smallimg = Mat::zeros(400, 640, CV_8UC1);

    int expos_t;
    expos_t = 10000;

    int NIMAGES = 10000;

    int gain = 12;
    float apert = 3.5;
    int my_bit_depth = 12;
    int last_acq = 0;

    FILE* fid = fopen("my_setting.cfg", "r");
    char str1[100];
    fgets(str1, 100, fid);
    fclose(fid);
    float fgain=0;
    sscanf(str1, "%d %f %f", &expos_t, &fgain, &apert);      // 2855 12 6.2 
    gain=int(fgain);
    printf("expo %d gain %d apert %f\n", expos_t, gain, apert);


    for (int n=0; n<2; n++){
	    memset(&(img70mb[n]), 0, sizeof(img70mb[n]));
	    img70mb[n].size = sizeof(XI_IMG);
    }
	
	HANDLE xiH = NULL;
	XI_RETURN stat = XI_OK;

    camera_param_init(&xiH, expos_t, apert, gain);

    if (graphical_target)
        if (go_full_screen){
            namedWindow("ximea", WND_PROP_FULLSCREEN);
            setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
        }else{
            namedWindow("ximea", WINDOW_OPENGL);
        }

    int record_id;
    record_id = rand()/1000;

    pthread_t tid0, tid1;
    struct arg_struct arg0;
    arg0.pmutx = &mtx[0];
    arg0.pxim = &img70mb[0];
    arg0.record_id;

    pthread_create(&tid0, NULL, &save_disk_task, &arg0);

    struct arg_struct arg1;
    arg1.pmutx = &mtx[1];
    arg1.pxim = &img70mb[1];
    arg1.record_id;
    pthread_create(&tid1, NULL, &save_disk_task, &arg1);

	stat = xiStartAcquisition(xiH);
	HandleResult(stat,"xiStartAcquisition");

    struct timeval tp1, tp2;
    if (!graphical_target){
        while (1){
            int key = getch();
            if (key == 27){
                rec_state ^= 1;
            }    
            if (key == 'q'){
                break;
            }
            for (int n=0;n<2;n++){
                mtx[n].lock();
                stat = xiGetImage(xiH, 5000, &(img70mb[n]));
                if (!rec_state){
                    printf("ACQ %d\n", img70mb[n].acq_nframe);
                    if (img70mb[n].acq_nframe - last_acq > 1){
                        printf("Frame missed\n");
                    }
                    last_acq = img70mb[n].acq_nframe;
                }
                mtx[n].unlock();
                usleep(1000);
            }
        }
    }
    
    if (graphical_target){
        int n=0;
        while(1){
            stat = xiGetImage(xiH, 5000, &(img70mb[0]));
            cv::Mat mat(7920, 6004, CV_8U, img70mb[0].bp );
            //img70mb[0].bp_size
            cv::resize(mat, mat, cv::Size(1024, 600));
            imshow("", mat);
            int key = waitKey(10);
            if (key == 'q'){
                break;
            }
            printf("%d    %d\n",n++, img70mb[0].bp_size);
        }
    }
        //        cv::resize(mimage, smallimg, cv::Size(1024, 600));
        //        char str1[100];
        //        sprintf(str1, "%.2f ms", expos_t/1000.0);
        //            putText(smallimg, str1,
        //        Point(0, 30), 0, 1.0, Scalar(255,255,255));
        //        sprintf(str1, "%d dB", gain);
        //        putText(smallimg, str1,
        //            Point(170, 30), 0, 1.0, Scalar(255,255,255));
        //        sprintf(str1, "f%.1f ", apert);
        //        putText(smallimg, str1,
        //            Point(300, 30), 0, 1.0, Scalar(255,255,255));
        //        
        //        imshow("ximea", smallimg);
        //        int key = waitKey(1);
        //        
        //        if (key != 255){
        //            printf("%d\r\n", key);
        //        }
        //        
        //        if (key==27){
        //            break;
        //        }
        //        
        //        if (key==10){
        //            if (rec_state!=1){
        //        
        //                // generate an unique id so file name could be different
        //                // for every record 
        //                record_id = rand()/1000;
        //        
        //                cvDestroyAllWindows();
        //        
        //            	stat = xiStopAcquisition(xiH);
        //            	HandleResult(stat,"xiStartAcquisition");
        //        
        //                stat = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT,        XI_FRM_TRANSPORT_DATA);
        //                HandleResult(stat, "XI_PRM_IMAGE_DATA_FORMAT");
        //        
	    //                stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH,    my_bit_depth);
        //                HandleResult(stat, "XI_PRM_OUTPUT_DATA_BIT_DEPTH");
        //        
        //            	stat = xiSetParamInt(xiH, XI_PRM_SENSOR_DATA_BIT_DEPTH,    my_bit_depth);
        //                HandleResult(stat, "XI_PRM_SENSOR_DATA_BIT_DEPTH");
        //        
        //            	stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
        //            	stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
        //        
        //            	stat = xiStartAcquisition(xiH);
        //            	HandleResult(stat,"xiStartAcquisition");
        //        
        //                rec_state = 1;
        //            }
        //        }
        //        
        //        if (key == 82){
        //            expos_t *= 1.1;
        //            xiSetParamInt(xiH, XI_PRM_EXPOSURE, expos_t);
        //            printf("%d\r\n", expos_t);
        //        }else if (key == 84){
        //            expos_t /= 1.1;
        //            xiSetParamInt(xiH, XI_PRM_EXPOSURE, expos_t);
        //            printf("%d\r\n", expos_t);
        //        }else if (key == 85){
        //            apert /= 1.1;
        //        	stat = xiSetParamFloat(xiH, XI_PRM_LENS_APERTURE_VALUE, apert);                    
        //        }else if (key == 86){
        //            apert *= 1.1;
        //            stat = xiSetParamFloat(xiH, XI_PRM_LENS_APERTURE_VALUE, apert);
        //        }else if (key == 190){
        //            gain = 0;
        //            xiSetParamInt(xiH, XI_PRM_GAIN, gain);
        //        }else if (key == 191){
        //            gain = 6;
        //            xiSetParamInt(xiH, XI_PRM_GAIN, gain);
        //        }else if (key == 192){
        //            gain = 12;
        //            xiSetParamInt(xiH, XI_PRM_GAIN, gain);
        //        }
    
        
    


finish:
    xiCloseDevice(xiH);
    return 0;

}


