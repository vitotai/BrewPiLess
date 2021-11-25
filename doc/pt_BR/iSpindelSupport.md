### Connection setup for iSpindel

After 4.1, **iSpindel Enabled** must be set to enable iSpindel support.

BrewPiLess supports iSpindel by accepting data from iSpindel and acting an **AP** for iSpindel to connect to, BrewPiLess and iSpindel can connect to the same router. 
To support **softAP**, set the correct settings in `System configuration`. Please note that the password(passphrase) should be at least **8** characters. The same password(pass phrase) is used for setting and for connection certification. Default value is `brewpiless`.

![Gravity Sensor](image/gdsetting.jpg?raw=true)

| Setting   | Description       |
| -------------- |:---------------------------------|
| iSpindel         | To enable iSpindel support.   	   |
| Calculated by BPL | Do the conversion from tilt angle to gravity by BrewPiLess. If this option is OFF, all the following options are not used. |
| SG Calibration   |  Deprecated. |
| Temp. Correction | Apply temperature correction to the calculated gravity reading. Celsius only. Usually it is 20&deg;C(68&deg;F) or 15&deg;C(59&deg;F). | 
| Coefficients | The coefficients of the formula to calculate gravity. Note: this set of coefficients is for calcuation of **specific gravity**, **not** plato. Use 0 for x^3 term if quadratic polynomial is used.|
| LowPass Filter Coefficient | 0~1. See following description|
| Gravity Stability Threshold | Integer value. 1 point = 0.001.  |

When running "Brew N Cal", the coefficients of the formula will be updated by the web page frontend. BPL will calculate gravity based on the updated formula automatically. Please note that, due to the limited precision of float on ESP8266, the values might be different from what you can see on the browser. 

After v2.6, Offset of gravity reading is not available. You can add the offset to the last parameter of the formula,the constant term, and have the same result.

### iSpindel Settting
 * the **iSpindel Name** must start with `iSpindel`, like `iSpindel000` 
 * Select `Generic HTTP`
 * Server address set to `192.168.4.1` if iSpindel connect to the AP created by BrewPiless. or use the ip of BrewPiless if iSpindel connects the AP that BrewPiLess connects to.
 * set url to `/gravity`

For other iSpindel setting, like network settting, please refer to iSpindel project.

### About low Pass Filter
![Low Pass Filter](image/lowpassfilter.jpg)
The coefficient defines the 'a' in this LPF:
y = y[i-1] + a ( x - y[i-1] )
It is usually set to 1/f. So, 1/60 for one minute reporting period, and 1/6 for 10 minute reporteing period.


[calibrationSG.htm in /extra folder](extra/calibrationSG.htm) is an utility HTML file which can be used to derive the coefficients instead of using the excel from iSpindel.
