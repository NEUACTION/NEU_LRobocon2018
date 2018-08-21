#include "includes.h"
#include <app_cfg.h>
#include "misc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "timer.h"
#include "gpio.h"
#include "usart.h"
#include "can.h"
#include "elmo.h"
#include "movebase.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_usart.h"
#include "pps.h"
#include "stm32f4xx_adc.h"

#define Pulse2mm COUNTS_PER_ROUND/(WHEEL_DIAMETER*Pi
#define pai 3.1415926f
#define aa 1
#define bb 0
#define cc 0
#define FB 2
#define xx 0
#define yy -2200
//#define r 1000
//#define Set_Clock 1
//顺1逆2
#define Speed 1
#define COLLECT_BALL_ID (8)
#define PUSH_BALL_ID (6)
#define PUSH_POSITION (4500)
#define PUSH_RESET_POSITION (5)
#define GUN_YAW_ID (7)
#define COUNT_PER_ROUND (4096.0f)
#define COUNT_PER_DEGREE  (COUNT_PER_ROUND/360.0f)
#define YAW_REDUCTION_RATIO (4.0f)
/*
===============================================================
						信号量定义
===============================================================
*/
OS_EXT INT8U OSCPUUsage;
OS_EVENT *PeriodSem;

static OS_STK App_ConfigStk[Config_TASK_START_STK_SIZE];
static OS_STK WalkTaskStk[Walk_TASK_STK_SIZE];

void App_Task()
{
	CPU_INT08U os_err;
	os_err = os_err; /*防止警告...*/

	/*创建信号量*/
	PeriodSem = OSSemCreate(0);

	/*创建任务*/
	os_err = OSTaskCreate((void (*)(void *))ConfigTask, /*初始化任务*/
						  (void *)0,
						  (OS_STK *)&App_ConfigStk[Config_TASK_START_STK_SIZE - 1],
						  (INT8U)Config_TASK_START_PRIO);

	os_err = OSTaskCreate((void (*)(void *))WalkTask,
						  (void *)0,
						  (OS_STK *)&WalkTaskStk[Walk_TASK_STK_SIZE - 1],
						  (INT8U)Walk_TASK_PRIO);
}

/*
   ===============================================================
   初始化任务
   ===============================================================
*/
int startflag=0;
extern int posokflag;
int left,right;
void ConfigTask(void)
{
	CPU_INT08U os_err;
	os_err = os_err;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//定时器10ms pwm CAN通信 右1左2 轮子CAN2  引脚 串口3/4
	TIM_Init(TIM2,999,83,0x00,0x00);
	Adc_Init();
	//用串口1发射枪电机发送转速
	USART1_Init(921600);
	//串口1接收定位系统返回的信息
	USART3_Init(115200);
	//连接主控板和电脑，使电脑读到定位系统给主控板发回的信息
	UART4_Init(921600);
	CAN_Config(CAN1,500,GPIOB,GPIO_Pin_8,GPIO_Pin_9);
	CAN_Config(CAN2,500,GPIOB,GPIO_Pin_5,GPIO_Pin_6);
	//驱动器初始化
	ElmoInit(CAN1);
	ElmoInit(CAN2);
	//轮子 左2右1 用CAN2
	VelLoopCfg(CAN2,1,1000,0);
	VelLoopCfg(CAN2,2,1000,0);
	//棍子收球电机位置环初始化
	VelLoopCfg(CAN1, 8, 50000, 50000);
	//推球电机位置环初始化
	PosLoopCfg(CAN1, PUSH_BALL_ID, 50000,50000,20000);
	//使能右轮
	MotorOn(CAN2,1);
	//使能左轮
	MotorOn(CAN2,2);
	//使能棍子收球电机
	MotorOn(CAN1,8);
	//使能推球电机
	MotorOn(CAN1,6);
	//使能航向电机
	MotorOn(CAN1,7);
	delay_s(2);
	/*一直等待定位系统初始化完成*/
	WaitOpsPrepare();
	while(!startflag)
	{
	  left=Get_Adc_Average(15,5);
	  right=Get_Adc_Average(14,5);
	  if(left<100)
		  startflag=1;
	  else if(right<100)
		  startflag=2;
	  else startflag=0;
		USART_OUT(UART4,(uint8_t*)"%d\t%d\t%d\r\n",left,right,startflag);
	}
	USART_OUT(UART4,(uint8_t*)"ok\r\n");
	OSTaskSuspend(OS_PRIO_SELF);
}

