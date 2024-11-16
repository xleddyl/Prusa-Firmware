# how to compile

git clone https://github.com/prusa3d/Prusa-Firmware
cd Prusa-Firmware

## activate venv

## automatically setup dependencies

./utils/bootstrap.py

## configure and build

mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/AvrGcc.cmake
ninja
