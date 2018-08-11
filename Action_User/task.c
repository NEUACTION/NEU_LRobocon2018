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
#include "stm32f4xx_it.h"
#include "stm32f4xx_usart.h"
/*           车的基本信息         */
#define COUNT_PER_ROUND (4096.0f)  //电机旋转一周的脉冲数 
#define WHEEL_DIAMETER (120.0f)    //轮子直径(mm)
#define ROBOT_LENGTH (492.0f)      //调试小车车长(mm)
#define ROBOT_WIDTH (490.0f)       //调试小车车宽(mm)
#define WHEEL_WIDTH (40.0f)        //轮子宽度(mm)
#define WHEEL_TREAD (434.0f)       //两个轮子的中心距离(mm)
#define PI (3.14)
#define CIRCLE_LENGTH (PI*WHEEL_DIAMETER) //车轮转一圈走的距离

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
						  2);

	os_err = OSTaskCreate((void (*)(void *))WalkTask,
						  (void *)0,
						  (OS_STK *)&WalkTaskStk[Walk_TASK_STK_SIZE - 1],
						  3);
}

/*
   ===============================================================
   初始化任务
   ===============================================================
   */
void ConfigTask(void)
{
	CPU_INT08U os_err;
	os_err = os_err;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TIM_Init(TIM2, 1000-1, 83, 0, 2);
	USART3_Init(115200);
	USART4_Init(921600);  
	CAN_Config(CAN1,500,GPIOB,GPIO_Pin_8,GPIO_Pin_9);
	CAN_Config(CAN2,500,GPIOB,GPIO_Pin_5,GPIO_Pin_6);
	ElmoInit(CAN2);
	VelLoopCfg(CAN2,1,1000,0);
	VelLoopCfg(CAN2,2,1000,0);
	MotorOn(CAN2,1);
	MotorOn(CAN2,2);
	
	OSTaskSuspend(OS_PRIO_SELF);
}
void walk_direct(float v) //左轮速度,单位m/s
{
	float vel=0,temp_vel=0.;
	temp_vel=COUNT_PER_ROUND*v/CIRCLE_LENGTH;
	vel=1000*temp_vel;
	VelCrl(CAN2,1,vel); //设置右轮速度
	VelCrl(CAN2,2,-vel); //设置左轮速度
}

float value(float r)//求取两个轮子的速度比值
{
	float value=0,r1=0,r2=0;
	r1=r+(WHEEL_TREAD/2);
	r2=r-(WHEEL_TREAD/2);
	value=r1/r2;
	return value;
}
void turn(float r,float L_w)//转弯的弯道的半径，左轮的角速度
{
	float L_vel=0,R_vel=0;
	L_vel=-L_w*WHEEL_TREAD*COUNT_PER_ROUND/CIRCLE_LENGTH/2;
	R_vel=L_vel*value(0);
	VelCrl(CAN2,1,R_vel);
	VelCrl(CAN2,2,-L_vel);
}
void WalkTask(void)
{
	int time=0,stage=0;
	CPU_INT08U os_err;
	os_err = os_err;
	OSSemSet(PeriodSem, 0, &os_err);
	while (1)
	{
		OSSemPend(PeriodSem,0,&os_err);
		time++;
		stage=time/100;
		switch(stage)
		{
			case 4:turn(0,PI/2); break;
			case 9:turn(0,PI/2); break;
			case 14:turn(0,PI/2); break;
			case 19:turn(0,PI/2); break;
			default:walk_direct(0.5); break;
		}
		if(time>2000)
			time=0;
	}
}


	
	
