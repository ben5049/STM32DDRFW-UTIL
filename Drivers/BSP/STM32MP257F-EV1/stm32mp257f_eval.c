/**
  ******************************************************************************
  * @file    stm32mp257f_eval.c
  * @author  MCD Application Team
  * @brief   This file provides a set of firmware functions to manage
  *          LEDs
  *          COM ports
  *          available on STM32MP257F-EV1 board(MB170/4,5,6) from STMicroelectronics.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32mp257f_eval.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32MP257F_EV1
  * @{
  */

/** @defgroup LOW_LEVEL LOW LEVEL
  * @{
  */

/** @defgroup LOW_LEVEL_Private_TypesDefinitions  Private Types Definitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback)(void);
/**
  * @}
  */

/** @defgroup LOW_LEVEL_Private_FunctionPrototypes  Private Function Prototypes
  * @{
  */
#if defined (CORE_CA35) || defined (CORE_CM33)
static int32_t BSP_LED_MspInit(Led_TypeDef Led);
static int32_t BSP_LED_MspDeInit(Led_TypeDef Led);
#endif /* CORE_CA35 || CORE_CM33 */
#if (USE_BSP_COM_FEATURE > 0)
static void USART_MspInit(UART_HandleTypeDef *huart);
static void USART_MspDeInit(UART_HandleTypeDef *huart);
#endif /* USE_BSP_COM_FEATURE */
/**
  * @}
  */

/** @defgroup LOW_LEVEL_Exported_Variables Exported Variables
  * @{
  */
#if (USE_BSP_COM_FEATURE > 0)
UART_HandleTypeDef hcom_uart[COMn];
USART_TypeDef *COM_USART[COMn] =
#if defined (CORE_CA35) || defined (CORE_CM33)
{COM_CA35_UART, COM_CM33_UART, COM_CM0PLUS_UART};
#elif defined (CORE_CM0PLUS) /* Core_M0Plus uses only COM_CM0PLUS */
  {
    0, 0, COM_CM0PLUS_UART
  };
#endif /* CORE_<...> */

#endif /* USE_BSP_COM_FEATURE */
/**
  * @}
  */

/** @defgroup LOW_LEVEL_Private_Variables Private Variables
  * @{
  */
#if defined (CORE_CA35) || defined (CORE_CM33)
static GPIO_TypeDef *LED_PORT[LEDn] = { LED1_GPIO_PORT,
                                        LED2_GPIO_PORT
                                      };

static const uint32_t LED_PIN[LEDn] = { LED1_PIN,
                                        LED2_PIN
                                      };

static const GPIO_PinState GPIO_LED_OFF[LEDn] = { GPIO_PIN_SET,
                                                  GPIO_PIN_RESET
                                                };

static const GPIO_PinState GPIO_LED_ON[LEDn] = { GPIO_PIN_RESET,
                                                 GPIO_PIN_SET
                                               };


#elif defined (CORE_CM0PLUS)
#warning "Core M0 can't manage LED in eval Board"
#endif /* CORE_CA35 || CORE_CM33*/

#if (USE_COM_LOG > 0)
/* define default active port to port dedicated to current Core
 * This is important for Validation applications which are not explicitly setting active log port */
static COM_TypeDef COM_ActiveLogPort = COM1;
#endif /* USE_COM_LOG */
#if (USE_BSP_COM_FEATURE > 0)
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
static uint32_t IsComMspCbValid[COMn] = {0};
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
#endif /* USE_BSP_COM_FEATURE */

/**
  * @}
  */

/** @defgroup LOW_LEVEL_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  This method returns the STM32MP257CXX EVAL BSP Driver revision
  * @retval version: 0xXYZR (8bits for each decimal, R for RC)
  */
int32_t BSP_GetVersion(void)
{
  return (int32_t)STM32MP257F_EV1_BSP_VERSION;
}

/**
  * @brief  This method returns the board name
  * @retval pointer to the board name string
  */
const uint8_t *BSP_GetBoardName(void)
{
  return (uint8_t *)STM32MP257F_EV1_BSP_BOARD_NAME;
}