int mark=0;
float Out_Agl=0;
int vel1,vel2,vel,turnflag=0;
int X,Y,angle;
int Out_Pulse;
int flag=0,Flag=0;
float R_cover=2000;
extern Pos_t position;

void Round(float speed,float R);
void PID(int Agl_Flag);
void PID_Agl(float Set_Angle);
void PID_Awy(float D_err);
void Strght_Walk (float a,float b,float c,int F_B);
void Square_Walk(void);
void SET_openRound(float x,float y,float R,float clock,float speed);
void SET_closeRound(float x,float y,float R,float clock,float speed);
void Round_Cover(void);
void SendUint8(void);
void YawAngleCtr(float yawAngle);

int timCnt=0,pushCnt;
void WalkTask(void)
{
	CPU_INT08U os_err;
	os_err = os_err;
	OSSemSet(PeriodSem, 0, &os_err);
	while (1)
	{
		OSSemPend(PeriodSem, 0, &os_err);
		timCnt++;
		timCnt=timCnt%1000;
		if(timCnt%200==0) 
			pushCnt++;
		pushCnt=pushCnt%100;
		X=(int)(position.ppsX);
		Y=(int)(position.ppsY);
		angle=(int)(position.ppsAngle);
		USART_OUT(UART4,(uint8_t*)"%d\t%d\t%d\r\n",X,Y,angle);
		SendUint8();
		VelCrl(CAN1,COLLECT_BALL_ID,60*4096);
		if(pushCnt%2)
		  PosCrl(CAN1, PUSH_BALL_ID,ABSOLUTE_MODE,PUSH_POSITION);
		else
		  PosCrl(CAN1, PUSH_BALL_ID,ABSOLUTE_MODE,PUSH_RESET_POSITION);
		YawAngleCtr(45);
		//走遍大部分场地
    //Round_Cover();
    //SET_closeRound(xx,yy,r,Set_Clock,Speed);
		//Square_Walk();
		//Strght_Walk (aa,bb,cc,FB);
		//Round(0.1,0.2);
		//我的1号车走一个正方形
		/**switch(flag)
		{
			case 0:
				PID(0);
			  VelCrl(CAN2,1,7000);
		    VelCrl(CAN2,2,-7000-Out_Pulse);
				if(position.X<-1550)
				  flag=1;
	    	break;
			case 1:
				PID(1);
			  VelCrl(CAN2,1,0);
		    VelCrl(CAN2,2,-Out_Pulse);
			  if(position.angle>89)
					flag=2;
				break;
			case 2:
				PID(1);
			  VelCrl(CAN2,1,7000);
		    VelCrl(CAN2,2,-7000-Out_Pulse);
			  if(position.Y>1550)
					flag=3;
				break;
			case 3:
				PID(2);
			  VelCrl(CAN2,1,0);
		    VelCrl(CAN2,2,-Out_Pulse);
			  if(position.angle>178)
					flag=4;
				break;
			case 4:
				PID(2);
				VelCrl(CAN2,1,7000);
		    VelCrl(CAN2,2,-7000-Out_Pulse);
				if(position.X>-450)
					flag=5;
				break;
			case 5:
				PID(3);
			  VelCrl(CAN2,1,0);
		    VelCrl(CAN2,2,-Out_Pulse);
			  if(position.angle>-90.5&&position.angle<-89.5)
				  flag=6; 
				break;
			case 6:
				PID(3);
			  VelCrl(CAN2,1,7000);
		    VelCrl(CAN2,2,-7000-Out_Pulse);
			  if(position.Y<450)
				  flag=7; 
				break;
			case 7:
				PID(0);
			  VelCrl(CAN2,1,0);
		    VelCrl(CAN2,2,-Out_Pulse);
			  if(position.angle>-1)
				  flag=0; 
				break;
		}**/
	}
}
//1m/s走任0意固定直线PID：通过距离PID的输出值输入到角度PID，让距离越来越近的同时角度越来越接近设定角度值
/**void Strght_Walk (float a,float b,float c,int F_B)
{
	float k,Agl,Dis;;
	Dis=fabs(a*position.X+b*position.Y+c)/sqrt(pow(a,2)+pow(b,2));
	if(b==0)
  {
	  if(position.X>-c/a) 
		{
			if(F_B==1)
			{
				PID_Awy(Dis);
				PID_Agl(90-Out_Agl);
			}
			else if(F_B==2)
			{
				PID_Awy(Dis);
				PID_Agl(-90+Out_Agl);
			}
		}
		else if(position.X<-c/a)
		{
			if(F_B==1)
			{
				PID_Awy(Dis);
				PID_Agl(90+Out_Agl);
			}
			else if(F_B==2)
			{
				PID_Awy(Dis);
				PID_Agl(-90-Out_Agl);
			}
		}
		else
		{
			if(F_B==1)
				PID_Agl(90);
			else if(F_B==2)
				PID_Agl(-90);
		}
  }
	else if(a==0)
	{
	  if(position.Y>-c/b) 
		{
			if(F_B==1)
			{
				PID_Awy(Dis);
				PID_Agl(180+Out_Agl);
			}
			else if(F_B==2)
			{
				PID_Awy(Dis);
				PID_Agl(0-Out_Agl);
			}
		}
		else if(position.Y<-c/b)
		{
			if(F_B==1)
			{
				PID_Awy(Dis);
				PID_Agl(180-Out_Agl);
			}
			else if(F_B==2)
			{
				PID_Awy(Dis);
				PID_Agl(0+Out_Agl);
			}
		}
		else
		{
			if(F_B==1)
				PID_Agl(180);
			else if(F_B==2)
				PID_Agl(0);
		}
	}
  else
  {
		k=-a/b;
		Agl=(atan(-a/b))*180/pai;
	  if(k>0)
	  {
	    if(position.Y<k*X-c/b)
			{
				if(F_B==1)
				{
					PID_Awy(Dis);
					PID_Agl(180-Agl-Out_Agl);
				}
				else if(F_B==2)
				{
					PID_Awy(Dis);
				  PID_Agl(-Agl+Out_Agl);
				}
			}
		  else if(position.Y>k*X-c/b)
			{
				if(F_B==1)
				{
					PID_Awy(Dis);
			    PID_Agl(180-Agl+Out_Agl);
				}
				else if(F_B==2)
				{
					PID_Awy(Dis);
				  PID_Agl(-Agl-Out_Agl);
				}
			}
		  else 
			{
				if(F_B==1)
			    PID_Agl(180-Agl);
				else if(F_B==2)
				  PID_Agl(-Agl);
			}
	  }
	  else if(k<0)
	  {
		  if(position.Y>k*X-c/b)
			{
			  if(F_B==1)
				{
					PID_Awy(Dis);
			    PID_Agl(-Agl-Out_Agl);
				}
				else if(F_B==2)
				{
					PID_Awy(Dis);
				  PID_Agl(-Agl-180+Out_Agl);
				}
			}
		  else if(position.Y<k*X-c/b)
			{
			  if(F_B==1)
				{
					PID_Awy(Dis);
			    PID_Agl(-Agl+Out_Agl);
				}
				else if(F_B==2)
				{
					PID_Awy(Dis);
				  PID_Agl(-Agl-180-Out_Agl);
				}
			}
		  else 
			{
			  if(F_B==1)
			    PID_Agl(-Agl);
				else if(F_B==2)
				  PID_Agl(-Agl-180);
			}
	  }
  }
	VelCrl(CAN2,1,10865);
  VelCrl(CAN2,2,-10865-Out_Pulse);
}**/

