/*********************************************************************
 *               analog example for Versa2.0
 *	Analog capture on connector K1
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>

t_delay mainDelay;
unsigned int threshold = 120;
unsigned char presence = 0;
unsigned char triggered = 0;
unsigned int maximum;
t_delay maxDelay;
unsigned int millisCount = 0;
unsigned int window_ms = 100;
unsigned int restart_ms = 500;
//#define WINDOW_MS 100
//#define RESTART_MS 500

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
	
	analogSelect(0,K1);	// assign connector K1 to analog channel 0
}

void trampoService()
{
	unsigned int current = analogGet(0);
	unsigned char tmp_presence = analogGet(0) > threshold;
	
	if(presence == 0) {
		if(tmp_presence == 0) return;
		maximum = current;
		presence = 1;
		triggered = 1;
		millisCount = 0;
	} else {
		if(triggered) {
			if(current > maximum) maximum = current;
			if(millisCount > window_ms) {
				printf("CT %d\n", maximum);
				triggered = 0;
			}
			return;
		}
		if(tmp_presence) millisCount = 0;
		else {
			if(millisCount > restart_ms) presence = 0;
		}
	}
}

unsigned char analogSendCount;
void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	trampoService();
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		if(analogSendCount++ > 20) {
			analogSend();		// send analog channels that changed
			analogSendCount = 0;
		}
		millisCount += 5;
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

void fraiseReceive()
{
	unsigned char c = fraiseGetChar();
	
	switch(c) {
		PARAM_INT(1, threshold); break;
		PARAM_INT(2, window_ms); break;
		PARAM_INT(3, restart_ms); break;
	}
}
