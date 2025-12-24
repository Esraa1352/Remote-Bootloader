/*
 * usb_serial.c
 *
 *  Created on: Feb 25, 2025
 *      Author: Administrator
 */

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usb_serial.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

#define USB_FIFO_SIZE 2000

typedef struct {
	uint16_t ReceiveIndex;
	uint16_t ProcessIndex;
	uint8_t Fifo[USB_FIFO_SIZE];
	uint16_t Timeout;
	uint16_t CharTimeout;
} UsbTypeDef;

UsbTypeDef Usb = {
		.ReceiveIndex = 0,
		.ProcessIndex = 0,
		.Fifo = {0},
		.Timeout = 1000,
		.CharTimeout = 1
};

void CUSTOM_USB_Reconnect()
{
	GPIO_InitTypeDef pinCfg =
	{.Pin   = GPIO_PIN_12,
			.Mode  = GPIO_MODE_OUTPUT_PP,
			.Pull  = GPIO_PULLDOWN,
			.Speed = GPIO_SPEED_HIGH};

	HAL_GPIO_Init(GPIOA, &pinCfg);
	HAL_GPIO_WritePin(GPIOA,
			GPIO_PIN_12,
			GPIO_PIN_RESET);
	HAL_Delay(100);
}

int8_t CUSTOM_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	for (int i=0; i<*Len; i++)
	{
		Usb.Fifo[Usb.ReceiveIndex++] = Buf[i];
		if (Usb.ReceiveIndex == USB_FIFO_SIZE)
		{
			Usb.ReceiveIndex = 0;
		}
	}

	return (USBD_OK);
}

uint16_t USB_FifoCount()
{
	uint16_t ReceiveIndexNow = Usb.ReceiveIndex;

	if (ReceiveIndexNow >= Usb.ProcessIndex)
	{
		return ReceiveIndexNow - Usb.ProcessIndex;
	}
	else
	{
		return USB_FIFO_SIZE - Usb.ProcessIndex + ReceiveIndexNow;
	}
}

uint8_t USB_ReadNoWait(void *dataPtr, uint16_t dataLength)
{
	uint16_t fifoCountNow = USB_FifoCount();

	if (fifoCountNow >= dataLength)
	{

		for (int i=0; i<dataLength; i++)
		{
			((uint8_t*)dataPtr)[i] = Usb.Fifo[Usb.ProcessIndex++];

			if (Usb.ProcessIndex >= USB_FIFO_SIZE)
			{
				Usb.ProcessIndex -= USB_FIFO_SIZE;
			}
		}

		return 1;
	}

	return 0;
}

uint8_t USB_Read(void *dataPtr, uint16_t dataLength)
{
	if (USB_ReadNoWait(dataPtr, dataLength))
	{
		return 1;
	}
	else
	{
		int tmpTimeout = Usb.Timeout;

		while (tmpTimeout>0)
		{
			HAL_Delay(30);
			tmpTimeout -= 30;

			if (USB_ReadNoWait(dataPtr, dataLength))
			{
				return 1;
			}
		}

		Usb.ProcessIndex = Usb.ReceiveIndex;
		return 0;

	}

}

uint8_t USB_Write(void* data, uint16_t size)
{
	while (USBD_BUSY == CDC_Transmit_FS(data, size)) {};
	return 1;
}
