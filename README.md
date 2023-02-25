# crazy_hakz_spinner
Spinning LEDs


### Notes from 2-24-23
--------------------------------------------
The footprint for the through beam sensor is wrong - pins 3 and 4 should be switched.
Pin 4 is actually ground / emiter, and pin 3 is actually the collector / vcc.

The through beem should sensor should not be pulled up to VCC - which is 5v.  It should 
be pulled up to 3.3v.

The level shifter should be moved - as it's too close to the mounting bracket... maybe get a bigger one.

Consider moving the positional / counter through beam to the edge.

Consider adding a testpoint header.

