/********************************************************************
 nef52840�����Ҫʹ��spi���ܣ���Ҫ��������ļ�����
 
	nrf_drv_spi.c ..\integration\nrfx\legacy 		�ɰ汾 SPI �����ļ���
	nrfx_spi.c 		..\modules\nrfx\drivers\src 	�°汾 SPI �����ļ���
	nrfx_spim.c 	..\modules\nrfx\drivers\src 	SPI ���������ļ���
		

	����ADS1292ʹ��IO�������
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


static volatile bool spi_xfer_done;  //SPI���ݴ�����ɱ�־
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static uint8_t    spi_tx_buf[256];   /**< TX buffer. */
static uint8_t    spi_rx_buf[256];   /**< RX buffer. */

//�������ھ���Ĵ�������
uint8_t			regData[24]; 				

int				boardStat;
int				boardChannelDataInt[5];
uint8_t			lead_off;

void ADS1292_gpio_config()
{
	//����ADS1292��gpio��ʼ��
	nrf_gpio_cfg_output(SPI_SS_PIN);
	nrf_gpio_cfg_output(EEG_START);
	nrf_gpio_cfg_output(EEG_PWRST);

	CS_H;			//�ø�CS
	PWRST_H;	//����PWRST
	START_L;	//����START
}

/*	SPI�ж��¼�������	*/
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
  //����SPI�������  
	spi_xfer_done = true;
}



void SPI_User_init()
{
	/*	����SPI�����ֹ���ģʽ
		ģʽ				����
	ģʽ0			CPOL=0			CPHA=0
	ģʽ1			CPOL=0			CPHA=1
	ģʽ2			CPOL=1			CPHA=0
	ģʽ3			CPOL=1			CPHA=1	
	*/
	//���ö���spi��ʼ���ṹ�� = Ĭ������
	nrf_drv_spi_config_t	spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	//��дĬ�����������������
	spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;									//�û��Զ��壬�����������
	spi_config.mosi_pin = SPI_MOSI_PIN;
	spi_config.miso_pin =	SPI_MISO_PIN;
	spi_config.sck_pin = 	SPI_SCK_PIN;
	
	spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;		//�ж����ȼ�������ں궨���������õ���6
	spi_config.frequency = NRF_DRV_SPI_FREQ_8M;										//spiƵ������8M
	spi_config.orc = 0xFF;																				//ORC�Ĵ�������0xff
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;				//���ݴ����λ��ǰ
	spi_config.mode = NRF_DRV_SPI_MODE_1;													//ģʽ1
	
	  //��ʼ��SPI
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));	
	
}

//SPI��ָ����ַ�϶�ȡһ������
uint8_t	ADS_xfer(uint8_t byte)
{
	/*
		nrf52840 SDK�Ķ�д�Ƿ�װ��ͬһ���������
	*/
	uint8_t d_read;						
	
	//8λ��ַ
	spi_tx_buf[0] =	 byte;		

	//������ɱ�־����Ϊfalse
	spi_xfer_done = false;
	
	//�������ݴ���
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, 1, spi_rx_buf, 1));	
	
	//�ȴ�SPI�������
	while(!spi_xfer_done);
		
	d_read = spi_rx_buf[0];
	
//	//����������ɱ�־����Ϊfalse
//	spi_xfer_done = false;	
//	
//	//�������ݴ���
//	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, 0, &d_read, 1));	
//	
//	//�ȴ�SPI�������
//	while(!spi_xfer_done)
//	{}
		
	//���ض�ȡ��ֵ
	return	d_read;	
		
}



uint8_t	ADS_WREG(uint8_t	address,	uint8_t	cmd)
{
	uint8_t	Opcode1 = address + 0x40;						//����д��Ĵ�����ַ
	CS_L;																				//����CS
	ADS_xfer(Opcode1);  												//��һ�������룺�Ĵ���address
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(0x00);															//���������� 0x01 - 1 = 0x00
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(cmd); 															//д��Ĵ���ֵ
	for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;																				//CS�øߣ�ʹ������
	return regData[address]; 
}




