# zcamera-e1-liveview
Sample code for getting the ZCamera E1 liveview through WiFi or USB connection

Connect to the Z Camera through USB cable of WiFi and adjust the Z Camera IP in the file mainwindow.cpp (TODO add auto-detection).

## Requirements

Qt5 SDK with QtCreator (from http://qt.io/)

libavcodec-dev :

On linux debian / ubuntu based :

```
sudo apt-get install libavcodec-dev libswscale-dev
```

Needed for h264 decoding and resizing.

## WiFi

Connect to the Z Camera WiFi and adjust the camera IP to `10.98.32.1` in file mainwindow.cpp

## USB

Set USB mode to network in the camera, connect the camera to the computer, and set manual IP `192.168.168.2`.
Netmask : `255.255.255.0`.

Adjust the IP to `192.168.168.1` in file mainwindow.cpp

## Building

Open the project in QtCreator and build.
Or run :

```
$ /path/to/qmake
$ make
```

If needed, adjust libavcodec and libswscale include and lib path in file `liveview.pro`.


## Run

Start the program and connect by clicking the "connect" button.

## Liveview resolution summary

I get 432x240 in 4K mode.
I get 640x480 in all 4/3 modes.
I get 640x640 in all 1:1 modes.
I get 848x480 in 1080, 720 and WVGA modes

In file mainwindow.cpp the display resolution (including resizing) is fixed (1024x768).
But you can adjust it to match ratio.

## TODO

Lot of stuff of course to make the code more portable, auto detection of camera IP, free up all resources...

Intelligent resizing.

Add camera settings.

I don't promise at all that I'll do these.

## Contributing
Feel free to clone and improve this code.

## Thanks

People of github repo :

https://github.com/imaginevision/Z-Camera-Doc/
https://github.com/imaginevision/Z-Camera-Doc/issues/19
