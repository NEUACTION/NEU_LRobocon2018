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


/* Private typedef ------------------------------------------------------------------------------------*/
/* Private define -------------------------------------------------------------------------------------*/
/* Private macro --------------------------------------------------------------------------------------*/
/* Private variables ----------------------------------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------------------------------*/
/* Private functions ----------------------------------------------------------------------------------*/

/*****************定义的一些全局变量用于串口返回值****************************/



extern struct usartValue_{
	uint32_t cnt;//用于检测是否数据丢失
	float xValue;//串口输出x坐标
	float yValue;//串口输出y坐标
	float angleValue;//串口输出角度值
	float pidValueOut;//PID输出
	float d;
	float turnAngleValue;//
	uint8_t flagValue;
	float shootangle;
	float shootSp;
	float X;
	float Y;
	float V;
}usartValue;




uint8_t errFlg=0;
/**
  * @brief  PID 转弯
  * @note	
  * @param  angle：给定角度,为正左转，为负右转
  * @param  gospeed：基础速度
  * @retval None
  */

void Turn(float angle,float gospeed)
{
	int32_t pulseNum=0;
	int32_t bPulseNum=(gospeed*4095)/(PI*WHEEL_DIAMETER);
	float getAngle=0;
	float speed=0;
	getAngle=GetAngle();
	
	speed=AnglePid(angle,getAngle);	
	usartValue.pidValueOut=speed;
	
	pulseNum=(speed*4095)/(PI*WHEEL_DIAMETER);
	
	VelCrl(CAN2, 0x01,bPulseNum+pulseNum);
	VelCrl(CAN2, 0x02,pulseNum-bPulseNum);

}	
 
/**
  * @brief  PID 后退转弯
  * @note	
  * @param  angle：给定角度
  * @param  gospeed：基础速度
  * @retval None
  */

void BackTurn(float angle,float gospeed)
{
	int32_t pulseNum=0;
	int32_t bPulseNum=-(gospeed*4095)/(PI*WHEEL_DIAMETER);
	float getAngle=0;
	float speed=0;
	getAngle=GetAngle();
	
	speed=AnglePid(angle,getAngle);	
	usartValue.pidValueOut=speed;
	
	pulseNum=(speed*4095)/(PI*WHEEL_DIAMETER);
	VelCrl(CAN2, 0x01,bPulseNum+pulseNum);
	VelCrl(CAN2, 0x02,pulseNum-bPulseNum);

}


/**
  * @brief  后退走直线
  * @note	
  * @param  
  * @retval None
  */

uint8_t BackstraightLine(float A2,float B2,float C2,uint8_t dir)
{
	float setAngle=0;
	float getAngle=GetAngle();
	float getX=GetPosX();
	float getY=GetPosY();
	float distance=((A2*getX)+(B2*getY)+C2)/sqrt(A2*A2+B2*B2);
	float angleAdd=0.1*distance;
	if(angleAdd > 90)
	{
		angleAdd=90;
	}
	else if(angleAdd < -90)
	{
		angleAdd=-90;
	}
	
	
	if((B2 > -0.005) && (B2 < 0.005))
	{
		if(!dir)
		{
			if(A2 > 0)
				{
					setAngle=-180;
				}
				else
				{
					setAngle=180;
				}
		}
		else
		{
				setAngle=0;
		}
	}
	else
	{
		if(!dir)
		{
			setAngle=(atan(-A2/B2)*180/PI)-90;
		}
		else
		{
			setAngle=(atan(-A2/B2)*180/PI)+90;
		}
	}

	if(distance > 10)
	{
		if(setAngle >= 0)
		{
			BackTurn(setAngle-angleAdd,1000);
		}
		else
		{
			BackTurn(setAngle+angleAdd,1000);			
		}
	}
	else if(distance < -10)
	{
		if(setAngle >= 0)
		{
			BackTurn(setAngle-angleAdd,1000);
		}
		else
		{
			BackTurn(setAngle+angleAdd,1000);		
		}
	}
	else
	{
		BackTurn(setAngle,1000);
	}
	if((distance < 100) && (distance > -100))
		return 1;
	else
		return 0; 
}


