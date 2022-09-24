
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <espnow.h>

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

    UART Raspberry Pi: Beschaltung und Pinbelegung
Soll eine Kommunikation vom Raspberry Pi zu einem anderen kompatiblem Gerät via UART aufgebaut werden, so müssen die entsprechenden UART-GPIO-Pins am Raspberry Pi beschalten werden. Zur Kommunikation benötigen wir lediglich 3 Pins am Pi:

Pin 14: TxD, Sendeleitung     -- Pin 14
Pin 15: RxD, Empfangsleitung  -- Pin 15
Pin 6: GND, Masse
*/

/*SERIAL**************************************************************************************************/
const String indent = "          ";

// SERIAL1
const uint8_t RX01 = GPIO_NUM_16; // RPI Pin 14
const uint8_t TX01 = GPIO_NUM_17; // RPI Pin 15

enum enumData
{
  data = 0x01,
  strng
};

enum arrayContent_t
{
  isEmpty,
  isFrame,
  isString,
  isused,
  isunused
};

const uint8_t offset = 0x10;
const uint8_t buflng = 0x80;

const unsigned long baudrate = 115200;
static bool serial_connected = false;

const uint8_t CAN_FRAME_ARR_SIZE = 20;
uint8_t cntCANFramesUsed = 0;
uint8_t arrayCANFramesUsed[CAN_FRAME_ARR_SIZE] = {isunused};
uint8_t CAN_FRAME_ARR[CAN_FRAME_ARR_SIZE][CAN_FRAME_SIZE] = {0};

const uint8_t CAN_STRING_ARR_SIZE = 20;
uint8_t cntCANStringsUsed = 0;
uint8_t arrayCANStringUsed[CAN_STRING_ARR_SIZE] = {isunused};
String CAN_STRING_ARR[CAN_STRING_ARR_SIZE] = {""};

const uint8_t ARRAYS_SIZE = CAN_FRAME_ARR_SIZE + CAN_STRING_ARR_SIZE;
uint8_t CAN_ARRAY_Used = 0;
arrayContent_t CAN_ARRAY[ARRAYS_SIZE] = {isEmpty};

const uint16_t transmitDelay = 50;
const uint8_t delTime = 3;

void initSerial()
{
  // Starten der Schnittstelle
  // //////////////////// Serial1: RX Eingang: GPIO16; TX-Ausgang GPIO17;
  Serial1.begin(baudrate, SERIAL_8N1, RX01, TX01);
  Serial1.flush();
    // Startmeldung ausgeben
  Serial.println("Serial1 gestartet");
  serial_connected = false;
  cntCANFramesUsed = 0;
  cntCANStringsUsed = 0;
  CAN_ARRAY_Used = 0;
  memset(arrayCANFramesUsed, isunused, CAN_FRAME_ARR_SIZE);
  memset(arrayCANStringUsed, isunused, CAN_STRING_ARR_SIZE);
  memset(CAN_FRAME_ARR, 0x00, CAN_FRAME_ARR_SIZE);
  memset(CAN_STRING_ARR, 0x00, CAN_STRING_ARR_SIZE);
  memset(CAN_ARRAY, isEmpty, ARRAYS_SIZE);
}

// liefert die Info, ob ETHERNET aufgebaut ist
bool getSerialStatus()
{
  return serial_connected;
}

// setzt den Status, dass ETHERNET aufgebaut ist
void setSerialStatus(bool status)
{
  serial_connected = status;
}

void sendString(String str, bool newline = true, bool bindent = false)
{
  if (cntCANStringsUsed < CAN_STRING_ARR_SIZE)
  {
    // den nächsten freien Arrayplatz suchen
    for (uint8_t p = 0; p < CAN_STRING_ARR_SIZE; p++)
    {
      if (arrayCANStringUsed[p] == isunused)
      {
        arrayCANStringUsed[p] = isused;
        cntCANStringsUsed++;
        CAN_ARRAY[CAN_ARRAY_Used] = isString;
        CAN_ARRAY_Used++;
        if (bindent)
          CAN_STRING_ARR[p] = indent;
        CAN_STRING_ARR[p] += str;
        if (newline)
          CAN_STRING_ARR[p] += '\n';
        break;
      }
    }
  }
  else
    Serial.println("cntCANStringsUsed - Überlauf!!");
}

void transmitString()
{
  delay(transmitDelay);
  // den nächsten belegten Arrayplatz suchen
  for (uint8_t p = 0; p < CAN_STRING_ARR_SIZE; p++)
  {
    if (arrayCANStringUsed[p] == isused)
    {
      arrayCANStringUsed[p] = isunused;
      cntCANStringsUsed--;
      CAN_ARRAY[CAN_ARRAY_Used] = isEmpty;
      CAN_ARRAY_Used--;
      Serial1.write(strng);
      for (uint8_t i = 0; i < CAN_STRING_ARR[p].length(); i++)
      {
        Serial1.write(CAN_STRING_ARR[p].charAt(i));
      //  Serial.print(CAN_STRING_ARR[p].charAt(i), HEX);
      }
      //        Serial.println("<: " + String(p));
      Serial1.write('\0');
      // Ende der Datenübertragung
      break;
    }
  }
}

