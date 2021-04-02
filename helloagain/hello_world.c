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
// switch_mod =0--Mul1--衰减; =1--Mul2--5; =2--Mul3--100;

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
int switch_mod = 0;
int ifChoosed = 0, trigged = 0, nosig = 0;

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
		while(!ifRun);
//		printf("////////////////Begin to while/////////////////////\n");
		/*********************Switch档位选择--Begin********************************/
		setSwitch();
//		printf("ifChoosed = %d.\n",ifChoosed);
		if(ifChoosed){
//			printf("Begin to check the switch.Switch_mod = %d.Times = %d.\n",switch_mod,Times);
			double vpp_init;
			while(!IORD(VPP_FOUND_BASE, 0));
			vpp_init = getVpp(IORD(VPP_BASE, 0),Mult);
//			printf("vpp_init = %f.\n",vpp_init);
			if(switch_mod == 2){						//--100
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
						IOWR(SWITCH_MOD_BASE, 0, 0);
						switch_mod = 0;
						Mult = Mul1;
						ifChoosed = 1;
					}
				}
			}
			else if(switch_mod == 1){					//--5
				if(vpp_init > 4000){
					/////////////////上限值要修改！！！
					IOWR(SWITCH_MOD_BASE, 0, 0);		//--1/3
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul1);
					switch_mod = 0;
					Mult = Mul1;
					ifChoosed = 1;
				}
				else if(vpp_init < 140){
					IOWR(SWITCH_MOD_BASE, 0, 2);		//--100
					switch_mod = 2;
					Mult = Mul3;
					ifChoosed = 1;
				}
			}
			else if(switch_mod == 0){					//--1/3
				if(vpp_init < 127){
					IOWR(SWITCH_MOD_BASE, 0, 1);
					for(int i = 0; i<100000; i++);
					while(!IORD(VPP_FOUND_BASE, 0));
					vpp_init = getVpp(IORD(VPP_BASE, 0),Mul2);
					if(vpp_init > 139){					//--5
						switch_mod = 1;
						Mult = Mul2;
						ifChoosed = 1;
					}
					else{
						IOWR(SWITCH_MOD_BASE, 0, 2);	//--100
						switch_mod = 2;
						Mult = Mul3;
						ifChoosed = 1;
					}
				}
			}
//			printf("End to Choosed switch. ifChoosed = %d.\n",ifChoosed);
		}

		ifChoosed = (ifChoosed+1)%20;

		/*********************Switch档位选择--End********************************/

		if(!ifCall){
			fre_meas = getFre();	//包含显示功能
			Amp();					//vpp | rdv | rdmax | rdmin
			printf("switch_mod = %d. vpp_rd = %d,Vpp_meas = %f.\n",switch_mod, rdv,vpp);
		}

		if((!ifCall)&&(!ifSingle))
			Auto();
		/*********************手动调波--Begin**************************************/
		if((!ifAuto)&&(!ifSingle)&&(!ifCall)){
//			printf("-------------------------手动调波-------------------------\n");
			XTrans(fre_meas);
			for(int i = 0; i < 100000; i++);
			fifo_rd();
			YTrans();

			int tri_num;
			if(trigged == 0){
				for(int i = 2; i < 400; i++){
					if((sig1_real[i-1]>=(trigger_v*1000)) && (sig1_real[i-2]<(trigger_v*1000)) && (sig1_real[i-1]>sig1_real[i-2])){
						tri_num = i-1;
						printf("MODE=1: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num-1,sig1_real[tri_num-1],tri_num,sig1_real[tri_num]);
						printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
						if(tri_num < 201)
							trigged = 1;
						break;
					}
/*					else if((sig1_real[i]>=(trigger_v*1000)) && (sig1_real[i-2]>=(trigger_v*1000)) && (sig1_real[i-1]>=(trigger_v*1000))&&(sig1_real[i-1]==sig1_real[i-2])&&(sig1_real[i-1]==sig1_real[i])){
						tri_num = i-2;
//						printf("tri_num = %d.\n",tri_num);
						printf("MODE=2: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num+1,sig1_real[tri_num+1],tri_num,sig1_real[tri_num]);
						printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
						if(tri_num < 201)
							trigged = 1;
						break;
					}
					*/
				}
			}

			if(trigged == 1){
				for(int i = tri_num; i < tri_num+200; i++){
					lcdDrawLine((i-tri_num)*2+220, ycachu[i-tri_num], (i-tri_num)*2+222, ycachu[i+1-tri_num], WHITE);
					lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], RED);
//					printf("X_before[%d] = %d; Y_before[%d] = %d.\n\n",i-tri_num,(i-tri_num)*2+220,i-tri_num,ytrans[i]);
				}
				for(int i = tri_num; i < tri_num+200; i++){
					ycachu[i-tri_num] = ytrans[i];
				}
				trigged = 0;
			}

		}//手动调波最后的括号

