/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "define.h"
#include "ctrl.h"
#include "system.h"

#define Vref 2000		// 0V参考输入值
#define Mul1 3.917
#define Mul2 1/5.6
#define Mul3 1/117.33
// switch_mod =2--Mul1; =1--Mul2; =0--Mul3;

double vpp, vmax, vmin, fre_meas, vac_zero;
double vpp2, vmax2, vmin2, fre_meas2, vac_zero2;
u16 rdv = 0, rdmax = 0, rdmin = 0, rdac_zero, rdac_zero_2;
int KW_word, Times;
int signal_num = 0;
int sig1[512] = {0}, sig2[512] = {0};		// 1当前 2存储
float sig1_real[512] = {0}, sig2_real[512] = {0};
float ytrans[400] = {0}, ycachu[400] = {0};
double Mult,Mult2;
double trig_show, trig_cachu;

//flag
int clk_rd, storage_done;	//0未1已
int KW_init = pow(2,32)/401, Times_init = 100100;
int switch_mod = 2;
int ifChoosed = 1;

void init();
void init_dis();
void init_flag();

void getSampclk(float fre_now);
double getFre();
void disFre(double fre_now);

double getVpp(double vrd,double multi);
void Amp();
void disAmp(double v2wr,int w);

void fifo_rd(void);
void XTrans(float fre_now);
void YTrans();
void Storage(/*int ifStore*/);

void Auto(/*int ifAuto*/);
void Single();
void Recall();

void setSwitch(/*int ifChoosed*/);

int main() {
	//开始执行程序
	printf("Hello lemon\n");
	if (deviceInit() < 0) {
		printf("device initial failed\n");
		return -1;
	}

//	lcdRectClear(0, 0, 799, 479, WHITE);

	init();

	//核心代码
	while (1)
	{
		/*********************Switch档位选择--Begin********************************/
		setSwitch();

		if(ifChoosed){
			double vpp_init;
			while(!IORD(VPP_FOUND_BASE, 0));
			vpp_init = getVpp(IORD(VPP_BASE, 0),Mult);
			if(switch_mod == 0){						//--100
				if(vpp_init > 4000){
					/////////////////上限值要修改！！！
					IOWR(SWITCH_MOD_BASE, 0, 1);		//--5
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul2);
					if(vpp_init > 139 && vpp_init < 4000){
						switch_mod = 1;
						Mult = Mul2;
						ifChoosed = 1;
					}
					else if(vpp >= 4000){				//--1/3
						IOWR(SWITCH_MOD_BASE, 0, 2);
						switch_mod = 2;
						Mult = Mul1;
						ifChoosed = 1;
					}
				}
			}
			else if(switch_mod == 1){					//--5
				if(vpp_init > 4000){
					/////////////////上限值要修改！！！
					IOWR(SWITCH_MOD_BASE, 0, 2);		//--1/3
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul1);
					switch_mod = 2;
					Mult = Mul1;
					ifChoosed = 1;
				}
				else if(vpp_init < 140){
					IOWR(SWITCH_MOD_BASE, 0, 0);		//--100
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul3);
					switch_mod = 0;
					Mult = Mul3;
					ifChoosed = 1;
				}
			}
			else if(switch_mod == 2){					//--1/3
				if(vpp_init < 127){
					IOWR(SWITCH_MOD_BASE, 0, 1);
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul2);
					if(vpp_init > 139){					//--5
						switch_mod = 1;
						Mult = Mul2;
					}
					else{
						IOWR(SWITCH_MOD_BASE, 0, 0);	//--100
						switch_mod = 0;
						Mult = Mul3;
					}
				}
			}
		}

		ifChoosed = (ifChoosed+1)%20;

		/*********************Switch档位选择--End********************************/

		fre_meas = getFre();
		disFre(fre_meas);		//fre_meas
		Amp();					//vpp | rdv | rdmax | rdmin

		Auto(ifAuto);
		/*********************手动调波--Begin**************************************/
