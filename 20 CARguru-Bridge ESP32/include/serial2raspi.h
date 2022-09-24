
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>


// Definition der beiden Schnittstellen
/*
 * There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.
 *
 * U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
 * U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
 * U2UXD is unused and can be used for your projects.
 *
 *
    UART	RX IO	  TX IO
    (UART0	GPIO3	  GPIO1)
    UART1	GPIO9	  GPIO10
    UART2	GPIO16	GPIO17
*/

/*SERIAL**************************************************************************************************/

void initSerial();
void sendString(String str, bool newline = true, bool bindent = false);
void sendCANFrame(uint8_t *frame);
//void displayIP(IPAddress ip);
int serialDataAvail();
void serialRead(uint8_t *buffer);
bool getSerialStatus();
void setSerialStatus(bool status);
void transmitData();

/*************************** End of file ****************************/
#endif
