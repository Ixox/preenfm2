/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    22-July-2011
  * @brief   This file is responsible to offer board support package and is
  *          configurable by user.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "stm32f4xx_conf.h"

/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_BSP
  * @brief This file is responsible to offer board support package
  * @{
  */

/** @defgroup USB_BSP_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_BSP_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */





/** @defgroup USB_BSP_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_BSP_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup USBH_BSP_Private_FunctionPrototypes
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_BSP_Private_Functions
  * @{
  */


/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{

#ifdef USB_OTG_FS_CORE
	  GPIO_InitTypeDef GPIO_InitStructure;

	  // USB HOST : CORE_FS
	  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);

	  /* Configure  ID DM DP Pins */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 |    GPIO_Pin_12;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ;
	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;
#endif

#ifdef USB_OTG_HS_CORE
	  GPIO_InitTypeDef GPIO_InitStructure2;
	  // USB HOST : CORE_HS
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);

	  GPIO_InitStructure2.GPIO_Pin =  GPIO_Pin_14 | GPIO_Pin_15;
	  GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_100MHz;
	  GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_AF;
	  GPIO_Init(GPIOB, &GPIO_InitStructure2);

	  GPIO_PinAFConfig(GPIOB,GPIO_PinSource14, GPIO_AF_OTG2_FS) ;
	  GPIO_PinAFConfig(GPIOB,GPIO_PinSource15, GPIO_AF_OTG2_FS) ;
	  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS, ENABLE) ;
#endif

}
/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Enabele USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	  /* Enable USB Interrupt */

	  NVIC_InitTypeDef NVIC_InitStructure;

#ifdef USB_OTG_HS_CORE
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	  // HS
	  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef USB_OTG_FS_CORE
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	  // FS
	  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
#endif

}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  speed : Full, Low
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{

}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  Speed : Full, Low
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
  * @brief  USB_OTG_BSP_TimeInit
  *         Initialises delay unit Systick timer /Timer2
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_TimeInit ( void )
{

}

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */

void USB_OTG_BSP_uDelay (const uint32_t usec) {
	PreenFM2_uDelay(usec);
}




/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
    PreenFM2_uDelay(msec * 1000);
}


/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_TimerIRQ (void)
{

}

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
