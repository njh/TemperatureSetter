Temperature Setter
------------------

Arduino based circuit to set target temperature for my heating system.

* Subscribes to MQTT target temperature topic and turns on/off LEDs
* When button is pressed, publishes a _set temperature_ message


| Pin | Description                                                      |
|-----|------------------------------------------------------------------|
| A0  | Push button to toggle Radiator temperature between 20°C / 10°C   |
| A1  | Push button to toggle Radiator temperature between 22°C / 10°C   |
| A2  | Push button to toggle Underfloor Heating temperature 20°C / 10°C |
| D5  | LED indicating radiator target temperature is 20°C               |
| D6  | LED indicating radiator target temperature is 22°C               |
| D6  | LED indicating Underfloor Heating target temperature is 20°C     |


License
-------

This Arduino sketch is licensed under the terms of the MIT license.
See the file [LICENSE](LICENSE.md) for details.
