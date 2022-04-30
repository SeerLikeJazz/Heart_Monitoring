#ifndef ADS1292_H
#define ADS1292_H

#include <stdbool.h>
#include <stdint.h>

/************************************************************************
SPI总线包括以下四种信号线
1）MOSI
2）MISO
3）SCK
4）CS		也称SS，信号使能

	nRF52840片内集成SPI三种，SPI\SPIM\SPIS
	SPI：不含 EasyDMA 的 SPI 主机，不推荐使用。
	SPIM：含 EasyDMA 的 SPI 主机。
	SPIS：含 EasyDMA 的 SPI 从机。

	52840内部集成4个SPIM外设，其中SPIM3比较特殊，支持最大速率32Mbps，同时支持硬件片选和D/CX输出

**************************************************************************/

//定义使用SPI管脚
#define  SPI_SS_PIN     NRF_GPIO_PIN_MAP(0,9)	//输出模式
#define  SPI_SCK_PIN    NRF_GPIO_PIN_MAP(0,7)
#define  SPI_MISO_PIN   NRF_GPIO_PIN_MAP(0,6)
#define  SPI_MOSI_PIN   NRF_GPIO_PIN_MAP(0,8)
//#define  SPI_SS_PIN     29
//#define  SPI_SCK_PIN    3
//#define  SPI_MISO_PIN   28
//#define  SPI_MOSI_PIN   2
//ADS1292外留常用管脚


#define  EEG_START			NRF_GPIO_PIN_MAP(0,10)	//输出模式
#define  EEG_PWRST			NRF_GPIO_PIN_MAP(0,11)	//输出模式


#define	CS_H	nrf_gpio_pin_set(SPI_SS_PIN)					//CS置高
#define	CS_L	nrf_gpio_pin_clear(SPI_SS_PIN)				//CS拉低

#define	START_H	nrf_gpio_pin_set(EEG_START)					//START置高
#define	START_L	nrf_gpio_pin_clear(EEG_START)				//START拉低

#define	PWRST_H	nrf_gpio_pin_set(EEG_PWRST)					//PWRST置高
#define	PWRST_L	nrf_gpio_pin_clear(EEG_PWRST)				//PWRST拉低



/*======================ADS1292寄存去列表===========================*/
//Table	15.command Definitions
//System Command
#define _WAKEUP 			0x02			// 唤醒命令
#define _STANDBY 			0x04 			// 进入待机模式
#define _RESET 				0x06 			// 重置该设备
#define _START				0x08 			// 重新启动同步转换
#define _STOP 				0x0A 			// 停止转换
#define _OFFSETCAL 		0x1A

#define _RDATAC 			0x10 			// 进入连续读取模式，通电时默认
#define _SDATAC 			0x11 			// 停止连续读取模式
#define _RDATA 				0x12 			// 按命令读取数据，支持多个回读

#define ADS_ID			0x53				// product ID for ADS1299
#define	ID_REG			0x00				//ID控制寄存器 bit[1:0] 11 代表是ADS1292		OR
#define	CONFIG1			0x01				//配置寄存器1  bit7：连续转换模式/单发模式	bit[2:0]--通道1和通道2的采样率
#define	CONFIG2			0x02				//配置寄存器2	 bit6：开关电极检测		bit5：使能参考缓冲		bit4：2.42/4.033参考电压		bit3：使能内部晶振		bit1：测试信号选择		bit0：测试信号频率
#define	LOFF				0x03				//lead-off控制寄存器		电流、频率、百分比详细配置参考P54
#define	CH1SET			0x04				//通道1设置		 bit[3:0]--通道1输入选择			bit[6:4]--PGA增益设置		bit7：通道1掉电
#define	CH2SET			0x05				//通道2设置		 bit[3:0]--通道2输入选择			bit[6:4]--PGA增益设置		bit7：通道1掉电
#define	RLD_SENS		0x06				//右腿驱动感应设置	bit[7:6]--频率设置		bit5：RLD power				bit4：RLD 传感器功能		bit3：通道2负极输入		bit2：通道2正极输入		bit1：通道1负极输入		bit0：通道1正极输入
#define	LOFF_SENS		0x07				//Lead-Off Sense Selection								bit5：lead-off通道2		bit4：lead-off通道1			bit3：通道2负极输入		bit2：通道2正极输入		bit1：通道1负极输入		bit0：通道1正极输入
#define	LOFF_STAT		0x08				//Lead-Off Sense状态寄存器		bit6：CLK_DIV 0(512Khz)	1(2.048Mhz)
#define	RESP1				0x09				//呼吸控制寄存器			bit7：使能通道1上解调电路		bit6：允许通道1上解调电路		bit[5:2]：借条相位		bit0：内置/外置呼吸电路
#define	RESP2				0x0A				//呼吸控制寄存器2			bit7：使能偏移校准					bit2：呼吸频率
#define	ADS_GPIO		0x0B				//通用输入IO寄存器		bit[3:2]




void ADS1292_gpio_config(void);
void SPI_User_init(void);

uint8_t	ADS_xfer(uint8_t byte);
uint8_t	ADS_WREG(uint8_t	address,	uint8_t	cmd);
uint8_t ADS_RREG(uint8_t address);
void	ADS_RESET(void);
void ADS_SDATAC(void);
uint8_t	ADS_getDeviceID(void);
void  ADS_START(void);
void	ADS_RDATAC(void);
void	ADS_Offsetcal(void);
void short_test(void);
void test_wave(void);
void eeg_signal(void);
void initialize_ads(uint8_t mode);
void updateBoardData(void);








#endif



