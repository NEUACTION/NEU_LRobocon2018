#ifndef POS_H
#define POS_H

#include "math.h"

//位置,成员为x,y,角度
typedef struct
{
    float posX;
    float posY;
    float angle;
}Pos;

//速度,成员为x,y角速度
typedef struct
{
    float speedX;
    float speedY;
    float angle;
}speed;

//位置,用直角坐标和极坐标两种表示方法
typedef struct
{
    float x;
    float y;
    float a;
    float r;
}point;

//直线方向枚举类型,可以为前或后
typedef enum
{
    forward = 1,
    backward = 0,
}dir;

//直线解析式,成员aX+bY+c=0的三个参数
typedef struct
{
    float a;
    float b;
    float c;
    dir linedir;
}linewithdir;


//相对位置枚举类型,可以为左和右
typedef enum
{
    right,
    left,
    ontheline,
}reldir;

//获取当前位置
float GetX(void);
float GetY(void);
float GetA(void);

//获取当前位置
float GetSpeedX(void);
float GetSpeedY(void);
float GetSpeedA(void);

//返回点距离特定直线的距离
float Point2Line(linewithdir thisline, const point thispoint);

/**计算直线方向问题
*  返回值为-180到180间的一个数,与小车定位器的返回值所取坐标系相同(y轴正向为0度)
*/
float LineDir(const linewithdir thisline);

//设置一个点的位置,通过直角坐标或极坐标
point setPointXY(float x, float y);
point setPointRA(float r, float a);

//获取当前车的绝对位置;
point GetNowPoint(void);

//返回点与目标直线的相对位置,返回左右
reldir RelDir2Line(const linewithdir thisline, const point thispoint);

//返回点之间的距离
float Point2Point(const point thispoint, const point thatpoint);

//返回thispoint指向thatpoint的带方向的直线
linewithdir DirlinePoint2Point(const point thispoint, const point thatpoint);

//返回thatpoint相对于thispoint的相对位置
point RelPos(const point thispoint, const point thatpoint);

//返回有向直线thisline的thisdir侧的垂直于其的方向
float VDirForLine(const linewithdir thisline, reldir thisdir);

#endif
