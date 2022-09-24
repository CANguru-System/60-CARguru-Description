
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
#include <CANguruDefs.h>
#include "CANguru.h"
//#include "utils.h"
#include <serial2raspi.h>

uint8_t slaveCnt;
uint8_t slaveCurr;
decoderStruct gate;

// willkürlich festgelegte MAC-Adresse
const uint8_t masterCustomMac[] = {0x30, 0xAE, 0xA4, 0x89, 0x92, 0x71};

slaveInfoStruct slaveInfo[maxSlaves];
slaveInfoStruct tmpSlaveInfo;
esp_now_peer_info_t cand;
String ssidSLV = "CNgrSLV";

bool bSendLokBuffer;
bool SYSseen;
uint8_t cntConfig;
uint8_t Clntbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,

// identifiziert einen Slave anhand seiner UID
uint8_t matchUID(uint8_t *buffer)
{
  uint8_t slaveCnt = get_slaveCnt();
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    uint32_t uid = UID_BASE + s;
    if (
        (buffer[5] == (uint8_t)(uid >> 24)) &&
        (buffer[6] == (uint8_t)(uid >> 16)) &&
        (buffer[7] == (uint8_t)(uid >> 8)) &&
        (buffer[8] == (uint8_t)uid))
      return s;
  }
  return 0xFF;
}

// kopiert die Struktur slaveInfoStruct von source nach dest
void cpySlaveInfo(slaveInfoStruct dest, slaveInfoStruct source)
{
  dest.slave = source.slave;
  dest.peer = source.peer;
  dest.initialData2send = source.initialData2send;
  dest.gotHandshake = source.gotHandshake;
  dest.no = source.no;
  dest.decoderType = source.decoderType;
}

