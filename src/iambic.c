// Note: This file only has the iambic logic and no I/O, interrupts
// etc.  Hence, it could be reused in other projects. Reading events
// and producing sound/other outputs and actual interfacing with
// hardware should be done elsewhere.

typedef enum states {
    IDLE,
    DIT,
    DAH,
    DIDAH,  // useful while a DIT is in progress, a DAH event comes in, 
    DADIT,  // useful while a DAH is in progress, a DIT event comes in
    INTER_ELEMENT,
    LAST_STATE
} state;

typedef enum events {
    PRESS_DIT,
    PRESS_DAH,
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
    LAST_EVENT
} event;

// A few helpful documents on Iambic keying and on Mode A and Mode B differences.
//
// https://users.ox.ac.uk/~malcolm/radio/8044print.pdf
// https://ag6qr.net/index.php/2017/01/06/iambic-a-or-b-or-does-it-matter/#comment-61
// http://morse-rss-news.sourceforge.net/keyerdoc/K7QO_Iambic_Paddle.pdf
// http://wb9kzy.com/modeab.pdf

// Notes
//
// 1. As we sample the dot and dash inputs, put the pair (event,
// current state) into a queue
// 2. Only Mode A is going to be implemented for now.

state initial_state = IDLE;

typedef state (*event_handler)(state cur_state, event incoming_event);

state transitions[LAST_STATE][LAST_EVENT] = {
    [IDLE] = {
	[PRESS_DIT] = DIT,
	[PRESS_DAH] = DAH,
	[TIMER_EVENT] = IDLE,
    },
};
