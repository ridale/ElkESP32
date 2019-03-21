# ElkESP32
Code that uses the ESP32 module to send ELKM1 serial codes to a connected bluetooth device.

The comms are deliberately one way to ensure that anyone connected to bluetooth is not able to disarm the alarm.

The pinpad codes are not sent to ensure that anyone connected to bluetooth does not see the users pins.

## TODO
* use authenticated encrypted communications.
* allow some writes (e.g. set time and get names)

