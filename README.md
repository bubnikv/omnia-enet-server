# omnia-enet-server
UDP server for streaming IQ audio from/to Omnia SDR on Android and Linux

This is a lightweight UDP server for streaming data of an Omnia SDR transceiver to HDSDR using 
[Vojtech's ExtIO_Omnia](https://github.com/bubnikv/ExtIO_Omnia) HDSDR extension module.

The server is very basic. On one side, the server talks to the Omnia transceiver over USB with the help 
of the libusb-1.0 library. On the other side it talks to the HDSDR application over UDP using the enet
library. While the Omnia hardware implements USB Audio class device to be compatible with operating system audio driver, this server circumvents the operating system
driver and it accesses the USB device directly using the libusb library by detaching the possible Linux audio driver (likely Alsa) from the USB device.
Accessing the Omnia audio stream directly is a good idea on Android, because USB Audio is not really well supported and the Android may not want 
to open the Omnia audio device at the only sample rate it implements. Lastly Android only supports one audio device opened at a time, while Vojtech OK1IAK plans to implement a SDR
receiver with Omnia on Android, needing the phone internal audio device for receive audio output.

This prototype was sketeched in 4 ours on Android in Termux terminal emulation. It will not run on Linux yet, but the necessary modifications are trivial (just change how the USB device is opened
by libusb).

To run the test on Android, you first have to install Termux and Termux::API from the Google app store, then you have to install `pkg install termux-api` inside Termux environment, see
the [Termux API Wiki](https://wiki.termux.com/wiki/Termux:API). Then install the development tools cmake, c++, git and compile this project.

Accessing TCPIP on Android is smooth, however accessing USB requires user authorization on modern Android. Consult the [Termux Wiki](https://wiki.termux.com/wiki/Termux-usb)
on how to access USB in Termux.

First you need to find out the device number of your USB device. Type `termux-usb -l` to list out all the devices detected by the underlying Linux kernel. Unfortunately neither the VID/PID USB identifiers nor the device name are displayed
by the tool, so you have to guess which one is the Omnia. Luckily this server refuses to open a non-Omnia USB device.

To start the omnia server on a given USB device, you should execute

```
termux-usb -r -e ./omnia-enet-server /dev/bus/usb/001/00x
```

with the correct device ID. The parameter `-r` is there for termux-usb to request USB access rights by Android, therefore you will likely have to confirm USB access on the Android phone screen when you first start the server.
Unfortunately neither Termux nor Android remembers that, so you will have to allow USB access again after each phone reboot.


