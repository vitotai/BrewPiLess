The original Arduino BrewPi supports rotary encoder by turning for changing values and pushdown for set(or Enter).  BPL uses TWO buttons to simulate the operation of original rotary encoder. While using UP and DOWN are intuitive, the SET/Enter operation is by pressing both button and releasing them at the same time. Keep pressing UP or DOWN when you want to change to a far different value.

_You might feel lag when using the buttons. Press the buttons a little longer to make sure BPL detects it. ESP8266 is not your latest iPhone X. The menu handling code doesn't run as a high priory task. It might takes a few tenth seconds before BPL has a chance to process it._

To enter setup, press `UP+DOWN` at the same time. `Mode` will start to blink, indicating the current editing item.  Press `UP+DOWN` to change control mode, or `UP` to change editing item. 

