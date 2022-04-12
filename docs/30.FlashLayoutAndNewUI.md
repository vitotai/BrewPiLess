You can find the detail information about flash layout the following page. 
https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst

To support OTA update, the total program space should be at least twice of the program. The most common used layout for 4M byte flash is 1m program and 3m file space, which is default configuration of BPL. However, having html pages embedded in the program, it is already at the edge of limit. Using Tom's pretty interface does make it over the limit.

Fortunately, framework 1.8.0 sports 4m2m(2m program space) by default. Therefore, we don't need to [manually create](https://github.com/vitotai/BrewPiLess/wiki/2M-sketch-Space) the linkage description file.

Because of the different flash layout, change one layout to another should be done over USB instead of OTA. Morover, the file system will be corrupted and all data will be gone.

