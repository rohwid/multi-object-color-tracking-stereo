#!/bin/bash

# Configuration
VERSION=3.4.3

# Installation
sudo apt update
sudo apt install -y build-essential
sudo apt install -y cmake
sudo apt install -y libgtk2.0-dev
sudo apt install -y pkg-config
sudo apt install -y libopencv-dev
sudo apt install -y python-numpy python-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
sudo apt install -y libtbb2 libjpeg-dev libtbb-dev libpng-dev libtiff5-dev libdc1394-22-dev

read -n1 -r -p "Download opencv-${VERSION}. press ENTER to continue!" ENTER
if [[ ! -f opencv-${VERSION}.zip ]]; then
	wget -O opencv-${VERSION}.zip https://github.com/opencv/opencv/archive/${VERSION}.zip
fi

read -n1 -r -p "Extract opencv. press ENTER to continue!" ENTER
if [[ ! -d opencv-${VERSION} ]]; then
	unzip opencv-${VERSION}.zip
fi

read -n1 -r -p "Select build with CMAKE. press ENTER to continue!" ENTER
cd opencv-${VERSION}
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..

read -n1 -r -p "Build and install opencv. press ENTER to continue!" ENTER
make all -j4

ldconfig

sudo make install

read -n1 -r -p "Clean installation files. press ENTER to continue!" ENTER
cd ../..
rm -rf opencv_contrib-${VERSION} opencv.zip opencv_contrib.zip

echo " "
echo "  + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "  +                                               +"
echo "  +   Thanks to:                                  +"
echo "  +   Dito Prabowo and Nur Rohman Widiyanto       +"
echo "  +                                               +"
echo "  +                             ~ B201 - NetDev   +"
echo "  +                                               +"
echo "  + + + + + + + + + + + + + + + + + + + + + + + + +"
echo " "
