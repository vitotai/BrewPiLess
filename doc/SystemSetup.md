Default username and password are both **brewpiless**.

![](image/configv251.jpg?raw=true)

| Field          | Note       |
| -------------- |:--------------------|
| LCD auto off   | The timer to turn LCD/backlight off.	0 to keep it always on. |
| Title         | The name shown on the homepage, next to LCD.	|
| Host/Network Name         | SSID and hostname	|
| User Name/password         | Username and password to access certain pages |
| Always need password | If enabled, you will be asked username/password when trying to connect to it. |
| Network | Station, AP, Station+AP: set the network operation mode of BPL |
| Fixed IP | Fixed IP to be used as Station. Leave blank if using DHCP. |
| Gateway | Gateway for fixed IP setting. Leave blank if using DHCP. |
| Netmask | Net mask for fixed IP setting. Leave blank if using DHCP. |


Note: Change of all settings except `LCD auto off` will result in re-boot of BPL. The setting of LCD auto off is supposed to be effective right away after submit.

If Network is set Station+AP mode, BPL will create a WiFi network even when it is connecting to a AP. It is useful and good to set Station+AP mode when working with iSpindel. In most case, BPL is placed near the fermenter, so it is near iSpindel, which should solve connection issue of iSpindel, if any, and save the power of iSpindel.

***
[Index](index.md)