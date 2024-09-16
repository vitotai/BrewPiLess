# Parasite (Glycol) Temperature Control

Sometimes an additonal temperature controller to control the temperature of glycol itself is necessary when using glycol to cool beer temperature. BrewPiLess provides a simple temperature control named Parasite Temperature Control, or PTC.

To use PTC, an additional actuator(control PIN) is needed, and "room" sensor is used to monitor the temperature of glycol.

PTC is enabled by assigning a control PIN, which should connect to a relay.
![Pin assignment for PTC](image/setup_ptc.jpg?raw=true)

If PTC is enabled, a section of PTC parameters will show up in Control Page.
![PTC Control](image/control_ptc.jpg?raw=true)

* Target Temperature
The cooling will stop when the temperature is equal or lower than this value.
* Triggering Temperature
The cooling will be started when the temperature is greater than this value. This value should be at least 0.5 higher than “Target Temperature”
* Minimum Cooling Time
Must be greater than or equal to180 (seconds).
* Minimum Idle Time
Must be greater than or equal to180 (seconds).

The status of PTC will also show up at the status pane on "Graph" page.

![PTC Status](image/status_ptc.jpg?raw=true)
