/*
 * usb_serial.h
 *
 *  Created on: Feb 25, 2025
 *      Author: Administrator
 */

#ifndef MY_USB_SERIAL_H_
#define MY_USB_SERIAL_H_

extern void CUSTOM_USB_Reconnect();
extern uint16_t USB_FifoCount();
extern uint8_t USB_ReadNoWait(void *dataPtr, uint16_t dataLength);
extern uint8_t USB_Read(void *dataPtr, uint16_t dataLength);
extern uint8_t USB_Write(void* data, uint16_t size);
extern int8_t CUSTOM_Receive_FS(uint8_t* Buf, uint32_t *Len);

#endif /* MY_USB_SERIAL_H_ */
