
#include <Arduino.h>
#include "EEPROM.h"
#include "CANguruDefs.h"

// Funktion stellt sicher, dass keine unerlaubten 8-Bit-Werte geladen werden können
// adr: EEPROM-Adresse, val: Ersatzwert, min: unterer Grenzwert, max: oberer Grenzwert
uint8_t readValfromEEPROM(uint16_t adr, uint8_t val, uint8_t min, uint8_t max)
{
  uint8_t v = EEPROM.readByte(adr);
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    EEPROM.write(adr, val);
    EEPROM.commit();
    return val;
  }
}

// Funktion stellt sicher, dass keine unerlaubten 16-Bit-Werte geladen werden können
// adr: EEPROM-Adresse, val: Ersatzwert, min: unterer Grenzwert, max: oberer Grenzwert
uint16_t readValfromEEPROM16(uint16_t adr, uint16_t val, uint16_t min, uint16_t max)
{
  uint16_t v = EEPROM.readUShort(adr);
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    EEPROM.writeUShort(adr, val);
    EEPROM.commit();
    return val;
  }
}

// Mit testMinMax wird festgestellt, ob ein Wert innerhalb der
// Grenzen von min und max liegt und oldval ungleich val ist
bool testMinMax(uint16_t oldval, uint16_t val, uint16_t min, uint16_t max)
{
  return (oldval != val) && (val >= min) && (val <= max);
}

char highbyte2char(int num){
  num /= 10;
  return char ('0' + num);
}

char lowbyte2char(int num){
  num = num - num / 10 * 10;
  return char ('0' + num);
}

uint8_t oneChar(uint16_t val, uint8_t pos) {
	char buffer [5];
	itoa (val, buffer, 10);
	return buffer[4 - pos];
}

uint8_t dev_type = DEVTYPE_BASE;

uint8_t hex2dec(uint8_t h){
  return h / 16 * 10 + h % 16;
}

void print_can_frame(const uint8_t *data){
  for (uint8_t i = 0; i < CAN_FRAME_SIZE; i++)
  {
    Serial.print(data[i], HEX);
  }
  Serial.println();
}