//		printf("Finish show wave!\n");
		/*********************手动调波--End**************************************/
		Single();
		Storage();
		Recall();
	}//while的括号

	return 0;
}

/*****************************************初始化部分**************************************************/

void init(){
	init_dis();
	//传输数据初始化 | 标志位初始化
	init_flag();
	
	trigger_v = 0;
	trigger_v_ed = 0;
	lcdDrawLine(220, 188-320*trigger_v_ed/4, 620, 188-320*trigger_v_ed/4, WHITE);
	lcdDrawLine(220, 188-320*trigger_v/4, 620, 188-320*trigger_v/4, BLUE);
	printf("trigger_v = %f\n",188-320*trigger_v/4);

//	lcdDrawLine(220, trigger_v_ed, 620, trigger_v_ed, WHITE);
//	lcdDrawLine(220, trigger_v, 620, trigger_v, BLUE);
}

void init_dis(){
	lcdRectClear(0, 0, 799, 479, WHITE);
	lcdDispNumtable(Num_X, Num_Y);
//	lcdDrawRect(220, 28, 620, 348, BLACK);
	lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);
	lcdDispStringSmall(Num_X+5, Num_Y+370, BLACK, WHITE, "V_Tri/V");

	lcdDispStringSmall(230, 358, BLUE, WHITE, "Vpp");
	lcdDispStringSmall(230, 378, BLUE, WHITE, "Fre");
//	lcdDispStringSmall(230, 398, BLUE, WHITE, "Vmax");
//	lcdDispStringSmall(230, 418, BLUE, WHITE, "Vmin");
}