//一轮向前一轮向后原地转-正方形
/**void Square_Walk(void)

{
	float Dis;
	switch(flag)
	{
		case 0:
			Dis=-position.Y;
			PID_Awy(Dis);
		  PID_Agl(0+Out_Agl);
		  if(position.X<-1500)
				flag=1;
			break;
		case 1:
		  Dis=-(position.X+2000);
		  PID_Awy(Dis);
		  PID_Agl(90+Out_Agl);
		  if(position.Y>1500)
				flag=2;
			break;
		case 2:
			Dis=position.Y-2000;
		  PID_Awy(Dis);
		  PID_Agl(180+Out_Agl);
		  if(position.X>-500)
			  flag=3;
			break;
		case 3:
			Dis=position.X;
		  PID_Awy(Dis);
		  PID_Agl(-90+Out_Agl);
		  if(position.Y<500)
				flag=0;
			break;
	}
		VelCrl(CAN2,1,8000-Out_Pulse);
		VelCrl(CAN2,2,-8000-Out_Pulse);
}**/

//走任意固定圆
float Agl;
void SET_closeRound(float x,float y,float R,float clock,float speed)
{
	float Distance=0;
	float k;
	Distance=sqrt(pow(position.ppsX-x,2)+pow(position.ppsY-y,2))-R;
	k=(position.ppsX-x)/(y-position.ppsY);
	//顺1逆2
	if(clock==1)
	{
		if(position.ppsY>y)
		  Agl=90+atan(k)*180/pai;
	  else if(position.ppsY<y)
		  Agl=-90+atan(k)*180/pai;
	  else if(position.ppsY==y&&position.ppsX>=x)
		  Agl=0;
	  else if(position.ppsY==y&&position.ppsX>x)
		  Agl=180;
	  PID_Awy(Distance);
		PID_Agl(Agl-Out_Agl);
	}
	else if(clock==2)
	{
		if(position.ppsY>y)
		  Agl=-90+atan(k)*180/pai;
	  else if(position.ppsY<y)
		  Agl=90+atan(k)*180/pai;
	  else if(position.ppsY==y&&position.ppsX>=x)
		  Agl=180;
	  else if(position.ppsY==y&&position.ppsX>x)
		  Agl=0;
	  PID_Awy(Distance);
		PID_Agl(Agl+Out_Agl);
	}
	vel=(int)(speed*10865);
	VelCrl(CAN2,1,vel+Out_Pulse);
	VelCrl(CAN2,2,-vel+Out_Pulse);
}

