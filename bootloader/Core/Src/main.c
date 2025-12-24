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

/* USER CODE BEGIN PV */

// 0 = MAIN APPLICATION
// 1 = BOOTLOADER
#define BOOTLOADER 1
#define APP_ADDRESS 0x08010000

/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Error_Handler(void);
void JumpToApplication(uint32_t appAddress);

/* ================= JUMP FUNCTION ================= */
void JumpToApplication(uint32_t appAddress)
{
    uint32_t appSP = *(volatile uint32_t*)appAddress;
    uint32_t appEntry = *(volatile uint32_t*)(appAddress + 4);

    __disable_irq();

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    for(int i=0;i<8;i++){
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    __set_MSP(appSP);
    SCB->VTOR = appAddress;

    __enable_irq();
    ((void (*)(void))appEntry)();
}

/* ================= MAIN ================= */
int main(void)
{
    if (BOOTLOADER)
    {
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_RCC_BKP_CLK_ENABLE();

        uint32_t bootFlag = *((uint32_t*)(BKP_BASE + 4));

        __HAL_RCC_PWR_CLK_DISABLE();
        __HAL_RCC_BKP_CLK_DISABLE();

        // If NO request to stay in bootloader → jump to app if valid
        if ((bootFlag != 0x5555) &&
            (*(uint32_t*)APP_ADDRESS != 0xFFFFFFFF))
        {
            JumpToApplication(APP_ADDRESS);
        }
    }

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    CUSTOM_USB_Reconnect();
    USBD_Interface_fops_FS.Receive = CUSTOM_Receive_FS;
    MX_USB_DEVICE_Init();

    while (1)
    {
        uint8_t cmd;

        if (USB_ReadNoWait(&cmd, 1))
        {
            switch (cmd)
            {
            /* DEVICE INFO */
            case 0x01:
            {
                uint8_t response[4] =
                {
                    0xFF,
                    BOOTLOADER,
                    0x01,
                    0x00
                };
                USB_Write(response, 4);
                break;
            }

            /* READ FLASH */
            case 0x02:
            {
                uint32_t readAddr;
                if (USB_Read(&readAddr, 4))
                {
                    uint32_t flashWord = *(uint32_t*)readAddr;
                    USB_Write(&flashWord, 4);
                }
                break;
            }

            /* ERASE APP FLASH ONLY */
            case 0x03:
            {
                HAL_FLASH_Unlock();

                FLASH_EraseInitTypeDef erase;
                uint32_t pageError;

                erase.TypeErase   = FLASH_TYPEERASE_PAGES;
                erase.PageAddress = APP_ADDRESS;
                erase.NbPages     = 32;    // NOT 64

                HAL_FLASHEx_Erase(&erase, &pageError);

                HAL_FLASH_Lock();

                uint8_t ok = 0xFF;
                USB_Write(&ok, 1);
                break;
            }


            /* WRITE FLASH */
            case 0x04:
            {
                uint32_t addr, data;
                if (USB_Read(&addr, 4) &&
                    USB_Read(&data, 4))
                {
                    HAL_FLASH_Unlock();
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
                    HAL_FLASH_Lock();

                    uint8_t ok = 0xFF;
                    USB_Write(&ok, 1);
                }
                break;
            }

            /* SWITCH MODE (legacy used by HTML) */
            case 0x05:
            {
                uint8_t ok = 0xFF;
                USB_Write(&ok, 1);

                __HAL_RCC_BKP_CLK_ENABLE();
                HAL_PWR_EnableBkUpAccess();

                if (BOOTLOADER)
                    *(uint32_t*)(BKP_BASE + 4) = 0;
                else
                    *(uint32_t*)(BKP_BASE + 4) = 0x5555;

                HAL_NVIC_SystemReset();
                break;
            }

            /* NEW → PURE RESET (DO NOT CHANGE FLAGS) */
            case 0x08:
            {
                uint8_t ok = 0xFF;
                USB_Write(&ok, 1);
                HAL_NVIC_SystemReset();
                break;
            }

            /* NEW → FORCE STAY IN BOOTLOADER NEXT RESET */
            case 0x09:
            {
                uint8_t ok = 0xFF;
                USB_Write(&ok, 1);

                __HAL_RCC_BKP_CLK_ENABLE();
                HAL_PWR_EnableBkUpAccess();
                *(uint32_t*)(BKP_BASE + 4) = 0x5555;

                HAL_NVIC_SystemReset();
                break;
            }
            }
        }
    }
}

/* ================== CubeMX Generated Code (UNCHANGED) ================== */

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

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
