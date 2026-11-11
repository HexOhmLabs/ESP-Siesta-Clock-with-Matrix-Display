/*
  ESP Siesta Clock - NEC IR Remote Tester
  2026 by HexOhm

  Standalone helper utility. This does NOT modify the clock firmware.
  Connect the IR receiver exactly as in the clock:
    OUT -> D5 / GPIO14

  Requires Arduino-IRremote (IRremote.hpp).
*/
#include <Arduino.h>
#include <IRremote.hpp>

#define IR_PIN 14

static uint16_t clockWord(uint8_t value) {
  return ((uint16_t)value << 8) | (uint8_t)(value ^ 0xFF);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
  Serial.println();
  Serial.println("ESP Siesta Clock - NEC IR Remote Tester");
  Serial.println("Press one remote button at a time.");
}

void loop() {
  if (!IrReceiver.decode()) return;

  auto &d = IrReceiver.decodedIRData;

  Serial.println("----------------------------------------");
  Serial.print("Protocol : "); Serial.println(getProtocolString(d.protocol));
  Serial.print("Bits     : "); Serial.println(d.numberOfBits);
  Serial.print("Address  : 0x"); Serial.println(d.address, HEX);
  Serial.print("Command  : 0x"); Serial.println(d.command, HEX);

  if (d.protocol == NEC || d.protocol == NEC2) {
    uint16_t addressWord = clockWord((uint8_t)d.address);
    uint16_t commandWord = clockWord((uint8_t)d.command);

    Serial.print("Clock address word : 0x");
    if (addressWord < 0x1000) Serial.print("0");
    if (addressWord < 0x0100) Serial.print("0");
    if (addressWord < 0x0010) Serial.print("0");
    Serial.println(addressWord, HEX);

    Serial.print("Clock command word : 0x");
    if (commandWord < 0x1000) Serial.print("0");
    if (commandWord < 0x0100) Serial.print("0");
    if (commandWord < 0x0010) Serial.print("0");
    Serial.println(commandWord, HEX);
  }

  Serial.print("Raw data : 0x");
  Serial.println((uint32_t)d.decodedRawData, HEX);
  IrReceiver.printIRResultRawFormatted(&Serial, true);
  Serial.println();

  IrReceiver.resume();
}