void init_flag(){
	ifAuto = 1;
	ifCall = 0;
	ifSingle = 0;
	trigged = 0;
	IOWR(CLK_SAMPLE_KW_BASE, 0, KW_init);
	IOWR(SAMPLE_TIME_BASE, 0, Times_init);
	IOWR(SWITCH_MOD_BASE, 0, 0);
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

//	printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d\n",rdv,rdmax,rdmin,rdac_zero);//无数据输入时rdmax=rdmin=4095?

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
*	注意：频率测量时要用浮点数，不然测量值会溢出。
*************************************************************************************************/

double getFre(){
//	printf("Begin to measure fre.\n");
	int clk_num, sig_num;
	clk_num = IORD(FMEASURE_CLK_BASE, 0);
	sig_num = IORD(FMEASURE_SQR_BASE, 0);
//	printf("clk_num = %d, sig_num = %d\n",clk_num,sig_num);
	if(clk_num)
		fre_meas = sig_num*100000000.0/(clk_num);
	else
		fre_meas = 0;

//	printf("***1***:%d\n***2***:%d\n***3***:%d\n",(sig_num*100000000/(clk_num)),(sig_num*10000000/(clk_num))*10,(sig_num*1000000/(clk_num))*100);
//	printf("***4***:%d\n***5***:%d\n***6***:%d\n",(sig_num*100000/(clk_num))*1000,(sig_num*10000/(clk_num))*10000,(sig_num*1000/(clk_num))*100000);
//	printf("***1***:%f\n***2***:%f\n\n",(sig_num*100000000.0/(clk_num)),fre_meas);

	disFre(fre_meas);

//	printf("Finish measuring fre.\n\n");
	return fre_meas;
}

void getSampclk(float fre_now){
	if(fre_now == 0){
		KW_word = pow(2,32)/400;
		Times =200;
		nosig = 1;
		IOWR(CLK_SAMPLE_KW_BASE, 0, KW_word);
		IOWR(SAMPLE_TIME_BASE, 0, Times);
//		printf("0-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else if(fre_now <= 50){	//1000Hz--20ms
		x_mod = 2;
		nosig = 0;
//		KW_word = pow(2,32)/400000;
//		Times = (50/fre_now)*40;
	}
	else if(fre_now <= 50000){//1MHz--20us
		x_mod = 3;
		nosig = 0;
	}
	else if(fre_now<=500000){		//实时采样频率  10M
		x_mod = 1;
		nosig = 0;
//		KW_word = pow(2,32)/40;
//		Times =(500000/fre_now)*40;
//		printf("1-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}
	else{		//等效采样时钟200M--p130差拍时钟顺序等效采样法。
		x_mod = 0;
		nosig = 0;
//		KW_word = (1/(1/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
//		Times = 4*pow(10,8)/fre_now;
//		printf("2-------Times = %d,fre = %f,KW_word = %d\n",Times,fre_now,KW_word);
	}

	lcdRectClear(Num_X+75, Num_Y+305, Num_X+150, Num_Y+328, WHITE);
	switch(x_mod){
	case 0: lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "100ns");
		break;
	case 1: lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "2us");
		break;
	case 2: lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "20ms");
		break;
	case 3: lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "20us");
		break;
	default: lcdDispStringSmall(Num_X+80, Num_Y+308, RED, WHITE, "20ms");
		break;
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
*	注意：等效采样时fre需要缩放~~不然实际采样频率还是太大了。
****************************************************************/

