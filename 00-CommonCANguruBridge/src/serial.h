
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
const uint8_t rx1 = GPIO_NUM_16;
const uint8_t tx1 = GPIO_NUM_17;
const uint8_t frameLng = 13;
const uint8_t delimiterTrans = 0x00;
const uint8_t data = delimiterTrans + 0x01;
const uint8_t strng = data + 0x01;

unsigned long baudrate = 115200;

void initSerial()
{
  Serial1.begin(baudrate, SERIAL_8N1, rx1, tx1);
}

void displayStr(String str)
{
// Beginn der Übertragung
  Serial1.write(delimiterTrans);
// Stringübertragung
  Serial1.write(strng);
  Serial1.println(str);
// Ende der Datenübertragung
  Serial1.write(delimiterTrans);
}

void displayIP(IPAddress ip)
{
  displayStr("ip");
}

/*************************** End of file ****************************/
#endif
