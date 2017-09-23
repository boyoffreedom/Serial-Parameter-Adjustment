

int DEBUG_INS[3];
unsigned int S_RECI_EVENT = 0;
float P_ctrl[3],I_ctrl[3];

void Event_Process(void)
{
	if(S_RECI_EVENT != 0)
	{
		if(S_RECI_EVENT & RECI_PARA)	//判断事件
		{
			unsigned int tmp = 0;
			tmp = (DEBUG_INS[1]<<8)+DEBUG_INS[2];
			switch(DEBUG_INS[0])
			{
			case 0x01:S_SEND_CNT = tmp;break;
			case 0x02:P_ctrl[0] = ((float)tmp)/32768;break;
			case 0x03:I_ctrl[0] = ((float)tmp)/32768;break;
			case 0x04:P_ctrl[1] = ((float)tmp)/32768;break;
			case 0x05:I_ctrl[1] = ((float)tmp)/32768;break;
			case 0x06:P_ctrl[2] = ((float)tmp)/32768;break;
			case 0x07:I_ctrl[2] = ((float)tmp)/32768;break;
			case 0x08:S_RECI_EVENT |= RETURN_PARA;break;
			default:break;
			}
			S_RECI_EVENT ^= RECI_PARA;		//清除当前事件标志位
		}
		if(S_RECI_EVENT &= RETURN_PARA)
		{
			Return_Para();				//返回当前事件
			S_RECI_EVENT ^= RETURN_PARA;
		}
		else
		{
			S_RECI_EVENT = 0;
		}
	}
}

void Return_Para(void)
{
	int i;
	int para[12];
	unsigned int tmp[6];
	tmp[0] = (unsigned int)(P_ctrl[0]*32768);
	tmp[1] = (unsigned int)(I_ctrl[0]*32768);
	tmp[2] = (unsigned int)(P_ctrl[1]*32768);
	tmp[3] = (unsigned int)(I_ctrl[1]*32768);
	tmp[4] = (unsigned int)(P_ctrl[2]*32768);
	tmp[5] = (unsigned int)(I_ctrl[2]*32768);

	para[0]  = (unsigned int)((tmp[0]&0xff00)>>8);
	para[1]  = (unsigned int)(tmp[0]&0x00ff);
	para[2]  = (unsigned int)((tmp[1]&0xff00)>>8);
	para[3]  = (unsigned int)(tmp[1]&0x00ff);

	para[4]  = (unsigned int)((tmp[2]&0xff00)>>8);
	para[5]  = (unsigned int)(tmp[2]&0x00ff);
	para[6]  = (unsigned int)((tmp[3]&0xff00)>>8);
	para[7]  = (unsigned int)(tmp[3]&0x00ff);

	para[8]  = (unsigned int)((tmp[4]&0xff00)>>8);
	para[9]  = (unsigned int)(tmp[4]&0x00ff);
	para[10] = (unsigned int)((tmp[5]&0xff00)>>8);
	para[11] = (unsigned int)(tmp[5]&0x00ff);

	Send_Char(0xaa);
	Send_Char(0x55);
	for(i = 0; i < 12; i++)
	{
		Send_Char(para[i]);		//根据芯片型号定义Send_Char函数，通过串口发送一个字节数据
	}
	Send_Char(0xff);
}

//中断函数 需根据芯片寄存器更改部分参数
interrupt void SCIRXINTB_ISR(void)     // SCI-B
{
	static unsigned int Sci_Rx_Buf = 0;
	static unsigned int i = 0;
	static unsigned int  start_flag = 0;

//通信帧内容：0xaa 0xab 起始帧   可自己定义，但相应的也要MATLAB上位机发送帧格式
//0x?? 0x?? 0x?? 3位数据内容帧
//0xff 结尾帧	可自己定义，但相应的也要MATLAB上位机发送帧格式
//如果格式不对，丢弃一帧数据
	Sci_Rx_Buf = ScibRegs.SCIRXBUF.all; //接收数据

	if(start_flag == 0 && Sci_Rx_Buf == 0xaa)	//起始帧第一位 0xaa
	{
		start_flag = 1;
	}
	else if(start_flag == 1 && Sci_Rx_Buf == 0xab)	//起始帧第二位 0xab 接收就绪
	{
		start_flag = 2;
		i = 0;
	}
	else if(start_flag == 2 && i < 3)		//三位数据位 数据入栈
	{
		DEBUG_INS[i++] = Sci_Rx_Buf;
	}
	else if(i == 3 && Sci_Rx_Buf == 0xff)	//第六位 结束位0xff
	{
		S_RECI_EVENT |= RECI_PARA;	//生成事件
		start_flag = 0;
	}
	else
	{
		i = 0;
		start_flag = 0;
	}

    PieCtrlRegs.PIEACK.all=0x0100;  //使得同组其他中断能够得到响应
    EINT;  //开全局中断vcc
}
