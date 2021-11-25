To add gravity reading record, click the number after **SG:**. The number entered with the time when it is entered will be saved as a SG reading record. To enter OG, click the number after **OG:**. Once OG is availble, the Attenuation and ABV will be calculated when SG reading is updated. The OG is for calculation of attenuation and ABV only.
![Input gravity](image/inputgravity.jpg?raw=true)
The SG values and OG value are stored in the local logging file. If local logging is not enabled, those values will disappear after a few hours.

When using "Brew and Calibrate" iSpindel, the same way is used to input the gravity readings measured by a hydrometer.

OG, original gravity, is always manually input. The last one will be used if multiple inputs are available. The presence of OG is to calculate attenuation. 

Please note that input of OG won't generate a "SG" record. If local logging is started around pitching time, then input of OG and input of SG, the same as OG, must be done separately. There are cases that **current** SG is not the same as OG at the time you input OG.

![Input SG](image/gravity_input.jpg?raw=true)

When input SG/OG, the temperature of wort/beer should be input at the same time. BPL will correct the reading based on the correction temperature set in "Gravity Device" and the temperature of Beer Temp. If you correct the hydrometer reading, enter the calibration temperature of your hydrometer.
