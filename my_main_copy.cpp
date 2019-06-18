// xiSample.cpp : program that captures 10 images


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

using namespace std;
using namespace cv;
using namespace std::chrono;

int graphical_target = 0;
int use_12bit = 1;
int go_full_screen = 0;
int my_fps = 20;

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


void my_camera_param_init(HANDLE* xiH, int expos_t, float apert, int gain){

    XI_RETURN stat = XI_OK;

    //xiSetParamInt(0, XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF);
    //xiSetParamInt(0, XI_PRM_PROC_NUM_THREADS, 1);      // looks like it doesn't do anything

	printf("Opening camera...\n");
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

    int xxx=0;
    xiGetParamInt(*xiH, XI_PRM_BUFFERS_QUEUE_SIZE, &xxx);
    printf("BUFFERS_QUEUE_SIZE %d\n",xxx);


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
        //stat = xiSetParamInt(*xiH, XI_PRM_SENSOR_DATA_BIT_DEPTH,    12);
        //if (stat!=0) exit(0);
        //stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
        //stat = xiSetParamInt(*xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
    }
}


std::mutex mtx;
int filled_0 = 0;
int filled_1 = 0;

void* my_save_disk_task(void* arg){
    usleep(100000);
    mtx.lock();
    printf("BOB, DO SOMETHING\n");
    mtx.unlock();
}

int last_ts;

