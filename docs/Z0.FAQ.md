* Q: in Gravity Device setting page, the coefficients of formula are different from what I input.

   A: Note: The formula coefficients might be different from the value input or calculated. It is due to the limited precision of float point. The difference of calculated gravity should be within 0.0001. Therefore, it should be fine.
* Q: I can't save the device setting.

    A: Click the "Erase Setting" and try again.

* Q: The meaing of WiFi signal.

    A:
    
| bars  | Signal Strength   | Note       |
| ----- |:-------------:| :--------------------|
|  4    |  > -67 dBm   | Very good.	|
|  3    |  -67 ~ -70 dBm   |good.	|
|  2    |  -70 ~ -80 dBm   | minimum for reliable connectivity.	|
|  1    |  -80 ~ -90 dBm   | minimum for basic connectivity. unreliable.	|
|  0    |  < -90 dBm   | Unstable signal	|

* Q: The mode(beer constant or fridge constant) is not kept after reset.

     A: Please make sure the EEPROM is "formated", which is done by "Erase Setting".
     
* Q: How to recover default settings.

     A: "Erase Setting". The hardware configuration will be gone after erasing. You might need to save the hardware configuration and restore after erashing.


* Q: BrewPiLess can't find my sensors.

    A: Check if they are parasitic power only version. 
 [REF#1](https://community.brewpi.com/t/ds18b20-oddities-clones/1656)
[REF#2](https://www.homebrewtalk.com/forum/threads/howto-make-a-brewpi-fermentation-controller-for-cheap.466106/page-136#post-7556259)


