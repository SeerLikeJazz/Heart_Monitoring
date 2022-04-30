#ifndef ADS1292_H
#define ADS1292_H

#include <stdbool.h>
#include <stdint.h>

/************************************************************************
SPI���߰������������ź���
1��MOSI
2��MISO
3��SCK
4��CS		Ҳ��SS���ź�ʹ��

	nRF52840Ƭ�ڼ���SPI���֣�SPI\SPIM\SPIS
	SPI������ EasyDMA �� SPI ���������Ƽ�ʹ�á�
	SPIM���� EasyDMA �� SPI ������
	SPIS���� EasyDMA �� SPI �ӻ���

	52840�ڲ�����4��SPIM���裬����SPIM3�Ƚ����⣬֧���������32Mbps��ͬʱ֧��Ӳ��Ƭѡ��D/CX���

**************************************************************************/

//����ʹ��SPI�ܽ�
#define  SPI_SS_PIN     NRF_GPIO_PIN_MAP(0,9)	//���ģʽ
#define  SPI_SCK_PIN    NRF_GPIO_PIN_MAP(0,7)
#define  SPI_MISO_PIN   NRF_GPIO_PIN_MAP(0,6)
#define  SPI_MOSI_PIN   NRF_GPIO_PIN_MAP(0,8)
//#define  SPI_SS_PIN     29
//#define  SPI_SCK_PIN    3
//#define  SPI_MISO_PIN   28
//#define  SPI_MOSI_PIN   2
//ADS1292�������ùܽ�


#define  EEG_START			NRF_GPIO_PIN_MAP(0,10)	//���ģʽ
#define  EEG_PWRST			NRF_GPIO_PIN_MAP(0,11)	//���ģʽ


#define	CS_H	nrf_gpio_pin_set(SPI_SS_PIN)					//CS�ø�
#define	CS_L	nrf_gpio_pin_clear(SPI_SS_PIN)				//CS����

#define	START_H	nrf_gpio_pin_set(EEG_START)					//START�ø�
#define	START_L	nrf_gpio_pin_clear(EEG_START)				//START����

#define	PWRST_H	nrf_gpio_pin_set(EEG_PWRST)					//PWRST�ø�
#define	PWRST_L	nrf_gpio_pin_clear(EEG_PWRST)				//PWRST����



/*======================ADS1292�Ĵ�ȥ�б�===========================*/
//Table	15.command Definitions
//System Command
#define _WAKEUP 			0x02			// ��������
#define _STANDBY 			0x04 			// �������ģʽ
#define _RESET 				0x06 			// ���ø��豸
#define _START				0x08 			// ��������ͬ��ת��
#define _STOP 				0x0A 			// ֹͣת��
#define _OFFSETCAL 		0x1A

#define _RDATAC 			0x10 			// ����������ȡģʽ��ͨ��ʱĬ��
#define _SDATAC 			0x11 			// ֹͣ������ȡģʽ
#define _RDATA 				0x12 			// �������ȡ���ݣ�֧�ֶ���ض�

#define ADS_ID			0x53				// product ID for ADS1299
#define	ID_REG			0x00				//ID���ƼĴ��� bit[1:0] 11 ������ADS1292		OR
#define	CONFIG1			0x01				//���üĴ���1  bit7������ת��ģʽ/����ģʽ	bit[2:0]--ͨ��1��ͨ��2�Ĳ�����
#define	CONFIG2			0x02				//���üĴ���2	 bit6�����ص缫���		bit5��ʹ�ܲο�����		bit4��2.42/4.033�ο���ѹ		bit3��ʹ���ڲ�����		bit1�������ź�ѡ��		bit0�������ź�Ƶ��
#define	LOFF				0x03				//lead-off���ƼĴ���		������Ƶ�ʡ��ٷֱ���ϸ���òο�P54
#define	CH1SET			0x04				//ͨ��1����		 bit[3:0]--ͨ��1����ѡ��			bit[6:4]--PGA��������		bit7��ͨ��1����
#define	CH2SET			0x05				//ͨ��2����		 bit[3:0]--ͨ��2����ѡ��			bit[6:4]--PGA��������		bit7��ͨ��1����
#define	RLD_SENS		0x06				//����������Ӧ����	bit[7:6]--Ƶ������		bit5��RLD power				bit4��RLD ����������		bit3��ͨ��2��������		bit2��ͨ��2��������		bit1��ͨ��1��������		bit0��ͨ��1��������
#define	LOFF_SENS		0x07				//Lead-Off Sense Selection								bit5��lead-offͨ��2		bit4��lead-offͨ��1			bit3��ͨ��2��������		bit2��ͨ��2��������		bit1��ͨ��1��������		bit0��ͨ��1��������
#define	LOFF_STAT		0x08				//Lead-Off Sense״̬�Ĵ���		bit6��CLK_DIV 0(512Khz)	1(2.048Mhz)
#define	RESP1				0x09				//�������ƼĴ���			bit7��ʹ��ͨ��1�Ͻ����·		bit6������ͨ��1�Ͻ����·		bit[5:2]��������λ		bit0������/���ú�����·
#define	RESP2				0x0A				//�������ƼĴ���2			bit7��ʹ��ƫ��У׼					bit2������Ƶ��
#define	ADS_GPIO		0x0B				//ͨ������IO�Ĵ���		bit[3:2]




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



