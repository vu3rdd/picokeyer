#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

// GPIO pin definitions
const uint   DahPin          = 8;      // Dah paddle input or PTT
const uint   DitPin          = 11;       // Dit paddle input or KEY
const uint   LED_PIN         = PICO_DEFAULT_LED_PIN;

const uint   pwm_cw_out      = 4; // pwm "audio" out (for sidetone)
const uint   keyer_out       = 15; // actual dit/dah key down signal to drive the rig

unsigned int        wpm = 25;

uint64_t ditTime = 1200/wpm;
uint64_t dahTime = 3 * ditTime;
uint64_t keyDownTime = ditTime;
unsigned int        sidetone_freq = 800;        // 800 Hz


enum KSTYPE {IDLE, CHK_DIT, CHK_DAH, KEYED_PREP, KEYED, INTER_ELEMENT };

int keyerState = IDLE;

int ditPressed = 0;
int dahPressed = 0;
int ditProcess = 0; // dit is being processed

uint64_t millis(void) {
    uint64_t t1 = time_us_64();

    return t1/1000;
}

void update_paddle_state(void) {
    if (gpio_get(DitPin) == 0) {
	ditPressed = 1;
    }
    if (gpio_get(DahPin) == 0) {
	dahPressed = 1;
    }
}

// code from https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
uint32_t pwm_set_freq_duty(uint slice_num,
			   uint chan,uint32_t f, int d)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / f / 4096 +
	(clock % (f * 4096) != 0);
    if (divider16 / 16 == 0)
	divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / f - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16/16,
                                     divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * d / 100);

    return wrap;
}

int main() {
    uint64_t t1 = 0;
    bi_decl(bi_program_description("First Blink"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(keyer_out);
    gpio_set_dir(keyer_out, GPIO_OUT);

    gpio_init(DahPin);
    gpio_pull_up(DahPin);
    gpio_set_dir(DahPin, GPIO_IN);
    gpio_set_input_hysteresis_enabled(DahPin, true);

    gpio_init(DitPin);
    gpio_pull_up(DitPin);
    gpio_set_dir(DitPin, GPIO_IN);
    gpio_set_input_hysteresis_enabled(DitPin, true);

    gpio_set_function(pwm_cw_out, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pwm_cw_out);
    uint channel = pwm_gpio_to_channel(pwm_cw_out);

    uint32_t wrap = pwm_set_freq_duty(slice, channel, 800, 50);

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
	    // t1 = time_us_64();
	    t1 = millis();
	    pwm_set_enabled(slice, true);
	    //gpio_put(LED_PIN, 1);         // turn the LED on
	    gpio_put(keyer_out, 1);
	    ktimeout = t1 + keyDownTime;
	    keyerState = KEYED;

	    break;
	case KEYED:
	    if (millis() > ktimeout) {
		t1 = millis();
		pwm_set_enabled(slice, false);
		// gpio_put(LED_PIN, 0);
		gpio_put(keyer_out, 0);

		// we now enter inter-element time
		keyerState = INTER_ELEMENT;
		ktimeout = t1 + ditTime;
	    }
	    break;
	case INTER_ELEMENT:
	    if (millis() > ktimeout) {
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