/**
  * @brief  This method returns the board ID
  * @retval pointer to the board name string
  */
const uint8_t *BSP_GetBoardID(void)
{
  return (uint8_t *)STM32MP257F_EV1_BSP_BOARD_ID;
}

#if defined (CORE_CA35) || defined (CORE_CM33)
/**
  * @brief  Configures LED on GPIO.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t BSP_LED_Init(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*BSP LED Msp Init Configuration*/
  if (BSP_LED_MspInit(Led) != BSP_ERROR_NONE)
  {
    return BSP_ERROR_MSP_FAILURE;
  }

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = LED_PIN[Led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  HAL_GPIO_Init(LED_PORT[Led], &GPIO_InitStruct);

  /* By default, turn off LED */
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_LED_OFF[Led]);
  return ret;
}

/**
  * @brief  DeInit LEDs.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t  BSP_LED_DeInit(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef  gpio_init_structure;

  /* DeInit the GPIO_LED pin */
  gpio_init_structure.Pin = LED_PIN [Led];
  /* Turn off LED */
  HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN[Led], GPIO_LED_OFF[Led]);
  HAL_GPIO_DeInit(LED_PORT [Led], gpio_init_structure.Pin);

  /* BSP LED  MSP DeInit configuration*/
  if (BSP_LED_MspDeInit(Led) != BSP_ERROR_NONE)
  {
    return BSP_ERROR_MSP_FAILURE;
  }

  return ret;
}

/**
  * @brief  MspInit LEDs.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval int32_t
  */
