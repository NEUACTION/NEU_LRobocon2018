/**
  ******************************************************************************
  * @file	  moveBase.c
  * @author	  Action
  * @version   V1.0.0
  * @date	  2018/08/09
  * @brief	 2018省赛底盘运动控制部分
  ******************************************************************************
  * @attention
  *			None
  ******************************************************************************
  */
/* Includes -------------------------------------------------------------------------------------------*/

#include "moveBase.h"
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
#include "math.h"
#include "pps.h"
/* Private typedef ------------------------------------------------------------------------------------*/
/* Private define -------------------------------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------------------------------*/
/* Private functions ----------------------------------------------------------------------------------*/
/**
  * @brief  
  * @note
  * @param  
  * @retval None
  */

float Kp=90,Ki=0,Kd=0,err=0,lastErr=0,Sumi=0,Vk=0,errl,lastErr1;
extern int R;
void Straight(float v)
{
	VelCrl(CAN1,1,-v*CAR_WHEEL_COUNTS_PER_ROUND*REDUCTION_RATIO*WHEEL_REDUCTION_RATIO/(pi*WHEEL_DIAMETER));
	VelCrl(CAN1,2,-Vk*CAR_WHEEL_COUNTS_PER_ROUND*REDUCTION_RATIO*WHEEL_REDUCTION_RATIO/(pi*TURN_AROUND_WHEEL_DIAMETER));
}	
void Spin(float R,float v)
{
	VelCrl(CAN2,1,v*4096/(pi*WHEEL_DIAMETER));
	VelCrl(CAN2,2,-v*4096*(R+(WHEEL_TREAD-WHEEL_WIDTH)/2)/((R-(WHEEL_TREAD-WHEEL_WIDTH)/2)*pi*WHEEL_DIAMETER));
}
void TurnRight(float angle,float v)
{
	VelCrl(CAN2,1,0);
	VelCrl(CAN2,2,-v*4096*WHEEL_TREAD*angle*2/((WHEEL_DIAMETER)*360));
}	
void AnglePID(float setAngle,float feedbackAngle)
{
	err=setAngle-feedbackAngle;
	if(err>180)
		err=-(360-err);
	if(err<-180)
		err=360+err;
	Sumi+=Ki*err;
	Vk=Kp*err+Sumi+Kd*(err-lastErr);
	if(Vk>1300)
		Vk=1300;
	if(Vk<-1300)
		Vk=-1300;
	lastErr=err;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																									
}	
float k,b,lAngle,setAngle,d;
int flag;
extern float yawAngle,x,y,lastX,lastY;
extern int status;
void GetFunction(float x1,float y1,float x2,float y2)
{
		if(x1-0.1<x2&&x2<x1+0.1)
		{
			if(y2-y1>0)
				lAngle=0;
			else
				lAngle=180;
			flag=0;
		}
	  	else
		{
			k=(y2-y1)/(x2-x1);
			b=y1-k*x1;
			if(k>0)
			{	
				if(y2-y1>=0)
					lAngle=(atan(k)*180/pi)-90;
				else 
					lAngle=(atan(k)*180/pi)+90;
			}	
			else 
			{
				if(y2-y1>=0)
					lAngle=(atan(k)*180/pi)+90;
				else 
					lAngle=(atan(k)*180/pi)-90;
			}	
			if(y1-0.5<y2&&y2<y1+0.5)
			{	
				if(x2-x1>0)
					lAngle=-90;
				if(x2<x1)
					lAngle=90;
			}	
			flag=1;
		}
}	
void linePID(float x1,float y1,float x2,float y2,float v)
{
		
		GetFunction(x1,y1,x2,y2);
		if(flag)
		{
			if(k>0)
				if((y-k*x-b)*(y2-y1)>0)
					setAngle=lAngle-90*(1-1000/(y-k*x-b+1000));
				else
					setAngle=lAngle+90*(1-1000/(-y+k*x+b+1000));
			else
				if((y-k*x-b)*(y2-y1)>0)
					setAngle=lAngle+90*(1-1000/(y-k*x-b+1000));
				else
					setAngle=lAngle-90*(1-1000/(-y+k*x+b+1000));
			//斜率为0	
			if(y1-0.5<y2&&y2<y1+0.5)
			{	
				if(x2>x1)
					if(y>y1)
						setAngle=lAngle-90*(1-1000/(y-b+1000));
					else
						setAngle=lAngle+90*(1-1000/(-y+b+1000));
				else
					if(y>y1)
						setAngle=lAngle+90*(1-1000/(y-b+1000));
					else
						setAngle=lAngle-90*(1-1000/(-y+b+1000));	
			}	
		}	
		//斜率不存在
	 	else
		{	
			if((y2-y1)*(x-x1)<0)
					setAngle=lAngle-90*(1-1000/(fabs(x-x1)+1000));
			else
					setAngle=lAngle+90*(1-1000/(fabs(x-x1)+1000));	
		}
		AnglePID(setAngle,GetAngle());
		Straight(v);
		
}	
void CirclePID(float x0,float y0,float R,float v,int status)
{
	x=GetX();
	y=GetY();
	GetFunction(x,y,x0,y0);
	d=(x-x0)*(x-x0)+(y-y0)*(y-y0);
	//逆时针
	if(status==0)
	{	
		if(sqrtf(d)>R)
			setAngle=lAngle-90*(600/(fabs(sqrtf(d)-R)+600));	
		else
			setAngle=lAngle-180+90*(600/(fabs(sqrtf(d)-R)+600));		
	}	 
	//顺时针
	if(status==1)
	{	
		if(sqrtf(d)>R)
			setAngle=lAngle+90*(600/(fabs(sqrtf(d)-R)+600));
		else
			setAngle=lAngle+180-90*(600/(fabs(sqrtf(d)-R)+600));
	}	
	AnglePID(setAngle,GetAngle());
	Straight(v);
}	
/********************* (C) COPYRIGHT NEU_ACTION_2018 ****************END OF FILE************************/
extern float Distance,shootX,shootY,angle,antiRad,location[4][2];
extern int bingoFlag[4][2],haveShootedFlag,errTime,throwFlag,RchangeFlag;
void GetYawangle(uint8_t StdId)
{
	if(status==0)
	{	
		GetFunction(shootX,shootY,location[StdId][0],location[StdId][1]);
		if((GetAngle()+antiRad*180/pi)<-90&&lAngle>90)
			yawAngle=360+(GetAngle()+antiRad*180/pi)-lAngle;
		else
			yawAngle=(GetAngle()+antiRad*180/pi)-lAngle;		
	}
	else
	{
		GetFunction(shootX,shootY,location[StdId][0],location[StdId][1]);
		if(lAngle<-90&&(GetAngle()-antiRad*180/pi)>90)
			yawAngle=-360+(GetAngle()-antiRad*180/pi)-lAngle;
		else
			yawAngle=(GetAngle()-antiRad*180/pi)-lAngle;	
	}	
}
void GetDistance(uint8_t StdId)
{
	Distance=sqrtf((shootX-location[StdId][0])*(shootX-location[StdId][0])+(shootY-location[StdId][1])*(shootY-location[StdId][1]));
}	
void BingoJudge(uint8_t StdId)
{
	if(haveShootedFlag==1&&bingoFlag[StdId][0]==0)
	{	
		bingoFlag[StdId][0]=5-errTime;
		haveShootedFlag=0;
	}	
	else if(haveShootedFlag==1&&bingoFlag[StdId][1]==0)
	{	
		bingoFlag[StdId][1]=5-errTime;
		haveShootedFlag=0;
	}					
	if(bingoFlag[StdId][1]!=0)
		throwFlag=0;
	else if(bingoFlag[StdId][0]!=0)
	{
		for(uint8_t i=0;i<4;i++)
		{
			if(bingoFlag[i][0]==0&&i!=StdId)
			{
				throwFlag=0;
				break;
			}	
		}
	}
}

