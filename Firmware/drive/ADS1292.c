/********************************************************************
 nef52840如果想要使用spi功能，需要添加驱动文件如下
 
	nrf_drv_spi.c ..\integration\nrfx\legacy 		旧版本 SPI 驱动文件。
	nrfx_spi.c 		..\modules\nrfx\drivers\src 	新版本 SPI 驱动文件。
	nrfx_spim.c 	..\modules\nrfx\drivers\src 	SPI 主机驱动文件。
		

	驱动ADS1292使用IO情况如下
	CS		:P0.23
	CLK		:P0.22
	MISO	:P0.21
	MOSI	:P0.20



********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_delay.h"


#include "ads1292.h"
#include "nrf_drv_spi.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */


static volatile bool spi_xfer_done;  //SPI数据传输完成标志
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static uint8_t    spi_tx_buf[256];   /**< TX buffer. */
static uint8_t    spi_rx_buf[256];   /**< RX buffer. */

//数组用于镜像寄存器数据
uint8_t			regData[24]; 				

int				boardStat;
int				boardChannelDataInt[5];
uint8_t			lead_off;

void ADS1292_gpio_config()
{
	//配置ADS1292的gpio初始化
	nrf_gpio_cfg_output(SPI_SS_PIN);
	nrf_gpio_cfg_output(EEG_START);
	nrf_gpio_cfg_output(EEG_PWRST);

	CS_H;			//置高CS
	PWRST_H;	//拉低PWRST
	START_L;	//拉低START
}

/*	SPI中断事件服务函数	*/
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
  //设置SPI传输完成  
	spi_xfer_done = true;
}



void SPI_User_init()
{
	/*	关于SPI的四种工作模式
		模式				描述
	模式0			CPOL=0			CPHA=0
	模式1			CPOL=0			CPHA=1
	模式2			CPOL=1			CPHA=0
	模式3			CPOL=1			CPHA=1	
	*/
	//配置定义spi初始化结构体 = 默认配置
	nrf_drv_spi_config_t	spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	//重写默认配置里的引脚配置
	spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;									//用户自定义，交给软件处理
	spi_config.mosi_pin = SPI_MOSI_PIN;
	spi_config.miso_pin =	SPI_MISO_PIN;
	spi_config.sck_pin = 	SPI_SCK_PIN;
	
	spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;		//中断优先级，这个在宏定义里面设置的是6
	spi_config.frequency = NRF_DRV_SPI_FREQ_8M;										//spi频率设置8M
	spi_config.orc = 0xFF;																				//ORC寄存器内容0xff
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;				//数据传输高位在前
	spi_config.mode = NRF_DRV_SPI_MODE_1;													//模式1
	
	  //初始化SPI
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));	
	
}

//SPI在指定地址上读取一个数据
uint8_t	ADS_xfer(uint8_t byte)
{
	/*
		nrf52840 SDK的读写是封装在同一个函数里的
	*/
	uint8_t d_read;						
	
	//8位地址
	spi_tx_buf[0] =	 byte;		

	//传输完成标志设置为false
	spi_xfer_done = false;
	
	//启动数据传输
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, 1, spi_rx_buf, 1));	
	
	//等待SPI传输完成
	while(!spi_xfer_done);
		
	d_read = spi_rx_buf[0];
	
//	//继续传输完成标志设置为false
//	spi_xfer_done = false;	
//	
//	//启动数据传输
//	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, 0, &d_read, 1));	
//	
//	//等待SPI传输完成
//	while(!spi_xfer_done)
//	{}
		
	//返回读取数值
	return	d_read;	
		
}



uint8_t	ADS_WREG(uint8_t	address,	uint8_t	cmd)
{
	uint8_t	Opcode1 = address + 0x40;						//计算写入寄存器地址
	CS_L;																				//拉低CS
	ADS_xfer(Opcode1);  												//第一个操作码：寄存器address
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(0x00);															//计算器数量 0x01 - 1 = 0x00
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(cmd); 															//写入寄存器值
	for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;																				//CS置高，使能总线
	return regData[address]; 
}




//ADS读取1个寄存器的值
uint8_t ADS_RREG(uint8_t address)
{
	uint8_t	Opcode1 = address + 0x20;						//计算读取寄存器地址
	CS_L;																				//拉低CS
	ADS_xfer(Opcode1);  												//第一个操作码：寄存器address
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(0x00);															//计算器数量 0x01 - 1 = 0x00
	for(int j=250;j>0;j--){};										//4 Tclk delay
	regData[address] = ADS_xfer(0x00); 					// update mirror location with returned byte
	for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;	

	return regData[address]; 										// return requested register value
}


//ADS复位
void	ADS_RESET(void)
{
	CS_L;
	ADS_xfer(_RESET);
	nrf_delay_ms(1);
	CS_H;	

}