void sendCANFrame(uint8_t *frame)
{
  if (cntCANFramesUsed < CAN_FRAME_ARR_SIZE)
  {
    // den nächsten freien Arrayplatz suchen
    for (uint8_t p = 0; p < CAN_FRAME_ARR_SIZE; p++)
    {
      if (arrayCANFramesUsed[p] == isunused)
      {
        arrayCANFramesUsed[p] = isused;
        cntCANFramesUsed++;
        CAN_ARRAY[CAN_ARRAY_Used] = isFrame;
        CAN_ARRAY_Used++;
        memcpy(&CAN_FRAME_ARR[p][0x00], frame, CAN_FRAME_SIZE);
        //        Serial.println(">: "+String(p));
        break;
      }
    }
  }
  else
    Serial.println("cntCANFramesUsed - Überlauf!!");
}

void transmitFrame()
{
  /*
  char inBuffer[buflng];
  delay(transmitDelay);
  // den nächsten belegten Arrayplatz suchen
  for (uint8_t p = 0; p < CAN_FRAME_ARR_SIZE; p++)
  {
    if (arrayCANFramesUsed[p] == isused)
    {
      arrayCANFramesUsed[p] = isunused;
      cntCANFramesUsed--;
      CAN_ARRAY[CAN_ARRAY_Used] = isEmpty;
      CAN_ARRAY_Used--;
      //        Serial.println("<: " + String(p));
      uint8_t lng = sprintf(inBuffer, "%02X(%02X)%02X%02X[%02X]%02X%02X%02X%02X%02X%02X%02X%02X",
                            CAN_FRAME_ARR[p][0x00], CAN_FRAME_ARR[p][0x01],
                            CAN_FRAME_ARR[p][0x02], CAN_FRAME_ARR[p][0x03],
                            CAN_FRAME_ARR[p][0x04], CAN_FRAME_ARR[p][0x05],
                            CAN_FRAME_ARR[p][0x06], CAN_FRAME_ARR[p][0x07],
                            CAN_FRAME_ARR[p][0x08], CAN_FRAME_ARR[p][0x09],
                            CAN_FRAME_ARR[p][0x0A], CAN_FRAME_ARR[p][0x0B],
                            CAN_FRAME_ARR[p][0x0C]);
      write1Byte2Serial1(data);
      for (uint8_t i = 0; i < lng; i++)
      {
        write1Byte2Serial1(inBuffer[i]);
        //          Serial.print(inBuffer[i], HEX);
      }
      //        Serial.println("<: " + String(p));
      write1Byte2Serial1('\n');
      write1Byte2Serial1('\0');
      // Ende der Datenübertragung
      break;
    }
  }
  */
//  delay(transmitDelay);
  // den nächsten belegten Arrayplatz suchen
  for (uint8_t p = 0; p < CAN_FRAME_ARR_SIZE; p++)
  {
    if (arrayCANFramesUsed[p] == isused)
    {
      arrayCANFramesUsed[p] = isunused;
      cntCANFramesUsed--;
      CAN_ARRAY[CAN_ARRAY_Used] = isEmpty;
      CAN_ARRAY_Used--;
      Serial1.write(data);
      for (uint8_t i = 0; i < CAN_FRAME_SIZE; i++)
      {
        Serial1.write(CAN_FRAME_ARR[p][i]);
      }
      // Ende der Datenübertragung
      break;
    }
  }
}

void transmitData()
{
  while (CAN_ARRAY_Used > 0)
  {
    // der älteste Eintrag zuerst
    switch (CAN_ARRAY[0x00])
    {
    case isString:
      transmitString();
      break;
    case isFrame:
      transmitFrame();
      break;

    default:
      break;
    }
    // reorg
    for (uint8_t i = 0; i < CAN_ARRAY_Used; i++)
    {
      CAN_ARRAY[i] = CAN_ARRAY[i+1];
    }
    CAN_ARRAY[CAN_ARRAY_Used] = isEmpty;
  }
}

int serialDataAvail()
{
  return Serial1.available();
}

void serialRead(uint8_t *buffer)
{
  char bytes[] = "00";
  uint8_t uindex;
  if (Serial1.read() != data)
    return;
  for (uint8_t i = 0; i < CAN_FRAME_SIZE; i++)
  {
    uindex = Serial1.read() - offset;
    bytes[0] = Serial1.read();
    bytes[1] = Serial1.read();
    char *pEnd;
    buffer[uindex] = strtol(bytes, &pEnd, 16);
    //    Serial.print(buffer[uindex], HEX);
  }
}

/*
void displayIP(IPAddress ip)
{
  sendString(false, "ip");
}

*/