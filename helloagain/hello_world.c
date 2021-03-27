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
u16 rdv = 0, rdmax = 0, rdmin = 0, rdac_zero;
int KW_word, Times;
int signal_num = 0;
int sig1[512] = {0};
double sig_real2[512] = {0};		// 1当前 2存储
float sig1_real[512] = {0};

//flag
int clk_rd, storage_done;	//0未1已
int KW_init = pow(2,32)/401, Times_init = 100100;
int switch_mod = 3, choosed = 0;
double Mult;

//int x_mod,y_mod;

void init();
void init_dis();
void init_flag();

void getSampclk(float fre_now);
double getFre();
void disFre(double fre_now);
double getVpp(double vrd);
void Amp(int ifAmp);
void disAmp(double v2wr,int w);

void fifo_sto(void);
void XYTrans(int ww,float fre_now);
void Storage(int ifStore);
void Auto(int ifAuto);

void setSwitch();

int main() {
	//开始执行程序
	printf("Hello lemon\n");
	if (deviceInit() < 0) {
		printf("device initial failed\n");
		return -1;
	}

	lcdRectClear(0, 0, 799, 479, WHITE);

	init();

	//核心代码
	while (1)
	{
//		getSampclk(500000);
//		fifo_sto();
/*
		setSwitch();
		if(choosed == 1){
			getSampclk(getFre());
			fifo_sto();

			choosed = 0;
		}
		*/
//		disFre(getFre());
		if(flag == 1){

			flag = 0;
		}
	}

	return 0;
}

/*****************************************初始化部分**************************************************/

