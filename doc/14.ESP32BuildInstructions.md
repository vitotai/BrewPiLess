# Building ESP32 instructions
by @sedgington [Original Post](https://github.com/vitotai/BrewPiLess/issues/128#issuecomment-652571197)
 At first, I thought it must be something wrong with the development board I had ordered:
https://www.amazon.com/gp/product/B0718T232Z/ref=ppx_yo_dt_b_asin_title_o08_s00?ie=UTF8&psc=1
so I ordered a different board:
https://www.amazon.com/gp/product/B07DBNHJW2/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1
same story.
This sent me down the road of building and uploading my own .bin file using Visual Studio Code with PlatformIO--again on the same macbook pro. I'm happy to say this was successful in the end, but unlike Nodemcu, I had to physically press the boot or Ioo button and then (sometimes) the reset button while still holding down the boot button to get the board to go into bootloader mode so that PlatformIO could communicate with the device ( the last board of the second group of boards, balked on communicating through the USB port if I hit the reset button and would only work when I hit the boot button).
So here are is the short version of the steps I had to do to get the ESP32 boards to successfully load brewpiless:

1. Download Visual Studio Code and install it
1. Add PlatformIO to VSC
1. Download the brewpiless-4.0pre branch from github: https://github.com/vitotai/BrewPiLess/tree/v4.0pre
1. Unzip this branch and copy it to a subdirectory of PlatformIO (mine was in the Documents folder)
1. Open this folder that contains the platformio.ini from within VSC/Platformio
1. Click on platformio.ini so that it is open in PlatformIO.
1. At the top of the file find the line default_envs = esp8266dev under [platformio] and change it to #default_envs = esp8266dev
1. Under that line add: default_envs = esp32-dev
1. Save the file
1. find the line
upload_port = /dev/cu.SLAB_USBtoUART under [env:esp32-dev]
and change it to upload_port = /dev/cu.usbserial-0001 (I knew this was my usb port because Nodemcu had told me that information when I went to load the .bin file).
1. Do the same for the line under it that begins monitor_port =
1. Under "View" in VS Code, click on "Command Palette" begin typing in PlatformIO: Build until it finds the command. It will then run the build command. And don't forget to save the file again.
1. Connect your development board to the USB port of your computer. Be sure to use a cable that is capable of transmitting data and is not just a power cable. The red led will come on. Either push and hold the "boot" or "Ioo" button and while still holding it down, briefly hit the "reset" button, then release the button you have been holding down. (if this doesn't work, just hold down the "boot" button and release it.)
1. Assuming that the "PlatformIO: Build" command ran successfully, repeat the steps of 12. but begin typing in "PlatformIO: Upload" until the command is in the box. It will then run automatically.
1. If you successfully found the correct upload port for your usb connection to the board, the program will run and at the bottom of the console you will see under the line that begins with "Environment" the line:
esp32-dev SUCCESS 00:00:43.070
All the other Environments will state "IGNORED".
1. Unplug the development board from the USB cable and then plug it back in.
1. Monitor your wireless network for "brewpiless" and click on that network. Open a web browser after you successfully join brewpiless and type in 192.168.4.1 to bring you to the homepage of brewpiless. Go to the system tab, scan for your local network, select it, put in the password, save it and then, importanly, hit the "submit" button on the bottom right part of the screen. Brewpiless will join your network and you can set it up as desired.
Now, why is this pain worth it? Well, the ESP32 seems to be much faster, has more memory, and should be more stable than the 8266 boards. In addition, it has Bluetooth built in which means that down the road it might support Tilt Hydrometers as well as the currently supported wifi-enabled iSpindel Hydrometers (which are difficult to source in the USA because Tilt has apparently intimidated the German supplier of iSpindel kits to the degree that he will not ship to the USA).

I hope this helps someone and saves them from the pain of getting their ESP32 board to work. If I have missed something or there is a better way, please comment on this post on your way of doing this.