//Wave form Storage
void fifo_rd(void){
//	printf("----------------------Begin to fifo!---------------------------------------\n");
	clk_rd = ~clk_rd;
	IOWR(CLK_RD_BASE,0,clk_rd);//900k
	for(int i = 0; i < 100000; i++);
	int num_n = 0;
	while(!storage_done)
	{
		if(signal_num < 513)
		{
//			if(flag == 1){
			clk_rd = ~clk_rd;
			IOWR(CLK_RD_BASE,0,clk_rd);
			clk_rd = ~clk_rd;
			IOWR(CLK_RD_BASE,0,clk_rd);
			(signal_num)?1:(IOWR(WRD_FLAG_BASE,0,1));
			sig1[signal_num] = IORD(FIFO_OUT_BASE,0);
			signal_num++;
//			if(sig1[signal_num]==sig1[signal_num-1]){
//				num_n++;
//			}
//			else
//				num_n = 0;
			///
/*			sig1_real[signal_num] = getVpp((sig1[signal_num]-rdac_zero),Mult);
			ytrans[signal_num] = 188-320*getVpp((sig1[signal_num]-rdac_zero),Mult)/8;
			if(ytrans[signal_num]>348){
				ytrans[signal_num]=348;
			}
			else if(ytrans[signal_num]<28){
				ytrans[signal_num]=28;
			}
			*/
			///
//			printf("num_n = %d, signal[%d] = %d\n",num_n,signal_num,sig1[signal_num]);
//			printf("vpp = %f,sig1[%d]=%d;sig1_real[%d]=%f.\n",getVpp((sig1[signal_num]-rdac_zero),Mult),signal_num,ytrans[signal_num],signal_num,sig1_real[signal_num]);

//			flag = 0;
//			}
		}
		else
		{
			signal_num = 0;
			storage_done = 1;
//			printf("Storage over!\n");
		}
	}
	while(storage_done)
	{
		IOWR(WRD_FLAG_BASE,0,0);
		for(int num = 0;num<1025;num++)
		{
			(num == 1024)?1:(  storage_done = 0 );
//			clk_rd = ~clk_rd;
//			IOWR(CLK_RD_BASE,0,clk_rd);
//			for(int y = 0;y<10;y++);
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

	if(fre_now < 1){
		nosig = 1;
	}
	else
		nosig = 0;

	if(nosig == 0){
//		printf("To change the kw and times.\n");
		if(x_mod == 0){	//100ns---200M---等效采样频率fre=1/△
//			dr_sig_num = fre_now*pow(10,-6);
			KW_word = (1/(10/fre_now+0.5*pow(10,-8))*pow(2,32))/400000000;		//KW = (fre_out*pow(2,32))/400000000;
			Times = 4*pow(10,8)/fre_now;
//			dr_point_num = 200;	//	dr_sig_num = clk_sample*xlable*10
		}
		else if(x_mod == 1){	//2us---10M---理论上来说也是等效采样 懒得改了
//			dr_sig_num = fre_now*2*pow(10,-5);
			if(fre_now <= 50000){
				KW_word = pow(2,32)/400;		//1M
			}
			else
				KW_word = (1/(1/fre_now+pow(10,-7))*pow(2,32))/400000000;
			Times =(500000/fre_now)*40;
//			dr_point_num = 200;
		}
		else if(x_mod == 2){	//20ms--1000Hz采样频率
//			dr_sig_num = fre_now*0.2;
			KW_word = pow(2,32)/400000;
			Times = (50/fre_now)*40;
//			dr_point_num = 200;
		}
		else if(x_mod == 3){		//1M
			KW_word = pow(2,32)/400;		//1M
			Times = (50000/fre_now)*40;;
		}
	}
	else{
		KW_word = pow(2,32)/400;
		Times =200;
	}

//	printf("x_mod = %d; fre_now = %f. KW_word = %d. Times = %d.\n",x_mod,fre_now,KW_word, Times);
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
			sig2_real[i-2] = getVpp((sig2[i]-rdac_zero_2),Mult2);
			ytrans[i-2] = 188-320*getVpp((sig2[i]-rdac_zero_2),Mult2)/n;
			if(ytrans[i-2]>348){
				ytrans[i-2]=348;
			}
			else if(ytrans[i-2]<28){
				ytrans[i-2]=28;
			}
		}
		else{			//实时
//		    if(i==2)	 printf("Begin to trans_y_vpp.\n");
			sig1_real[i-2] = getVpp((sig1[i]-rdac_zero),Mult);
			ytrans[i-2] = 188-320*getVpp((sig1[i]-rdac_zero),Mult)/n;
			if(ytrans[i-2]>348){
				ytrans[i-2]=348;
			}
			else if(ytrans[i-2]<28){
				ytrans[i-2]=28;
			}
//			printf("signal[%d] = %d\n",i-2,sig1[i-2]);
//			printf("vpp = %f,sig1[%d]=%f;sig1_real[%d]=%f.\n",getVpp((sig1[i]-rdac_zero),Mult),(i-2),ytrans[i-2],(i-2),sig1_real[i-2]);

//			printf("sig1[%d]=%d;sig1_real[%d]=%f.\n",i-2,ytrans[i-2],i-2,sig1_real[i-2]);
//			ycachu[i-2] = ytrans[i-2];
//			printf("%d: Vpp = %f,量程 = %d,被减数 = %f,ytrans[%d]=%f\n",i-2,getVpp((sig1[i]-rdac_zero),Mult),n,320*getVpp((sig1[i]-rdac_zero),Mult)/n,i-2,ytrans[i-2]);
		}
	}
}