//ADS停止连续读取模式
void ADS_SDATAC(void)
{
	CS_L;
  ADS_xfer(_SDATAC);
  for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;	
  nrf_delay_ms(1); 														//must wait at least 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}



//ADS获取芯片ID
uint8_t	ADS_getDeviceID(void)
{
  uint8_t data = ADS_RREG(ID_REG);
  return data;
}


//ADS开启信号
void  ADS_START(void)
{
	CS_L;
  ADS_xfer(_START); 												// KEEP ON-BOARD AND ON-DAISY IN SYNC
  for(int j=250;j>0;j--){};									//4 Tclk delay
	CS_H;
}



//ADS连续读取模式
void    ADS_RDATAC(void)
{
	CS_L;
  ADS_xfer(_RDATAC); 									// read data continuous
  for(int j=250;j>0;j--){};								//4 Tclk delay
	CS_H;
  nrf_delay_ms(1);
}


//ADS通道偏移校准
void    ADS_Offsetcal(void)
{
	CS_L;
  ADS_xfer(_OFFSETCAL); 								// read data continuous
  for(int j=250;j>0;j--){};								//4 Tclk delay
	CS_H;
  nrf_delay_ms(1);
}



/*		测试模式			*/
//内部噪声测试信号
void short_test(void)
{
	ADS_WREG(CONFIG1,0x01);//////////
	nrf_delay_ms(10);
	ADS_WREG(CONFIG2,0xA0);////////////////
	nrf_delay_ms(10);
	ADS_WREG(CH1SET,0x01);/////////////01
	nrf_delay_ms(10);
	ADS_WREG(CH2SET,0x01);/////////////
	nrf_delay_ms(10);


	ADS_WREG(RESP2,0x83);
	nrf_delay_ms(100);
	ADS_Offsetcal();
	nrf_delay_ms(100);

}
//内部1mv测试信号
void test_wave(void)
{
	ADS_WREG(CONFIG1,0x01);//////////
	nrf_delay_ms(10);
	ADS_WREG(CONFIG2,0xA3);////////////////
	nrf_delay_ms(10);
	ADS_WREG(CH1SET,0x05);/////////////
	nrf_delay_ms(10);
	ADS_WREG(CH2SET,0x05);/////////////
	nrf_delay_ms(10);

	ADS_WREG(RESP2,0x83);
	nrf_delay_ms(100);
	ADS_Offsetcal();
	nrf_delay_ms(100);
}
//采集EEG信号
void eeg_signal(void)
{
	/* lead-off + signal */
	ADS_WREG(CONFIG1,0x01);/*Continuous conversion mode；转换速率250SPS*/
	nrf_delay_ms(10);
	ADS_WREG(CONFIG2,0xE0);/*使用2.42 V内部参考电压*/
	nrf_delay_ms(100);
	ADS_WREG(LOFF,0x10);/*95% threshold；6nA*/
	nrf_delay_ms(100);
	ADS_WREG(CH1SET,0x00);/*放大倍数6倍*/
	nrf_delay_ms(100);
//	ADS_WREG(CH2SET,0x01);/*关闭通道2*/
	ADS_WREG(CH2SET,0x00);/*放大倍数6倍*/
	nrf_delay_ms(100);
	ADS_WREG(RLD_SENS,0x2F);////////////////
	nrf_delay_ms(10);
	ADS_WREG(LOFF_SENS,0x0F);////////////////
	nrf_delay_ms(100);
	ADS_WREG(RESP1,0x02);/* must write 02H*/
	nrf_delay_ms(100);
	ADS_WREG(RESP2,0x03);
	nrf_delay_ms(100);
	ADS_WREG(ADS_GPIO,0x0C);
	nrf_delay_ms(100);	
}


uint8_t device_id;
//ADS上电初始化
void initialize_ads(uint8_t mode)
{
//	uint8_t device_id;

	/* power up and reset */
//	PWRST_H;
//	nrf_delay_ms(1000);
//	PWRST_L;
//	for(int j=250;j>0;j--);
//	PWRST_H;
	nrf_delay_ms(200);
	ADS_RESET();
	nrf_delay_ms(10);
	/*stop continue collecting */
	ADS_SDATAC();
	nrf_delay_ms(10);
	/* read ADS1292 ID */
	device_id = ADS_getDeviceID();
	nrf_delay_ms(100);
//	ADS_WREG(LOFF_STAT,0x50);		/*bit6 配合使用外部晶振*/
//	nrf_delay_ms(100);	
	//reg setting 
	switch(mode)
	{
		case 0:short_test();break;
		case 1:test_wave();break;
		case 2:eeg_signal();break;
		default:;
	}
	//start collecting
//	START_H;
//	ADS_RDATAC();
//	nrf_delay_ms(10);
	ADS_START();
	nrf_delay_ms(40);
	ADS_RDATAC();
	nrf_delay_ms(10);

}


//单次读取一个ADS数据发送至主板
/*
SCLK 	_| |__| |__| |_          _| |__| |__| |_             _| |__| |__| |_


DOUT 		===|STAT|===						===|CH1|===										===|CH2|===

					24-Bit									24-Bit												24-Bit
DIN		--------------------------------------------------------------------------

STAT	:		1100 + LOFF_STAT[4:0]	+	GPIO[1:0]	+	13'0'S
*/
uint8_t  eCon_Message[15];
int byteCounter = 0;

void updateBoardData(void)
{
	uint8_t inByte;
	
	CS_L;
	
	for(int j=250;j>0;j--){};	
	
	//循环3次把数据存放到STAT内
	for (int i = 0; i < 3; i++)
	{
		inByte = ADS_xfer(0x00); 									//每次都只都0x01 一个数据  发送0x01 - 1 = 0x00；
		boardStat = (boardStat << 8) | inByte;
	}
	//判断
	if(((boardStat>>15)&0x01) == 0x01)
		lead_off=1;
	else
		lead_off=0;

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      inByte = ADS_xfer(0x00);
			if(i == 0) {
				eCon_Message[byteCounter] = inByte;
				byteCounter++;
			}
			if(byteCounter == 15) {
				byteCounter = 0;
			}
			
      boardChannelDataInt[i] = (boardChannelDataInt[i] << 8) | inByte; // int data goes here
    }
  }
  CS_H; // close SPI
	
	for (int i = 0; i < 2; i++)
	{ //convert 3 byte 2's compliment to 4 byte 2's compliment
		if ((boardChannelDataInt[i] & 0x00800000) == 0x00800000)
		{
			boardChannelDataInt[i] |= 0xFF000000;
		}
		else
		{
			boardChannelDataInt[i] &= 0x00FFFFFF;
		}
	}
}








