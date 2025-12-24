/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "usb_serial.h"
/* USER CODE END Includes */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* ===================== MAIN ===================== */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* APP vector table base */
  SCB->VTOR = 0x08010000;

  MX_GPIO_Init();

  /* ------ USB Like Bootloader ------ */
  CUSTOM_USB_Reconnect();
  USBD_Interface_fops_FS.Receive = CUSTOM_Receive_FS;
  MX_USB_DEVICE_Init();

  while (1)
  {
      /* Your application keeps running normally */
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
      HAL_Delay(200);

      /* -------- Command Handling -------- */
      uint8_t cmd;

      if (USB_ReadNoWait(&cmd, 1))
      {
          switch(cmd)
          {
          /* -------- IDENTIFY -------- */
          case 0x01:
          {
              uint8_t response[4] =
              {
                  0xFF,
                  0x00,   // means APPLICATION mode
                  0x01,
                  0x00
              };
              USB_Write(response, 4);
              break;
          }

          /* -------- REQUEST BOOTLOADER -------- */
          case 0x05:
          {
              uint8_t ok = 0xFF;
              USB_Write(&ok, 1);

              __HAL_RCC_BKP_CLK_ENABLE();
              HAL_PWR_EnableBkUpAccess();

              *(uint32_t*)(BKP_BASE + 4) = 0x5555;  // tell bootloader to stay

              HAL_NVIC_SystemReset();
              break;
          }

          /* -------- RESET ONLY -------- */
          case 0x08:
          {
              uint8_t ok = 0xFF;
              USB_Write(&ok, 1);
              HAL_NVIC_SystemReset();
              break;
          }

          /* ignore other codes */
          }
      }
  }
}

/* ===================== SYSTEM CLOCK ===================== */
void SystemClock_Config(void)
{
	 RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	    HAL_RCC_OscConfig(&RCC_OscInitStruct);

	    RCC_ClkInitStruct.ClockType =
	        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
	        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/* ===================== GPIO ===================== */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1){}
}
