
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */
#include <Arduino.h>

#include <serial2raspi.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>

#include "CANguruDefs.h"
#include <SPI.h>
#include <Wire.h>
#include <espnow.h>
#include "CANguru.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <miniz.h>
#include "SPIFFS.h"

//#define debug

// buffer for receiving and sending data
uint8_t M_PATTERN[] = {0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t cntLoks;

const uint8_t maxPackets = 30;
bool initialDataAlreadySent;

// forward declaration
char readConfig(char index);

// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------

void printMSG(uint8_t no)
{
  uint8_t MSG[] = {0x00, no, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendout2GW(MSG, MSGfromBridge);
}

#include <utils.h>

//////////////// Ausgaberoutinen

// sendet den Frame, auf den der Zeiger buffer zeigt, über ESPNow
// an alle Slaves
void send2AllClients(uint8_t *buffer)
{
  uint8_t slaveCnt = get_slaveCnt();
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    sendTheData(s, buffer, CAN_FRAME_SIZE);
  }
}

// wenn aus der UID des Frames ein Slave identifizierbar ist, wird der
// Frame nur an diesen Slave geschickt, ansonsten an alle
void send2OneClient(uint8_t *buffer)
{
  uint8_t ss = matchUID(buffer);
  if (ss == 0xFF)
    send2AllClients(buffer);
  else
    sendTheData(ss, buffer, CAN_FRAME_SIZE);
}

void sendout2GW(uint8_t *buffer, CMD cmd)
{
  //%
  buffer[0] = cmd;
  sendCANFrame(buffer);
  buffer[0] = 0x00;
}

void sendOutClnt(uint8_t *buffer, CMD dir)
{
  switch (buffer[0x01])
  {
  case CONFIG_Status:
    send2OneClient(buffer);
    break;
  case SYS_CMD:
    switch (buffer[0x09])
    {
    case SYS_STAT:
    case RESET_MEM:
    case START_OTA:
      send2OneClient(buffer);
      break;
    default:
      send2AllClients(buffer);
      break;
    }
    break;
  // send to all
  default:
    send2AllClients(buffer);
    break;
  }
  sendout2GW(buffer, dir);
}

// prüft, ob es Slaves gibt und sendet den CAN-Frame buffer dann an die Slaves
void proc2Clnts(uint8_t *buffer, CMD dir)
{
  // to Client
  if (get_slaveCnt() > 0)
  {
    sendOutClnt(buffer, dir);
  }
}

//////////////// Empfangsroutinen

// Behandlung der Kommandos, die der CANguru-Server aussendet
void proc_fromMASTER2Clnt()
{
  uint8_t UDPbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
  if (serialDataAvail() >= CAN_FRAME_SIZE_SERIAL)
  // if there's data available, read a packet
  {
    // read the packet into packetBuffer
    serialRead(UDPbuffer);
    // send received data via usb and CAN
      Serial.print("CMD: ");
      Serial.println(UDPbuffer[0x1], HEX);
    switch (UDPbuffer[0x1])
    {
    case SYS_CMD:
    case 0x36:
    case 0x02:
    case 0x04:
    case 0x06:
    case PING:
    case SEND_IP:
    case Lok_Function:
    case Lok_Direction:
    case Lok_Speed:
    case CONFIG_Status:
      proc2Clnts(UDPbuffer, toClnt);
      break;
    case ReadConfig:
    case WriteConfig:
      break;
    case restartBridge:
      proc2Clnts(UDPbuffer, toClnt);
      for (uint8_t i = 0; i < 50; i++)
      {
        if (i % 2 == 0)
          digitalWrite(BUILTIN_LED, HIGH);
        else
          digitalWrite(BUILTIN_LED, LOW);
      delay(20);
      }
      ESP.restart();
      break;
    }
  }
}

//////////////// Startroutine

// Standardroutine; diverse Anfangsbedingungen werden hergestellt
void setup()
{
  Serial.begin(bdrMonitor);
  //  Serial.setDebugOutput(true);
  Serial.println("\r\n\r\nC A N g u r u - B r i d g e - " + CgVersionnmbr);
  // das Display wird initalisiert
  initSerial();
  setSerialStatus(false);
  // noch nicht nach Slaves scannen
  //  set_time4Scanning(false);
  // der Timer wird initialisiert
  stillAliveBlinkSetup();
  // start the raspiClient
}

// die Routin antwortet auf die Anfrage des CANguru-Servers mit CMD 0x88;
// damit erhält er die IP-Adresse der CANguru-Bridge und kann dann
// damit eine Verbindung aufbauen
void proc_startSystem()
{
  uint8_t UDPbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
  // if there's data available, read a packet
  if (serialDataAvail() >= CAN_FRAME_SIZE_SERIAL)
  {
    // read the packet into packetBuffer
    serialRead(UDPbuffer);
    // send received data via ETHERNET
    switch (UDPbuffer[0x1])
    {
    case CALL4CONNECT:
      delay(5);
      sendString("CANguru-Bridge " + CgVersionnmbr);
      sendString("From (own)IP ");
      sendString("Connected to "); // + ip2strng(tlntClnt.remoteIP()));
      sendString("");              // + ip2strng(tlntClnt.remoteIP()));
      espInit();
      set_SYSseen(false);
      set_cntConfig();
      stillAliveBlinking();
      findSlaves();
      delay(100);
      UDPbuffer[0x05] = get_slaveCnt();
      UDPbuffer[0x01]++;
      sendout2GW(UDPbuffer, toMaster);
      setSerialStatus(true);
    }
  }
}

// Meldung, dass SYS gestartet werden kann
void goSYS()
{
  printMSG(StartTrainApplication);
}

// standard-Hauptprogramm
void loop()
{
  // die folgenden Routinen werden ständig aufgerufen
  if (getSerialStatus())
  {
    transmitData();
    proc_fromMASTER2Clnt();
  }
  else
  {
    proc_startSystem();
  }
}