void GetShootSituation(uint8_t StdId)
{
	GetYawangle(StdId);
	GetDistance(StdId);
	BingoJudge(StdId);
}	
int FirstshootJudge(void)
{
	int StdId,maxPriority;
	maxPriority=bingoFlag[0][0];
	for(int i=1;i<3;i++)
	{
		if(bingoFlag[i][0]<bingoFlag[0][0])
		{
			maxPriority=bingoFlag[i][0];
			StdId=i;
		}	
	}	
	return StdId;
}	
int RchangeTime=125; 
void Rchange(int Rchange)
{
	if(--RchangeTime>=0)
		R+=Rchange/125;
	else
	{
		RchangeFlag=0;
		RchangeTime=125;
	}
}
void IncreaseR(int Radium)
{	
	if(status==0)
	{
		if(x>-100&&lastX<-100&&y<2400&&R<Radium)
			RchangeFlag=1;
	}	
	else
	{
		if(x<100&&lastX>100&&y<2400&&R<Radium)
			RchangeFlag=1;
	}		
	if(RchangeFlag)
		Rchange(500);	
}	
void DecreaseR(int Radium)
{	
	if(status==0)
	{
		if(x>-100&&lastX<-100&&y<2400&&R>Radium)
			RchangeFlag=1;
	}	
	else
	{
		if(x<100&&lastX>100&&y<2400&&R>Radium)
			RchangeFlag=1;
	}		
	if(RchangeFlag)
		Rchange(-500);	
}	
extern int errFlag,semiPushCount,count,backwardCount;
extern float angle,speed,speedY,speedX;
void Avoidance()
{
		static int errSituation1,errSituation2,statusFlag,lastTime=0,time=0;
		static float changeAngle;
		if(errFlag==1)
		{	
			if(errSituation1)
			{
				Kp=40;
				AnglePID(changeAngle,GetAngle());
				Straight(-1500);
			}
			if(errSituation2)
			{
				AnglePID(changeAngle,GetAngle());
				Straight(1800);
			}	
			if(errTime%2==0&&statusFlag)
			{
				status=1-status;
				statusFlag=0;
				PosCrl(CAN2,PUSH_BALL_ID,ABSOLUTE_MODE,(--semiPushCount)*PUSH_POSITION/2+count*PUSH_POSITION);
			}	
			if(Get_Time_Flag())	
				errFlag=0;
			time=0;
			lastTime=0;
			throwFlag=0;
		}	
		else
		{
			time++;
			//与对手相持
			if((fabs(atan((tan(angle)-speedY/speedX)/(1-tan(angle)*speedY/speedX)))*pi/180)<20&&speed>1200)
				lastTime=time;
			else				
			{
				if((fabs(atan((tan(angle)-speedY/speedX)/(1-tan(angle)*speedY/speedX)))*pi/180)>20)
				{
					errSituation2=1;
					changeAngle=-atan(speedY/speedX)*180/pi;
				}	
				else if(speed>1200)
				{
					errSituation1=1;
					GetFunction(x,y,0,2400);
					if(R<=1100)
						changeAngle=lAngle;
					else
						changeAngle=lAngle+180;
				}			
			}	
			if(time-lastTime>=80)
			{	
				errFlag=1;
				statusFlag=1;
				errTime++;
				backwardCount=0;
			}	
		}	
}	
/*激光模式*/
float getLingtAngle(float xi,float yi,int tragetCnt)
{
	static float angii=0;	
	if(tragetCnt==0)
		angii=GetAngle()+90-180/pi*atan((0-yi)/(2400-xi));
	if(tragetCnt==1)
		angii=GetAngle()+90-180/pi*atan((4800-yi)/(2400-xi));
	if(tragetCnt==2)
		angii=GetAngle()-90-180/pi*atan((4800-yi)/(-2400-xi));
	if(tragetCnt==3)
		angii=GetAngle()-90-180/pi*atan((0-yi)/(-2400-xi));
	return angii;
}