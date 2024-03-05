# ctrl.energy
 ESPHome-based node for energy monitoring with additional functionality

## Brief Description
 This module made for my own house as main power meter sensor and gate / outdoor lights control. There're two ESP32 nodes on PCB. One of them performs energy meter monitoring via RS-485 / MODBUS interface. Another one is used to control sliding gates control with Wiegand-based keys and outdoor lights. Also it monitors gate state.

## Hardware
 IMPORTANT!!! Working with HOT LINE AC is dangerous and could harm or even kill! Don't do anything if you don't know how it could behave.
 IMPORTANT 2!!! This hardware works from DC12 power source (J2). It can commutate 5 channels of 16A 230AC via K2-K5 relays.

 You can view PCB and schematic in the _/hardware_ folder in this repository. It was made in KiCAD, so you don't need to buy expensive PCB toolkit to modify or view it. Just keep in mind following mistakes (not the only these, I suppose):
  - Incorrect placements for reed switch based relays (K7 and K8). I've found this error only when recieved produced PCBs - so I decided not to fix it.
  - You'll need better heatsink. LM1084-3.3 could be very hot when powered by 12V. So you should use good thermal interface for it.

 *Note*: PCB successfully soldered and working well (it works almost two weeks.)

## Capabilities

### Energy control node:
  - Temperature monitoring in House Energy Box via D18B20 sensor (J1).
  - Monitor UPS state: battery health (binary sensor) and main power source state (binary sensor) (J3).
  - Monitor state of main power circuit breaker (J4) with auxilary contact like [this](https://automationforum.co/what-is-auxiliary-contact-and-how-it-works-with-mcb/).
  - Monitor case intrusion to House Energy Box via switch connected to J6.
  - Writing logs and settings to microSD card module (J18).
  - Integrated ds3232 RTC clock (EEPROM and alarms functionality is currently not supported by ESPHome) + cr2032 lithium battery.
  - Modbus / RS485 interface via RS485 to UART converter (J12).
  - UART logs from ESP32 (J9).
  - Power Control for Gate control node to reduce power consumption when Main Power source is off the grid.
  - Send messages of problems, daily and monthly report via Telegram Bot (you should create it, see Telegram bot reference to know how to create own telegram bot).

### Gate control node:
  - Control sliding gates in two modes (i.e. full open mode and pedestrian mode) (J16).
  - Sense gate state (opened or closed) with any switch or button (J17).
  - Stores up to 8 Wiegand-26 keys to open gate - use *+ KEY* (SW3) to add a new key, short press *- KEY* (SW4) to remove one key, long press *- KEY* to remove all keys from memory of ESP32.
  - RFID (Wiegand-26 protocol) with J15 pins 5-8.
  - Indicates success or failure with red/green LEDs (J15, pins 2 and 3 as control _ground pins_, pin 1 as +12V power pin for LEDs)
  - Controls up to 5 lamps/zones (*HOT ZONE!!! Up to AC 230v*) - connect AC power source to J5, connect lamps to J7, J8, J10, J11 and J13.

## Configuration

### Energy control node:

  You should configure _secrets.yaml_ to use telegram bot. Both *tg_token_id* and *tg_chat_id* are required for messaging. 

  Set *pincode* to define code to arm/disarm case intrusion protection.

  Set *energy_provider_name* to you Energy Company name or anything else.

  Set *ha_url* to your Home Assistant instance URL and *grafana_url* to Grafana Dashboard. This links will be available in telegram messages.

  Another options are pretty common for ESPHome configs. See _config.yaml_ for all required variables.

### Gate control node:

  This node configuration is also contains common for ESPHome secret variables. 