int main(int argc, char* argv[]) {

    int rec_state = 0;
    char fname[256];

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

    FILE* fid = fopen("my_setting.cfg", "r");
    char str1[100];
    fgets(str1, 100, fid);
    fclose(fid);
    sscanf(str1, "%d %d %f", &expos_t, &gain, &apert);      // 2855 12 6.2 

    int record_id;

	XI_IMG img;
	memset(&img, 0, sizeof(img));
	img.size = sizeof(XI_IMG);

	XI_IMG img70mb;
	memset(&img70mb, 0, sizeof(img70mb));
    img70mb.size = sizeof(XI_IMG);

	XI_IMG img70mb_odd;
	memset(&img70mb_odd, 0, sizeof(img70mb_odd));
    img70mb_odd.size = sizeof(XI_IMG);

	HANDLE xiH = NULL;
	XI_RETURN stat = XI_OK;

    my_camera_param_init(&xiH, expos_t, apert, gain);

    if (graphical_target)
        if (go_full_screen){
            namedWindow("ximea", WND_PROP_FULLSCREEN);
            setWindowProperty("ximea", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
        }else{
            namedWindow("ximea", WINDOW_OPENGL);
        }

    pthread_t tid;
    int err = pthread_create(&tid, NULL, &my_save_disk_task, NULL);

	stat = xiStartAcquisition(xiH);
	HandleResult(stat,"xiStartAcquisition");


    struct timeval tp1, tp2;

    while (1){

        if (getch() == 27){
            rec_state ^= 1;
            //break;
        }

//gettimeofday(&tp1, NULL);

        if (filled_prm && filled_sec){
            usleep(10000);
        }
        else{
            if (filled_0){
                stat = xiGetImage(xiH, 5000, &img70mb_prm);
                mimage.data = (unsigned char*)img70mb_prm.bp;
            }else{
                stat = xiGetImage(xiH, 5000, &img70mb_sec);
                mimage.data = (unsigned char*)img70mb_sec.bp;
            }
        }
//gettimeofday(&tp2, NULL);

//int dft = tp2.tv_usec-tp1.tv_usec;
//if (dft <0){
//    dft +=1000000;
//}
//printf("%d\n",dft);

        int tdiff = img70mb.tsUSec-last_ts;
        if (tdiff < 0){
            tdiff += 1000000;
        }

        last_ts = img70mb.tsUSec;

        usleep(1000);

        if (rec_state){
            sprintf(fname, "/media/ins/red_ssd/my_image/ima_%05d_%08d.raw", 
                            record_id, img70mb.acq_nframe);

            //FILE* fid = fopen(fname, "wb");
            //fwrite(img70mb.bp, 1, img70mb.bp_size, fid);
            //fclose(fid);

            int fid = open(fname, O_CREAT|O_TRUNC|O_WRONLY);
            int nbyte = write(fid, img70mb.bp, img70mb.bp_size);
            close(fid);

        }

        if (rec_state){
            printf("REC n %d  size %d   td %d uS\n", 
                img70mb.acq_nframe, img70mb.bp_size, tdiff);
        }else{
            printf("    n %d  size %d  td %d uS\n", 
                img70mb.acq_nframe, img70mb.bp_size, tdiff);
        }


            if (graphical_target){

                cv::resize(mimage, smallimg, cv::Size(1024, 600));
                char str1[100];
                sprintf(str1, "%.2f ms", expos_t/1000.0);
                    putText(smallimg, str1,
                Point(0, 30), 0, 1.0, Scalar(255,255,255));
                sprintf(str1, "%d dB", gain);
                putText(smallimg, str1,
                    Point(170, 30), 0, 1.0, Scalar(255,255,255));
                sprintf(str1, "f%.1f ", apert);
                putText(smallimg, str1,
                    Point(300, 30), 0, 1.0, Scalar(255,255,255));
                
                imshow("ximea", smallimg);
                int key = waitKey(1);
                
                if (key != 255){
                    printf("%d\r\n", key);
                }
                
                if (key==27){
                    break;
                }
                
                if (key==10){
                    if (rec_state!=1){
                
                        // generate an unique id so file name could be different
                        // for every record 
                        record_id = rand()/1000;
                
                        cvDestroyAllWindows();
                
                    	stat = xiStopAcquisition(xiH);
                    	HandleResult(stat,"xiStartAcquisition");
                
                        stat = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT,        XI_FRM_TRANSPORT_DATA);
                        HandleResult(stat, "XI_PRM_IMAGE_DATA_FORMAT");
                
	                    stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH,    my_bit_depth);
                        HandleResult(stat, "XI_PRM_OUTPUT_DATA_BIT_DEPTH");
                
                    	stat = xiSetParamInt(xiH, XI_PRM_SENSOR_DATA_BIT_DEPTH,    my_bit_depth);
                        HandleResult(stat, "XI_PRM_SENSOR_DATA_BIT_DEPTH");
                
                    	stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING_TYPE, XI_DATA_PACK_PFNC_LSB_PACKING);
                    	stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_PACKING,      XI_ON);
                
                    	stat = xiStartAcquisition(xiH);
                    	HandleResult(stat,"xiStartAcquisition");
                
                        rec_state = 1;
                    }
                }
                
                if (key == 82){
                    expos_t *= 1.1;
                    xiSetParamInt(xiH, XI_PRM_EXPOSURE, expos_t);
                    printf("%d\r\n", expos_t);
                }else if (key == 84){
                    expos_t /= 1.1;
                    xiSetParamInt(xiH, XI_PRM_EXPOSURE, expos_t);
                    printf("%d\r\n", expos_t);
                }else if (key == 85){
                    apert /= 1.1;
                	stat = xiSetParamFloat(xiH, XI_PRM_LENS_APERTURE_VALUE, apert);                    
                }else if (key == 86){
                    apert *= 1.1;
                    stat = xiSetParamFloat(xiH, XI_PRM_LENS_APERTURE_VALUE, apert);
                }else if (key == 190){
                    gain = 0;
                    xiSetParamInt(xiH, XI_PRM_GAIN, gain);
                }else if (key == 191){
                    gain = 6;
                    xiSetParamInt(xiH, XI_PRM_GAIN, gain);
                }else if (key == 192){
                    gain = 12;
                    xiSetParamInt(xiH, XI_PRM_GAIN, gain);
                }
            }
        
    }


finish:
    xiCloseDevice(xiH);
    return 0;

}


