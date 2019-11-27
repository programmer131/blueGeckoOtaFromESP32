# blueGeckoOtaFromESP32
esp32 sketch to upload ebl files to Blue gecko modules, 
for now, just for demonstration purpose, I've set bgm121 device name characteristic as "FACTORY"
ESP32 detect any BT device named as FACTORY, connect to it, search for OTA characteristic, and perform complete OTA process as described in Silabs OTA guide. 
