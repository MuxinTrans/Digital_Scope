/*
 * i2c.h
 *
 *  Created on: May 23, 2017
 *      Author: hailiang
 */

#ifndef I2C_H_
#define I2C_H_

#include "define.h"

#define SCL_H IOWR(TOUCH_SCL_BASE, 0, 1)
#define SCL_L IOWR(TOUCH_SCL_BASE, 0, 0)
#define SDA_H IOWR(TOUCH_SDA_BASE, 0, 1)
#define SDA_L IOWR(TOUCH_SDA_BASE, 0, 0)

#define SDA_OUT IOWR(TOUCH_SDA_BASE, 1, 1)
#define SDA_IN IOWR(TOUCH_SDA_BASE, 1, 0)

#define SDA_R IORD(TOUCH_SDA_BASE, 0)

void i2cInit(void);
void i2cDelay(void);
bool i2cStart(void);
void i2cStop(void);
void i2cAck(void);
void i2cNoAck(void);
bool i2cWaitAck(void);

void i2cSendByte(u8 byte);
u8 i2cReadByte(void);
bool i2cWriteReg(u8 slave, u8 reg, u8 data);
bool i2cWriteMulti(u8 slave, u8 reg, u8 *buff, u8 cnt);
u8 i2cReadReg(u8 slave, u8 reg);
bool i2cReadMulti(u8 slave, u8 reg, u8 *buff, u8 cnt);

#endif /* I2C_H_ */