// ESPNow wird initialisiert
void espInit()
{
  slaveCnt = 0;
  slaveCurr = 0;
  bSendLokBuffer = false;
  initVariant();
  if (esp_now_init() == ESP_OK)
  {
    sendString("ESP_Now started!");
  }
  else
  {
    sendString("ESP_Now INIT FAILED....");
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

// gibt die Anzahl der gefundenen Slaves zurück
uint8_t get_slaveCnt()
{
  return slaveCnt;
}

// gibt an, ob SYS gestartet ist
bool get_SYSseen()
{
  return SYSseen;
}

// setzt, dass SYS gestartet ist
void set_SYSseen(bool SYS)
{
  SYSseen = SYS;
}

// setzt die Variable cntConfig auf Null
void set_cntConfig()
{
  cntConfig = 0;
}

// fordert einen Slave dazu auf, Anfangsdaten bekannt zu geben
void set_initialData2send(uint8_t slave)
{
  slaveInfo[slave].initialData2send = true;
}

// quittiert, dass Anfangsdaten übertragen wurden
void reset_initialData2send(uint8_t slave)
{
  slaveInfo[slave].initialData2send = false;
}

// gibt zurück, ob noch Anfangsdaten von einem Slave zu liefern sind
bool get_initialData2send(uint8_t slave)
{
  return slaveInfo[slave].initialData2send;
}
// gibt zurück, um welchen Decodertypen es sich handelt
uint8_t get_decoder_type(uint8_t no)
{
  return slaveInfo[no].decoderType;
}

// liefert die Struktur slaveInfo zurück
slaveInfoStruct get_slaveInfo(uint8_t slave)
{
  return slaveInfo[slave];
}

// gibt eine MAC-Adresse aus
void printMac(uint8_t m[macLen])
{
  char macStr[20] = {0};
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  sendString(macStr);
}

// vergleicht zwei MAC-Adressen
bool macIsEqual(uint8_t m0[macLen], uint8_t m1[macLen])
{
  for (uint8_t ii = 0; ii < macLen; ++ii)
  {
    if (m0[ii] != m1[ii])
    {
      return false;
    }
  }
  return true;
}

// Wir suchen nach Slaves
// Scannt nach Slaves
void Scan4Slaves()
{
  int8_t scanResults = WiFi.scanNetworks();
  if (scanResults == 0)
  {
    sendString("Noch kein WiFi Gerät im AP Modus gefunden");
  }
  else
  {
    for (int i = 0; i < scanResults; ++i)
    {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      // Prüfen ob der Netzname mit `Slave` beginnt
      if (SSID.indexOf(ssidSLV) == 0)
      {
        // Ja, dann haben wir einen Slave gefunden
        // MAC-Adresse aus der BSSID ses Slaves ermitteln und in der Slave info struktur speichern
        int mac[macLen];
        if (macLen == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
        {
          for (int ii = 0; ii < macLen; ++ii)
          {
            cand.peer_addr[ii] = (uint8_t)mac[ii];
          }
        }
        bool notAlreadyFound = true;
        for (uint8_t s = 0; s < slaveCnt; s++)
        {
          if (macIsEqual(slaveInfo[s].slave.peer_addr, cand.peer_addr) == true)
          {
            notAlreadyFound = false;
            break;
          }
        }
        if (notAlreadyFound == true)
        {
          memcpy(&slaveInfo[slaveCnt].slave.peer_addr, &cand.peer_addr, macLen);
          slaveInfo[slaveCnt].slave.channel = WIFI_CHANNEL;
          slaveInfo[slaveCnt].slave.encrypt = 0;
          slaveInfo[slaveCnt].peer = &slaveInfo[slaveCnt].slave;
          slaveInfo[slaveCnt].gotHandshake = false;
          slaveInfo[slaveCnt].initialData2send = false;
          slaveInfo[slaveCnt].no = slaveCnt;
          slaveCnt++;
        }
      }
    }
  }
  // clean up ram
  WiFi.scanDelete();
}

// setzt die vorgegebene MAC-Adresse des Masters
void initVariant()
{
  WiFi.mode(WIFI_MODE_STA);
  esp_err_t setMacResult = esp_wifi_set_mac((wifi_interface_t)ESP_IF_WIFI_STA, &masterCustomMac[0]);
  if (setMacResult == ESP_OK)
  {
    sendString("Init Variant ok!");
  }
  else
  {
    sendString("Init Variant failed!");
  }
  WiFi.disconnect();
}

// addiert und registriert gefundene Slaves
void addSlaves()
{
  macAddressesstruct macAddresses[maxSlaves];
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    if (esp_now_add_peer(slaveInfo[s].peer) == ESP_OK)
    {
      for (uint8_t i = 0; i < macLen; i++)
      {
        macAddresses[s].peer_addr[i] = slaveInfo[s].slave.peer_addr[i];
      }
      macAddresses[s].mac = 0;
      for (uint8_t m = 0; m < macLen - 1; m++)
      {
        macAddresses[s].mac += macAddresses[s].peer_addr[m];
        macAddresses[s].mac = macAddresses[s].mac << 8;
      }
      macAddresses[s].mac += macAddresses[s].peer_addr[macLen - 1];
    }
  }
  // bubblesort
  uint8_t peer_addr[macLen];
  for (uint8_t s = 0; s < (slaveCnt - 1); s++)
  {
    for (int o = 0; o < (slaveCnt - (s + 1)); o++)
    {
      if (macAddresses[o].mac > macAddresses[o + 1].mac)
      {
        //
        uint64_t t = macAddresses[o].mac;
        memcpy(peer_addr, macAddresses[o].peer_addr, macLen);
        cpySlaveInfo(tmpSlaveInfo, slaveInfo[o]);
        //
        macAddresses[o].mac = macAddresses[o + 1].mac;
        memcpy(macAddresses[o].peer_addr, macAddresses[o + 1].peer_addr, macLen);
        cpySlaveInfo(slaveInfo[o], slaveInfo[o + 1]);
        //
        macAddresses[o + 1].mac = t;
        memcpy(macAddresses[o + 1].peer_addr, peer_addr, macLen);
        cpySlaveInfo(slaveInfo[o], tmpSlaveInfo);
      }
    }
  }
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    printMac(slaveInfo[s].slave.peer_addr);
    char chs[30];
    sprintf(chs, " -- Added Slave %d", s + 1);
    sendString(chs);
  }
}

void findSlaves()
{
  uint8_t Clntbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
  Scan4Slaves();
  addSlaves();
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    for (uint8_t cnt = 0; cnt < macLen; cnt++)
      Clntbuffer[cnt] = slaveInfo[s].slave.peer_addr[cnt];
    // device-Nummer übermitteln
    Clntbuffer[macLen] = s;
    sendTheData(s, Clntbuffer, macLen + 1);
  }
}

