# lslpub_LabJack
C++ programs that gets the data from the labjack T7 and pub them in a LSL stream.


## Architecture
### INPUTS:
- ethernet packets from labjack
### OUTPUTS:
- LSL stream

## Installation
### Ubuntu 18
#### Requirements
#### Steps

### Windows 10
#### Requirements
download and install cmake: https://cmake.org/download/ (download the installer)
download qt installer : https://www.qt.io/download (open source version)
download boost lib: https://sourceforge.net/projects/boost/files/boost-binaries/ (last binaries version)
download and install git:
https://git-for-windows.github.io/

compile lsllib (https://github.com/sccn/labstreaminglayer/blob/master/doc/BUILD.md)
git clone --recurse-submodules https://github.com/labstreaminglayer/labstreaminglayer.git
cd labstreaminglayer
mkdir build
cd build
cmake-gui
configure
select the right version of visualstudio
and the right qt path
generate
cmake --build . --config Release --target install

#### Steps
open the project in QT creator
not shadow build
build
run


