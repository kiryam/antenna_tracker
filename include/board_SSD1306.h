#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include <board_SSD1306.h>
#include "stdint.h"
#include "stm32f10x.h"

//#define SSD1306_PAGE_PREFIX		0x40			 		// Co = 0, D/C = 1
#define SSD1306_I2C I2C1
#define SSD1306_I2C_ADDRESS   	0x78
#define SSD1306_SDA_PORT		GPIOB
#define SSD1306_SDA_PIN			GPIO_Pin_9
#define SSD1306_SCL_PORT		GPIOB
#define SSD1306_SCL_PIN			GPIO_Pin_8
#define SSD1306_SH1106          TRUE

static GFXINLINE void init_board(GDisplay *g) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

	g->board = 0;
	switch(g->controllerdisplay) {
	case 0:
		I2C_DeInit(SSD1306_I2C);
		GPIO_InitTypeDef gpio;
		I2C_InitTypeDef i2c;
		GPIO_StructInit(&gpio);
		I2C_StructInit(&i2c);

		gpio.GPIO_Pin = SSD1306_SDA_PIN | SSD1306_SCL_PIN;
		gpio.GPIO_Mode = GPIO_Mode_AF_OD;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &gpio);

		i2c.I2C_ClockSpeed = 400000;
		i2c.I2C_Mode = I2C_Mode_I2C;
		i2c.I2C_DutyCycle = I2C_DutyCycle_2;
		i2c.I2C_OwnAddress1 = 0x15;
		i2c.I2C_Ack = I2C_Ack_Disable;
		i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		I2C_Init(SSD1306_I2C, &i2c);
		I2C_Cmd(SSD1306_I2C, ENABLE);
		break;
	}
}

static GFXINLINE void post_init_board(GDisplay *g) {
	(void) g;
}

static GFXINLINE void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
}

static GFXINLINE void acquire_bus(GDisplay *g) {
	(void) g;
}

static GFXINLINE void release_bus(GDisplay *g) {
	(void) g;
}

uint16_t ssd1306_I2C_Start(){
    while(I2C_GetFlagStatus(SSD1306_I2C, I2C_FLAG_BUSY));
    I2C_GenerateSTART(SSD1306_I2C, ENABLE);
    while(!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(SSD1306_I2C, SSD1306_I2C_ADDRESS, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    return 0;
}

void ssd1306_I2C_WriteData(uint8_t data) {
    I2C_SendData(SSD1306_I2C, data);
    while(!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t ssd1306_I2C_Stop() {
	SSD1306_I2C->CR1 |= I2C_CR1_STOP;
	return 0;
}

static GFXINLINE void write_cmd(GDisplay *g, uint8_t cmd) {
	(void) g;

	ssd1306_I2C_Start();
	ssd1306_I2C_WriteData(0x00);
	ssd1306_I2C_WriteData(cmd);
	ssd1306_I2C_Stop();
}

static GFXINLINE void write_data(GDisplay *g, uint8_t* data, uint16_t length) {
	(void) g;

	uint8_t i;
	ssd1306_I2C_Start();
	ssd1306_I2C_WriteData(0x40);
	for (i = 0; i < length; i++) {
		ssd1306_I2C_WriteData(data[i]);
	}
	ssd1306_I2C_Stop();
}

#endif /* _GDISP_LLD_BOARD_H */
