# Qt4 imageformat plugins
Some unofficial image format plugins for Qt4, Qt5 (and maybe Qt6)  

### Supported Image Formats :
* Avif
* JPEG2000 (JP2)  
* WebP  

### Build and Install
Install Build Dependencies  
* libqt4-dev (or qtbase5-dev)  
* build dependencies for each plugin  

To build all plugins Open terminal in src/ directory and run  
`qmake`  
`make -j4`  
`sudo make install`  

Or to make individual plugins, open terminal in each plugin directory and run  
`qmake`  
`make`  
`sudo make install`  

After build is complete, keep runtime dependencies and uninstall build dependencies.  

### Avif
Build Dependencies:  
* libavif-dev  

Runtime Dependencies:  
* libavif7  

### JPEG2000
Build Dependencies:  
* libopenjp2-7-dev  

Runtime Dependencies:  
* libopenjp2-7  

### WebP
Build Dependencies:  
* libwebp-dev  

Runtime Dependencies:  
* libwebp6  
