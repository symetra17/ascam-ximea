g++ -I/usr/local/include/opencv \
-I/usr/local/include/opencv2 \
-L/usr/local/lib/ \
-g -o my_main \
my_main.cpp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video   -lopencv_imgcodecs \
-lm3api -lpthread


