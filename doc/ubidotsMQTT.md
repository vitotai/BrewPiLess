## Setup ubidots.com to control BPL
### Step 1. create variables
Go to the device page, and add two variables, "mode" and "setTemp".

![](image/ubidots.variable.jpg?raw=true)

Make sure the "api label" is correctly set.

### Step 2. create controls
Go to dashboard. Click "+"(Add widget) to add a **slider** control:
![New Control](image/ubidots.add.control.jpg?raw=true)

Choose the just added variable, mode:
Set the range 0-3, while 0 is off, 1 is "Fridge Constant", 2 is "Beer Constant", and 3 is "Beer Profile".

![New Widget](image/ubidots.widget.creation.jpg?raw=true)

Do the same to the "settemp" variable with correct ranges.

Now you will have it.

![](image/ubidots.control.widget.jpg?raw=true)


## Settings for ubidots.com
Get the full information from [Ubidots Docs](https://ubidots.com/docs/hw/#mqtt).


For free/educational users, use
* MQTT Server: things.ubidots.com
* MQTT Port : 1883
* User Name: [ubidots token]
* Password: (left blank)

The path is in format of '/v1.6/devices/{DEVICE_LABEL}/{LABEL_VARIABLE}/lv'.
Let the device label be 'fermenter2' and mode and setting temperature variables be 'mode', and 'settemp'.
* Mode path: /v1.6/devices/fermenter2/mode/lv
* Setting Temperature path: /v1.6/devices/fermenter2/settemp/lv

You will have to create two variables named 'mode' and 'settemp' in device 'fermenter2'. 
![MQTT Remote Settings for ubidots.com](image/ubidots.mqtt.settings.jpg?raw=true)

## Usage

Slide the **mode slider** to change mode and **settemp slider** to change temperature. The "settemp" should be set __**after**__ mode is changed so that it's not ambiguous.

***
[Index](index.md)