# Build and Upload Notes

**2026 by HexOhm**

## Target hardware

- ESP8266 / NodeMCU-class controller
- DS3231 RTC
- 32 x 8 WS2812 RGB matrix
- SSD1306 128 x 64 I2C OLED
- NEC IR receiver and matching remote
- Boot/config push button

## Required Arduino libraries

- WiFiManager
- NTPClient
- Adafruit NeoPixel
- U8g2
- RTClib

The firmware also uses ESP8266 core headers including ESP8266WiFi, ESP8266HTTPClient, WiFiClientSecureBearSSL, TZ, WiFiUdp and EEPROM.

## Upload

1. Install the ESP8266 board support package in Arduino IDE.
2. Select the correct ESP8266/NodeMCU board and serial port.
3. Install the libraries listed above.
4. Open `firmware/ESP_Siesta_Clock_with_Matrix_Display.ino`.
5. Compile and upload.
6. On first setup, hold the hardware configuration button during boot to enter WiFiManager setup.
7. Join the `AutoConnectAP` access point and open `192.168.4.1` to store WiFi credentials.

The clock keeps normal time from the DS3231 and uses WiFi only for setup, synchronization and public-holiday refresh when required.

Provided as-is.
