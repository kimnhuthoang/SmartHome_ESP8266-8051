
#ifndef	_soft_i2c_h_
#define _soft_i2c_h_

#define SCL P2_6
#define SDA P2_7

// Init
void i2c_init();

// Master generate Start signal
void i2c_start();

// Master generate Stop signal
void i2c_stop();

// Write data to Slaver, and get ACK/NACK from Slaver
bit i2c_write(unsigned char dat);

// Read data from Slaver
unsigned char i2c_read(bit ack);

#endif
