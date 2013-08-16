This is a small hack to the splendid emonlib (https://github.com/openenergymonitor/EmonLib) to cater for Attiny chipsets. 

Essentially, I've removed the serialPrint function as it uses hardware serial which isn't available on the Attiny.

Working example of this in here https://github.com/exsertus/Arduino/tree/master/XBeeATTinySensor, called XBeeEMONSensor