/*		if((!ifAuto)&&(!ifSingle)){
			XTrans(fre_meas);
			fifo_rd();
			YTrans();

			int tri_num;
			for(int i = 2; i < 400; i++){
				if((sig1_real[i]>=trigger_v) && (sig1_real[i-2]<trigger_v) && (sig1_real[i-1]<trigger_v)){
					tri_num = i;
					printf("tri_num = %d.\n",tri_num);
					break;
				}
			}
			for(int i = tri_num; i < tri_num+200; i++){
				lcdDrawLine((i-tri_num)*2+220, ycachu[i], (i-tri_num)*2+222, ycachu[i+1], WHITE);
				lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], BLACK);
			}

		}//手动调波最后的括号
		/*********************手动调波--End**************************************/
//		Single();

		if(flag == 1){

			flag = 0;
		}
	}//while的括号

	return 0;
}

/*****************************************初始化部分**************************************************/

void init(){
	init_dis();
	//传输数据初始化 | 标志位初始化
	init_flag();
}

void init_dis(){
	lcdRectClear(0, 0, 799, 479, WHITE);
	lcdDispNumtable(Num_X, Num_Y);
//	lcdDrawRect(220, 28, 620, 348, BLACK);
	lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);
	lcdDispStringSmall(Num_X+15, Num_Y+370, BLACK, WHITE, "V_Tri");

	lcdDispStringSmall(230, 358, BLUE, WHITE, "Vpp");
	lcdDispStringSmall(230, 378, BLUE, WHITE, "Fre");
//	lcdDispStringSmall(230, 398, BLUE, WHITE, "Vmax");
//	lcdDispStringSmall(230, 418, BLUE, WHITE, "Vmin");
}

void init_flag(){
	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_init);
	IOWR(SAMPLE_TIME_BASE, 0, Times_init);
}

/*************************************************************************************************
*	幅度测量模块---直流模式和交流模式
*	峰峰值、最大值、最小值测量:单位mV
*	实际电压值测量					----拟合
*	幅度显示：保留4位有效数字			mV|V
*************************************************************************************************/

//Amplitude
void Amp(){	//ifshow =0:初始化判断，不显示电平		=1:实际判断，显示电平
	while(!IORD(VPP_FOUND_BASE, 0));
	rdv = IORD(VPP_BASE, 0);
	vpp = getVpp(rdv,Mult);
	disAmp(vpp, 0);//w=0-vpp; 1-vmax; 2-vmin

	rdmax = IORD(V_MAX_BASE, 0);
	rdmin = IORD(V_MIN_BASE, 0);
	rdac_zero = (rdmax+rdmin)/2;

//	vmax = getVpp(rdmax-Vref);
//	disAmp(vmax, 1);//w=0-vpp; 1-vmax; 2-vmin
//	vmin = getVpp(rdmin-Vref);
//	disAmp(vmin, 2);//w=0-vpp; 1-vmax; 2-vmin

}

double getVpp(double vrd,double multi){
	double vin;
	vin = vrd * 1.312-1.323;
	if(vin <= 0){
		vin = vrd;
	}
	vin = vin*multi;
	return vin;
}

void disAmp(double v2wr, int w){//w=0-vpp; 1-vmax; 2-vmin
	double v_dis;
	int liangji = 0;	//0-mV;1-V
	if(v2wr < 1000){
		v_dis = v2wr;
		liangji = 0;
	}
	else{
		v_dis = v2wr/1000;
		liangji = 1;
	}

	int y_s, y_e;
	switch(w){
	case 0:	y_s = 358; y_e = 375;
		break;
	case 1: y_s = 398; y_e = 415;
		break;
	case 2: y_s = 418; y_e = 435;
		break;
	default:break;
	}
	lcdRectClear(270, y_s, 350, y_e, WHITE);
	lcdDispFloatSmall_4(270, y_s, BLACK, WHITE, v_dis);
	switch(liangji){
	case 0: lcdDispStringSmall(318, y_s, BLACK, WHITE, "mV");
		break;
	case 1: lcdDispStringSmall(318, y_s, BLACK, WHITE, "V");
		break;
	default: lcdDispStringSmall(318, y_s, BLACK, WHITE, "mV");
		break;
	}

}

