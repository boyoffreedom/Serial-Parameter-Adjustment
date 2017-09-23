
#ifndef __SPC_H__
#define __SPC_H__

#define RECI_PARA 0x0001		//定义事件
#define RETURN_PARA 0x0002

extern int DEBUG_INS[3];	 //上位机指令数组:DEBUG_INS[0]选择操作的参数
				 // 		  DEBUG_INS[1]上位机参数高8位
				 //		  DEBUG_INS[2]上位机参数低8位
extern unsigned int S_RECI_EVENT;//串口接收事件标志位
extern float P_ctrl[3],I_ctrl[3];//控制变量（可定制）
extern unsigned int S_SEND_CNT;

extern void Event_Process(void);//事件处理机，放主函数内循环调用即可
extern void Return_Para(void);//参数返回函数，用于验证参数是否返回成功，或读取当前参数
extern void Send_Matlab(int mode,int *ch);		//matlab示波器接口函数,ch为9个元素的数组

#endif
