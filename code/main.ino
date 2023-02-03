
/*
This is our main file, and it's ino!!!!
*/

#include <SPI.h>
/* SPI PINS  */

SPIClass hspi = SPIClass(HSPI);


void blink(uint8_t val, uint8_t bright) {
    int i;

    hspi.transfer(0x00);
    hspi.transfer(0x00);
    hspi.transfer(0x00);
    hspi.transfer(0x00);

    for( i = 0; i < 64; i ++ ) {
        hspi.transfer(0xE0 | bright);
        hspi.transfer(val);
        hspi.transfer(val);
        hspi.transfer(val);
    }

    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
    hspi.transfer(0xFF);
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Setup");
    hspi.begin();

}

void loop() {
    static uint8_t val, bright;

    bright++;
    if( bright > 0x1F ) {
        bright = 0;
    }
    // blink(0, bright);
    // delay(500);
    blink(0xff, bright);
    delay(500);

}