/*************************************************************************************************
*	频率处理模块
*	频率测量:单位Hz			~~~频率测量模块时钟为100M
*	采样频率的确定			// 每一个图200个点：1000Hz~10MHz~200MHz
*	频率测量的显示			保留4位有效数字	Hz|kHz|MHz
*************************************************************************************************/

double getFre(){
	printf("Begin to measure fre.\n");
	int clk_num, sig_num;
	clk_num = IORD(FMEASURE_CLK_BASE, 0);
	sig_num = IORD(FMEASURE_SQR_BASE, 0);
	printf("clk_num = %d, sig_num = %d\n",clk_num,sig_num);
	if(clk_num)
		fre_meas = sig_num*100000000.0/(clk_num);
	else
		fre_meas = 0;

//	printf("***1***:%d\n***2***:%d\n***3***:%d\n",(sig_num*100000000/(clk_num)),(sig_num*10000000/(clk_num))*10,(sig_num*1000000/(clk_num))*100);
//	printf("***4***:%d\n***5***:%d\n***6***:%d\n",(sig_num*100000/(clk_num))*1000,(sig_num*10000/(clk_num))*10000,(sig_num*1000/(clk_num))*100000);
//	printf("***1***:%f\n***2***:%f\n\n",(sig_num*100000000.0/(clk_num)),fre_meas);

	disFre(fre_meas);

	printf("clk_num = %d, sig_num = %d.\n",clk_num,sig_num);
	printf("Finish measuring fre.\n\n");
	return fre_meas;
}

