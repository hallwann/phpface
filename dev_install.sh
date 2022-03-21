
sudo cp ./linux_so/* /usr/local/lib
sudo apt-get install build-essential libgtk2.0-dev libavcodec-dev libavformat-dev libjpeg-dev libswscale-dev libtiff5-dev libgtk2.0-dev  pkg-config gcc g++ cmake zip ffmpeg
wget https://codeload.github.com/opencv/opencv/zip/refs/tags/3.4.16
unzip opencv-3.4.16.zip
sudo cd opencv-3.4.16
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j8
make install
echo "/usr/local/lib" >> /etc/ld.so.conf
sudo ldconfig
