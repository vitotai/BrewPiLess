**_The feature is not yet verified._**


By using a pressure transducer, BPL can read pressure and control the pressure if capping is supported.
![Pressure Settings](image/pressure-setting.jpg?raw=true)

## Pressure Reading
The pressure transducers supported is something like this: [Pressure Transducer](http://www.auberins.com/index.php?main_page=product_info&cPath=38&products_id=311). Using other types might be possible, but the output should be linear voltage.

BPL uses the ADC pin, A0, to read the voltage from pressure transducer and converts the reading into pressure. ADC of ESP8266 reads voltage from 0-1.0V, and there are resistors on D1 mini, and NodeMcu, to make ADC input range 0-3.3v. A resistor might be needed to extend the range 0-5v. The resistors and the specification of the pressure transducer together determine the parameters used to convert A0 reading to pressure, in PSI. The formula is 

PSI=(A0_Reading - b) * a

Take the pressure transducer I used for example, it
output: 0.5 - 4.5v linear
maximum pressure: 80psi (4.5v)

There the output is

| pressure | voltage | A0 reading |
| ------- | ----- | ----- |
| 0    |  0.5v | 1023 * 0.5/3.3= 155 |
| 40   |  2.5v | 1023 * 2.5/3.3= 775 |
| 80   |  4.5v | 1023 (saturated) |

Given the fact that I don't really care the reading that exceeds 40psi. I just save a resistor and connect the output direct to A0. My formula is

PSI=(A0_Reading - 155) * (40-0)/(775-155) 
=> PSI=(A0_Reading - 155) * 0.06452

In practice, there are errors and the readings sometimes are not exact the same as expected. (In fact, it's close.) A simple way to get the formula is using "calibration" function by
1. Step 1: make sure there is "NO pressure". And click the button to get "b".
2. Step 2: put the transducer under a pressure closer maximum the better. Input the pressure and click "Step 2" button.

![Pressure Calibration](image/pressure-cal.jpg?raw=true)

BPL will then derive the parameters automatically.

## Pressure Control
"Capping control" is necessary to support pressure control. A solenoid is needed to vent the gas/co2 when necessary. The most tricky part is the venting. A small pin hole or gap should work. **Venting flow should be just a little greater than the speed of CO2 generation.

** BPL checks the pressure **every second**, if the pressure is greater than specified value, the solenoid will be open. The less the gas/pressure is released in 1 second, the more precise the pressure control is. It might sound like difficult. The fact is that it is more difficult to make something air tight. A threaded pipe and cap without seal might do the job.

***
[Index](index.md)