if g++ -I/usr/local/include/opencv \
-I/usr/local/include/opencv2 -L/usr/local/lib/ \
-g -o conv12  conv12.cpp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lm3api
then
./conv12 "/media/ins/red_ssd/my_image/ima_00000_00000039.raw" output.raw 7920 6004
fi
