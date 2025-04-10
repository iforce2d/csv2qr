# csv2qr

This utility takes a CSV file as input and outputs a PDF containing QR codes and comment text. The CSV file should have at least three columns and be in ASCII format (not UTF-8).

Columns will be taken as:

 - Column 1 - content of the QR code
 - Column 2 - first line of text comment
 - Column 3 - second line of text comment

For example (quote delimited CSV is not supported):

    C105428, C105428, 4.7K R0603
    C129272, C129272, 22uF C3216
    C131314, C131314, Level shifter 
    C14663, C14663, 100nF C0603
    C15849, C15849, 1uF C0603
    ...


<img src="https://www.iforce2d.net/tmp/qr.png" width="640" height="360">



### Setup

Building this code requires libpng, libz, libharu and ZXing libraries. Should be easy to compile on Linux and Mac, no idea about Windows. The libpng and libz can likely be found in standard packages, for the others, build steps for Fedora and Ubuntu are shown below. For MacOS, you'll probably need to change `/usr` prefixes to `/usr/local`, and limit concurrent compilations explicitly, eg. make -j4.

    (Fedora)
    sudo dnf install zlib-devel libpng-devel stb_image_write-devel 

    (Ubuntu)
    sudo apt install libz-dev libpng-dev libstb-dev
    
    (MacOS)
    # equivalent packages as above can be obtained from brew, not sure the names sorry! For the stb one, try this:
    git clone https://github.com/nothings/stb.git
    sudo cp stb/stb_image_write.h /usr/local/include/

    git clone https://github.com/zxing-cpp/zxing-cpp.git --recursive --single-branch --depth 1
    cd zxing-cpp
    mkdir build
    cd build
    cmake --install-prefix=/usr -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=false ..
    make -j$(nproc)
    sudo make install
    cd ../..
    
    git clone https://github.com/libharu/libharu.git
    cd libharu
    mkdir build
    cd build
    cmake --install-prefix=/usr -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=false ..
    make -j$(nproc)
    sudo make install
    cd ../..

### Building

(On Ubuntu, probably need to change `#include <stb_image_write.h>` to `#include <stb/stb_image_write.h>`)

    make
    ./csv2qr

### Usage

    Usage: ./csv2qr -i infile.csv -o outfile.pdf [options]
    
      Options:
         -v                Print verbose output
         -q size           QR code size (default = 24)
         -f name           Font name (default = Helvetica)
         -s size           Font size (default = 9)
         -r distance       Separation between rows (default = 10)
         -c distance       Separation between columns (default = 100)
         -t distance       Separation between QR and text (default = 5)
         -m distance       Page margin (default = 20)
         -n count          Repetitions (default = 1)
    
      Note that all numeric options are integers.
      Input file must be plain ASCII, not utf-8 !

Repetitions means that the same list will simply be repeated `count` times. With my printer, I found that small QR codes sometimes failed to be detected because every few centimeters the pixels at the bottom edge seemed to be stretched like this: 

<img src="https://www.iforce2d.net/tmp/qr2.png">

I think this might be because the printer roller is not perfectly round...? One solution is to cut the paper very carefully to trim off the excess at the edge, but that's quite fiddly. I figure if I just print a whole extra bunch of the same codes, somewhere there will be one that's ok.

<br>
<br>
<br>
<br>
<br>
<br>
