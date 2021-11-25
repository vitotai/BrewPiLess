The version of BrewPi ported to BrewPiLess is designed to control fermenting temperature in a fridge or freezer. To control glycol cooling, some hacks are necessary:

1. Set minimum cooling/heating on/off time
    open the page `http://brewpiless.local/testcmd.htm`
    Set the following options: `minCoolTime`, `minCoolIdleTime`, `minHeatTime`, `minHeatIdleTIme`, `deadTime`. The `deadTime` is the minimum time between cooling and heating. It also defines the minimum waiting time after booting up. Issue a command like this to set minimum cooling on and off to 10 seconds:
    
    `j{"minCoolTime":10,"minCoolIdleTime":10}`
    
2. Set P.I.D. parameter
    Setting all P.I.D. to zero will result in the "fridge set" equal to "beer set". _This might not be necessary._
    
    `j{"Kp":0,"Ki":0,"Kd":0}`

    You can use `c` (yes, only one single lower case "c" character.) to read back the setting value. If the values don't change, you might need to erase the flash.

3. Use only beer sensor and control the pump. The fridge temperature will read from beer sensor.

### Special Note:
You might notice that temperatures of beer and fridge from the same sensor are different. The reason is the values are filtered and and they have different filtering parameters. Those parameters also can be changed by the JSON commands. `fridgeFastFilt`, `fridgeSlowFilt`, `firdgeSlopeFilt`, `beerFastFilt`, `beerSlowFilt`, and `beerSlopeFilt`.


Ref:
[JSON commands](JsonCommand.md)
[Parasite Temperature Control](ParasiteTemperatureControl.md)

