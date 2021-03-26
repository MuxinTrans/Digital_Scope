/*
 * touch.c
 *
 *  Created on: May 23, 2017
 *      Author: hailiang
 */

#define bit_length 8

#include "touch.h"
#include "lcd.h"

extern int Num_X = 30, Num_Y = 20;
extern int flag = 0;
extern int x_mod = 2, y_mod = 2;		// xmod:0-100ns	1-2us 2-20ms	ymod:0-2mV 1-0.1V 2-1V

int bit_array[bit_length] = {0};
int bit_pointer = 0;
int Range_w = 0;//用来表征修改几个不同位置的数字的值
double num_output = 0;
int mode = 1;


int touchInit(void) {
	i2cInit();

	// normal mode
	i2cWriteReg(FT_DEVICE_ADDR, FT_DEV_MODE, 0);
	// polling mode
	i2cWriteReg(FT_DEVICE_ADDR, FT_ID_G_MODE, 0);
	// threshold
	i2cWriteReg(FT_DEVICE_ADDR, FT_ID_G_THGROUP, 22);
	// auto calibration mode
	i2cWriteReg(FT_DEVICE_ADDR, FT_ID_G_AUTO_CLB_MODE, 0);
	// period of active status 12
	i2cWriteReg(FT_DEVICE_ADDR, FT_ID_G_PERIODACTIVE, 12);

	touchIrqInit();
	return 0;
}

int touchIrqInit(void) {
	IOWR(TOUCH_IRQ_BASE, 2, 0x01);
	IOWR(TOUCH_IRQ_BASE, 3, 0x00);
	return alt_irq_register(TOUCH_IRQ_IRQ, NULL, (void *) touchIsr);
}

tcdata local = { .id = 1, .status = TOUCH_NONE, };

int touchResponse(int x_low, int x_high, int y_low, int y_high, int page) {//按键响应函数，按下变色
	if((local.now.x > x_low) && (local.now.x < x_high) && (local.now.y > y_low) && (local.now.y < y_high) && (local.status == 0))
	{
		lcdRectClear(x_low, y_low, x_high,y_high, DGRAY);
//		printf("Touch Response!\n");
		return 1;
	}
	else if((local.now.x > x_low) && (local.now.x < x_high) && (local.now.y > y_low) && (local.now.y < y_high) && (local.status == 1))
	{
		lcdRectClear(x_low, y_low, x_high,y_high, BACKGROUND);
		return 0;
	}
	else
		return 2;
}

unsigned char TouchValidFlag=0;
char TouchValue='0';
void touchIsr(void* isr_context) {
	IOWR(TOUCH_IRQ_BASE, 3, 0x00);

	touchGetRaw(&local);
	TouchValidFlag=1;

	button_table(0);

}

void Change_value(int num_in){
	if((bit_pointer == 0) && (Range_w)){
		lcdRectClear(126, 425, 198, 437, BACKGROUND);
	}
	else if((bit_pointer == 0) && (!Range_w)){
		lcdRectClear(126, 385, 198, 397, BACKGROUND);
	}

	if(bit_pointer <= bit_length-1)
	{
		bit_pointer++;
		bit_array[bit_pointer]=num_in;
		if(Range_w)
			lcdDispDecSmall(120+(bit_pointer*9), 425, BLACK, WHITE, bit_array[bit_pointer]);
		else
			lcdDispDecSmall(120+(bit_pointer*9), 385, BLACK, WHITE, bit_array[bit_pointer]);
	}
}