void Round_Cover(void)
{
	static int count=0,lastcount=0,set=0,number=0,figure=0,Cnt=0,stopflag=0;
	Cnt++;
	if(Cnt>=100) Cnt=100;//限幅
	if(Cnt>50)//防止一开局的停止进入这里
	{
	  if(sqrt(pow(position.ppsSpeedX,2)+pow(position.ppsSpeedY,2))<400)
		  number++;
	  else number=0;
	  if(number>50)//长时间停下 倒退标志位 置1
	  {
		  number=0;
		  stopflag=1;
	  }
  }
	USART_OUT(UART4,(uint8_t*)"%d\t%d\t%d\t%d\t%d\t%d\r\n",(int)(position.ppsSpeedX),(int)(position.ppsSpeedY),Cnt,number,stopflag,(int)(R_cover));
	if(stopflag)//倒退 改变半径
	{
		figure++;
		VelCrl(CAN2,1,-10865);
	  VelCrl(CAN2,2,10865);
		if(figure>100)//后退0.5秒后出去以新半径转圈
		{
			figure=0;
			stopflag=0;
			if(R_cover>400)
				R_cover=R_cover-400;
			else
				R_cover=R_cover+400;
		}
	}
	else//转圈
	{
	  SET_closeRound(xx,yy,R_cover,startflag,Speed);
	  if(startflag==1)
	  {
		  if(Agl>93&&Agl<95)
		    count=1;
	    else 
		    count=0;
	    if(Agl>-90&&Agl<0)
		    set=1;
    }
  	else if(startflag==2)
	  {
		  if(Agl>-95&&Agl<-93)
		    count=1;
	    else 
	     	count=0;
		  if(Agl>0&&Agl<90)
		    set=1;
    }
	  if(R_cover>1800)
		  Flag=1;
	  else if(R_cover<600)
		  Flag=0;
	  if(count==1&&lastcount==0&&set)
	  {
		  if(Flag)
			  R_cover=R_cover-400;
		  else
		  	R_cover=R_cover+400;
		  set=0;
	  }
	  lastcount=count;
  }

}
void PID_Agl(float Set_Angle)
{
	float A_err;
	static float Last_Aerr,Sum_Aerr;
	A_err=Set_Angle-position.ppsAngle;
	if(A_err>180)
		A_err=A_err-360;
	if(A_err<-180)
		A_err=A_err+360;
	Sum_Aerr=Sum_Aerr+A_err;
	Out_Pulse=(int)(300*A_err);
		//+5*Sum_Aerr+5*(A_err-Last_Aerr));
	Last_Aerr=A_err;
}
void PID_Awy(float D_err)
{
	static float Last_Derr,Sum_Derr;
	Sum_Derr=Sum_Derr+D_err;
	Out_Agl=D_err/10;
	if(Out_Agl>=90)
    Out_Agl=90;
	//+5*Sum_Derr+5*(D_err-Last_Derr)); 
  Last_Derr=D_err;
}

