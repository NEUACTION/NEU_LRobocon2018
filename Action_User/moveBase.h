/**
  ******************************************************************************
  * @file    .h
  * @author  ACTION_2017
  * @version V0.0.0._alpha
  * @date    2017//
  * @brief   This file contains all the functions prototypes for 
  *          
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOVEBASE_H
#define __MOVEBASE_H


/* Includes ------------------------------------------------------------------*/



/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  
  */


 
/* Exported constants --------------------------------------------------------*/



/** @defgroup 
  * @{
  */

//#define 

//电机旋转一周的脉冲数
#define COUNTS_PER_ROUND (4096)
//轮子直径（单位：mm）
#define WHEEL_DIAMETER (106.8f)
//调试小车车长（单位：mm）
#define MOVEBASE_LENGTH (500.0f)
//调试小车车宽(单位：mm)
#define MOVEBASE_WIDTH (403.0f)
//轮子宽度（单位：mm）
#define WHEEL_WIDTH (46.0f)
//两个轮子中心距离（单位：mm）
#define WHEEL_TREAD (355.4f)
//车轮周长(mm)
#define WHEEL_CIRCUMFERENCE (1116.3f)

#include "elmo.h"
/**
  * @}
  */
#define P_Angle 35
#define I_Angle 0
#define D_Angle 2

#define P_Location 0.1f
#define I_Location 0
#define D_Location 0

#define P_CircleDistance 0.1f
#define I_CircleDistance 0
#define D_CircleDistance 0

void CircleAround(float CenterXpos, float CenterYpos,float radius, float BasicSpeed, uint8_t direction); //(圆心横坐标(mm), 圆心纵坐标(mm), 半径(mm)), 基础线速度(mm/s), 绕行方向(1为顺时针, 2为逆时针))
void RectangleAround(float length, float width, float BasicSpeed); //(长(mm), 宽(mm), 速度(mm/s))
void LockLineMove(uint8_t ExistSlope,double k, float b, float SetXpos, float BasicSpeed, uint8_t direction); //(是否存在斜率(1为存在 0为不存在), 设定直线斜率, 设定直线截距, 若不存在斜率则填写设定直线横坐标, 基础速度, 方向(1为沿X轴正向, 0为负向))
float AnglePID(float Kp, float Ki, float Kd, float AngleSet, float AngleActual);
float LocationPID(float Kp, float Ki, float Kd, float PointSet, float PointActual);
float CircleDistancePID(float Kp, float Ki, float Kd, float RadiusSet, float DistanceToCenterActual);
void Move(float SpeedL, float  SpeedR);
void SetAngle(float val);
void SetXpos(float val);
void SetYpos(float val);
float GetAngle(void);
float GetXpos(void);
float GetYpos(void);
float AbsoluteValue(float value);





#endif /* ___H */



/************************ (C) COPYRIGHT NEU_ACTION_2017 *****END OF FILE****/

