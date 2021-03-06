#include "my_can.h"
#include "delay.h"
//#include "usart.h"
#include "canopen.h"

BaseHalSt gHalData;

#if CAN1_RX0_INT_ENABLE	//使能RX0中断

//************************************
// 函数:    CAN1_RX0_IRQHandler
// 返回值:   void
// 描述:can接收到数据包触发接收中断
// 参数列表: void
//作者：LIN.HRG
//************************************
void CAN1_RX0_IRQHandler(void)
{
	unsigned char rxbuf[8];
	unsigned int id;
	unsigned char ide, rtr, len;
	CAN1_Rx_Msg(0, &id, &ide, &rtr, &len, rxbuf);
	if ((len == 8) && (rxbuf[1] == MOTORDRIVE_CMD_GET_ACT_VELOCITY))
	{
		gHalData.WheelHal[id - 1].FbVel = (double)((int)(((uint32_t)rxbuf[5]) + ((uint32_t)rxbuf[4] << 8) + ((uint32_t)rxbuf[3] << 16) + ((uint32_t)rxbuf[2] << 24))) / 10.0f;
	}

}
#endif

/*
函数功能：设定电机转速rpm
参数    ：id  设备节点
		  spd 速度值
*/
unsigned char MOTECSetMotorSpd(unsigned char addr, int spd)//spd:rpm
{
	unsigned char buff[8];
	unsigned short check;
	spd = spd * 10;
	buff[0] = addr;
	buff[1] = (unsigned char)(MOTORDRIVE_CMD_SET_VELOCITY & 0xff);
	buff[2] = (unsigned char)((spd & 0xff000000) >> 24);
	buff[3] = (unsigned char)((spd & 0x00ff0000) >> 16);
	buff[4] = (unsigned char)((spd & 0x0000ff00) >> 8);
	buff[5] = (unsigned char)(spd & 0x000000ff);
	check = ((((unsigned short)buff[0] << 8) & 0xff00) | (((unsigned short)buff[1]) & 0x00ff))
		^ ((((unsigned short)buff[2] << 8) & 0xff00) | (((unsigned short)buff[3]) & 0x00ff))
		^ ((((unsigned short)buff[4] << 8) & 0xff00) | (((unsigned short)buff[5]) & 0x00ff));
	buff[6] = (unsigned char)((check & 0xff00) >> 8);
	buff[7] = (unsigned char)(check & 0x00ff);
	CAN1_Send_Msg(buff, SPD_SET_LEN, addr);
	return 0;
}

//************************************
// 函数:    syntronMotorVeloSet
// 返回值:   void
// 描述:
// 参数列表: u8 adr驱动器节点地址
// 参数列表: int spd驱动器转速（RPM）
//作者：LIN.HRG
//************************************
void syntronMotorVeloSet(u8 adr, int spd)
{
	CMD_SEND_DRIVER_UNION_TYPE cmd;
	u16 addr = adr + 0x100;
	cmd.cmdSrtuct.srcID = adr << 4;
    cmd.cmdSrtuct.reg = 0;
	cmd.cmdSrtuct.funcCode = _FUN_CODE_WR_CMD_BYTE;
	cmd.cmdSrtuct.cmdL = (spd<<8) | (spd>>8);
    cmd.cmdSrtuct.cmdH = 0;
	CAN1_Send_Msg(cmd.cmdByte, 8 , addr);
}

/*
函数功能：设定motec 参数表参数
参数    ：id  设备节点

*/
unsigned char set_param_motec(unsigned char addr, unsigned short param_num, unsigned short param_value)
{
	unsigned char buff[8];
	unsigned short check;

	buff[0] = addr;
	buff[1] = (unsigned char)(MOTORDRIVE_CMD_SET_PARAM & 0xff);
	buff[2] = (unsigned char)((param_num & 0xff00) >> 8);
	buff[3] = (unsigned char)(param_num & 0x00ff);
	buff[4] = (unsigned char)((param_value & 0xff00) >> 8);
	buff[5] = (unsigned char)(param_value & 0x00ff);
	check = ((((unsigned short)buff[0] << 8) & 0xff00) | (((unsigned short)buff[1]) & 0x00ff))
		^ ((((unsigned short)buff[2] << 8) & 0xff00) | (((unsigned short)buff[3]) & 0x00ff))
		^ ((((unsigned short)buff[4] << 8) & 0xff00) | (((unsigned short)buff[5]) & 0x00ff));
	buff[6] = (unsigned char)((check & 0xff00) >> 8);
	buff[7] = (unsigned char)(check & 0x00ff);
	CAN1_Send_Msg(buff, SPD_SET_LEN, addr);
	return 0;
}

