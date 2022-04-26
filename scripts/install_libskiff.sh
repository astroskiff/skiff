git clone https://github.com/astroskiff/libskiff
cd libskiff
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j9
sudo make install