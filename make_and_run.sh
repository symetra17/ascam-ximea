
echo 2500000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
echo 2500000 > /sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq

if g++ -I/usr/local/include/opencv \
-I/usr/local/include/opencv2 \
-L/usr/local/lib/ \
-g -o my_main \
my_main.cpp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video   -lopencv_imgcodecs \
-lm3api -lpthread

then
echo "Compilation done."
sudo nice -n -20 ./my_main
else
echo "Compilation failed."
fi
