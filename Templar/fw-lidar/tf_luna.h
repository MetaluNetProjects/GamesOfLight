#ifndef _TF_LUNA_H_
#define _TF_LUNA_H_

typedef enum {
	TFL_DIST_LOW = 		0x00, // cm
	TFL_DIST_HIGH = 		0x01,
	TFL_AMP_LOW = 		0x02,
	TFL_AMP_HIGH = 		0x03,
	TFL_TEMP_LOW = 		0x04, // Unit: 0.01 Celsius
	TFL_TEMP_HIGH = 	0x05,
	TFL_SAVE = 			0x20, // Write 0x01 to save current setting
	TFL_SHUTDOWN = 		0x21, // SHUTDOWN/REBOOT Write 0x02 to reboot
	TFL_SLAVE_ADDR = 	0x22, // Range: [0x08, 0x77]
} TfLuna_I2Cregs;

#endif // _TF_LUNA_H_