void Storage(/*int ifStore*/){
	if(ifStore){
//		printf("Begin to store the wave.\n");
		for(int i = 0; i< 512; i++){
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
//		printf("******************************自动调波****************************\n");
		getSampclk(fre_meas);
		XTrans(fre_meas);
		if(vpp <= 15){
			y_mod = 0;		//--2mV
		}
		else if(vpp <= 790){
			y_mod = 1;		//--0.1V
		}
		else{
			y_mod = 2;		//--1V
		}

		lcdRectClear(Num_X+75, Num_Y+335, Num_X+150, Num_Y+358, WHITE);
		switch(y_mod){
		case 0: lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "0.1V");
			break;
		case 1: lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "1V");
			break;
		case 2: lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "2mV");
			break;
		default: lcdDispStringSmall(Num_X+80, Num_Y+338, RED, WHITE, "2mV");
			break;
		}

		fifo_rd();
		YTrans();

//		printf("trigger_v = %f.\n",trigger_v);
		int tri_num;
		if(trigged == 0){
			for(int i = 2; i < 400; i++){
				if((sig1_real[i-1]>=(trigger_v*1000)) && (sig1_real[i-2]<(trigger_v*1000)) && (sig1_real[i-1]>sig1_real[i-2])){
					tri_num = i-1;
					printf("MODE=1: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num-1,sig1_real[tri_num-1],tri_num,sig1_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
/*				else if((sig1_real[i]>=(trigger_v*1000)) && (sig1_real[i-2]>=(trigger_v*1000)) && (sig1_real[i-1]>=(trigger_v*1000))&&(sig1_real[i-1]==sig1_real[i-2])&&(sig1_real[i-1]==sig1_real[i])){
					tri_num = i-2;
//					printf("tri_num = %d.\n",tri_num);
					printf("MODE=2: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num+1,sig1_real[tri_num+1],tri_num,sig1_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
				*/
			}
		}
		
		/*
		if(trigged == 0){
			for(int i = 2; i < 400; i++){
//				if(i == 2)
//					printf("sig1_real[%d]=%f\n",i,sig1_real[i]);
//				if((sig1_real[i]>=trigger_v) && (sig1_real[i-2]<trigger_v) && (sig1_real[i-1]<trigger_v)){
				if((sig1_real[i-1]>=trigger_v) && (sig1_real[i-2]<trigger_v)){
					tri_num = i-1;
					printf("tri_num = %d.\n",tri_num);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
				else if((sig1_real[i]>=trigger_v) && (sig1_real[i-2]>=trigger_v) && (sig1_real[i-1]>=trigger_v)){
					tri_num = i-2;
//					printf("tri_num = %d.\n",tri_num);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
			}
		}
		*/
		if(trigged ==1){
//			printf("Begin to Drawing----~~~~!!!!!\n");
			for(int i = tri_num; i < tri_num+199; i++){
				lcdDrawLine((i-tri_num)*2+220, ycachu[i-tri_num], (i-tri_num)*2+222, ycachu[i+1-tri_num], WHITE);
				lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], RED);
//				if(i == tri_num)
//					printf("y_sig[%d] = %d; ytrans[%d] = %f; ycachu[%d] = %f\n",(i-tri_num),sig1[i+2],(i-tri_num),ytrans[i],(i-tri_num),ycachu[i-tri_num]);
			}
			for(int i = tri_num; i < tri_num+200; i++){
				ycachu[i-tri_num] = ytrans[i];
			}
			trigged = 0;
		}
	}
}

