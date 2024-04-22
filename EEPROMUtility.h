
#include <EEPROM.h>

void saveValueToEEPROM(float value, int address) {
  EEPROM.put(address, value);
}