/**
  * @brief  沿直线走，能回到直线,距离速度pid
  * @note	给定直线以Ax+By+C=0形式
  * @param  A：
  * @param  B:
  * @param  C:
  * @param  dir:dir为0，向右；dir为1，向左
  * @retval None
  */

void HighSpeedStraightLine(float A1,float B1,float C1,uint8_t dir,float highSpeed)
{
	float setAngle=0;
	float getAngle=GetAngle();
	float getX=GetPosX();
	float getY=GetPosY();
	float distance=((A1*getX)+(B1*getY)+C1)/sqrt(A1*A1+B1*B1);
	float angleAdd=DistancePid(distance,0);
	
	

	if((B1 > -0.005) && (B1 < 0.005))
	{
		if(!dir)
		{
			setAngle=0;
		}
		else
		{
				if(A1 > 0)
				{
					setAngle=-180;
				}
				else
				{
					setAngle=180;
				}
		}
	}
	else
	{
		if(!dir)
		{
			setAngle=(atan(-A1/B1)*180/PI)-90;
		}
		else
		{
			setAngle=(atan(-A1/B1)*180/PI)+90;
		}
	}

	if(distance > 10)
	{
		if(setAngle < 0)
		{
			Turn(setAngle-angleAdd,highSpeed);
		}
		else
		{
			Turn(setAngle+angleAdd,highSpeed);		
		}
	}
	else if(distance < -10)
	{
		if(setAngle >= 0)
		{
			Turn(setAngle+angleAdd,highSpeed);
		}
		else
		{
			Turn(setAngle-angleAdd,highSpeed);		
		}
	}
	else
	{
		Turn(setAngle,highSpeed);
	}
	
}

/**
  * @brief  顺时针走方形，面积渐渐变大，偏离轨道后能回去
  * @note	
  * @param 
  * @retval None
  */
uint8_t flagOne=0;
extern float outMax3;
extern float outMin3; 

