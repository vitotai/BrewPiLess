Auto capping (Spunding) is a function that user can specify **time** or **gravity** to "cap" the fermenter for starting of spunding.

Auto capping is enabled by assigning an **_actuator_**, a **_pin_**, in "Device Setup". Currently, only three pins can be used on ESP8266. They are cooler, heater, and door pins, default to D7, D5, and D0(after v2.7).

![Pin assignment for capping](image/capping_setup.jpg?raw=true)

The setting is the same as cooler or heater by assign the function to "**Capper**" and set correct _**PIN Type**_, inverted or non-inverted. 

The capping condition can be time, gravity, or manually controlled. The control pane will be shown only when a Pin is assigned for it. The default state, after assigning the pin without further action, is **Manual Open**.
![Capping Control and status](image/capping_info.jpg?raw=true)

Note: to cap, BPL will set the PIN to "active". Most of the solenoids are normal closed, which is closed with power supply while most mechanical relay modules are inverted, which means it "connects" when the signal is LOW. If you are using an inverted relay to control a normal closed solenoid, PIN Type will be "non-inverted" so that when BPL set the pin to High for **Active**, the relay will in **OPEN** state, and the solenoid will be **CLOSE** state, which means "caping" the fermenter. The following table illustrates the combination of relays and solenoid type.

| Relay, SSR type  | Solenoid Type | PIN Type     | Solenoid Action   |
| -------------- |:-------------:| :--------------| :--------------|    
| Inverted          |   Normal closed |	Inverted |  Active: open, inactive: close |
| Inverted          |   Normal closed |	Not-Inverted |  Active: close, inactive: open |
| Inverted          |   Normal open |	Inverted |  Active: close, inactive: open |
| Inverted          |   Normal open |	Not-Inverted |  Active: open, inactive: close |
| Not Inverted/SSR  |   Normal closed |	Inverted |  Active: close, inactive: open |
| Not Inverted/SSR  |   Normal closed |	Not-Inverted |  Active: open, inactive: close |
| Not Inverted/SSR  |   Normal open |	Inverted |  Active: open, inactive: close |
| Not Inverted/SSR  |   Normal open |	Not-Inverted |  Active: close, inactive: open |

Active="capped".