/*
函数功能：设定motec 参数表参数
参数    ：id  设备节点

*/
unsigned char send_cmd_motordrive(unsigned char addr, unsigned char cmd_id, unsigned short sdata1, unsigned short sdata2)
{
	unsigned char buff[8];
	unsigned short check;

	buff[0] = addr;
	buff[1] = cmd_id;
	buff[2] = (unsigned char)((sdata1 & 0xff00) >> 8);
	buff[3] = (unsigned char)(sdata1 & 0x00ff);
	buff[4] = (unsigned char)((sdata2 & 0xff00) >> 8);
	buff[5] = (unsigned char)(sdata2 & 0x00ff);
	check = ((((unsigned short)buff[0] << 8) & 0xff00) | (((unsigned short)buff[1]) & 0x00ff))
		^ ((((unsigned short)buff[2] << 8) & 0xff00) | (((unsigned short)buff[3]) & 0x00ff))
		^ ((((unsigned short)buff[4] << 8) & 0xff00) | (((unsigned short)buff[5]) & 0x00ff));
	buff[6] = (unsigned char)((check & 0xff00) >> 8);
	buff[7] = (unsigned char)(check & 0x00ff);
	CAN1_Send_Msg(buff, SPD_SET_LEN, addr);
	return 0;
}

/*
函数功能：获取电机转速rpm
参数    ：id  设备节点
*/


unsigned char getMotorSpeed_MOTEC(unsigned char addr)//spd:rpm
{
	unsigned char buff[8];
	unsigned short check;
	buff[0] = addr;
	buff[1] = (unsigned char)(MOTORDRIVE_CMD_GET_ACT_VELOCITY & 0xff);
	buff[2] = (unsigned char)(0x00);
	buff[3] = (unsigned char)(0x00);
	buff[4] = (unsigned char)(0x00);
	buff[5] = (unsigned char)(0x00);
	check = (((unsigned short)buff[0] << 8) & 0xff00) | (((unsigned short)buff[1]) & 0x00ff)
		^ (((unsigned short)buff[2] << 8) & 0xff00) | (((unsigned short)buff[3]) & 0x00ff)
		^ (((unsigned short)buff[4] << 8) & 0xff00) | (((unsigned short)buff[5]) & 0x00ff);
	buff[6] = (unsigned char)((check & 0xff00) >> 8);
	buff[7] = (unsigned char)(check & 0x00ff);
	CAN1_Send_Msg(buff, SPD_SET_LEN, addr);
	return 0;
}

//************************************
// 函数:    getMotorVelo
// 返回值:   void
// 描述:获取相应节点驱动器当前转速
// 参数列表: u8 adr 节点地址
//作者：LIN.HRG
//************************************
void getMotorVelo(u8 adr)
{
	CMD_SEND_DRIVER_UNION_TYPE cmd;
	u16 addr = adr + 0x100;
	cmd.cmdSrtuct.funcCode = _FUN_CODE_RD_STATUS_BYTE;
	cmd.cmdSrtuct.srcID = adr << 4;
    cmd.cmdSrtuct.reg = 0x00;
	cmd.cmdSrtuct.cmdL = 0x00;
	cmd.cmdSrtuct.cmdH = 0x00;
	CAN1_Send_Msg(cmd.cmdByte, 8, addr);
}