/*
// steuert den Registrierungsprozess der Slaves
void espNowProc()
{
  if (time4Scanning == true)
  {
    Scan4Slaves();
  }
  if (time4Scanning == false && slaveCnt > 0 && waiting4Handshake == true)
  {
    // add slaves
    addSlaves();
    uint8_t Clntbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
    for (uint8_t s = 0; s < slaveCnt; s++)
    {
      for (uint8_t cnt = 0; cnt < macLen; cnt++)
        Clntbuffer[cnt] = slaveInfo[s].slave.peer_addr[cnt];
      // device-Nummer übermitteln
      Clntbuffer[macLen] = s;
      sendTheData(s, Clntbuffer, macLen + 1);
    }
    delay(50);
  }
}
*/
// Überprüft, ob alle Slaves erkannt wurden und macht dann das Handshaking
void AllSlavesOnboard(const uint8_t *data)
{
  uint8_t d[macLen];
  for (uint8_t cnt = 0; cnt < macLen; cnt++)
  {
    d[cnt] = data[cnt];
  }
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    if (macIsEqual(slaveInfo[s].slave.peer_addr, d) == true)
    {
      slaveInfo[s].gotHandshake = true;
      break;
    }
  }
  bool handshakeFinished = true;
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    if (slaveInfo[s].gotHandshake == false)
    {
      handshakeFinished = false;
      break;
    }
  }
  if (handshakeFinished == true)
  {
    sendString(String(slaveCnt) + " slave(s) on board!");
  }
}

// Fehlermeldungen
void printESPNowError(esp_err_t Result)
{
  if (Result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    // How did we get so far!!
    sendString("ESPNOW not Init.");
  }
  else if (Result == ESP_ERR_ESPNOW_ARG)
  {
    sendString("Invalid Argument");
  }
  else if (Result == ESP_ERR_ESPNOW_INTERNAL)
  {
    sendString("Internal Error");
  }
  else if (Result == ESP_ERR_ESPNOW_NO_MEM)
  {
    sendString("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (Result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    if (slaveCnt > 0)
    {
      sendString("Peer not found!");
    }
  }
  else if (Result == ESP_ERR_ESPNOW_IF)
  {
    sendString("Interface Error.");
  }
  else
  {
    int res = Result;
    char chs[30];
    sprintf(chs, "\r\nNot sure what happened\t%d", res);
    sendString(chs);
  }
}

// sendet daten über ESPNow
void sendTheData(uint8_t slave, const uint8_t *data, size_t len)
{
  esp_err_t sendResult = esp_now_send(slaveInfo[slave].slave.peer_addr, data, len);
  if (sendResult != ESP_OK)
  {
    printESPNowError(sendResult);
  }
  delay(25);
}

// nach dem Versand von Meldungen können hier Fehlermeldungen abgerufen werden
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
}

// empfängt Daten über ESPNow
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  if (data_len == macLen)
  {
    AllSlavesOnboard(data);
  }
  else
  {
    memcpy(Clntbuffer, data, data_len);
    switch (data[0x01])
    {
    case PING_R:
      sendout2GW(Clntbuffer, fromClnt);
      for (uint8_t s = 0; s < slaveCnt; s++)
      {
        uint8_t m[macLen];
        for (uint8_t cnt = 0; cnt < macLen; cnt++)
        {
          m[cnt] = mac_addr[cnt];
        }
        if (macIsEqual(slaveInfo[s].slave.peer_addr, m))
        {
          slaveInfo[s].decoderType = data[12];
        }
      }
      break;
    case CONFIG_Status_R:
      sendout2GW(Clntbuffer, fromClnt);
      break;
    case SEND_IP_R:
      sendout2GW(Clntbuffer, fromClnt);
      if (!SYSseen)
      {
        cntConfig++;
        if (cntConfig == slaveCnt)
          goSYS();
      }
      break;
    case S88_EVENT_R:
      // Meldungen vom Gleisbesetztmelder
      // an das Gateway
      sendout2GW(Clntbuffer, fromClnt);
      // nur wenn Win-DigiPet gestartet ist
      break;
    default:
      // send received data via Ethernet to GW and evtl to SYS
      sendout2GW(Clntbuffer, fromClnt);
      break;
    }
  }
}