void getSampclk(float fre_now){
	if(fre_now == 0){
		KW_word = pow(2,32)/400;
		Times =200;
		IOWR(CLK_SAMPLE_KW_BASE, 0, KW_word);
		IOWR(SAMPLE_TIME_BASE, 0, Times);
		printf("0-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else if(fre_now <= 50){	//1000Hz
		x_mod = 0;
//		KW_word = pow(2,32)/400000;
//		Times = (50/fre_now)*40;
	}
	else if(fre_now<=500000){		//实时采样频率  10M
		x_mod = 1;
//		KW_word = pow(2,32)/40;
//		Times =(500000/fre_now)*40;
//		printf("1-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else{		//等效采样时钟200M--p130差拍时钟顺序等效采样法。
		x_mod = 2;
//		KW_word = (1/(1/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
//		Times = 4*pow(10,8)/fre_now;
//		printf("2-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
}

void disFre(double fre_now){
	double fre_dis;
	int liangji = 0;	//0-Hz;1-kHz;2-MHz
	if(fre_now < 1000){
		fre_dis = fre_now;
		liangji = 0;
	}
	else if(fre_now < 1000000){
		fre_dis = fre_now/1000;
		liangji = 1;
	}
	else{
		fre_dis = fre_now/1000000;
		liangji = 2;
	}

	lcdRectClear(270, 378, 350, 395, WHITE);
	lcdDispFloatSmall_4(270, 378, BLACK, WHITE, fre_dis);
	switch(liangji){
	case 0: lcdDispStringSmall(318, 378, BLACK, WHITE, "Hz");
		break;
	case 1: lcdDispStringSmall(318, 378, BLACK, WHITE, "kHz");
		break;
	case 2: lcdDispStringSmall(318, 378, BLACK, WHITE, "MHz");
		break;
	default: lcdDispStringSmall(318, 378, BLACK, WHITE, "Hz");
		break;
	}
}

/****************************************************************
*	波形处理部分
*	读取fifo--512个点		--头两个点丢弃
*	X轴坐标与Y轴坐标的变换		--与水平分辨率、垂直分辨率相关
*	波形存储
****************************************************************/

//Wave form Storage
void fifo_rd(void){
	clk_rd = ~clk_rd;
	IOWR(CLK_RD_BASE,0,clk_rd);//900k
	for(int i = 0; i < 100000; i++);
	if(!storage_done)
	{
		if(signal_num < 513)
		{
			(signal_num)?1:(IOWR(WRD_FLAG_BASE,0,1));
			sig1[signal_num] = IORD(FIFO_OUT_BASE,0);
			signal_num++;
			printf("signal[%d] = %d\n",signal_num,sig1[signal_num]);
		}
		else
		{
			signal_num = 0;
			storage_done = 1;
			printf("Storage over!");
		}
	}
	else
	{
		IOWR(WRD_FLAG_BASE,0,0);
		for(int num = 0;num<1025;num++)
		{
			(num == 1024)?1:(  storage_done = 0 );
			clk_rd = ~clk_rd;
			IOWR(CLK_RD_BASE,0,clk_rd);
			for(int y = 0;y<10;y++);
		}
	}
	clk_rd = ~clk_rd;
	IOWR(CLK_RD_BASE,0,clk_rd);
	for(int i = 0;i < 20;i++);
}

void XTrans(float fre_now){
	//一周期点数：Times/2	//波形数：10*xlable*fre
//	float xtrans[400];
//	int dr_point_num;		//画的点数
//	int dr_sig_num;			//画的周期数

	if(x_mod == 0){	//100ns---200M
//		dr_sig_num = fre_now*pow(10,-6);
		KW_word = (1/(1/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
		Times = 4*pow(10,8)/fre_now;
//		dr_point_num = 200;	//	dr_sig_num = clk_sample*xlable*10
	}
	else if(x_mod == 1){	//2us---10M---理论上来说也是等效采样 懒得改了
//		dr_sig_num = fre_now*2*pow(10,-5);
		KW_word = pow(2,32)/40;
		Times =(500000/fre_now)*40;
//		dr_point_num = 200;
	}
	else if(x_mod == 2){	//20ms--1000Hz采样频率
//		dr_sig_num = fre_now*0.2;
		KW_word = pow(2,32)/400000;
		Times = (50/fre_now)*40;
//		dr_point_num = 200;
	}
	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_word);
	IOWR(SAMPLE_TIME_BASE, 0, Times);
}

void YTrans(){
//	float ytrans[400];
//	float ycachu[400];

	int n;
	switch(y_mod){
	case 0: n = 16;		//--2mV
		break;
	case 1: n = 800;	//--0.1V
		break;
	case 2: n = 8000;	//--2V
		break;
	default:break;
	}

	for(int i = 2; i < 402; i++){	//Y点坐标=中位线-总列点数*vpp(读数值-rd_zero)/量程
		if(ifCall){		//回调
			sig2_real[i] = getVpp((sig2[i]-rdac_zero_2),Mult2);
			ytrans[i] = 188-320*getVpp((sig2[i]-rdac_zero_2),Mult2)/n;
		}
		else{			//实时
			sig1_real[i] = getVpp((sig1[i]-rdac_zero),Mult);
			ytrans[i] = 188-320*getVpp((sig1[i]-rdac_zero),Mult)/n;
			ycachu[i] = ytrans[i];
		}
	}
}

void Storage(/*int ifStore*/){
	if(ifStore){
		for(int i = 0; i< 200; i++){
			sig2[i] = sig1[i];
		}
		vpp2 = vpp;
		vmax2 = vmax;
		vmin2 = vmin;
		fre_meas2 = fre_meas;
		rdac_zero_2 = rdac_zero;
		Mult2 = Mult;

		ifStore = 0;
	}
}

/******************************************************************************************
*	波形显示部分
*	Auto键：自动选择水平、垂直分辨率
*	波形回调与清除
*	单次采样
*******************************************************************************************/

void Auto(/*int ifAuto*/){
	if(ifAuto){

		getSampclk(fre_meas);
		XTrans(fre_meas);

		if(vpp <= 15){
			y_mod = 0;		//--2mV
		}
		else if(vpp <= 790){
			y_mod = 1;		//--0.1V
		}
		else{
			y_mod = 2;
		}

		trigger_v = 0;
		fifo_rd();
		YTrans();

		int tri_num;
		for(int i = 2; i < 400; i++){
			if((sig1_real[i]>=trigger_v) && (sig1_real[i-2]<trigger_v) && (sig1_real[i-1]<trigger_v)){
				tri_num = i;
				printf("tri_num = %d.\n",tri_num);
				break;
			}
		}
		for(int i = tri_num; i < tri_num+200; i++){
			lcdDrawLine((i-tri_num)*2+220, ycachu[i], (i-tri_num)*2+222, ycachu[i+1], WHITE);
			lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], BLACK);
		}

		ifAuto = 0;
	}
}

void Recall(){
	if(ifCall){
		YTrans();
		lcdRectClear(220, 28, 620, 348, WHITE);
		lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);

		int tri_num;
		for(int i = 2; i < 400; i++){
			if((sig2_real[i]>=trigger_v) && (sig2_real[i-2]<trigger_v) && (sig2_real[i-1]<trigger_v)){
				tri_num = i;
				printf("tri_num = %d.\n",tri_num);
				break;
			}
		}
		for(int i = tri_num; i < tri_num+200; i++){
			lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], BLACK);
		}
	}
	else{
		lcdRectClear(220, 28, 620, 348, WHITE);
		lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);
	}
}

