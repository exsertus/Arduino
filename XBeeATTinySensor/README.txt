XBeeTempHumSensor

Example XBee sensor code for ATTiny85 + XBee S2 + DHT22 temperature and humidity sensor.

Xbee runs in transparent mode, with pin sleep enabled. Using the endpoint device firmware profile
ATTiny uses sleep mode in-between sample intervals (set to 15min, but configurable via the INTERVAL constant).

Hardware can be powered by a number of supplies. Running experiments with 3.6v lipo, single AA with 3v3 booster, and 2xAA's with no regulator. Watch out for Xbee's 3v3 max supply voltage!

Code wakes up Xbee and sends data string to XBee co-ordinator in the format:

	dataset,value,vcc,xbeeaddress

Where:

dataset  = String to denote the sensor dataset (ie temperature, humidity etc)
value = the raw sensor value
vcc = current supply voltage of sensor (useful for monitoring battery life of remote sensors)
xbeeaddress = the lower half of the 64bit unique address. Allows for multiple datasets of the same name across different xbee sensors.
