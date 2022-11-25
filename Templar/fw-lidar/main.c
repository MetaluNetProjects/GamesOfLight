/*********************************************************************
 *               BNO055 example for Versa1.0
 *	SCL is K5 and SDA is K7.
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <i2c_master.h>
//#include <analog.h>
#include "tf_luna.h"

t_delay mainDelay;
#define LIDAR_BASEADDRESS 20
unsigned char autoLidar = 1;

#define LIDAR_IS_POWERED (digitalRead(LIDAR_POWEROFF) == 0)

void i2cWriteReg(byte address, byte reg, byte value)
{
	i2cm_begin(address,0);
	i2cm_writechar(reg);
	i2cm_writechar(value);
	i2cm_stop();
}

byte i2cReadReg(byte address, byte reg)
{
	byte c;
	i2cm_begin(address, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(address, 1);
	c = i2cm_readchar();
	i2cm_stop();
	return c;
}

int i2cReadInt(byte address, byte reg)
{
	int16_t i;
	i2cm_begin(address, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(address, 1);
	i = i2cm_readchar();
	i2cm_ack(); 
	i |= (i2cm_readchar() << 8);
	i2cm_stop();
	return i;
}

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED);		// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED
	delayStart(mainDelay, 50000);// init the mainDelay to 5 ms

	// setup I2C master:
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, 255/*(unsigned char)(FOSC/100000/4-1)*/);
	
	//analogInit();
	//analogSelect(0, K1);

	pinModeDigitalOut(LIDAR_POWEROFF);
	digitalClear(LIDAR_POWEROFF);
}

unsigned char buf[11];
unsigned char len;

void distanceService()
{
	static unsigned char n = 0;
	static unsigned char buf[14] = {'B', 10};
	unsigned int i;

	i = i2cReadInt(LIDAR_BASEADDRESS + n, 0);
	buf[n * 2 + 2] = i >> 8;
	buf[n * 2 + 3] = i & 255;
	n++;
	if(n == 5) {
		n = 0;
		buf[12] = '\n';
		fraiseSend(buf, 13);
	}
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	//distanceService();
	//analogService();
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 2000); 	// re-init mainDelay
		//analogSend();
		if(autoLidar) distanceService();
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
}

void fraiseReceive() // receive
{
	unsigned char c;
	int i;
	
	c=fraiseGetChar();
	if(c == 10){		//accel Threshold
		//accelThreshold = fraiseGetInt();
	}
	else if(c == 20) {
		distanceService();
	} 
	else if(c == 21) {
		autoLidar = (fraiseGetChar() != 0);
	} 
	else if(c == 40) {
		c = fraiseGetChar();
		i = i2cReadInt(c, 0);
		printf("Cl %d %d\n", c, i);
	}
	else if(c == 41) { // set address
		unsigned char old_addr, new_addr;
		old_addr = fraiseGetChar();
		new_addr = fraiseGetChar();
		i2cWriteReg(old_addr, TFL_SLAVE_ADDR, new_addr);
		//i2cWriteReg(new_addr, TFL_SAVE, 1);
	}
	else if(c == 42) { // reboot
		c = fraiseGetChar();
		i2cWriteReg(c, TFL_SHUTDOWN, 2);
	}
	else if(c == 43) { // save
		c = fraiseGetChar();
		i2cWriteReg(c, TFL_SAVE, 0x01);
	}
	else if(c == 100) { // restart i2c
		i2cm_init(I2C_MASTER, I2C_SLEW_ON, 255/*(unsigned char)(FOSC/100000/4-1)*/);
	}
	else if(c == 110) { // poweroff lidar
		c = fraiseGetChar();
		digitalWrite(LIDAR_POWEROFF, c != 0);
	}

}

