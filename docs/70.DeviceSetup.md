The setup page is a simplified version of original one of BrewPi. Before the setup, `Erase EEPROM` to initialize the EEPROM. Then, list all devices available by `Refresh Device List`. Two types of devices are concerned: the sensors and the actuators.

![](image/devicesetup.jpg?raw=true)

All sensors found by BPL should be listed with temperature readings. Use exclusive ``Device Slot`` number, choose the `Function`, and press `apply`.

For cooling and heating control, you will need to chose `Pin type`, which is Not Inverted for SSR and Inverted for mechanical relays in most case. 

After editing and applying all devices, click `Refresh Device List` to review the setting. If the assigned devices are not listed in the "installed" section, you might need to `Erase EEPROM` and try again.

To uninstall a device, set the `Function` to `None`.