void init(){
	init_dis();

	//传输数据初始化
	//标志位初始化
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
*************************************************************************************************/

//Amplitude
void Amp(int ifAmp){	//ifshow =0:初始化判断，不显示电平		=1:实际判断，显示电平
	if(ifAmp){
		while(!IORD(VPP_FOUND_BASE, 0));
		rdv = IORD(VPP_BASE, 0);
		vpp = getVpp(rdv);
		disAmp(vpp, 0);//w=0-vpp; 1-vmax; 2-vmin

		rdmax = IORD(V_MAX_BASE, 0);
		rdmin = IORD(V_MIN_BASE, 0);
		rdac_zero = (rdmax+rdmin)/2;

//		vmax = getVpp(rdmax-Vref);
//		disAmp(vmax, 1);//w=0-vpp; 1-vmax; 2-vmin
//		vmin = getVpp(rdmin-Vref);
//		disAmp(vmin, 2);//w=0-vpp; 1-vmax; 2-vmin

	}
}

double getVpp(double vrd){
	double vin;
	vin = vrd * 1.312-1.323;
	if(vin <= 0){
		vin = vrd;
	}
	if(choosed){
		vin = vin*Mult;
	}
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
		printf("0-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else if(fre_now <= 50){	//1000Hz
		KW_word = pow(2,32)/400000;
		Times = (50/fre_now)*40;
	}
	else if(fre_now<=500000){		//实时采样频率  10M
		KW_word = pow(2,32)/40;
		Times =(500000/fre_now)*40;
//		printf("1-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else{		//等效采样时钟200M--p130差拍时钟顺序等效采样法。
		KW_word = (1/(1/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
		Times = 4*pow(10,8)/fre_now;
//		printf("2-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}

	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_word);
	IOWR(SAMPLE_TIME_BASE, 0, Times);
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
*	波形存储
*	基本绘图
****************************************************************/

//Wave form Storage
void fifo_sto(void){
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

void XYTrans(int ww,float fre_now){
	//ww:which wave form:0~当前波形；1~调用存储
	//一周期点数：Times/2	//波形数：10*xlable*fre
	float xtrans[400], ytrans[400];
	int dr_point_num;		//画的点数
	int dr_sig_num;		//画的周期数

	if(x_mod == 0){	//100ns---200M
		dr_sig_num = fre_now*pow(10,-6);
		KW_word = (1/(1/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
		Times = 4*pow(10,8)/fre_now;
		dr_point_num = 200;	//	dr_sig_num = clk_sample*xlable*10
	}
	else if(x_mod == 1){	//2us---10M---理论上来说也是等效采样 懒得改了
		dr_sig_num = fre_now*2*pow(10,-5);
		KW_word = pow(2,32)/40;
		Times =(500000/fre_now)*40;
		dr_point_num = 200;
	}
	else if(x_mod == 2){	//20ms--1000Hz采样频率
		dr_sig_num = fre_now*0.2;
		KW_word = pow(2,32)/400000;
		Times = (50/fre_now)*40;
		dr_point_num = 200;
	}
	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_word);
	IOWR(SAMPLE_TIME_BASE, 0, Times);

	for(int i = 0;i < 200; i++){
		sig1_real[i] = getVpp(sig1[i]-rdac_zero);
	}
	if(dr_point_num < 401){
		if(y_mod == 0){	//2mV
			for(int i = 1; i< dr_point_num; i++){
				if(abs(sig1_real[i])<=8){
					ytrans[i-1] = -(sig1_real[i]/8.0)*160 + 188;
				}
				else if(sig1_real[i]>0)
					ytrans[i-1] = 28;
				else
					ytrans[i-1] = 348;
			}
		}
		else if(y_mod == 1){	//0.1V
			for(int i = 1; i< dr_point_num; i++){
				if(abs(sig1_real[i])<=800){
					ytrans[i-1] = -(sig1_real[i]/800.0)*160 + 188;
				}
				else if(sig1_real[i]>0)
					ytrans[i-1] = 28;
				else
					ytrans[i-1] = 348;
			}
		}
		else if(y_mod == 2){	//1V
			for(int i = 1; i< dr_point_num; i++){
				if(abs(sig1_real[i])<=8000){
					ytrans[i-1] = -(sig1_real[i]/8000.0)*160 + 188;
				}
				else if(sig1_real[i]>0)
					ytrans[i-1] = 28;
				else
					ytrans[i-1] = 348;
			}
		}
	}
	else{
		for(int i = 0; i < Times+1; i++){

		}
		if(y_mod == 0){	//2mV

		}
		else if(y_mod == 1){	//0.1V

		}
		else if(y_mod == 2){	//1V

		}
	}

	lcdRectClear(220, 28, 620, 348, WHITE);
	lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);

	if(dr_sig_num == 200){
		for(int i = 0; i < 200; i++){
			lcdDrawLine(2*i+220, ytrans[i], 2*(i+1)+220, ytrans[i+1], BLACK);
		}
	}
	else if(dr_sig_num == 2){

	}
	else{

	}
/*
	if(ww){//存储--Sig2
		for(int i = 0;i<dr_sig_num;i++){
			lcdDrawLine(xtrans[i], ytrans[i], xtrans[i+1], ytrans[i+1], BLACK);
		}
	}
	else{//实时--Sig1

	}
	*/
}

void Storage(int ifStore){
	if(ifStore){
		for(int i = 0; i< 200; i++){
			sig_real2[i] = getVpp(sig1[i]);
		}
		vpp2 = vpp;
		vmax2 = vmax;
		vmin2 = vmin;
		fre_meas2 = fre_meas;

		ifStore = 0;
	}
}

void Auto(int ifAuto){

}
/***********************************************************************************
*	Switch 档位选择
*	0 | 0mV~25mV  :x100		117.33		~~~~/100		| 理论判断值：0~2500		|	0~2933.25
*	1 | 25mV~500mV:x5		5.6		~~~~/5				| 理论判断值：125~2500	|	140~2800
*	2 | 500mV~8V  :x1/3		1/3.917		~~~~*3			| 理论判断值：167~2667	|	127~2042
***********************************************************************************/

void setSwitch(){
	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_init);
	IOWR(SAMPLE_TIME_BASE, 0, Times_init);

	u16 rdv = 0;
	double vpp_init;
	while(!IORD(VPP_FOUND_BASE, 0));
	rdv = IORD(VPP_BASE, 0);
	vpp_init = getVpp(rdv);


	IOWR(SWITCH_MOD_BASE, 0, 2);	//衰减3
	if(!choosed && (vpp_init > 126)){
		switch_mod = 2;
		Mult = Mul1;
		choosed = 1;
		IOWR(SWITCH_MOD_BASE, 0, switch_mod);
		printf("switch_mode == 2\n");
	}
	else if(!choosed){
		IOWR(SWITCH_MOD_BASE, 0, 1);	//放大5
		if(!choosed && (vpp_init > 139)){
			switch_mod = 1;
			Mult = Mul2;
			choosed = 1;
			printf("switch_mode == 1\n");
		}
		else{
			IOWR(SWITCH_MOD_BASE, 0, 0);	//放大100
			switch_mod = 0;
			Mult = Mul3;
			choosed = 1;
			printf("switch_mode == 0\n");
		}
	}
}
