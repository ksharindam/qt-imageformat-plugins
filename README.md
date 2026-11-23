# Qt imageformat plugins
Some unofficial image format plugins for Qt6  

### Supported Image Formats :
* Avif  
* JPEG2000 (JP2)  
* WebP  
* SVG (using librsvg2)  
* Additional formats supported by the installed gdk-pixbuf loaders

### Build and Install
Install Build Dependencies  
* qt6-base-dev  
* build-essential  
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

### Svg
Build Dependencies:  
* librsvg2-dev  
* libcairo2-dev  

Runtime Dependencies:  
* librsvg2  
* libcairo2  

### GdkPixbuf
Build Dependencies:  
* libgdk-pixbuf-2.0-dev  

Runtime Dependencies:  
* libgdk-pixbuf-2.0-0  

