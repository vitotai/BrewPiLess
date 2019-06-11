MQTT Remote Control enables controlling BPL by a MQTT server. When enabled, BPL subscribes the specified path and changes the setting accordingly.
Currently, four item can be controlled in this way:

![MQTT Remote Control Settings](image/mqtt.remote.jpg?raw=true)

### Mode
The values can be 0,1,2,3 or o,f,b,p for 'Off', 'Fridge Constant', 'Beer Constant', and 'Beer Profile'.
If the mode is changed to 'Beer Profile', the current profile will be used and the profile starting time will be set to current time. That is, you should edit the profile beforehand, and expect the profile to be execute starting from the time it is set.

### Setting Temperature
It is valid in 'Fridge Constant' and 'Beer Constant' mode only.

### Capping/spunding, if enabled.
### PTC temperature, if enabled
To set PTC target temperature, the 'triggering temperature' will set to 3 degree higher.

## Some Notes
Connecting to MQTT server takes up a TCP connection and some resource. The program might stick to the setting on MQTT server. Therefore, enable it only when you need it.

There is latency of network and update of data. Therefore, expect some delay after changing the setting.