unsigned char EnableMotorDrive(unsigned char addr)//spd:rpm
{
	unsigned char buff[8];
	unsigned short check;
	buff[0] = addr;
	buff[1] = (unsigned char)(MOTORDRIVE_CMD_ENABLE & 0xff);
	buff[2] = (unsigned char)(0x00);
	buff[3] = (unsigned char)(0x00);
	buff[4] = (unsigned char)(0x00);
	buff[5] = (unsigned char)(0x00);
	check = (((unsigned short)buff[0] << 8) & 0xff00) | (((unsigned short)buff[1]) & 0x00ff)
		^ (((unsigned short)buff[2] << 8) & 0xff00) | (((unsigned short)buff[3]) & 0x00ff)
		^ (((unsigned short)buff[4] << 8) & 0xff00) | (((unsigned short)buff[5]) & 0x00ff);
	buff[6] = (unsigned char)((check & 0xff00) >> 8);
	buff[7] = (unsigned char)(check & 0x00ff);
	CAN1_Send_Msg(buff, SPD_SET_LEN, addr);
	return 0;
}
/*
函数功能：控制左右轮速度
参数    ：speedLeft  左轮速度值
					speedRight 右轮速度值
*/
unsigned char wheelMotorSpdSet(int speedLeft, int speedRight)
{
	delay_us(100);
	MOTECSetMotorSpd(MOTORDRIVE_ID_LEFT, speedLeft);
	delay_us(100);
	//	delay_ms(1);
	MOTECSetMotorSpd(MOTORDRIVE_ID_RIGHT, speedRight);
	//	delay_ms(1);	
	delay_us(100);
	return 0;
}
/*
函数功能：获取左右轮速度
参数    ：无
*/
unsigned char getWheelMotorSpeed2Buf(void)
{
	getMotorSpeed_MOTEC(MOTORDRIVE_ID_LEFT);
	//	delay_ms(1);
	delay_us(100);
	getMotorSpeed_MOTEC(MOTORDRIVE_ID_RIGHT);
	//	delay_ms(1);	
	delay_us(100);
	return 0;
}
/*
函数功能：CAN初始化
参数    ：tsjw:重新同步跳跃时间单元.范围:1~3;
					tbs2:时间段2的时间单元.范围:1~8;
					tbs1:时间段1的时间单元.范围:1~16;
					brp :波特率分频器.范围:1~1024;(实际要加1,也就是1~1024) tq=(brp)*tpclk1
					注意以上参数任何一个都不能设为0,否则会乱.
					波特率=Fpclk1/((tbs1+tbs2+1)*brp);
					mode:0,普通模式;1,回环模式;
					Fpclk1的时钟在初始化的时候设置为42M,如果设置CAN1_Mode_Init(1,6,7,6,1);
					则波特率为:42M/((6+7+1)*6)=500Kbps
返回值  : 0   ,初始化OK;
				  其他,初始化失败;
*/

u8 can1ModeConfig(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode)
{
	u16 i = 0;
	if (tsjw == 0 || tbs2 == 0 || tbs1 == 0 || brp == 0)return 1;
	tsjw -= 1;//先减去1.再用于设置
	tbs2 -= 1;
	tbs1 -= 1;
	brp -= 1;

	RCC->AHB1ENR |= 1 << 0;  	//使能PORTA口时钟 
	GPIO_Set(GPIOA, PIN11 | PIN12, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_50M, GPIO_PUPD_PU);//PA11,PA12,复用功能,上拉输出
	GPIO_AF_Set(GPIOA, 11, 9);//PA11,AF9
	GPIO_AF_Set(GPIOA, 12, 9);//PA12,AF9 	   

	RCC->APB1ENR |= 1 << 25;//使能CAN1时钟 CAN1使用的是APB1的时钟(max:42M)
	CAN1->MCR = 0x0000;	//退出睡眠模式(同时设置所有位为0)
	CAN1->MCR |= 1 << 0;		//请求CAN进入初始化模式
	while ((CAN1->MSR & 1 << 0) == 0)
	{
		i++;
		if (i > 100)return 2;//进入初始化模式失败
	}
	CAN1->MCR |= 0 << 7;		//非时间触发通信模式
	CAN1->MCR |= 0 << 6;		//软件自动离线管理
	CAN1->MCR |= 0 << 5;		//睡眠模式通过软件唤醒(清除CAN1->MCR的SLEEP位)
	CAN1->MCR |= 1 << 4;		//禁止报文自动传送
	CAN1->MCR |= 0 << 3;		//报文不锁定,新的覆盖旧的
	CAN1->MCR |= 0 << 2;		//优先级由报文标识符决定
	CAN1->BTR = 0x00000000;	//清除原来的设置.
	CAN1->BTR |= mode << 30;	//模式设置 0,普通模式;1,回环模式;
	CAN1->BTR |= tsjw << 24; 	//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位
	CAN1->BTR |= tbs2 << 20; 	//Tbs2=tbs2+1个时间单位
	CAN1->BTR |= tbs1 << 16;	//Tbs1=tbs1+1个时间单位
	CAN1->BTR |= brp << 0;  	//分频系数(Fdiv)为brp+1
							//波特率:Fpclk1/((Tbs1+Tbs2+1)*Fdiv)
	CAN1->MCR &= ~(1 << 0);		//请求CAN退出初始化模式
	while ((CAN1->MSR & 1 << 0) == 1)
	{
		i++;
		if (i > 0XFFF0)return 3;//退出初始化模式失败
	}
	//过滤器初始化
	CAN1->FMR |= 1 << 0;		//过滤器组工作在初始化模式
	CAN1->FA1R &= ~(1 << 0);	//过滤器0不激活
	CAN1->FS1R |= 1 << 0; 		//过滤器位宽为32位.
	CAN1->FM1R |= 0 << 0;		//过滤器0工作在标识符屏蔽位模式
	CAN1->FFA1R |= 0 << 0;		//过滤器0关联到FIFO0
	CAN1->sFilterRegister[0].FR1 = 0X00000000;//32位ID
	CAN1->sFilterRegister[0].FR2 = 0X00000000;//32位MASK
	CAN1->FA1R |= 1 << 0;		//激活过滤器0
	CAN1->FMR &= 0 << 0;		//过滤器组进入正常模式

#if CAN1_RX0_INT_ENABLE
	//使用中断接收
	CAN1->IER |= 1 << 1;		//FIFO0消息挂号中断允许.	    
	MY_NVIC_Init(1, 0, CAN1_RX0_IRQn, 4);//组2
#endif
	return 0;
}

