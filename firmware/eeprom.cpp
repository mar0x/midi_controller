#include "eeprom.h"
#include <avr/eeprom.h>

namespace midi_controller {

uint8_t eeprom::read(addr_type addr) {
    return eeprom_read_byte( (uint8_t *) addr );
}

void eeprom::write(addr_type addr, uint8_t v) {
    eeprom_write_byte( (uint8_t *) addr, v );
}

void eeprom::update(addr_type addr, uint8_t v) {
    eeprom_update_byte( (uint8_t *) addr, v );
}

void eeprom::get(addr_type addr, uint8_t *b, size_type size) {
    for (; size != 0; --size, ++addr, ++b) {
        *b = read(addr);
    }
}

void eeprom::put(addr_type addr, const uint8_t *b, size_type size) {
    for (; size != 0; --size, ++addr, ++b) {
        update(addr, *b);
    }
}

}
