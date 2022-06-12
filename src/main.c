#include <stdio.h>
#include "pico/stdlib.h"

enum KSTYPE {IDLE, CHK_DIT, CHK_DAH, KEYED_PREP, KEYED, INTER_ELEMENT };

//  keyerControl bit definitions
//
#define     DIT_L      0x01     // Dit latch
#define     DAH_L      0x02     // Dah latch
#define     DIT_PROC   0x04     // Dit is being processed
#define     PDLSWAP    0x08     // 0 for normal, 1 for swap
#define     IAMBICB    0x10     // 0 for Iambic A, 1 for Iambic B
#define     IAMBICA    0x00     // 0 for Iambic A.

unsigned char       keyerState;
unsigned char       iambicMode = IAMBICA;       // or 0 for Iambic A
unsigned char       keyerControl;
unsigned int        wpm = 25;
unsigned long       ditTime;                    // No. milliseconds per dit
unsigned int        paddleReverse = 1;
unsigned int        sidetone_freq = 800;        // 800 Hz

// GPIO pin definitions
const int   DahPin          = 10;      // Dah paddle input or PTT
const int   DitPin          = 6;       // Dit paddle input or KEY
const int   GndPin          = 8;       // Ground
const int   ledPin          = 13;      //

const int   cw_sound_out    = 4;      // square wave out corresponding to the CW out


// global state
int ptt;                   // the current ptt value
int key;                   // the current key value

uint64_t millis() {
    uint64_t us = time_us_64();

    return us/1000;
}

void keyerFunc(void)
{
    static unsigned long ktimer;

    // Basic Iambic Keyer
    // keyerControl contains processing flags and keyer mode bits
    // Supports Iambic A and B
    // State machine based, uses calls to millis() for timing.
 
    switch (keyerState) {
    case IDLE:
        // Wait for direct or latched paddle press
        if ((gpio_get(DahPin) == 0) ||
	    (gpio_get(DitPin) == 0) ||
	    (keyerControl & 0x03)) {
            update_PaddleLatch();
            keyerState = CHK_DIT;
        }
        break;
 
    case CHK_DIT:
        // See if the dit paddle was pressed
        if (keyerControl & DIT_L) {
            keyerControl |= DIT_PROC;
            ktimer = ditTime;
            keyerState = KEYED_PREP;
        }
        else {
            keyerState = CHK_DAH;
        }
        break;
 
    case CHK_DAH:
        // See if dah paddle was pressed
        if (keyerControl & DAH_L) {
            ktimer = ditTime*3;
            keyerState = KEYED_PREP;
        }
        else {
            keyerState = IDLE;
        }
        break;
 
    case KEYED_PREP:
        // Assert key down, start timing, state shared for dit or dah
        gpio_put(ledPin, true);         // turn the LED on

        // turn on the sound on the sidetone pin
        tone(cw_sound_out, sidetone_freq);

	ktimer += millis();                 // set ktimer to interval end time
        keyerControl &= ~(DIT_L + DAH_L);   // clear both paddle latch bits
        keyerState = KEYED;                 // next state
        break;
 
    case KEYED:
        // Wait for timer to expire
        if (millis() > ktimer) {            // are we at end of key down ?
            gpio_put(ledPin, false);      // turn the LED off

	    // turn off the sound
	    noTone(cw_sound_out);

	    ktimer = millis() + ditTime;    // inter-element time
            keyerState = INTER_ELEMENT;     // next state
        }
        else if (keyerControl & IAMBICB) {
            update_PaddleLatch();           // early paddle latch in Iambic B mode
        }
        break;
 
    case INTER_ELEMENT:
        // Insert time between dits/dahs
        update_PaddleLatch();               // latch paddle state
        if (millis() > ktimer) {            // are we at end of inter-space ?
            if (keyerControl & DIT_PROC) {             // was it a dit or dah ?
                keyerControl &= ~(DIT_L + DIT_PROC);   // clear two bits
                keyerState = CHK_DAH;                  // dit done, check for dah
            }
            else {
                keyerControl &= ~(DAH_L);              // clear dah latch
                keyerState = IDLE;                     // go idle
            }
        }
        break;
    }
}

void update_PaddleLatch()
{
    // XXX: debounce DitPin and DahPin
    if (paddleReverse == 0) {
	if (gpio_get(DitPin) == 0) {
	    keyerControl |= DIT_L;
	}

	if (gpio_get(DahPin) == 0) {
	    keyerControl |= DAH_L;
	}
    } else {
	if (gpio_get(DahPin) == 0) {
	    keyerControl |= DIT_L;
	}

	if (gpio_get(DitPin) == 0) {
	    keyerControl |= DAH_L;
	}
    }
}

void loadWPM (int wpm)
{
    ditTime = 1200/wpm;
}

void setupIambic(int on) {
  if (on == 1) {
    keyerState = IDLE;
    keyerControl = iambicMode;
    loadWPM(wpm);
  } else {
    // interface mode
    ptt = gpio_get(DahPin);
    key = gpio_get(DitPin);
  }
}

void setupIambicMode(int modeB) {

  if (modeB == 1) {
    iambicMode = IAMBICB;
  } else {
    iambicMode = 0;
  }

  setupIambic(1);
}

int main() 
{
    stdio_init_all();
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(DahPin);
    gpio_pull_up(DahPin);
    gpio_set_dir(DahPin, GPIO_IN);
    gpio_set_input_hysteresis_enabled(DahPin, true);

    gpio_init(DitPin);
    gpio_pull_up(DitPin);
    gpio_set_dir(DitPin, GPIO_IN);
    gpio_set_input_hysteresis_enabled(DitPin, true);

    gpio_put(LED_PIN, 0);

    while (1) 
    {
	keyerFunc();
    }
}
