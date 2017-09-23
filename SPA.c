

int DEBUG_INS[3];			//上位机调试指令组
unsigned int S_RECI_EVENT = 0;		//事件组
float P_ctrl[3],I_ctrl[3];		//可调范围0-2，精度16位，可以自己调整
unsigned int S_SEND_CNT = 0;		//MATLAB_SCOP发送计数器

void Event_Process(void)
{
	if(S_RECI_EVENT != 0)			//有事件未处理
	{
		if(S_RECI_EVENT & RECI_PARA)	//判断事件
		{
			unsigned int tmp = 0;
			tmp = (DEBUG_INS[1]<<8)+DEBUG_INS[2];	//取成16位无符号整型
			switch(DEBUG_INS[0])			//第一个字节为指令功能字节，例如0x01为发送MATLAB_SCOP，第二字节和第三字节为发送次数。
			{
			case 0x01:S_SEND_CNT = tmp;break;
			case 0x02:P_ctrl[0] = ((float)tmp)/32768;break;	//0-2可调范围16位精度，上位机下位机精度与范围必须一致。下位机接收到数据后
			case 0x03:I_ctrl[0] = ((float)tmp)/32768;break;//再进行浮点数据转换处理
			case 0x04:P_ctrl[1] = ((float)tmp)/32768;break;
			case 0x05:I_ctrl[1] = ((float)tmp)/32768;break;
			case 0x06:P_ctrl[2] = ((float)tmp)/32768;break;
			case 0x07:I_ctrl[2] = ((float)tmp)/32768;break;
			case 0x08:S_RECI_EVENT |= RETURN_PARA;break;	//function instrument将下位机可调参数返回给上位机
			default:break;
			}
			S_RECI_EVENT ^= RECI_PARA;		//清除当前事件标志位
		}
		if(S_RECI_EVENT &= RETURN_PARA)
		{
			Return_Para();				//返回当前事件
			S_RECI_EVENT ^= RETURN_PARA;
		}
		else						//事件错误，清空标志位
		{
			S_RECI_EVENT = 0;
		}
	}
}
//使用串口将部分数据返回给上位机进行显示，MATLAB程序详见Matlab_Serial_Scop项目
//发送数据结构
void Send_Matlab(int mode,int *ch)
{
	unsigned int send_data[21]={0};
    int i;
    send_data[ 0] = 0xff;
    send_data[ 1] = 0xa5;
    send_data[ 2] = 0x5a+mode;
    send_data[ 3] = (unsigned int)((ch[0]&0xff00)>>8);
    send_data[ 4] = (unsigned int) (ch[0]&0x00ff);
    send_data[ 5] = (unsigned int)((ch[1]&0xff00)>>8);
    send_data[ 6] = (unsigned int) (ch[1]&0x00ff);
    send_data[ 7] = (unsigned int)((ch[2]&0xff00)>>8);
    send_data[ 8] = (unsigned int) (ch[2]&0x00ff);
    send_data[ 9] = (unsigned int)((ch[3]&0xff00)>>8);
    send_data[10] = (unsigned int) (ch[3]&0x00ff);
    send_data[11] = (unsigned int)((ch[4]&0xff00)>>8);
    send_data[12] = (unsigned int) (ch[4]&0x00ff);
    send_data[13] = (unsigned int)((ch[5]&0xff00)>>8);
    send_data[14] = (unsigned int) (ch[5]&0x00ff);
    send_data[15] = (unsigned int)((ch[6]&0xff00)>>8);
    send_data[16] = (unsigned int) (ch[6]&0x00ff);
    send_data[17] = (unsigned int)((ch[7]&0xff00)>>8);
    send_data[18] = (unsigned int) (ch[7]&0x00ff);
    send_data[19] = (unsigned int)((ch[8]&0xff00)>>8);
    send_data[20] = (unsigned int) (ch[8]&0x00ff);
    for(i=0;i<21;i++)
    {
    	Send_Char(send_data[i]);
    }
}
//上位机参数返回函数
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
	unsigned int Sci_Rx_Buf;
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
		start_flag = 0;			//清空状态标志
	}
	else
	{
		i = 0;
		start_flag = 0;
	}
	//记得清除中断标志位，不同芯片的标志位不同，在这就不迷惑大家了
}