static int32_t BSP_LED_MspInit(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_TypeDef *led = LED_PORT[Led];
  uint32_t pin = LED_PIN[Led];

  /* Enable the GPIO_LED clock */
  if (Led == LED1 && !(LED1_GPIO_IS_CLK_ENABLED()))
  {
    if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(97)))
    {
      LED1_GPIO_CLK_ENABLE();
    }
  }
  else if (Led == LED2 && !(LED2_GPIO_IS_CLK_ENABLED()))
  {
    if (IS_DEVELOPER_BOOT_MODE())
    {
      HAL_PWREx_EnableSupply(PWR_PVM_VDDIO3);
    }
    if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(93)))
    {
      LED2_GPIO_CLK_ENABLE();
    }
  }

  if (!IS_DEVELOPER_BOOT_MODE())
  {
    if ((led == LED1_GPIO_PORT && pin == LED1_PIN
         && ResMgr_Request(LED1_GPIO_RIF_RES_TYP_GPIO, LED1_GPIO_RIF_RES_NUM_GPIO) != RESMGR_STATUS_ACCESS_OK)
        || (led == LED2_GPIO_PORT && pin == LED2_PIN && \
            ResMgr_Request(LED2_GPIO_RIF_RES_TYP_GPIO, LED2_GPIO_RIF_RES_NUM_GPIO) != RESMGR_STATUS_ACCESS_OK))
    {
      ret = BSP_ERROR_MSP_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  MspDeInit LEDs.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval int32_t
  */
static int32_t BSP_LED_MspDeInit(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_TypeDef *led = LED_PORT[Led];
  uint32_t pin = LED_PIN[Led];

  if (!IS_DEVELOPER_BOOT_MODE())
  {
    if ((led == LED1_GPIO_PORT && pin == LED1_PIN
         && ResMgr_Release(LED1_GPIO_RIF_RES_TYP_GPIO, LED1_GPIO_RIF_RES_NUM_GPIO) != RESMGR_STATUS_ACCESS_OK)
        || (led == LED2_GPIO_PORT && pin == LED2_PIN && \
            ResMgr_Release(LED2_GPIO_RIF_RES_TYP_GPIO, LED2_GPIO_RIF_RES_NUM_GPIO) != RESMGR_STATUS_ACCESS_OK))
    {
      ret = BSP_ERROR_MSP_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led LED to be set on
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t  BSP_LED_On(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;

  HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_LED_ON[Led]);
  return ret;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led LED to be set off
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t  BSP_LED_Off(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_LED_OFF[Led]);
  return ret;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led LED to be toggled
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t  BSP_LED_Toggle(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  HAL_GPIO_TogglePin(LED_PORT[Led], (uint16_t)LED_PIN[Led]);
  return ret;
}
/**
  * @brief  Get the selected LED state.
  * @param  Led LED to be get its state
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval LED status
  */
int32_t BSP_LED_GetState(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  ret = (int32_t)HAL_GPIO_ReadPin(LED_PORT [Led], (uint16_t)LED_PIN [Led]);
  return ret;
}


#endif /* defined (CORE_CA35) || defined (CORE_CM33) */

#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Configures COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be COM1, COM2, COM3
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                configuration information for the specified USART peripheral.
  * @retval BSP status
  */
int32_t BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set the COM Instance */
    hcom_uart[COM].Instance = COM_USART[COM];

#if defined (CORE_CM33)
    if (!IS_DEVELOPER_BOOT_MODE() && COM == COM_VCP_CM33)
    {
      if (ResMgr_Request(COM_CM33_RIF_RES_TYP_TX_PIN, COM_CM33_RIF_RES_NUM_TX_PIN) != RESMGR_STATUS_ACCESS_OK
          && ResMgr_Request(COM_CM33_RIF_RES_TYP_RX_PIN, COM_CM33_RIF_RES_NUM_RX_PIN) != RESMGR_STATUS_ACCESS_OK
          && ResMgr_Request(COM_CM33_RIF_RES_TYP_UART, COM_CM33_RIF_RES_NUM_UART) != RESMGR_STATUS_ACCESS_OK)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif /* defined (CORE_CM33) */

    /* Init the UART Msp */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART_MspInit(&hcom_uart[COM]);
#else
    if (IsComMspCbValid[COM] == 0U)
    {
      if (BSP_COM_RegisterDefaultMspCallbacks(COM) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

    if (MX_USART_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DeInit COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be COM1, COM2, COM3
  * @retval BSP status
  */
int32_t BSP_COM_DeInit(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* USART configuration */
    hcom_uart[COM].Instance = COM_USART[COM];

    if (HAL_UART_DeInit(&hcom_uart[COM]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART_MspDeInit(&hcom_uart[COM]);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 0) */
  }

  return ret;
}

/**
  * @brief  Configures COM<n> port.
  * @param  huart USART handle
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                  configuration information for the specified USART peripheral.
  * @retval HAL error code
  */
__weak HAL_StatusTypeDef MX_USART_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init)
{
  HAL_StatusTypeDef ErrorCode;

  /* USART configuration */
  huart->Init.BaudRate     = (uint32_t)COM_Init->BaudRate;
  huart->Init.Mode         = UART_MODE_TX_RX;
  huart->Init.Parity       = (uint32_t)COM_Init->Parity;
  huart->Init.WordLength   = (uint32_t)COM_Init->WordLength;
  huart->Init.StopBits     = (uint32_t)COM_Init->StopBits;
  huart->Init.HwFlowCtl    = (uint32_t)COM_Init->HwFlowCtl;
  huart->Init.OverSampling = UART_OVERSAMPLING_8;
  ErrorCode = HAL_UART_Init(huart);

  return ErrorCode;
}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/**
  * @brief Register Default COM Msp Callbacks
  * @param  COM COM port to be configured.
  * @retval BSP status
  */
int32_t BSP_COM_RegisterDefaultMspCallbacks(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register default MspInit/MspDeInit Callback */
    if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, USART_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, USART_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }

  /* BSP status */
  return ret;
}

/**
  * @brief  Register COM<n> Msp Callback registering
  * @param  COM COM port to be configured.
  * @param  Callbacks     pointer to COM MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_COM_RegisterMspCallbacks(COM_TypeDef COM, BSP_COM_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }

  /* BSP status */
  return ret;
}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

#if (USE_COM_LOG > 0)
/**
  * @brief  Select the active COM port.
  * @param  COM COM port to be activated.
  *          This parameter can be COM1, COM2, COM3
  * @retval BSP status
  */
int32_t BSP_COM_SelectLogPort(COM_TypeDef COM)
{
  if (COM_ActiveLogPort != COM)
  {
    COM_ActiveLogPort = COM;
  }
  return BSP_ERROR_NONE;
}

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
  (void)HAL_UART_Transmit(&hcom_uart[COM_ActiveLogPort], (uint8_t *) &ch, 1, COM_POLL_TIMEOUT);
  return ch;
}

#ifdef __GNUC__
int __io_getchar(void)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&hcom_uart[COM_ActiveLogPort], (uint8_t *)&ch, 1, COM1_RX_TIMEOUT);
  return ch;
}
#endif /* __GNUC__ */
#endif /* USE_COM_LOG */
#endif /* USE_BSP_COM_FEATURE */

/** @defgroup STM32MP257F_EV1_LOW_LEVEL_Private_Functions Private Functions
  * @{
  */

#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Initializes UART MSP.
  * @param  huart UART handle
  * @retval None
  */
static void USART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;

  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pull = GPIO_PULLUP;

#if defined (CORE_CA35) || defined (CORE_CM33)

  if (huart->Instance == COM_CA35_UART)
  {
    /* USART2 clock config */
    ResMgr_Request(RESMGR_RESOURCE_RIFSC, COM_CA35_RIF_RES_NUM_UART);

    /* Enable USART clock */
    COM_CA35_TX_GPIO_CLK_ENABLE();
    COM_CA35_RX_GPIO_CLK_ENABLE();
    COM_CA35_CLK_ENABLE();
    /* Configure USART Tx as alternate function */
    gpio_init_structure.Pin = COM_CA35_TX_PIN;
    gpio_init_structure.Alternate = COM_CA35_TX_AF;
    HAL_GPIO_Init(COM_CA35_TX_GPIO_PORT, &gpio_init_structure);

    /* Configure USART Rx as alternate function */
    gpio_init_structure.Pin = COM_CA35_RX_PIN;
    gpio_init_structure.Alternate = COM_CA35_RX_AF;
    HAL_GPIO_Init(COM_CA35_RX_GPIO_PORT, &gpio_init_structure);
  }
  else if (huart->Instance == COM_CM33_UART)
  {
    /* Enable UART clock */
    COM_CM33_CLK_ENABLE();

    if (!COM_CM33_TX_GPIO_IS_CLK_ENABLED())
    {
      if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(96)))
      {
        COM_CM33_TX_GPIO_CLK_ENABLE();
      }
    }
    if (!COM_CM33_RX_GPIO_IS_CLK_ENABLED())
    {
      if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(96)))
      {
        COM_CM33_RX_GPIO_CLK_ENABLE();
      }
    }

    /* Configure USART Tx as alternate function */
    gpio_init_structure.Pin = COM_CM33_TX_PIN;
    gpio_init_structure.Alternate = COM_CM33_TX_AF;
    HAL_GPIO_Init(COM_CM33_TX_GPIO_PORT, &gpio_init_structure);

    /* Configure USART Rx as alternate function */
    gpio_init_structure.Pin = COM_CM33_RX_PIN;
    gpio_init_structure.Alternate = COM_CM33_RX_AF;
    HAL_GPIO_Init(COM_CM33_RX_GPIO_PORT, &gpio_init_structure);
  }
  else if (huart->Instance == COM_CM0PLUS_UART)
  {
    /* LPUART clock config */
    ResMgr_Request(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_LPUART1_ID);

    /* Enable Backup domain access */
    HAL_PWR_EnableBkUpD3Access();

    /* Enable USART clock */
    COM_CM0PLUS_TX_GPIO_CLK_ENABLE();
    COM_CM0PLUS_RX_GPIO_CLK_ENABLE();
    COM_CM0PLUS_CLK_ENABLE();

    /* Configure USART Tx as alternate function */
    gpio_init_structure.Pin = COM_CM0PLUS_TX_PIN;
    gpio_init_structure.Alternate = COM_CM0PLUS_TX_AF;
    HAL_GPIO_Init(COM_CM0PLUS_TX_GPIO_PORT, &gpio_init_structure);

    /* Configure USART Rx as alternate function */
    gpio_init_structure.Pin = COM_CM0PLUS_RX_PIN;
    gpio_init_structure.Alternate = COM_CM0PLUS_RX_AF;
    HAL_GPIO_Init(COM_CM0PLUS_RX_GPIO_PORT, &gpio_init_structure);
  }