void BiggerSquareOne(void)
{
	float sTAngle=GetAngle();
	float sTX=GetPosX();
	float sTY=GetPosY();
	float speed1=0;
	if(flagOne<8)
	{		
		outMax3=1300;
		outMin3=1300; 
	}
	else 
	{
		outMax3=1300;
		outMin3=1300; 
	}
	switch(flagOne)
	{
		case 0:
			if(sTY < 1900)
			{
				N_Strght_Walk(1,0,700,1,1300);
				
			}
			else
				flagOne++;
			break;
		case 1:
			if(sTX < -200)
			{
				N_Strght_Walk(0,1,-2900,1,1300);
				
			}
			else
				flagOne++;
			break;
		case 2:
			if(sTY > 2600)
			{
				N_Strght_Walk(1,0,-700,2,1300);
				
			}
			else
				flagOne++;
			break;
		case 3:
			if(sTX > -300)
			{
				N_Strght_Walk(0,1,-1600,2,1300);
				
			}
			else
				flagOne++;
			break;
			
		case 4:
			if(sTY < 2600)
			{
				if(sTY < 2150)
					speed1=SpeedPid(sTY,950);
				else
					speed1=SpeedPid(3200,sTY);
				N_Strght_Walk(1,0,1300,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 5:
			if(sTX < 300)
			{
				if(sTX > -400)
					speed1=SpeedPid(900,sTX);
				else
					speed1=SpeedPid(sTX,-2000);
				N_Strght_Walk(0,1,-3600,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 6:
			if(sTY > 1900)
			{
				if(sTY < 2850)
					speed1=SpeedPid(sTY,1350);
				else
					speed1=SpeedPid(4300,sTY);
				N_Strght_Walk(1,0,-1300,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 7:
			if(sTX > -800)
			{
				if(sTX > 0)
					speed1=SpeedPid(2000,sTX); 
				else
					speed1=SpeedPid(sTX,-1450);
				N_Strght_Walk(0,1,-900,2,speed1);
				
			}
			else
				flagOne++;
			break;
			
		case 8:
			if(sTY < 2800)
			{
				if(sTY < 2150)
					speed1=SpeedPid(sTY,250);
				else
					speed1=SpeedPid(3550,sTY);
				N_Strght_Walk(1,0,1800,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 9:
			if(sTX < 800)
			{
				if(sTX > 0)
					speed1=SpeedPid(1150,sTX);
				else
					speed1=SpeedPid(sTX,-2550);
				N_Strght_Walk(0,1,-4200,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 10:
			if(sTY > 1500)
			{
				if(sTY < 2650)
					speed1=SpeedPid(sTY,1150);
				else
					speed1=SpeedPid(4950,sTY);
				N_Strght_Walk(1,0,-1800,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 11:
			if(sTX > -800)
			{
				if(sTX > 500)
					speed1=SpeedPid(2550,sTX);
				else
					speed1=SpeedPid(sTX,-1150);
				N_Strght_Walk(0,1,-500,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 12:
			if(sTY < 3200)
			{
				if(sTY < 2150)
					speed1=SpeedPid(sTY,-250);
				else
					speed1=SpeedPid(3550,sTY);
				N_Strght_Walk(1,0,1800,1,speed1);
				
			}
			else
				flagOne=9;
			break;
		default: flagOne=0;
			break;
	}
}

/**
  * @brief  逆时针走方形，面积渐渐变大，偏离轨道后能回去
  * @note	
  * @param 
  * @retval None
  */


void BiggerSquareTwo(void)
{
	float sTAngle=GetAngle();
	float sTX=GetPosX();
	float sTY=GetPosY();
	float speed1=0;
	usartValue.flagValue=flagOne;
	switch(flagOne)
	{
		case 0:
			if(sTY < 2150)
			{
				N_Strght_Walk(1,0,-700,1,1300);
				
			}
			else
				flagOne++;
			break;
		case 1:
			if(sTX > 200)
			{
				N_Strght_Walk(0,1,-3050,2,1300);

			}
			else
				flagOne++;
			break;
		case 2:
			if(sTY > 2550)
			{
				N_Strght_Walk(1,0,700,2,1300);
				
			}
			else
				flagOne++;
			break;
		case 3:
			if(sTX < 250)
			{
				N_Strght_Walk(0,1,-1650,1,1300);
				
			}
			else
				flagOne++;
			break;
			
		case 4:
			if(sTY < 2600)
			{
				if(sTY < 2200)
					speed1=SpeedPid(sTY,1000);
				else
					speed1=SpeedPid(3250,sTY);
				N_Strght_Walk(1,0,-1350,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 5:
			if(sTX > -250)
			{
				if(sTX > 400)
					speed1=SpeedPid(2000,sTX);
				else
					speed1=SpeedPid(sTX,-900);
				N_Strght_Walk(0,1,-3700,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 6:
			if(sTY > 2100)
			{
				if(sTY > 2850)
					speed1=SpeedPid(4350,sTY);
				else
					speed1=SpeedPid(sTY,1450);
				N_Strght_Walk(1,0,1350,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 7:
			if(sTX < 750)
			{
				if(sTX < 0)
					speed1=SpeedPid(sTX,-2000);
				else
					speed1=SpeedPid(1400,sTX);
				N_Strght_Walk(0,1,-1000,1,speed1);
				
			}
			else
				flagOne++;
			break;
			
		case 8:
			if(sTY < 2750)
			{
				if(sTY < 2200)
					speed1=SpeedPid(sTY,350);
				else
					speed1=SpeedPid(3500,sTY);
				N_Strght_Walk(1,0,-1850,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 9:
			if(sTX > -400)
			{
				if(sTX > 0)
					speed1=SpeedPid(2600,sTX);
				else
					speed1=SpeedPid(sTX,-1150);
				N_Strght_Walk(0,1,-4200,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 10:
			if(sTY > 1950)
			{
				if(sTY > 2300)
					speed1=SpeedPid(4950,sTY);
				else
					speed1=SpeedPid(sTY,1200);
				N_Strght_Walk(1,0,1850,2,speed1);
				
			}
			else
				flagOne++;
			break;
		case 11:
			if(sTX < 400)
			{
				if(sTX < 0 )
					speed1=SpeedPid(sTX,-2600);
				else
					speed1=SpeedPid(1150,sTX);
				N_Strght_Walk(0,1,-500,1,speed1);
				
			}
			else
				flagOne++;
			break;
		case 12: 
			if(sTY < 2750)
			{
				if(sTY < 2200)
					speed1=SpeedPid(sTY,-250);
				else
					speed1=SpeedPid(3500,sTY);
				N_Strght_Walk(1,0,-1850,1,speed1);
				
			}
			else
				flagOne=9;
			break;
		default: flagOne=0;
			break;
	}
}

/**
  * @brief  x方向速度
  * @note	
  * @param 
  * @retval None
  */

float Speed_X(void)
{
	static float tXLast=0;
	float tX=GetPosX();
	float speedX=(tX-tXLast)*100;
	tXLast=tX;
	return speedX;
}


/**
  * @brief  y方向速度
  * @note	
  * @param 
  * @retval None
  */

float Speed_Y(void)
{
	static float tYLast=0;
	float tY=GetPosY();
	float speedY=(tY-tYLast)*100;
	tYLast=tY;
	return speedY;
}


void Walk(uint8_t getAdcFlag)
{
	uint8_t ready=0;
	static float X_Now=0;
	static float Y_Now=0;
	static uint8_t errFlag=0;
	static uint16_t walkCnt=0;
	float speedx=0;
	float speedy=0;
	
	
	speedx=Speed_X();
	speedy=Speed_Y();
	
	//故障判断
	if((speedx < 100) && (speedx > -100) && (speedy < 100) && (speedy > -100))
	{
		walkCnt++;
		if(walkCnt >= 120)
		{
			errFlag=!errFlag;
			X_Now=GetPosX();
			Y_Now=GetPosY();
			walkCnt=0;
		} 
		if(errFlag == 1)
		{
			if(errFlg > 3)
				errFlg=3;
			else
				errFlg++;
			
		}
	}
	else
		errFlg=0;
	
	//故障处理，后退
	if(errFlag)
	{
		if(((X_Now+2300) <= Y_Now) && ((2300-X_Now) <= Y_Now))
		{
			if(Y_Now > 3200)
			{
				ready=BackstraightLine(0,1,500-Y_Now,getAdcFlag);
			}
			else if((Y_Now <= 3200) && (Y_Now > 2300))
			{
				ready=BackstraightLine(0,1,-500-Y_Now,getAdcFlag);
			}
		}
		else if(((X_Now+2300) > Y_Now) && ((2300-X_Now) > Y_Now))
		{
			if((Y_Now <= 2300) && (Y_Now > 1200))
			{
				ready=BackstraightLine(0,1,500-Y_Now,!getAdcFlag);
			}
			else if(Y_Now <= 1200)
			{
				ready=BackstraightLine(0,1,-500-Y_Now,!getAdcFlag);
			}
		}
		else if(((X_Now+2300) < Y_Now) && ((2300-X_Now) >= Y_Now))
		{
			if(X_Now <= 0 && X_Now > -1000)
			{
				ready=BackstraightLine(1,0,500-X_Now,!getAdcFlag);
			}
			else if(X_Now <= -1000)
			{
				ready=BackstraightLine(1,0,-500-X_Now,!getAdcFlag);
			}
		}
		else if(((X_Now+2300) >= Y_Now) && ((2300-X_Now) < Y_Now))
		{
		
			if(X_Now >= 0 && X_Now < 1000)
			{
				ready=BackstraightLine(1,0,-500-X_Now,getAdcFlag);
			}
			
			else if(X_Now >= 1000)
			{
				ready=BackstraightLine(1,0,500-X_Now,getAdcFlag);
			}
		}
		
		if(ready)
		{
			if(flagOne >= 8)
				flagOne=flagOne-4;
			else
				flagOne=flagOne+4;
			errFlag=!errFlag;
		}
		else;
	}
	
	//
	else
	{
		if(!getAdcFlag)
		{
			BiggerSquareOne();
		}
		else
		{
			BiggerSquareTwo();
		}
	}

}



//非差速直线 F_B=1上 F_B下 右
void N_Strght_Walk (float a,float b,float c,int F_B,float backspeed)
{
	float k,Dis;
	float Agl=0,setangle=0,frontspeed=0,addangle=0;
	Dis=fabs(a*GetPosX()+b*GetPosY()+c)/sqrt(pow(a,2)+pow(b,2));
	addangle=DistancePid(Dis,0);
	usartValue.X=GetPosX();
	usartValue.Y=GetPosY();
	if(b==0)
  {
	  if(GetPosX()>-c/a) 
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(0+addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(180-addangle,GetAngle());
			}
		}
		else if(GetPosX()<-c/a)
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(0-addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(180+addangle,GetAngle());
			}
		}
		else
		{
			if(F_B==1)
				frontspeed=AnglePid(0,GetAngle());
			else if(F_B==2)
				frontspeed=AnglePid(180,GetAngle());
		}
  }
	else if(a==0)
	{
	  if(GetPosY()>-c/b) 
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(-90-addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(90+addangle,GetAngle());
			}
		}
		else if(GetPosY()<-c/b)
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(-90+addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(90-addangle,GetAngle());
			}
		}
		else
		{
			if(F_B==1)
				frontspeed=AnglePid(-90,GetAngle());
			else if(F_B==2)
				frontspeed=AnglePid(90,GetAngle());
		}
	}
  else
  {
		k=-a/b;
		Agl=(atan(-a/b))*180/PI;
	  if(k>0)
	  {
	    if(GetPosY()<k*GetPosX()-c/b)
			{
				if(F_B==1)
				{
					setangle=Agl-90;
					frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
			}
		  else if(GetPosY()>k*GetPosX()-c/b)
			{
				if(F_B==1)
				{
					setangle=Agl-90;
			    frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
			}
		  else 
			{
				if(F_B==1)
				{
					setangle=Agl-90;
			    frontspeed=AnglePid(setangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle-90,GetAngle());
				}
			}
	  }
	  else if(k<0)
	  {
		  if(GetPosY()>k*GetPosX()-c/b)
			{
			  if(F_B==1)
				{
					setangle=Agl+90;
			    frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
			}
		  else if(GetPosY()<k*GetPosX()-c/b)
			{
			  if(F_B==1)
				{
					setangle=Agl+90;
			    frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
			}
		  else 
			{
			  if(F_B==1)
				{
					setangle=Agl+90;
			    frontspeed=AnglePid(setangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle,GetAngle());
				}
			}
	  }
  }
	usartValue.V=frontspeed;
	VelCrl(CAN1,1,-backspeed*REDUCTION_RATIO*NEW_CAR_COUNTS_PER_ROUND/(PI*WHEEL_DIAMETER));//后轮
  VelCrl(CAN1,2,-frontspeed*REDUCTION_RATIO*NEW_CAR_COUNTS_PER_ROUND/(PI*TURN_AROUND_WHEEL_DIAMETER));//前轮
}
void N_Back_Strght_Walk (float a,float b,float c,int F_B,float backspeed)
{
	float k,Dis;
	float Agl=0,setangle=0,frontspeed=0,addangle=0;
	Dis=fabs(a*GetPosX()+b*GetPosY()+c)/sqrt(pow(a,2)+pow(b,2));
	addangle=DistancePid(Dis,0);
	if(b==0)
  {
	  if(GetPosX()>-c/a) 
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(180+addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(0-addangle,GetAngle());
			}
		}
		else if(GetPosX()<-c/a)
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(180-addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(0+addangle,GetAngle());
			}
		}
		else
		{
			if(F_B==1)
				frontspeed=AnglePid(180,GetAngle());
			else if(F_B==2)
				frontspeed=AnglePid(0,GetAngle());
		}
  }
	else if(a==0)
	{
	  if(GetPosY()>-c/b) 
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(90-addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(-90+addangle,GetAngle());
			}
		}
		else if(GetPosY()<-c/b)
		{
			if(F_B==1)
			{
				frontspeed=AnglePid(90+addangle,GetAngle());
			}
			else if(F_B==2)
			{
				frontspeed=AnglePid(-90-addangle,GetAngle());
			}
		}
		else
		{
			if(F_B==1)
				frontspeed=AnglePid(90,GetAngle());
			else if(F_B==2)
				frontspeed=AnglePid(-90,GetAngle());
		}
	}
  else
  {
		k=-a/b;
		Agl=(atan(-a/b))*180/PI;
	  if(k>0)
	  {
	    if(GetPosY()<k*GetPosX()-c/b)
			{
				if(F_B==1)
				{
					setangle=Agl+90;
					frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
			}
		  else if(GetPosY()>k*GetPosX()-c/b)
			{
				if(F_B==1)
				{
					setangle=Agl+90;
			    frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
			}
		  else 
			{
				if(F_B==1)
				{
					setangle=Agl+90;
			    frontspeed=AnglePid(setangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl-90;
				  frontspeed=AnglePid(setangle-90,GetAngle());
				}
			}
	  }
	  else if(k<0)
	  {
		  if(GetPosY()>k*GetPosX()-c/b)
			{
			  if(F_B==1)
				{
					setangle=Agl-90;
			    frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
			}
		  else if(GetPosY()<k*GetPosX()-c/b)
			{
			  if(F_B==1)
				{
					setangle=Agl-90;
			    frontspeed=AnglePid(setangle-addangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle+addangle,GetAngle());
				}
			}
		  else 
			{
			  if(F_B==1)
				{
					setangle=Agl-90;
			    frontspeed=AnglePid(setangle,GetAngle());
				}
				else if(F_B==2)
				{
					setangle=Agl+90;
				  frontspeed=AnglePid(setangle,GetAngle());
				}
			}
	  }
  }
	VelCrl(CAN2,1,backspeed*REDUCTION_RATIO*NEW_CAR_COUNTS_PER_ROUND/(PI*WHEEL_DIAMETER));//后轮
  VelCrl(CAN2,2,frontspeed*REDUCTION_RATIO*NEW_CAR_COUNTS_PER_ROUND/(PI*TURN_AROUND_WHEEL_DIAMETER));//前轮
}


/********************* (C) COPYRIGHT NEU_ACTION_2018 ****************END OF FILE************************/

void The_Second_Round(void)
{
	float sTAngle=GetAngle();
	float sTX=GetPosX();
	float sTY=GetPosY();
	switch(flagOne)
	{
		case 0:
			if(sTY < 2600)
			{
				N_Strght_Walk(1,0,1300,1,1000);
				
			}
			else
				flagOne++;
			break;
		case 1:
			if(sTX < 300)
			{
				N_Strght_Walk(0,1,-3600,1,1000);
				
			}
			else
				flagOne++;
			break;
		case 2:
			if(sTY > 2000)
			{
				N_Strght_Walk(1,0,-1300,2,1000);
				
			}
			else
				flagOne++;
			break;
		case 3:
			if(sTX > -300)
			{
				N_Strght_Walk(0,1,-1000,2,1000);
				
			}
			else
				flagOne=0;
			break;
	 }
}