void Recall(){
	if(ifCall){
//		printf("Begin to show the stored wave.\n");
		YTrans();
		if(clearscr){
			printf("ifCall = %d, clearscr = %d.\n",ifCall,clearscr);
			lcdRectClear(220, 28, 620, 348, WHITE);
			lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);
			clearscr = 0;
		}

		int tri_num;
		if(trigged == 0){
			for(int i = 2; i < 400; i++){
				if((sig2_real[i-1]>=(trigger_v*1000)) && (sig2_real[i-2]<(trigger_v*1000)) && (sig2_real[i-1]>sig2_real[i-2])){
					tri_num = i-1;
					printf("MODE=1: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num-1,sig2_real[tri_num-1],tri_num,sig2_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
/*				else if((sig2_real[i]>=(trigger_v*1000)) && (sig2_real[i-2]>=(trigger_v*1000)) && (sig2_real[i-1]>=(trigger_v*1000))&&(sig2_real[i-1]==sig2_real[i-2])&&(sig2_real[i-1]==sig2_real[i])){
					tri_num = i-2;
//					printf("tri_num = %d.\n",tri_num);
					printf("MODE=2: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num+1,sig2_real[tri_num+1],tri_num,sig2_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
				*/
			}
		}

		if(trigged == 1){
			for(int i = tri_num; i < tri_num+200; i++){
				lcdDrawLine((i-tri_num)*2+220, ycachu[i-tri_num], (i-tri_num)*2+222, ycachu[i+1-tri_num], WHITE);
				lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], RED);
			}
			for(int i = tri_num; i < tri_num+200; i++){
				ycachu[i-tri_num] = ytrans[i];
			}
			trigged = 0;
		}
	}
	else{
		if(huifu == 1){
			lcdRectClear(220, 28, 620, 348, WHITE);
			lcdDrawGrid(220, 28, 8, 10, 40, DGRAY);
			huifu = 0;
		}
	}
}

void Single(){
	if(ifSingle){
//		printf("Begin to single mod!\n");
		XTrans(fre_meas);
		fifo_rd();
		YTrans();

		int tri_num;
		if(trigged == 0){
			for(int i = 2; i < 400; i++){
				if((sig1_real[i-1]>=(trigger_v*1000)) && (sig1_real[i-2]<(trigger_v*1000)) && (sig1_real[i-1]>sig1_real[i-2])){
					tri_num = i-1;
					printf("MODE=1: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num-1,sig1_real[tri_num-1],tri_num,sig1_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
/*				else if((sig1_real[i]>=(trigger_v*1000)) && (sig1_real[i-2]>=(trigger_v*1000)) && (sig1_real[i-1]>=(trigger_v*1000))&&(sig1_real[i-1]==sig1_real[i-2])&&(sig1_real[i-1]==sig1_real[i])){
					tri_num = i-2;
//					printf("tri_num = %d.\n",tri_num);
					printf("MODE=2: tri_num = %d. sig1[%d] = %f. sig1[%d] = %f.\n",tri_num,tri_num+1,sig1_real[tri_num+1],tri_num,sig1_real[tri_num]);
					printf("rdv = %d; rdmax = %d; rdmin = %d; rdac_zero = %d; vpp = %f.\n",rdv,rdmax,rdmin,rdac_zero,vpp);
					if(tri_num < 201)
						trigged = 1;
					break;
				}
				*/
			}
		}
		if(trigged == 1){
			for(int i = tri_num; i < tri_num+199; i++){
				lcdDrawLine((i-tri_num)*2+220, ycachu[i-tri_num], (i-tri_num)*2+222, ycachu[i+1-tri_num], WHITE);
				lcdDrawLine((i-tri_num)*2+220, ytrans[i], (i-tri_num)*2+222, ytrans[i+1], RED);
//					printf("X_before[%d] = %d; Y_before[%d] = %d.\n\n",i-tri_num,(i-tri_num)*2+220,i-tri_num,ytrans[i]);
			}
			for(int i = tri_num; i < tri_num+200; i++){
				ycachu[i-tri_num] = ytrans[i];
			}
			trigged = 0;
		}

		ifStore = 1;
		Storage();

		ifRun = 0;
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
		IOWR(SWITCH_MOD_BASE, 0, 0);	//衰减3
		u16 rdv = 0;
		double vpp_init;
		while(!IORD(VPP_FOUND_BASE, 0));
		rdv = IORD(VPP_BASE, 0);
		vpp_init = getVpp(rdv, Mul1);

		if(vpp_init > 126){
			switch_mod = 0;
			Mult = Mul1;
			printf("switch_mode == 0, Mult = %f.\n",Mult);
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
				IOWR(SWITCH_MOD_BASE, 0, 2);	//放大100
				switch_mod = 2;
				Mult = Mul3;
				printf("switch_mode == 2, Mult = %f.\n",Mult);
			}
		}

		ifChoosed = 0;
	}
}
