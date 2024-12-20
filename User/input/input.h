#ifndef __INPUT_H
#define __INPUT_H

typedef enum{
    Input_Type_None = 0,
    Input_Type_Button,
    Input_Type_Up,
    Input_Type_Down,
    Input_Type_Left,
    Input_Type_Right,
    Input_Type_Clockwise,
    Input_Type_AntiClockwise
} Input_Type;

void Input_Init(void);
Input_Type Input_Pop(void);
void Button_HandleInt(void);

#endif
