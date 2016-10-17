/*
 * Copyright 2013
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PreenFM.h"
#include "Common.h"

#ifdef CVIN
uint16_t ADCBuffer[] = {
        0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA,
        0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA,
        0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA,
        0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA 
};
#endif


void strobePin(uint8_t count, uint32_t rate) {
	GPIO_ResetBits(GPIOB, LEDPIN);
	uint32_t c;
	while (count-- > 0) {
		for (c = rate; c > 0; c--) {
			asm volatile ("nop");
		}
		GPIO_SetBits(GPIOB, LEDPIN);
		for (c = rate; c > 0; c--) {
			asm volatile ("nop");
		}
		GPIO_ResetBits(GPIOB, LEDPIN);
	}
}


/**
 * @brief  Configures the USART Peripheral.
 * @param  None
 * @retval None
 */
void USART_Config() {

	/* --------------------------- System Clocks Configuration -----------------*/
	/* USART3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/*-------------------------- GPIO Configuration ----------------------------*/
	// TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	// Init USART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 31250;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	/* Here the USART3 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART3_IRQHandler() function
	 * if the USART3 receive interrupt occurs
	 */
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // enable the USART3 receive interrupt
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; // we want to configure the USART3 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10; // this sets the priority group of the USART3 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10; // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // the USART3 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure); // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	USART_Cmd(USART3, ENABLE);
}

void LED_Config() {
	// GPIOG Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure PB5 in output mode */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = LEDPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}




void ADC_Config() {
#ifdef CVIN
        ADC_InitTypeDef       ADC_InitStructure;
        ADC_CommonInitTypeDef ADC_CommonInitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        DMA_InitTypeDef DMA_InitStructure;
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        /* Enable timer (timer runs at 21 MHz)*/
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
                // 1000000/(42031/32) / 4( to make an average on 4 values)
        float period = 1000000.0f / (PREENFM_FREQUENCY / 32.0f) / 4.0f;
        TIM_TimeBaseStructure.TIM_Period = (int)(period - 1); // Must be called once per block of 32 samples  
        TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1; // Down to 1 MHz (adjust per your clock)
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
        NVIC_Init(&NVIC_InitStructure);

        /* Enable clock on DMA1 & GPIOC */
        /* Enable DMA2, thats where ADC is hooked on -> see Tab 20 (RM00090) */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2, ENABLE);

        /* Initialise GPIOs C0 (ADC123_IN10), C1 (ADC123_IN11), C2 (ADC123_IN12)*/
        GPIO_StructInit(&GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        /* Initialise DMA */
        DMA_StructInit(&DMA_InitStructure);

        /* config of DMAC */
        DMA_InitStructure.DMA_Channel = DMA_Channel_0; /* See Tab 20 */
        DMA_InitStructure.DMA_BufferSize = 16; /* 16 * memsize */
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; /* direction */
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; /* no FIFO */
        DMA_InitStructure.DMA_FIFOThreshold = 0;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; /* circular buffer */
        DMA_InitStructure.DMA_Priority = DMA_Priority_High; /* high priority */
        /* config of memory */
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADCBuffer; /* target addr */
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; /* 16 bit */
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_Init(DMA2_Stream0, &DMA_InitStructure); /* See Table 20 for mapping */
        DMA_Cmd(DMA2_Stream0, ENABLE);

        /* IMPORTANT: populate default values before use */
        ADC_StructInit(&ADC_InitStructure);
        ADC_CommonStructInit(&ADC_CommonInitStructure);

        /* reset configuration if needed, could be used for previous init */
        ADC_Cmd(ADC1, DISABLE);
        ADC_DeInit();

        /* init ADC clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

        /* init ADC */
        ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
        ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
        ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
        ADC_CommonInit(&ADC_CommonInitStructure);

        /* ADC1 Init: this is mostly done with ADC1->CR */
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_10b;
        ADC_InitStructure.ADC_ScanConvMode = ENABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfConversion = 4; /* 4 channels in total */
        ADC_Init(ADC1, &ADC_InitStructure);


        /* Configure channels */
        ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles);
        ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_28Cycles);
        ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_28Cycles);
        ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_28Cycles);

        /* Enable DMA request after last transfer (Single-ADC mode) */
        ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

        /* Enable ADC1 DMA */
        ADC_DMACmd(ADC1, ENABLE);

        /* Enable ADC1 **************************************************************/
        ADC_Cmd(ADC1, ENABLE);

        /* in main */
        TIM_Cmd(TIM2, ENABLE);
#endif
}


void MCP4922_Config() {
	// CONFIG SPI !!!!

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Peripheral Clock Enable -------------------------------------------------*/
	/* Enable the SPI clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* Enable GPIO clocks B & C*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

	/* SPI GPIO Configuration --------------------------------------------------*/
	/* Connect SPI pins to AF5 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// CONFIG  DAC1 (GPIO_Pin_4) & DAC2 (9)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// CONFIG LDAC
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// CONFIG  PB8 to know if we have PCB 4f or 4g
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* SPI configuration -------------------------------------------------------*/
	SPI_I2S_DeInit(SPI1);

	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	/* Enable the SPI peripheral */
	SPI_Cmd(SPI1, ENABLE);

}

void Systick_disable() {
	SysTick->CTRL  = 0;
}


void SysTick_Config(void) {
	int numberOfTick = SystemCoreClock / PREENFM_FREQUENCY / 4;


	if (SysTick_Config(numberOfTick)) {
		/* Capture error */
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Systick Error...");
		while (1);
	}

	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);
}


void RNG_Config(void)
{
 /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
}


void LCD_InitChars(LiquidCrystal *lcd) {
    unsigned char row0[8] = {
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
    };
    unsigned char row1[8] = {
            0b01111,
            0b01111,
            0b01111,
            0b01111,
            0b01111,
            0b01111,
            0b01111,
            0b01111,
    };
    unsigned char row2[8] = {
            0b00111,
            0b00111,
            0b00111,
            0b00111,
            0b00111,
            0b00111,
            0b00111,
            0b01111,
    };
    unsigned char row3[8] = {
            0b00011,
            0b00011,
            0b00011,
            0b00011,
            0b00011,
            0b00011,
            0b00011,
            0b00011,
    };
    unsigned char row4[8] = {
            0b00001,
            0b00001,
            0b00001,
            0b00001,
            0b00001,
            0b00001,
            0b00001,
            0b00001,
    };

    unsigned char row5[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
    };


    lcd->createChar(0, row0);
    lcd->createChar(1, row1);
    lcd->createChar(2, row2);
    lcd->createChar(3, row3);
    lcd->createChar(4, row4);
    lcd->createChar(5, row5);

}



