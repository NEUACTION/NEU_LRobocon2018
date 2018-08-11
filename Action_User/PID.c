#include "PID.h"


/**
 * [PIDCtrl 用来进行PID控制]
 * @param PIDStructure [用来传入PID控制一些关键变量的结构体]
 */
void PIDCtrl(PIDCtrlStructure * PIDStructure)
{
    err = (*PIDStructure).ExOut - ((*PIDStructure).GetVar)();
    iSum += err;
    dDiff = err - lasterr;
    lasterr = err;
    UOut = (*PIDStructure).KP * err;
    UOut += (*PIDStructure).KI * iSum;
    UOut += (*PIDStructure).KD * dDiff;
    (*(*PIDStructure).Ctrl)(UOut);
}

void PIDCtrlInit(void)
{
    UOut = 0;
    err = 0;
    iSum = 0;
    lasterr = 0;
    dDiff = 0;
}
