sudo cp ../linux_so/libarcsoft_face.so /usr/local/lib/
sudo cp ../linux_so/libarcsoft_face_engine.so /usr/local/lib/
sudo cp ../linux_so/libopencv_imgcodecs.so.3.4.16 /usr/local/lib/
sudo cp ../linux_so/libopencv_core.so.3.4.16 /usr/local/lib/
sudo cp ../linux_so/libopencv_imgproc.so.3.4.16 /usr/local/lib/
sudo ln -s /usr/local/lib/libopencv_imgcodecs.so.3.4.16 /usr/local/lib/libopencv_imgcodecs.so.3.4
sudo ln -s /usr/local/lib/libopencv_core.so.3.4.16 /usr/local/lib/libopencv_core.so.3.4
sudo ln -s /usr/local/lib/libopencv_imgproc.so.3.4.16 /usr/local/lib/libopencv_imgproc.so.3.4
sudo ldconfig
