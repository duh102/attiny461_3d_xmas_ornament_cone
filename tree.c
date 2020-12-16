#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdbool.h>
#include "ws2812.h"
#include "random.h"

#define CEILING(x,y) (((x) + (y) - 1) / (y))

#define STATES 16
#define LEDS 14

const uint8_t color_prog[STATES] PROGMEM = {
#ifndef ALTCODE
   0,
   20,
   16,
   18,
   12,
   14,
   10,
   12,
   8,
   10,
   6,
   8,
   4,
   6,
   2,
   4,
#else
   0,
   2,
   4,
   8,
   16,
   20,
   22,
   24,
   26,
   24,
   22,
   20,
   16,
   8,
   4,
   2,
#endif
};

volatile uint8_t colors[LEDS][3];

// format: [22221111] per byte
volatile uint8_t state[CEILING(LEDS,2)];
// format: [22221111] per byte
volatile uint8_t colorMask[CEILING(LEDS,2)];

int main() {
  // Allow changes to the clock prescaler
  CLKPR = 1<<CLKPCE;
  //  CLKPR[3:0] sets the clock division factor, set it to no scale, so 12MHz
  CLKPR = 0;

  // Set PB3 to be an output
  DDRB = 1<<PB4;

  random_init(0xfdea);


  // main loop
  while(1) {

#ifdef DEBUG
    cli();
    ws2812_set_single(0,0x9,0);
    sei();
    _delay_ms(1);
#else
    for(uint8_t i = 0; i < LEDS; i++) {
      bool odd = i%2;
      uint8_t idx = i/2;
      uint8_t st = odd? state[idx] & 0xf : (state[idx] & 0xf0) >> 4;
      uint8_t mask = odd? colorMask[idx] & 0xf : (colorMask[idx] & 0xf0) >> 4;

#ifndef ALTCODE
      if(st == 0 && (random()%100) < 10) {
        st = 1;
        mask = (random()%6)+1;
        state[idx] = (odd? (state[idx] & 0xf0) : state[idx] & 0xf) | (odd? st : st<<4 );
        colorMask[idx] = (odd? (colorMask[idx] & 0xf0) : colorMask[idx] & 0xf) | (odd? mask : mask<<4 );
        colors[i][0] = pgm_read_byte( &(color_prog[st]) ) * (mask&1);
        colors[i][1] = pgm_read_byte( &(color_prog[st]) ) * (mask&2);
        colors[i][2] = pgm_read_byte( &(color_prog[st]) ) * (mask&4);
      } else if(st > 0) {
        st = (st + 1) % STATES;
        state[idx] = (odd? (state[idx] & 0xf0) : state[idx] & 0xf) | (odd? st : st<<4 );
        colors[i][0] = pgm_read_byte( &(color_prog[st]) ) * (mask&1);
        colors[i][1] = pgm_read_byte( &(color_prog[st]) ) * (mask&2);
        colors[i][2] = pgm_read_byte( &(color_prog[st]) ) * (mask&4);
      }
    }
#else
      if(st == 0 && (random()%100) < 10) {
        st = 1;
        mask = (random()%6)+1;
        state[idx] = (odd? (state[idx] & 0xf0) : state[idx] & 0xf) | (odd? st : st<<4 );
        colorMask[idx] = (odd? (colorMask[idx] & 0xf0) : colorMask[idx] & 0xf) | (odd? mask : mask<<4 );
        colors[i][0] = pgm_read_byte( &(color_prog[st]) ) * (mask&1);
        colors[i][1] = pgm_read_byte( &(color_prog[st]) ) * (mask&2);
        colors[i][2] = pgm_read_byte( &(color_prog[st]) ) * (mask&4);
      } else if(st > 0 && ( st!=8 || (random()%100)<5 ) ) {
        st = (st + 1) % STATES;
        state[idx] = (odd? (state[idx] & 0xf0) : state[idx] & 0xf) | (odd? st : st<<4 );
        colors[i][0] = pgm_read_byte( &(color_prog[st]) ) * (mask&1);
        colors[i][1] = pgm_read_byte( &(color_prog[st]) ) * (mask&2);
        colors[i][2] = pgm_read_byte( &(color_prog[st]) ) * (mask&4);
      }
    }
#endif

    cli();
    for(uint8_t i = 0; i < LEDS; i++) {
      ws2812_set_single(colors[i][0], colors[i][1], colors[i][2]);
    }
    sei();

    // Allow changes to the clock prescaler
    CLKPR = 1<<CLKPCE;
    //  CLKPR[3:0] sets the clock division factor, set it to x/16, so 750kHz
    CLKPR = 0b100;

    _delay_ms(50);

    // Allow changes to the clock prescaler
    CLKPR = 1<<CLKPCE;
    //  CLKPR[3:0] sets the clock division factor, set it to no scale, so 12MHz
    CLKPR = 0;
#endif

  }
  return 0;
}
