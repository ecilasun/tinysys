# What is this?

usbcheck is a tool that will run on Linux to make sure the usb layer can reach the tinysys device (vendor=0xFFFF product=0x0001)

It will then send a dummy bulk command to the device as a test.

This tool is not to be used ordinarily and was written as an exercise on reaching to a known USB device on the bus.

# How to build

First, make sure you have libusb installed:
```
sudo apt-get install libusb-dev
```

Before compiling for the first time, set up waf environment:
```
python3 waf configure -obuild/release
```

For subsequent compiles use:
```
python waf build
```

To clean the build use:
```
python waf clean
```
