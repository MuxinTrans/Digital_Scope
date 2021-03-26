/*
 * ctrl.c
 *
 *  Created on: May 23, 2017
 *      Author: hailiang
 */

#include "ctrl.h"

int deviceInit() {
	if (keyInit() < 0) {
		printf("key initial failed\n");
		return -1;
	}
	if (touchInit() < 0) {
		printf("touch initial failed\n");
		return -1;
	}
	if (timerInit() < 0) {
		printf("timer initial failed\n");
		return -1;
	}
	lcdInit();

	return 0;
}




void ctrlInit() {
//	lcdDispStringBig(10, 10, BLACK, WHITE, "This is a new lcd test Program");
//	lcdDispStringBig(10, 40, BLACK, WHITE, "Decimal display test:");
//	lcdDispDecSmall(50, 70, BLACK, WHITE, -123123);
//	lcdDispStringBig(10, 100, BLACK, WHITE, "Float display test:");
//	lcdDispFloatBig(50, 130, BLACK, WHITE, -9876.54);
//	lcdDispStringBig(10, 160, BLACK, WHITE, "Touch test:");
//	lcdDispStringBig(50, 190, BLACK, WHITE, "draw with your finger");
//	lcdDispStringBig(10, 220, BLACK, WHITE, "Key test:");
//	lcdDispStringBig(50, 250, BLACK, WHITE, "press any key to clear");
//	lcdDispStringBig(10, 280, BLACK, WHITE, "Gesture test:");
//	lcdDrawGrid(350, 50, 8, 10, 40, BLUE);

//	lcdDispStringBig(735,410, BLUE, WHITE,"KEY");
}

int timerInit(void) {
	IOWR(TIMER0_BASE, 1, 0x07);
	return alt_irq_register(TIMER0_IRQ, NULL, (void*) timerIsr);
}

tcdata top = { .id = 1, .status = TOUCH_NONE, };

void timerIsr(void) {
	IOWR(TIMER0_BASE, 0, 0);

	touchGetData(&top);
	if (top.status == TOUCH_DOWN) {
		// do something
	} else if (top.status == TOUCH_CONTACT) {
		lcdDrawLine(top.old.x, top.old.y, top.now.x, top.now.y, RED);
	} else if (top.status == TOUCH_UP) {
		// do something
	}
}


int partition(float array[], int min, int max)
{
    float p;  p = array[min];
    //int len = max;
    while (min < max)
    {
        while (array[max] >= p && min < max)
        {
            max--;
        }
        array[min] = array[max];
        while (array[min] <= p && min < max)
        {
            min++;
        }
        array[max] = array[min];
    }
    array[min] = p;
    //printf("分界：%d\n", min);
    return  min;
}

void quicksort(float array[], int min, int max)
{
    int p;
    int i;
    p = partition(array, min, max);

    if (min < max)
    {
        quicksort(array, min, p - 1);
        quicksort(array, p + 1, max);
    }
}
void change(float array[],int n){// 参数分别是数组和数组大小（从1开始的）
int i;
float temp;
for(i=0;i<n/2;i++)
{
temp=array[i];
array[i]=array[n-1-i];
array[n-1-i]=temp;
}
}
