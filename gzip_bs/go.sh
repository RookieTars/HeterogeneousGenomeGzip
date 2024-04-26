rm -rf build
mkdir build
cd build
cmake ..
make -j4

adb push gzip_bs /data/local/tmp

adb shell "export LD_LIBRARY_PATH=\"/data/local/tmp/:$LD_LIBRARY_PATH\";/data/local/tmp/gzip_bs $1;"