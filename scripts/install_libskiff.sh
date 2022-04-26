git clone https://github.com/astroskiff/libskiff
cd libskiff
mkdir libskiff_build
cd libskiff_build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j9
sudo make install
cd ..