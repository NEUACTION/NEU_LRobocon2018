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

/**
  * @brief  
  * @note
  * @param  
  * @retval None
  */

void MoveBaseInit(void)
{
    CAN_Config(CAN1, 500, GPIOB, GPIO_Pin_8, GPIO_Pin_9);
    CAN_Config(CAN2, 500, GPIOB, GPIO_Pin_5, GPIO_Pin_6);
    VelLoopCfg(CAN2, 1, 10000000, 10000000);
    VelLoopCfg(CAN2, 2, 10000000, 10000000);
    ElmoInit(CAN2);
    MotorOn(CAN2, 1);
    MotorOn(CAN2, 2);
}

void GoStraight(float speed)
{
    WheelSpeed(speed, 1);
    WheelSpeed(speed, 2);
}    

void MakeCircle(float speed, float radius, direction drt)
{
    if(drt == RIGHT)
    {
        WheelSpeed(((speed / radius) * (radius - (WHEEL_TREAD / 2))), 1);
        WheelSpeed(((speed / radius) * (radius + (WHEEL_TREAD / 2))), 2);
    }
    else
    {
        WheelSpeed(-1 * ((speed / radius) * (radius - (WHEEL_TREAD / 2))), 1);
        WheelSpeed(-1 * ((speed / radius) * (radius + (WHEEL_TREAD / 2))), 2);
    }
}

void WheelSpeed(float speed, uint8_t ElmoNum)
{
    int32_t frequency = 0;
    frequency = (int32_t)((COUNTS_PER_ROUND * speed)/(PI * WHEEL_DIAMETER));
    if(ElmoNum == 1)
    {
        //VelCrl(CAN2, ElmoNum, frequency);
    }
    else if(ElmoNum == 2)
    {
        //VelCrl(CAN2, ElmoNum, -1 * frequency);
    }
}

/********************* (C) COPYRIGHT NEU_ACTION_2018 ****************END OF FILE************************/
