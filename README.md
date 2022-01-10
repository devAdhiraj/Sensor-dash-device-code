# Sensor-dash-device-code

This is the device code for the sensor-dash project.
The Progressive Web App is hosted at: [https://ad-sensor-dash.herokuapp.com/](https://ad-sensor-dash.herokuapp.com/)

Git Repo for Backend: [https://github.com/devAdhiraj/sensor-dash-backend](https://github.com/devAdhiraj/sensor-dash-backend)\
Git Repo for Frontend: [https://github.com/devAdhiraj/sensor-dash-frontend](https://github.com/devAdhiraj/sensor-dash-frontend)

The device is an Arduino that is connected to a 16x2 LCD, a touch sensor, an RGB and an ESP8266 wifi module, a DHT Humidty and Temperature sensor, and a photoresistor.

The ESP8266 uses a customized version of the Arduino Thingspeak Library, which essentially adds abstraction on the underlying [AT Commands Firmware](https://docs.espressif.com/projects/esp-at/en/latest/AT_Command_Set/index.html) that is used to send HTTP requests from the ESP8266 module. The Thingspeak header file is modified to work with custom APIs built using ExpressJS and NodeJS instead of using Thingspeak API.

The ESP8266 first connects with the wifi (SSID and password required), it then makes an HTTP POST request to the [add API](https://github.com/devAdhiraj/sensor-dash-backend) with the sensor data as payload (urlencoded) and an API Key Header that gives it authorization to add data to the database.

<img src="https://user-images.githubusercontent.com/75645547/148716162-a805d61f-d45d-41c7-94f7-d080ca3f39c0.png" width=300>
<img src="https://user-images.githubusercontent.com/75645547/148716242-e2b39236-22cb-4603-888b-20d711763fec.jpg" width=300>


