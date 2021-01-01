# WeatherStation
ESP8266 based project to create a sensor that can be integrated in a smart home

Note: This project ist still in "Proof of concept" stage. I didn't had any experience with MQTT devices so far and am approaching this technology myself. 

## Covered Functionality:
This deivce creates a WiFi network at the beginning to enable configuration of the home Wifi-connection and the MQTT-Device settings.
The Setup is safed on a partiion on the SPI-connected flash
After the configuration the device restarts, then establishes a connection to the set MQTT-broker and starts to publish its topics as configured.
So far there is only an SHTC3-Sensor (Temperature and Humidity) implemented and a connection to a 128x64 pixel OLED display is also prepared but currently buggy.
Also simple tasks like digital IO and the analogue input are enabled but currently not really in use.

This device will enable to measure the temperature profile in the basement and will be the basis for further small devices that can be hardly bought (special use cases that are not covered by comercially available devices).

