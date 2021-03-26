/*

* define.h
 *
 *  Created on: May 23, 2017
 *      Author: hailiang
 */


/*
 * 项目主要的头文件，要自定义一些值、变量的都可以在这里定义。
 * 此外，在生成.c文件的同时，必须配套地生成一个.h文件。
 * 其中，在生成的.h文件中，将define.h文件放进去。
 * */

#ifndef DEFINE_H_
#define DEFINE_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef enum {
	false = 0,
	true = 1,
}bool;

#define BIT0	0x01
#define BIT1	0x02
#define BIT2	0x04
#define BIT3	0x08
#define BIT4	0x10
#define BIT5	0x20
#define BIT6	0x40
#define BIT7	0x80

#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include "io.h"
#include "priv/alt_legacy_irq.h"
#include "unistd.h"


#endif /* DEFINE_H_ */
