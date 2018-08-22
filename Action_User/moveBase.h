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
#include "stdint.h"
#include "elmo.h"
#include "usart.h"
#include "can.h"
#include "stm32f4xx_gpio.h"


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
#define COUNTS_PER_ROUND (4096.0f)
//轮子直径（单位：mm）
#define WHEEL_DIAMETER (120.0f)
//调试小车车长（单位：mm）
#define MOVEBASE_LENGTH (492.0f)
//调试小车车宽(单位：mm)
#define MOVEBASE_WIDTH (490.0f)
//轮子宽度（单位：mm）
#define WHEEL_WIDTH (40.0f)
//两个轮子中心距离（单位：mm）
#define WHEEL_TREAD (434.0f)
//圆周率
#define PI (3.14f)
//定义转弯方向
typedef enum {RIGHT,LEFT}direction;

/**
  * @}
  */



/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * [MoveBaseInit移动系统电机初始化]
 */
void MoveBaseInit(void);

/**GoStraight 让小车走直线
*  speed 为小车走直线的速度(单位mm/s);
*/
void GoStraight(float speed);

/**MakeCircle 让小车转圈
*  speed 为小车中心线速度(单位mm/s);
*  radius 为小车中心转弯半径(mm)
*  direction 为小车转动的方向,其值可以为RIGHT,LEFT
*/
void MakeCircle(float speed, float radius, direction drt);

/**WheelSpeed 让单个轮子转动
*  speed 为轮子转动速度(单位mm/s);
*  ElmoNum 为所选轮子编号
*/
void WheelSpeed(float speed, uint8_t ElmoNum);



#endif /* ___H */



/************************ (C) COPYRIGHT NEU_ACTION_2017 *****END OF FILE****/