//ADS��ȡ1���Ĵ�����ֵ
uint8_t ADS_RREG(uint8_t address)
{
	uint8_t	Opcode1 = address + 0x20;						//�����ȡ�Ĵ�����ַ
	CS_L;																				//����CS
	ADS_xfer(Opcode1);  												//��һ�������룺�Ĵ���address
	for(int j=250;j>0;j--){};										//4 Tclk delay
	ADS_xfer(0x00);															//���������� 0x01 - 1 = 0x00
	for(int j=250;j>0;j--){};										//4 Tclk delay
	regData[address] = ADS_xfer(0x00); 					// update mirror location with returned byte
	for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;	

	return regData[address]; 										// return requested register value
}


//ADS��λ
void	ADS_RESET(void)
{
	CS_L;
	ADS_xfer(_RESET);
	nrf_delay_ms(1);
	CS_H;	

}


//ADSֹͣ������ȡģʽ
void ADS_SDATAC(void)
{
	CS_L;
  ADS_xfer(_SDATAC);
  for(int j=250;j>0;j--){};										//4 Tclk delay
	CS_H;	
  nrf_delay_ms(1); 														//must wait at least 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}



//ADS��ȡоƬID
uint8_t	ADS_getDeviceID(void)
{
  uint8_t data = ADS_RREG(ID_REG);
  return data;
}


//ADS�����ź�
void  ADS_START(void)
{
	CS_L;
  ADS_xfer(_START); 												// KEEP ON-BOARD AND ON-DAISY IN SYNC
  for(int j=250;j>0;j--){};									//4 Tclk delay
	CS_H;
}



//ADS������ȡģʽ
void    ADS_RDATAC(void)
{
	CS_L;
  ADS_xfer(_RDATAC); 									// read data continuous
  for(int j=250;j>0;j--){};								//4 Tclk delay
	CS_H;
  nrf_delay_ms(1);
}


//ADSͨ��ƫ��У׼
void    ADS_Offsetcal(void)
{
	CS_L;
  ADS_xfer(_OFFSETCAL); 								// read data continuous
  for(int j=250;j>0;j--){};								//4 Tclk delay
	CS_H;
  nrf_delay_ms(1);
}



/*		����ģʽ			*/
//�ڲ����������ź�
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
//�ڲ�1mv�����ź�
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
//�ɼ�EEG�ź�
void eeg_signal(void)
{
	/* lead-off + signal */
	ADS_WREG(CONFIG1,0x01);/*Continuous conversion mode��ת������250SPS*/
	nrf_delay_ms(10);
	ADS_WREG(CONFIG2,0xE0);/*ʹ��2.42 V�ڲ��ο���ѹ*/
	nrf_delay_ms(100);
	ADS_WREG(LOFF,0x10);/*95% threshold��6nA*/
	nrf_delay_ms(100);
	ADS_WREG(CH1SET,0x00);/*�Ŵ���6��*/
	nrf_delay_ms(100);
//	ADS_WREG(CH2SET,0x01);/*�ر�ͨ��2*/
	ADS_WREG(CH2SET,0x00);/*�Ŵ���6��*/
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
//ADS�ϵ��ʼ��
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
//	ADS_WREG(LOFF_STAT,0x50);		/*bit6 ���ʹ���ⲿ����*/
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


//���ζ�ȡһ��ADS���ݷ���������
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
	
	//ѭ��3�ΰ����ݴ�ŵ�STAT��
	for (int i = 0; i < 3; i++)
	{
		inByte = ADS_xfer(0x00); 									//ÿ�ζ�ֻ��0x01 һ������  ����0x01 - 1 = 0x00��
		boardStat = (boardStat << 8) | inByte;
	}
	//�ж�
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