unsigned char MOTECCanSend(unsigned char len, unsigned char addr, unsigned short cmdfunc, unsigned char *data)
{
	unsigned char buff[8];
	unsigned int id = (unsigned int)addr;
	buff[0] = len;
	buff[1] = addr;
	buff[2] = (unsigned char)(cmdfunc & 0x00ff);
	buff[3] = (unsigned char)((cmdfunc & 0xff00) >> 8);
	buff[4] = data[0];
	buff[5] = data[1];
	buff[6] = data[2];
	buff[7] = data[3];
	CAN1_Send_Msg(buff, len, id);
	return 0;
}
//id:标准ID(11位)/扩展ID(11位+18位)	    
//ide:0,标准帧;1,扩展帧
//rtr:0,数据帧;1,远程帧
//len:要发送的数据长度(固定为8个字节,在时间触发模式下,有效数据为6个字节)
//*dat:数据指针.
//返回值:0~3,邮箱编号.0XFF,无有效邮箱.
u8 CAN1_Tx_Msg(u32 id, u8 ide, u8 rtr, u8 len, u8 *dat)
{
	u8 mbox;
	if (CAN1->TSR&(1 << 26))mbox = 0;			//邮箱0为空
	else if (CAN1->TSR&(1 << 27))mbox = 1;	//邮箱1为空
	else if (CAN1->TSR&(1 << 28))mbox = 2;	//邮箱2为空
	else return 0XFF;					//无空邮箱,无法发送 
	CAN1->sTxMailBox[mbox].TIR = 0;		//清除之前的设置
	if (ide == 0)	//标准帧
	{
		id &= 0x7ff;//取低11位stdid
		id <<= 21;
	}
	else		//扩展帧
	{
		id &= 0X1FFFFFFF;//取低32位extid
		id <<= 3;
	}
	CAN1->sTxMailBox[mbox].TIR |= id;
	CAN1->sTxMailBox[mbox].TIR |= ide << 2;
	CAN1->sTxMailBox[mbox].TIR |= rtr << 1;
	len &= 0X0F;//得到低四位
	CAN1->sTxMailBox[mbox].TDTR &= ~(0X0000000F);
	CAN1->sTxMailBox[mbox].TDTR |= len;		   //设置DLC.
	//待发送数据存入邮箱.
	CAN1->sTxMailBox[mbox].TDHR = (((u32)dat[7] << 24) |
		((u32)dat[6] << 16) |
		((u32)dat[5] << 8) |
		((u32)dat[4]));
	CAN1->sTxMailBox[mbox].TDLR = (((u32)dat[3] << 24) |
		((u32)dat[2] << 16) |
		((u32)dat[1] << 8) |
		((u32)dat[0]));
	CAN1->sTxMailBox[mbox].TIR |= 1 << 0; //请求发送邮箱数据
	return mbox;
}
//获得发送状态.
//mbox:邮箱编号;
//返回值:发送状态. 0,挂起;0X05,发送失败;0X07,发送成功.
u8 CAN1_Tx_Staus(u8 mbox)
{
	u8 sta = 0;
	switch (mbox)
	{
	case 0:
		sta |= CAN1->TSR&(1 << 0);			//RQCP0
		sta |= CAN1->TSR&(1 << 1);			//TXOK0
		sta |= ((CAN1->TSR&(1 << 26)) >> 24);	//TME0
		break;
	case 1:
		sta |= CAN1->TSR&(1 << 8) >> 8;		//RQCP1
		sta |= CAN1->TSR&(1 << 9) >> 8;		//TXOK1
		sta |= ((CAN1->TSR&(1 << 27)) >> 25);	//TME1	   
		break;
	case 2:
		sta |= CAN1->TSR&(1 << 16) >> 16;	//RQCP2
		sta |= CAN1->TSR&(1 << 17) >> 16;	//TXOK2
		sta |= ((CAN1->TSR&(1 << 28)) >> 26);	//TME2
		break;
	default:
		sta = 0X05;//邮箱号不对,肯定失败.
		break;
	}
	return sta;
}
//接收数据
//fifox:邮箱号
//id:标准ID(11位)/扩展ID(11位+18位)	    
//ide:0,标准帧;1,扩展帧
//rtr:0,数据帧;1,远程帧
//len:接收到的数据长度(固定为8个字节,在时间触发模式下,有效数据为6个字节)
//dat:数据缓存区
void CAN1_Rx_Msg(unsigned char fifox, unsigned int *id, unsigned char *ide, unsigned char *rtr, unsigned char *len, unsigned char *dat)
{
	*ide = CAN1->sFIFOMailBox[fifox].RIR & 0x04;//得到标识符选择位的值  
	if (*ide == 0)//标准标识符
	{
		*id = CAN1->sFIFOMailBox[fifox].RIR >> 21;
	}
	else	   //扩展标识符
	{
		*id = CAN1->sFIFOMailBox[fifox].RIR >> 3;
	}
	*rtr = CAN1->sFIFOMailBox[fifox].RIR & 0x02;	//得到远程发送请求值.
	*len = CAN1->sFIFOMailBox[fifox].RDTR & 0x0F;//得到DLC
	//*fmi=(CAN1->sFIFOMailBox[FIFONumber].RDTR>>8)&0xFF;//得到FMI
	//接收数据
	dat[0] = CAN1->sFIFOMailBox[fifox].RDLR & 0XFF;
	dat[1] = (CAN1->sFIFOMailBox[fifox].RDLR >> 8) & 0XFF;
	dat[2] = (CAN1->sFIFOMailBox[fifox].RDLR >> 16) & 0XFF;
	dat[3] = (CAN1->sFIFOMailBox[fifox].RDLR >> 24) & 0XFF;
	dat[4] = CAN1->sFIFOMailBox[fifox].RDHR & 0XFF;
	dat[5] = (CAN1->sFIFOMailBox[fifox].RDHR >> 8) & 0XFF;
	dat[6] = (CAN1->sFIFOMailBox[fifox].RDHR >> 16) & 0XFF;
	dat[7] = (CAN1->sFIFOMailBox[fifox].RDHR >> 24) & 0XFF;
	if (fifox == 0)	CAN1->RF0R |= 0X20;//释放FIFO0邮箱
	else if (fifox == 1)	CAN1->RF1R |= 0X20;//释放FIFO1邮箱	 
}

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
unsigned char CAN1_Send_Msg(unsigned char * msg, unsigned char len, unsigned int id)
{
	unsigned char mbox;
	unsigned short int i = 0;
	mbox = CAN1_Tx_Msg(id, 0, 0, len, msg);
	while ((CAN1_Tx_Staus(mbox) != 0X07) && (i < 0XFFF))i++;//等待发送结束
	if (i >= 0XFFF)return 1;							//发送失败?
	return 0;										//发送成功;
}
//can口接收数据查询
//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//		 其他,接收的数据长度;