void Single(){
	if(ifSingle){
		YTrans();
		int tri_num;
		for(int i = 2; i < 400; i++){
			if((sig1_real[i]>=trigger_v) && (sig1_real[i-2]<trigger_v) && (sig1_real[i-1]<trigger_v)){
				tri_num = i;
				printf("tri_num = %d.\n",tri_num);
				break;
			}
		}
		for(int i = tri_num; i < tri_num+200; i++){
			lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], BLACK);
		}
	}
}

/***********************************************************************************
*	Switch 档位选择
*	0 | 0mV~25mV  :x100		117.33		~~~~/100		| 理论判断值：0~2500		|	0~2933.25	Mul3
*	1 | 25mV~500mV:x5		5.6		~~~~/5				| 理论判断值：125~2500	|	140~2800	Mul2
*	2 | 500mV~8V  :x1/3		1/3.917		~~~~*3			| 理论判断值：167~2667	|	127~2042	Mul1
***********************************************************************************/

void setSwitch(/*int ifChoosed*/){
	if(!ifChoosed){
		IOWR(SWITCH_MOD_BASE, 0, 2);	//衰减3
		u16 rdv = 0;
		double vpp_init;
		while(!IORD(VPP_FOUND_BASE, 0));
		rdv = IORD(VPP_BASE, 0);
		vpp_init = getVpp(rdv, Mul1);

		if(vpp_init > 126){
			switch_mod = 2;
			Mult = Mul1;
			printf("switch_mode == 2, Mult = %f.\n",Mult);
		}
		else{
			IOWR(SWITCH_MOD_BASE, 0, 1);
			for(int i = 0; i<100000; i++);
			while(!IORD(VPP_FOUND_BASE, 0));
			vpp_init = getVpp(IORD(VPP_BASE, 0),Mul2);
			if(vpp_init > 139){			//放大5
				switch_mod = 1;
				Mult = Mul2;
				printf("switch_mode == 1, Mult = %f.\n",Mult);
			}
			else{
				IOWR(SWITCH_MOD_BASE, 0, 0);	//放大100
				switch_mod = 0;
				Mult = Mul3;
				printf("switch_mode == 0, Mult = %f.\n",Mult);
			}
		}

		ifChoosed = 1;
	}
}