void button_table(int page){
//	if(touchResponse(10, 760, 10, 470, page) == 1)
//	{
//		/*按键功能定义区——Begin*/
//		flag = 1;
//		/*按键功能定义区——End*/
//	}
	/****************** -1- *********************/
	if(touchResponse(Num_X, Num_X+50, Num_Y, Num_Y+50, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(1);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+50, Num_Y, Num_Y+50, page) == 0)
	{
		lcddrawsqur(Num_X, Num_X+50, Num_Y, Num_Y+50, BLACK, "  1");
	}
	/****************** -2- *********************/
	if(touchResponse(Num_X+50, Num_X+100, Num_Y, Num_Y+50, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(2);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+50, Num_X+100, Num_Y, Num_Y+50, page) == 0)
	{
		lcddrawsqur(Num_X+50, Num_X+100, Num_Y, Num_Y+50, BLACK, "  2");
	}

	/****************** -3- *********************/
	if(touchResponse(Num_X+100, Num_X+150, Num_Y, Num_Y+50, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(3);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+100, Num_X+150, Num_Y, Num_Y+50, page) == 0)
	{
		lcddrawsqur(Num_X+100,Num_X+150,Num_Y,Num_Y+50,BLACK, "  3");
	}

	/****************** -4- *********************/
	if(touchResponse(Num_X, Num_X+50, Num_Y+50, Num_Y+100, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(4);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+50, Num_Y+50, Num_Y+100, page) == 0)
	{
		lcddrawsqur(Num_X,Num_X+50,Num_Y+50,Num_Y+100,BLACK, "  4");
	}

	/****************** -5- *********************/
	if(touchResponse(Num_X+50, Num_X+100, Num_Y+50, Num_Y+100, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(5);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+50, Num_X+100, Num_Y+50, Num_Y+100, page) == 0)
	{
		lcddrawsqur(Num_X+50, Num_X+100, Num_Y+50, Num_Y+100, BLACK, "  5");
	}

	/****************** -6- *********************/
	if(touchResponse(Num_X+100, Num_X+150, Num_Y+50, Num_Y+100, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(6);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+100, Num_X+150, Num_Y+50, Num_Y+100, page) == 0)
	{
		lcddrawsqur(Num_X+100, Num_X+150, Num_Y+50, Num_Y+100, BLACK, "  6");
	}

	/****************** -7- *********************/
	if(touchResponse(Num_X, Num_X+50, Num_Y+100, Num_Y+150, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(7);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+50, Num_Y+100, Num_Y+150, page) == 0)
	{
		lcddrawsqur(Num_X, Num_X+50, Num_Y+100, Num_Y+150, BLACK, "  7");
	}

	/****************** -8- *********************/
	if(touchResponse(Num_X+50, Num_X+100, Num_Y+100, Num_Y+150, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(8);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+50, Num_X+100, Num_Y+100, Num_Y+150, page) == 0)
	{
		lcddrawsqur(Num_X+50, Num_X+100, Num_Y+100, Num_Y+150, BLACK, "  8");
	}

	/****************** -9- *********************/
	if(touchResponse(Num_X+100, Num_X+150, Num_Y+100, Num_Y+150, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(9);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+100, Num_X+150, Num_Y+100, Num_Y+150, page) == 0)
	{
		lcddrawsqur(Num_X+100, Num_X+150, Num_Y+100, Num_Y+150, BLACK, "  9");
	}

	/****************** -.- *********************/
	if(touchResponse(Num_X, Num_X+50, Num_Y+150, Num_Y+200, page)==1)
	{
		/*按键功能定义区——Begin*/
		for(int i=1; i <= bit_pointer; i++)
		{
			num_output = num_output+bit_array[i]*pow(10,bit_pointer-i);
			bit_array[i]=0;
		}
		bit_pointer = 0;
		if(Range_w){
			if(num_output > 10000000){
				num_output = 10000000;
				lcdRectClear(126, 425, 198, 437, BACKGROUND);
				lcdDispDecSmall(129, 425, BLACK, WHITE, num_output);
			}
//			Range_up_data = num_output;
			num_output = 0;
		}
		else{
			if(num_output < 100){
				num_output = 100;
				lcdRectClear(126, 385, 198, 397, BACKGROUND);
				lcdDispDecSmall(129, 385, BLACK, WHITE, num_output);
			}
//			Range_down_data = num_output;
			num_output = 0;
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+50, Num_Y+150, Num_Y+200, page)==0)
	{
		lcddrawsqur(Num_X, Num_X+50, Num_Y+150, Num_Y+200, BLACK, " .");
	}

	/****************** -0- **********************/
	if(touchResponse(Num_X+50, Num_X+100, Num_Y+150, Num_Y+200, page) == 1)
	{
		/*按键功能定义区——Begin*/
		Change_value(0);
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+50, Num_X+100, Num_Y+150, Num_Y+200, page) == 0)
	{
		lcddrawsqur(Num_X+50, Num_X+100, Num_Y+150, Num_Y+200, BLACK, "  0");
	}

	/***************** -clc- *********************/
	if(touchResponse(Num_X+100, Num_X+150, Num_Y+150, Num_Y+200, page)==1)
	{
		/*按键功能定义区——Begin*/
		for(int i = 0; i <= bit_pointer; i++)
		{
			bit_array[i] = 0;
		}
		if(Range_w)
			lcdRectClear(126, 425, 198, 437, BACKGROUND);
		else
			lcdRectClear(126, 385, 198, 397, BACKGROUND);
		bit_pointer = 0;
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+100, Num_X+150, Num_Y+150, Num_Y+200, page) == 0)
	{
		lcddrawsqur(Num_X+100, Num_X+150, Num_Y+150, Num_Y+200, BLACK, " clc");
	}

	/***************** -Save- ********************/
	if(touchResponse(Num_X, Num_X+75, Num_Y+250, Num_Y+290, page)==1)
	{
		/*按键功能定义区——Begin*/

		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+75, Num_Y+250, Num_Y+290, page) == 0)
	{
		lcddrawsqur(Num_X, Num_X+75, Num_Y+250, Num_Y+290, BLACK, "Save");
	}

	/***************** -Call- *********************/
	if(touchResponse(Num_X+75, Num_X+150, Num_Y+250, Num_Y+290, page)==1)
	{
		/*按键功能定义区——Begin*/
		if(Range_w == 1){
			Range_w = 0;//修改F_L的值
			for(int i = 0; i <= bit_pointer; i++)
			{
				bit_array[i] = 0;
			}
			bit_pointer = 0;
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+75, Num_X+150, Num_Y+250, Num_Y+290, page) == 0)
	{
		lcddrawsqur(Num_X+75, Num_X+150, Num_Y+250, Num_Y+290, BLACK, "Call");
	}

	/***************** -Single- *********************/
	if(touchResponse(Num_X+75, Num_X+150, Num_Y+210, Num_Y+250, page)==1)
	{
		/*按键功能定义区——Begin*/
		if(!Range_w){
			Range_w = 1;//修改F_H
			for(int i = 0; i <= bit_pointer; i++)
			{
				bit_array[i] = 0;
			}
			bit_pointer = 0;
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X+75, Num_X+150, Num_Y+210, Num_Y+250, page) == 0)
	{
		lcddrawsqur(Num_X+75, Num_X+150, Num_Y+210, Num_Y+250, BLACK, "Single");
	}

	/****************** -SetTri- *********************/
	if(touchResponse(Num_X, Num_X+75, Num_Y+210, Num_Y+250, page)==1)
	{
		/*按键功能定义区——Begin*/
		for(int i=1; i <= bit_pointer; i++)
		{
			num_output = num_output+bit_array[i]*pow(10,bit_pointer-i);
			bit_array[i]=0;
		}
		bit_pointer = 0;
		if(Range_w){
			if(num_output > 10000000){
				num_output = 10000000;
				lcdRectClear(126, 425, 198, 437, BACKGROUND);
				lcdDispDecSmall(129, 425, BLACK, WHITE, num_output);
			}
//			Range_up_data = num_output;
			num_output = 0;
		}
		else{
			if(num_output < 100){
				num_output = 100;
				lcdRectClear(126, 385, 198, 397, BACKGROUND);
				lcdDispDecSmall(129, 385, BLACK, WHITE, num_output);
			}
//			Range_down_data = num_output;
			num_output = 0;
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+75, Num_Y+210, Num_Y+250, page)==0)
	{
		lcddrawsqur(Num_X, Num_X+75, Num_Y+210, Num_Y+250, BLACK, "SetTri");
	}

	/***************** -Xlable- *********************/
	if(touchResponse(Num_X, Num_X+70, Num_Y+300, Num_Y+330, page)==1)
	{
		/*按键功能定义区——Begin*/
		if(x_mod == 0){
			x_mod = 1;
			lcdRectClear(Num_X+75, Num_Y+305, Num_X+150, Num_Y+328, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "2us");
		}
		else if(x_mod == 1){
			x_mod = 2;
			lcdRectClear(Num_X+75, Num_Y+305, Num_X+150, Num_Y+328, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "20ms");
		}
		else if(x_mod == 2){
			x_mod = 0;
			lcdRectClear(Num_X+75, Num_Y+305, Num_X+150, Num_Y+328, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "100ns");
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+70, Num_Y+300, Num_Y+330, page) == 0)
	{
		lcddrawsqur(Num_X, Num_X+70, Num_Y+300, Num_Y+330, BLACK, "Xlable");
	}
	/***************** -Ylable- *********************/
	if(touchResponse(Num_X, Num_X+70, Num_Y+330, Num_Y+360, page)==1)
	{
		/*按键功能定义区——Begin*/
		if(y_mod == 0){
			y_mod = 1;
			lcdRectClear(Num_X+75, Num_Y+335, Num_X+150, Num_Y+358, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "0.1V");
		}
		else if(y_mod == 1){
			y_mod = 2;
			lcdRectClear(Num_X+75, Num_Y+335, Num_X+150, Num_Y+358, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "1V");
		}
		else if(y_mod == 2){
			y_mod = 0;
			lcdRectClear(Num_X+75, Num_Y+335, Num_X+150, Num_Y+358, WHITE);
			lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "2mV");
		}
		/*按键功能定义区——End*/
	}
	else if(touchResponse(Num_X, Num_X+70, Num_Y+330, Num_Y+360, page) == 0)
	{
		lcddrawsqur(Num_X, Num_X+70, Num_Y+330, Num_Y+360, BLACK, "Ylable");
	}
}

const u8 tpid[5] = { FT_TP1, FT_TP2, FT_TP3, FT_TP4, FT_TP5 };
void touchGetData(tcdata *data) {

	if (local.status == TOUCH_DOWN) {
		data->status = TOUCH_DOWN;
		data->now = local.now;

		local.status = TOUCH_CONTACT;
	} else if (local.status == TOUCH_CONTACT) {
		touchGetRaw(data);
	} else if (local.status == TOUCH_UP) {
		data->status = TOUCH_UP;
		data->now = local.now;

		local.status = TOUCH_NONE;
	} else {
		data->status = TOUCH_NONE;
	}
}


void touchGetRaw(tcdata *data) {
	u8 buff[4] = { 0 };

	i2cReadMulti(FT_DEVICE_ADDR, tpid[data->id - 1], buff, 4);

	data->status = (buff[0] >> 6) & 0x03;
	data->old = data->now;
	data->now.y = ((buff[0] & 0x0f) << 8) + buff[1];
	data->now.x = ((buff[2] & 0x0f) << 8) + buff[3];
}

u8 touchGetGesture(void) {
	return i2cReadReg(FT_DEVICE_ADDR, FT_GEST_ID);
}

