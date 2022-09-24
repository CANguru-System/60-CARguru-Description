#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

//#define IS_TEST
#define IS_NO_TEST

const uint8_t dest = 0x00;
const uint8_t opCmd = 0x01;
const uint8_t hash0 = 0x02;
const uint8_t hash1 = 0x03;
const uint8_t dataLength = 0x04;
const uint8_t data0 = 0x05;
const uint8_t data1 = 0x06;
const uint8_t data2 = 0x07;
const uint8_t data3 = 0x08;
const uint8_t data4 = 0x09;
const uint8_t data5 = 0x0A;
const uint8_t data6 = 0x0B;
const uint8_t data7 = 0x0C;


#define buflng 0x80
#define buflng10 0x10
#define buflng20 0x20
#define buflng30 0x30
#define buflng40 0x40
#define buflng 0x80

#define maxcanguru 20

const long baudrate = 115200;
const long i2c_address = 0x48;

#define window_height 700
#define window_width window_height / 6 * 9

#define upperEdge 0x00
#define leftEdge window_width * 0.005
#define firstBox window_width * 0.05
#define rightEdge window_width * 0.9

#define frame_offset 5

const uint16_t uid_lng = 0x04;

struct kanal_t
{
  uint8_t kanalnbr;
  uint8_t grundformat; // Kenner Auswahlliste (1); Kenner Slider (2)
  uint16_t untererWert;
  uint16_t obererWert;
  uint16_t aktuellerWert;
  gchar auswahlbezeichnung[buflng40];
  gchar start[buflng40];
  gchar ende[buflng40];
  gchar einheit[buflng40];
  gchar newValue[buflng10];
};

struct canguru_t
{
  uint8_t IP[4];
  uint8_t UID[uid_lng];
  uint8_t HASH[2];
  uint8_t numberofchannels;
  uint8_t device_kind;
  uint16_t adr;
  gchar name[buflng40];
  std::vector<kanal_t> Kanaele;
};
struct func_t
{
  uint16_t timer_number;
  uint16_t nbr;
  uint8_t fction;
};

enum press_t
{
  pressed,
  released
};

enum send_frames_t
{
  ping = 0x00,
  ip,
  start,
  restart
};

enum direction_t
{
  no_dir,
  faster,
  slower,
  stop
};

enum pagesData
{
  frames = 0x00,
  configure,
  play,
  empty
};

enum enumData
{
  no_data = 0x00,
  data,
  strng
};

enum status_input_t
{
  waiting,
  dataBytes
};

// Kommunikationslinien
enum commDir
{
  toBridge,
  fromBridge,
  toMaster,
  fromMaster,
  toClnt,
  fromClnt,
  MSGfromBridge
};

// CAR-BUTTONS
enum btns
{
  /*F00*/ Abblendlicht,
  /*F01*/ Blinker_links,
  /*F02*/ Blinker_rechts,
  /*F03*/ Warnblinker,
  /*F04*/ Fernlicht,
  /*F05*/ Lichthupe_TASTER,
  /*F06*/ Hupe_TASTER,
  /*F07*/ Martinshorn,
  /*F08*/ Frontblinker,
  /*F09*/ go_slower,
  /*F10*/ STOP,
  /*F11*/ go_faster,
  no_btn
};

#define SOCKET_ERROR (-1)
struct IPv4
{
  unsigned char b1, b2, b3, b4;
};

/*
 * CAN-Befehle (Märklin)
 */
#define SYS_CMD 0x00 // Systembefehle
#define SYS_CMD_R SYS_CMD + 1
#define SYS_GO 0x01 // System - Go
#define LokDiscovery 0x02
#define LokDiscovery_R LokDiscovery + 1
#define MFXVerify 0x07
#define MFXVerify_R MFXVerify + 1
#define Lok_Speed 0x08
#define Lok_Speed_R Lok_Speed + 1
#define Lok_Direction 0x0A
#define Lok_Function 0x0C
#define SYS_STAT 0x0B // System - Status (sendet geänderte Konfiguration)
#define ReadConfig 0x0E
#define ReadConfig_R ReadConfig + 1
#define WriteConfig 0x10
#define WriteConfig_R WriteConfig + 1
#define SWITCH_ACC 0x16   // Magnetartikel schalten
#define SWITCH_ACC_R 0x17 // Magnetartikel schalten
#define S88_Polling 0x20
#define S88_EVENT 0x22            // Rückmelde-Event
#define S88_EVENT_R S88_EVENT + 1 // Rück-Rückmelde-Event
#define PING 0x30                 // CAN-Teilnehmer anpingen
#define PING_R PING + 1           // CAN-Teilnehmer anpingen
#define CONFIG_Status 0x3A
#define CONFIG_Status_R CONFIG_Status + 1
#define ConfigDataCompressed 0x40
#define ConfigDataCompressed_R ConfigDataCompressed + 0x01
#define ConfigDataStream 0x42

/*
 * CAN-Befehle (eigene)
 */
#define ConfigData 0x40
#define ConfigData_R 0x41
#define MfxProc 0x50
#define MfxProc_R 0x51
#define sendInitialData 0x52
#define LoadCS2Data 0x56
#define LoadCS2Data_R LoadCS2Data + 0x01        // 0x57
#define GETCONFIG_RESPONSE LoadCS2Data_R + 0x01 // 0x58
#define DoCompress GETCONFIG_RESPONSE + 0x02    // 0x5A
#define DoNotCompress DoCompress + 0x01         // 0x5B
#define GetGleisbildNames DoNotCompress + 0x01  // 0x5C
#define BlinkAlive 0x60
#define restartBridge 0x62
#define SEND_IP 0x64
#define SEND_IP_R SEND_IP + 1
#define INIT_COMPLETE 0x66
#define CALL4CONNECT 0x88
#define CALL4CONNECT_R CALL4CONNECT + 0x01
#define sendCntLokBuffer 0x90
#define sendCntLokBuffer_R 0x91
#define sendLokBuffer 0x92
#define sendLokBuffer_R 0x93
#define startConfig 0x94
#define RESET_MEM 0xFE
#define START_OTA 0xFF

#define DEVTYPE_GFP 0x0000
#define DEVTYPE_GB 0x0010
#define DEVTYPE_CONNECT 0x0020
#define DEVTYPE_MS2 0x0030
#define DEVTYPE_WDEV 0x00E0
#define DEVTYPE_CS2 0x00FF
#define DEVTYPE_FirstCANguru 0x004F
#define DEVTYPE_BASE 0x0050
#define DEVTYPE_TRAFFICLIGHT 0x0051
#define DEVTYPE_SERVO 0x0053
#define DEVTYPE_RM 0x0054
#define DEVTYPE_LIGHT 0x0055
#define DEVTYPE_SIGNAL 0x0056
#define DEVTYPE_LEDSIGNAL 0x0057
#define DEVTYPE_CANFUSE 0x0058
#define DEVTYPE_GATE 0x0059
#define DEVTYPE_CAR_CAR 0x005A
#define DEVTYPE_CAR_RM 0x005B
#define DEVTYPE_CAR_SERVO 0x005C
#define DEVTYPE_LastCANguru 0x005F

/*
 * Adressbereiche:
 */
#define MM_ACC 0x3000    // Magnetartikel Motorola
#define DCC_ACC 0x3800   // Magbetartikel NRMA_DCC
#define MM_TRACK 0x0000  // Gleissignal Motorola
#define DCC_TRACK 0xC000 // Gleissignal NRMA_DCC

#endif