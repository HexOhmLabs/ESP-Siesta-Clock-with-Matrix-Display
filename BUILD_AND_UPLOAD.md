# Build and Upload Notes

**2026 by HexOhm**

Open `Siesta_WiFi_Clock_HOLIDAYS_WEEKENDS_ANIMATED_FINAL.ino` in Arduino IDE.

Required libraries:
- WiFiManager
- NTPClient
- Adafruit NeoPixel
- U8g2
- RTClib

The firmware also uses ESP8266 core headers including ESP8266WiFi, ESP8266HTTPClient, WiFiClientSecureBearSSL, TZ, WiFiUdp and EEPROM.

Select the appropriate ESP8266/D1 mini or NodeMCU-class board configuration, compile and upload.

For WiFi setup, hold the hardware configuration button during boot, join `AutoConnectAP`, then open `192.168.4.1`.

The separate `IR_Remote_Tester.ino` is optional and is only for identifying another compatible NEC remote. It is not part of the normal clock firmware.

Final firmware SHA-256: `8f3d867dc6df555c29daf55e5889b521067dd3d46866f3558864752d70b486a9`

Provided as-is.
