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

#include <stdint.h>
#include "pid.h"
#include "stm32f4xx_it.h"
#include "usart.h"
#include <math.h>
#include "elmo.h"
#include "trans.h"
/* Includes ------------------------------------------------------------------*/



/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  
  */


 
/* Exported constants --------------------------------------------------------*/



/** @defgroup 
  * @{
  */
  




#define PI (3.141593)
#define TURN_RIGHT 0
#define TURN_LEFT  1
#define KP_A (266.55/14000)
#define KP_D (0.23)
//后轮电机的CAN ID号
#define BACK_WHEEL_ID                                                 (1)
//前轮转向电机的CAN ID号
#define TURN_AROUND_WHEEL_ID                                          (2)
//轮子直径（单位：mm）
#define WHEEL_DIAMETER 												  (120.0f)
//两个轮子中心距离（单位：mm）
#define WHEEL_TREAD                                                   (337.0f)
//前轮转向电机到后轮两轮轴中心间距
#define TURN_AROUND_WHEEL_TO_BACK_WHEEL                               (266.55f)
//定位系统到后轮两轮轴中心间距
#define OPS_TO_BACK_WHEEL                                             (180.47f)
//前轮后轮都是3508转一周脉冲都为8192
#define NEW_CAR_COUNTS_PER_ROUND                                      (8192)
//其余电机转一周脉冲
#define OTHER_COUNTS_PER_ROUND                                        (32768)
//转向轮子直径
#define TURN_AROUND_WHEEL_DIAMETER                                    (50.72f)
//3508电机减速比，相当于给出去的脉冲要多乘上减速比
#define REDUCTION_RATIO                                               (19.2f)
//电机与轮传动比
#define TRANSMISSION_RATIO                                            (253/252)
//
/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void Turn(float angle,float gospeed);
void BackTurn(float angle,float gospeed);
uint8_t straightLine(float A1,float B1,float C1,uint8_t dir,float setSpeed);
uint8_t BackstraightLine(float A1,float B1,float C1,uint8_t dir,float setSpeed);
void Squre(void);
void Squre2(void);
void BiggerSquareOne(void);
void BiggerSquareTwo(void);
float Speed_X(void);
float Speed_Y(void);
void Walk(uint8_t getAdcFlag);
void Transformation(void);
void N_Strght_Walk (float a,float b,float c,int F_B,float backspeed);
void N_Back_Strght_Walk (float a,float b,float c,int F_B,float backspeed);
void The_Second_Round(void);
void N_SET_closeRound(float x,float y,float R,float clock,float backspeed);
void Tangencystraightline();
void The_Collect_Round();
#endif /* ___H */



/************************ (C) COPYRIGHT NEU_ACTION_2017 *****END OF FILE****/

