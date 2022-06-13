#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico_tone.hpp"

// GPIO pin definitions
const uint   DahPin          = 8;      // Dah paddle input or PTT
const uint   DitPin          = 11;       // Dit paddle input or KEY
const uint   LED_PIN         = PICO_DEFAULT_LED_PIN;

const uint   cw_out          = 7;

unsigned int        wpm = 12;

uint64_t ditTime = 1200000/wpm;
uint64_t dahTime = 3 * ditTime;
uint64_t keyDownTime = ditTime;
unsigned int        sidetone_freq = 800;        // 800 Hz


enum KSTYPE {IDLE, CHK_DIT, CHK_DAH, KEYED_PREP, KEYED, INTER_ELEMENT };

int keyerState = IDLE;

int ditPressed = 0;
int dahPressed = 0;
int ditProcess = 0; // dit is being processed

void update_paddle_state(void) {
    if (gpio_get(DitPin) == 0) {
	ditPressed = 1;
    }
    if (gpio_get(DahPin) == 0) {
	dahPressed = 1;
    }
}

int main() {
    uint64_t t1 = 0;
    bi_decl(bi_program_description("First Blink"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

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

    Tone myPlayer(cw_out);
    myPlayer.init(TONE_NON_BLOCKING);

    bool dit, dah;
    uint64_t ktimeout = 0;

    while (1) {
	switch (keyerState) {
	case IDLE:
	    if (gpio_get(DitPin) == 0) {
		update_paddle_state();
		keyerState = CHK_DIT;
	    }
	    if (gpio_get(DahPin) == 0) {
		update_paddle_state();
		keyerState = CHK_DIT;
	    }
	    break;
	case CHK_DIT:
	    // check once again;
	    if (gpio_get(DitPin) == 0) {
		// setup timer
		keyDownTime = ditTime;
		keyerState = KEYED_PREP;
		ditProcess = 1;
	    } else {
		keyerState = CHK_DAH;
	    }
	    break;
	case CHK_DAH:
	    if (gpio_get(DahPin) == 0) {
		dahPressed = 1;
		keyDownTime = 3*ditTime;
		keyerState = KEYED_PREP;
	    } else {
		keyerState = IDLE;
	    }
	    break;
	case KEYED_PREP:
	    gpio_put(LED_PIN, 1);         // turn the LED on
	    myPlayer.tone(sidetone_freq);

	    t1 = time_us_64();
	    ktimeout = t1 + keyDownTime;
	    keyerState = KEYED;

	    break;
	case KEYED:
	    if (time_us_64() > ktimeout) {
		gpio_put(LED_PIN, 0);
		myPlayer.stop();

		// we now enter inter-element time
		keyerState = INTER_ELEMENT;
		ktimeout = time_us_64() + ditTime;
	    }
	    break;
	case INTER_ELEMENT:
	    if (time_us_64() > ktimeout) {
		if (ditProcess) {
		    // reset dit
		    ditPressed = 0;
		    ditProcess = 0; // dit has been processed
		    keyerState = CHK_DAH;
		} else {
		    dahPressed = 0;
		    keyerState = IDLE;
		}
	    }
	    break;
	default:
	    break;
	}
    }
}
