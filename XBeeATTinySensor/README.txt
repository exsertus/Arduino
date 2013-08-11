XBeeSensor

Example XBee sensor code for ATTiny85 + XBee S2 + various sensors. 

The core code is compatible with ATTiny85 and ATTiny84 MCU's and is interchangable. Core function is:

1. Sleep (both ATTiny and XBee) for the majority of the time
2. Wake up (ATTiny and XBee) via watchdog interupts
3. Take a sensor reading using analog, digital and I2C (Attiny84 only)
4. Transmit reading, together with XBee address and current supply voltage as a string (via serial) to an XBee co-ordinator (mine is currently an Arduino ethernet which code to push the reading onto a web app via REST)

Code variants incuded in this pack are:

* Temperature and Humidity, via a DHT22 (or RHT03) sensor - you'll need at least 3v for this, I use 2xAA's with a Sparkfun 3v3 booster
* Moisture, via a simple moisture sensor I bought of ebay. Its just uses standard analogRead, so 0-1023 values.
* Temperature and Pressure, via a Sparkfun BMP05 sensor (I2C)
* Temperature only, via a TMP36 sensor. Again, just using analogRead (using the internal 1.1v ref voltage)

Xbee runs in transparent mode, with pin sleep enabled. Using the endpoint device firmware profile
ATTiny uses sleep mode in-between sample intervals (set to 15min, but configurable via the INTERVAL constant).

Hardware can be powered by a number of supplies. Running experiments with 3.6v lipo, single AA with 3v3 booster, and 2xAA's  with no regulator. Watch out for Xbee's 3v3 max supply voltage!
Generally, best performance seems to come from a 2xAA config and 15min sample interval. Paper calculation showing 2 years +.

Code wakes up Xbee and sends data string to XBee co-ordinator in the format:

	dataset,value,vcc,xbeeaddress

Where:

dataset  = String to denote the sensor dataset (ie temperature, humidity etc)
value = the raw sensor value
vcc = current supply voltage of sensor (useful for monitoring battery life of remote sensors)
xbeeaddress = the lower half of the 64bit unique address. Allows for multiple datasets of the same name across different xbee sensors.


Credits
Without these guys, neither the inspiration or solution would exist:
Rob Fauldi; http://www.faludi.com and his wonderful 'Building Wireless Sensor Networks', http://www.amazon.co.uk/Robert-Faludi/e/B004JKWA3C
Nathal Chantrell; http://nathan.chantrell.net who's done something very similar with nRF24L01 radios rather than XBee
Pete Mills; http://petemills.blogspot.co.uk/2012/02/attiny-candle.html, power saving with ATTiny
Suprising Edge; http://www.surprisingedge.com/low-power-atmegatiny-with-watchdog-timer/ more Attint power saving and watchdog stuff
http://hacking.majenko.co.uk/making-accurate-adc-readings-on-arduino, reading the supply voltage