/**void PID(int Agl_Flag)
{
	float err,Err1,Err2,Last_err,Sum_err;
	switch(Agl_Flag)
	{
		case 0:
			err=0-position.angle;
		  break;
		case 1:
			Err1=90-position.angle;
		  Err2=-270-position.angle;
		  if(fabs(Err1)<fabs(Err2))
			  err=Err1;
		  else err=Err2;
			break;
		case 2:
			Err1=180-position.angle;
		  Err2=-180-position.angle;
		  if(fabs(Err1)<fabs(Err2))
			  err=Err1;
		  else err=Err2;
			break;
		case 3:
			Err1=-90-position.angle;
		  Err2=270-position.angle;
		  if(fabs(Err1)<fabs(Err2))
			  err=Err1;
		  else err=Err2;
			break;
	}
	Sum_err=Sum_err+err;
	Out_Pulse=(int)(500*err);
		//+5*Sum_err+5*(err-Last_err));
	Last_err=err;
}**/
/**void Round(float speed,float R)
{
	vel1=(int)(10865*speed*(R-0.217)/R);
  vel2=-(int)(10865*speed*(R+0.217)/R);
}
**/


typedef union
{
    //这个32位整型数是给电机发送的速度（脉冲/s）
    int32_t Int32 ;
    //通过串口发送数据每次只能发8位
    uint8_t Uint8[4];

}num_t;

//定义联合体
num_t u_Num;
void SendUint8(void)
{
    u_Num.Int32 = 1000;

    //起始位
    USART_SendData(USART1, 'A');
    //通过串口1发数
    USART_SendData(USART1, u_Num.Uint8[0]);
    USART_SendData(USART1, u_Num.Uint8[1]);
    USART_SendData(USART1, u_Num.Uint8[2]);
    USART_SendData(USART1, u_Num.Uint8[3]);
    //终止位
    USART_SendData(USART1, 'J');
}
float YawTransform(float yawAngle)
{
	return (yawAngle * YAW_REDUCTION_RATIO * COUNT_PER_DEGREE);
}
void YawAngleCtr(float yawAngle)
{
	PosCrl(CAN1, GUN_YAW_ID, ABSOLUTE_MODE, YawTransform(yawAngle));
}