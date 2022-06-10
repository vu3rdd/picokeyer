// Note: This file only has the iambic logic and no I/O, interrupts etc.
// Hence, it could be reused in other projects. Reading events and producing
// outputs and actual interfacing with hardware should be done elsewhere.

typedef enum states {
    IDLE,
    DIT,
    DAH,
    DIDAH,  // useful while a DIT is in progress, a DAH event comes in, 
    DADIT,  // useful while a DAH is in progress, a DIT event comes in
    INTER_ELEMENT,
} state;

typedef enum events {
    PRESS_DIT,
    PESS_DAH,
    PRESS_BOTH, // NOT really possible - either a dit has to be
		// leading or a dah has to be leading.
    RELEASE_DIT,
    RELEASE_DAH,
    RELEASE_BOTH,
    TIMER_EVENT, // an event generated when "element time"
		 // expires. Element time is calculated from "wpm"
		 // needed and from the "PARIS" convention. PARIS is
		 // 50 elements. So, 20 wpm = 20*50 = 1000 elements
		 // per 60 seconds, so one element is 60/1000 seconds
		 // or 60ms.
} event;

// https://users.ox.ac.uk/~malcolm/radio/8044print.pdf
// https://ag6qr.net/index.php/2017/01/06/iambic-a-or-b-or-does-it-matter/#comment-61
// http://morse-rss-news.sourceforge.net/keyerdoc/K7QO_Iambic_Paddle.pdf

// Notes
//
// 1. As we sample the dot and dash inputs, put the pair (event,
// current state) into a queue
// 2. Only Mode A is going to be implemented for now.

state initial_state = IDLE;

state next_state[state cur_state][event incoming_event] = {
};