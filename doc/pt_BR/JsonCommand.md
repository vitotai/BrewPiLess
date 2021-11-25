By using `http://brewpiless.local/testcmd.htm`, you can control BrewPi core directly. For example, to set temperature to Fahrenheit. Open the testcm.htm page, and enter the following string, and send.

`j{"tempFormat":"F"}` 


You can set multiple parameters in one command. The command after `j` is in formal JSON format. Please include the double quote(") for key and string value.

_Don't forget to change **tempSetMin** and **tempSetMax** after chaning temperature unit._

| Key            | Meaning       | Note       |
| -------------- |:-------------:| :--------------------|
| tempFormat         | Temperature format          | F for Fahrenheit, C for Celsius. The algorithm always uses fixed point Celcius format internally, but it converts all settings that go in or out to the right format.	|
| tempSetMin        | Minimum temperature setting     | The fridge and beer temperatures cannot go below this value. |
| tempSetMax        | Maximum temperature setting    | The fridge and beer temperatures cannot go above this value.  |
| Kp                | Kp parameters of PID           | The beer temperature error is multiplied by Kp to give the proportional part of the PID value.  | 
| Ki                | Ki parameters of PID           | When the integral is active, the error is added to the integral every 30 seconds. The result is multiplied by Ki to give the integral part.  | 
| Kd                | Kd parameters of PID           | The derivative of the beer temperature is multiplied by Kd to give the derivative part of the PID value.  | 
| pidMax            | PID Maximum                      | You can define the maximum difference between the beer temp setting and fridge temp setting here. The fridge setting will be clipped to this range.  | 
| iMaxErr           | Integrator: maximum temp error °C                      | The integral is only active when the temperature is close to the target temperature. This is the maximum error for which the integral is active.  |
| idleRangeH        | Temperature idle range top             | When the fridge temperature is within this range, it won't heat or cool, regardless of other settings.  |
| idleRangeL        |  Temperature idle range bottom                   | When the fridge temperature is within this range, it won't heat or cool, regardless of other settings.  |
| heatTargetH       | Heating target upper bound             | When the overshoot lands under this value, the peak is within target range and the estimator is not adjusted.  |
| heatTargetL       |  Heating target lower bound             | When the overshoot lands above this value, the peak is within target range and the estimator is not adjusted.  |
| coolTargetH       | Cooling target upper bound             | When the overshoot lands under this value, the peak is within target range and the estimator is not adjusted.  |
| coolTargetL       | Cooling target lower bound             | When the overshoot lands above this value, the peak is within target range and the estimator is not adjusted.  |
| maxHeatTimeForEst | Maximum time in seconds for heating overshoot estimator         | The time the fridge has been heating is used to estimate overshoot. This is the maximum time that is taken into account.  |
| maxCoolTimeForEst | Maximum time in seconds for cooling overshoot estimator         | The time the fridge has been cooling is used to estimate overshoot. This is the maximum time that is taken into account.  |
| fridgeFastFilt    | Fridge fast filter delay time               |  The fridge fast filter is used for on-off control, display and logging. It needs to have a small delay. 0 = 9 Seconds, 1 = 18 Seconds, 2 = 39 Seconds, 3 = 78 Seconds, 4 = 159 Seconds, 5 = 318 Seconds, 6 = 639 Seconds|
| fridgeSlowFilt    | Fridge slow filter delay time            | The fridge slow filter is used for peak detection to adjust the overshoot estimators. More smoothing is needed to prevent small fluctuations to be recognized as peaks. 0 = 9 Seconds, 1 = 18 Seconds, 2 = 39 Seconds, 3 = 78 Seconds, 4 = 159 Seconds, 5 = 318 Seconds, 6 = 639 Seconds |
| fridgeSlopeFilt   | Fridge slope filter delay time              |  The fridge slope filter is not used in the current version. 0 = 27 Seconds, 1 = 54 Seconds, 2 = 2 Minutes, 3 = 4 Minutes, 4 = 8 Minutes, 5 = 16 Minutes, 6 = 32 Minutes.|
| beerFastFilt      | Beer fast filter delay time                 |  The beer fast filter is used for display and data logging. More filtering give a smoother line, but also more delay. 0 = 9 Seconds, 1 = 18 Seconds, 2 = 39 Seconds, 3 = 78 Seconds, 4 = 159 Seconds, 5 = 318 Seconds, 6 = 639 Seconds |
| beerSlowFilt      | Beer slow filter delay time                 |  The beer slow filter is used for the control algorithm. The fridge temperature setting is calculated from this filter. Because a small difference in beer temperature causes a large adjustment in the fridge temperature, more smoothing is needed. 0 = 9 Seconds, 1 = 18 Seconds, 2 = 39 Seconds, 3 = 78 Seconds, 4 = 159 Seconds, 5 = 318 Seconds, 6 = 639 Seconds |
| beerSlopeFilt     | Beer slope filter delay time                |  The slope is calculated every 30 seconds and fed to this filter. More filtering means a smoother fridge setting. 0 = 27 Seconds, 1 = 54 Seconds, 2 = 2 Minutes, 3 = 4 Minutes, 4 = 8 Minutes, 5 = 16 Minutes, 6 = 23 Minutes|
| lah               | Use light as heater                  | If this option is set to 'Yes'the light wil be used as a heater. 0 = No, 1 = Yes |
| hs                | Rotary encoder trigger step            | When you feel like you have to turn your rotary encoder two steps for every trigger, set this to half step. 0 = Full step, 1 = Half step |
| heatEst           | Heating overshoot estimator         | This is a self learning estimator for the overshoot when turning the heater off. It is adjusted automatically, but you can set adjust it manually here. This does not stop further automatic adjustment.  |
| coolEst           | Cooling overshoot estimator           | This is a self learning estimator for the overshoot when turning the cooler off. It is adjusted automatically, but you can set adjust it manually here. This does not stop further automatic adjustment.  |
| minCoolTime       | Minimum cooling time           |*1   |
| minCoolIdleTime   | Minimum idle time before cooling  |*1   |
| minHeatTime       | Minimum heating time            |*1   |
| minHeatIdleTime   | Minimum idle time before heating |*1   |
| deadTime     | Minimum idle time between switch of heating and cooling.    |*1   |


*1: Available if SettableMinimumCoolTime option is set to true, or EnableGlycol is set to true. After v2.4, it is enabled by default.

To view current temperature control setting, issue a single character

`c`

and send.


## Sensor Calibration
 The command to set sensor calibration is
 
 `U{i:0,j:0.36}`
 
 where **0** is the **Device Slot** that is assigned to the sensor during device setup, and **0.36** is the adjustment to the sensor.

## A note about PID control
The fridge temperature is controlled with PID. The fridge setting = beer setting + PID. The proportional part is linear with the temperature error. The integral part slowly increases when an error stays present, this prevents steady state errors. The derivative part is in the opposite direction to the proportional part. This prevents overshoot: it lowers the PID value when there's 'momentum' in the right direction.  
