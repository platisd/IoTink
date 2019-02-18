# IoTink
Your portable & connected, e-paper dashboard

![IoTink](https://platis.solutions/blog/wp-content/uploads/2019/02/iotink_feat.jpg)

## What?
**IoT + E-Ink = IoTink**

IoTink is a portable dashboard using an e-paper display and is connected to the
Internet. It is currently utilized to fetch public transport departures and
weather predictions from a
[web service](https://github.com/platisd/vasttrafik-google-assistant) but could
be easily repurposed to accomodate different use cases.

## Why?
IoTink is ideal for times when you need to fetch information via WiFi and display
them on a screen, but it is not feasible to install power cables.

As long as you do not need real-time or frequent updates, IoTink should be
able to remain powered up on 4 AA batteries for a while.

## How?
The concept is rather straightforward, with an ESP8266 microcontroller fetching
data over WiFi and displaying them on the e-paper screen. Then it goes to
deep sleep until it is time to ome back online and fetch updated data.

On a hardware level, things are kept simple, with a minimum ESP8266 circuit that
enables us to program the chip when necessary via UART. 2+2 AA batteries are
supplying just enough voltage for the microcontroller to operate seamlessly.

### Components
* [IoTink PCB rev.0](https://www.pcbway.com/project/shareproject/IoTink__Your_portable_e_paper_dashboard.html)
* [4.2" e-paper module by Waveshare](https://www.waveshare.com/wiki/4.2inch_e-Paper_Module)
* ESP12F
* [4x AA battery holders](https://www.electrokit.com/produkt/batterihallare-1xaa-pcb/) [Dimensions](https://i.imgur.com/DCF7JLv.jpg)
* 4x 10KOhm resistors
* 100nF capacitor
* On/Off tactile switch
* 2x 3-pin male headers
* 2x Jumpers
* 6-pin female header
* 8-pin female header

### Software dependencies
* [GxEPD](https://github.com/ZinggJM/GxEPD/) library for controlling the e-paper display
* [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) library for the graphics
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) library for parsing JSON

## Media
* [Background story](https://platis.solutions/blog/2019/02/18/iotink-your-portable-e-paper-dashboard/)
* [Demo video](https://youtu.be/FovcpUYtC_o)

## License
The IoTink project is released under a dual licensing scheme, because we
are obligated to use a copyleft license for the software.

### Software
**GPLv3** - The license had to be selected due to depending on GPL'd software,
i.e. the [GxEPD](https://github.com/ZinggJM/GxEPD/) library. Pull requests that
will enable this project adopt a more permissive license are more than welcome.

### Hardware
**CC-BY 4.0**