#elif defined(CORE_CM0PLUS)
  HAL_PWR_EnableBkUpAccess();
  /* Configure USART Tx as alternate function */
  gpio_init_structure.Pin = COM_CM0PLUS_TX_PIN;
  gpio_init_structure.Alternate = COM_CM0PLUS_TX_AF;
  HAL_GPIO_Init(COM_CM0PLUS_TX_GPIO_PORT, &gpio_init_structure);

  /* Configure USART Rx as alternate function */
  gpio_init_structure.Pin = COM_CM0PLUS_RX_PIN;
  gpio_init_structure.Alternate = COM_CM0PLUS_RX_AF;
  HAL_GPIO_Init(COM_CM0PLUS_RX_GPIO_PORT, &gpio_init_structure);
#warning "Core M0 can't manage RCC clock config"
#endif /* CORE_CA35 || CORE_CM33 */
}

/**
  * @brief  Initialize USART6 Msp part
  * @param  huart UART handle
  * @retval None
  */
static void USART_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef          gpio_init_structure;

#if defined (CORE_CA35) || defined (CORE_CM33)
  if (huart->Instance == COM_CA35_UART)
  {
    /* COM GPIO pin configuration */
    gpio_init_structure.Pin  = COM_CA35_TX_PIN;
    HAL_GPIO_DeInit(COM_CA35_TX_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin  = COM_CA35_RX_PIN;
    HAL_GPIO_DeInit(COM_CA35_RX_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable USART clock */
    COM_CA35_CLK_DISABLE();
  }
  else if (huart->Instance == COM_CM33_UART)
  {
    /* COM GPIO pin configuration */
    gpio_init_structure.Pin  = COM_CM33_TX_PIN;
    HAL_GPIO_DeInit(COM_CM33_TX_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin  = COM_CM33_RX_PIN;
    HAL_GPIO_DeInit(COM_CM33_RX_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable USART clock */
    COM_CM33_CLK_DISABLE();

    /* Release all hardware resources acquired using resource manager */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_UART5_ID);
    (void)ResMgr_Release(COM_CM33_RIF_RES_TYP_TX_PIN, COM_CM33_RIF_RES_NUM_TX_PIN);
    (void)ResMgr_Release(COM_CM33_RIF_RES_TYP_RX_PIN, COM_CM33_RIF_RES_NUM_RX_PIN);
  }
  else if (huart->Instance == COM_CM0PLUS_UART)
  {
    /* COM GPIO pin configuration */
    gpio_init_structure.Pin  = COM_CM0PLUS_TX_PIN;
    HAL_GPIO_DeInit(COM_CM0PLUS_TX_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin  = COM_CM0PLUS_RX_PIN;
    HAL_GPIO_DeInit(COM_CM0PLUS_RX_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable USART clock */
    COM_CM0PLUS_CLK_DISABLE();
  }

#elif defined(CORE_CM0PLUS)
  /* COM GPIO pin configuration */
  gpio_init_structure.Pin  = COM_CM0PLUS_TX_PIN;
  HAL_GPIO_DeInit(COM_CM0PLUS_TX_GPIO_PORT, gpio_init_structure.Pin);

  gpio_init_structure.Pin  = COM_CM0PLUS_RX_PIN;
  HAL_GPIO_DeInit(COM_CM0PLUS_RX_GPIO_PORT, gpio_init_structure.Pin);
#warning "Core M0 can't manage RCC clock config"
#endif /* CORE_CA35 || CORE_CM33 */
}
#endif /* USE_BSP_COM_FEATURE */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
