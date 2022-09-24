
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */
/*
Autor: DG                                         ###############################   #####
#############################   #######
Sketchname: [ESP32]-LED-PWM                       ###                     ###   ###
Version: 1.0                                      ###                   ###   ###
letzte Aktualisierung: 2022-05-07                 ###                 ###   ###
###               ###   ###
Zweck: Steuerung von LEDs mittels PWM             ###             ###   ###
###           ###   ###       #########
Bibliotheken: keine                               ###         ###   ###         #########
###       ###   ###                 ###
Board-Support: ESP32 WROOM-32 und alle            ###     ###   ###                   ###
seine Derivate                     ###   ###   ###                     ###
#######   #############################
Geräte-Support: alle LEDs                         #####   ###############################

/* ====================================================================================== +
  |
E S P  3 2  -  Helligkeitssteuerung von LEDs mittels PWM                 |
  |
+ ======================================================================================= +
> > >   I N F O R M A T I O N   < < <                          |
+ --------------------------------------------------------------------------------------- +
* PWM                                                                                  |
PWM steht für Pulsweitenmodulation. Diese leistungsstarke Technik wird verwendet     |
zur Steuerung analoger Schaltungen über die digitalen Ausgänge des ESP32.            |
Diese digitale Steuerung erfolgt binär, das heißt :                                  |
  |
0: keine Spannung (0v)                                                               |
1: volle Spannung (3.3v oder 5v)                                                     |
  |
Durch dieses Ein-/Ausschalt-Muster kann ein Rechteckwellensignal erzeugt             |
werden :                                                                             |
  |
+--25%--+                       +--25%--+                       +---    --1          |
|       |                       |       |                       |                    |
+       +----------75%----------+       +----------75%----------+       --0          |
  |
|--duty-|-----------------------|--duty-|-----------------------|---                 |
|-------frequenz-(=>100%)-------|-------frequenz-(=>100%)-------|---                 |
  |
wird beispielsweise eine 25%-ige Helligkeit der LED benötigt, kann man :             |
1. die Spannung, welche der LED zugeführt wird, vierteln oder                        |
2. die flexible PWM-Technik verwenden, indem man ein Rechteck-(Wellen)Signal         |
welches um 75% kürzer ist als die eingestellte Frequenzwellenlänge(=> 100%)       |
  |
* ESP32 LED-PWM-Controller                                                             |
:: 16 interne unabhängige Kanäle (0 - 15), die derart konfigurierbar sind, dass      |
sie jeweils PWM-Signale mit unterschiedlichen Eigenschaften generieren können,       |
d.h. es können max. 16 Geräte pro Pin gesteuert werden.                              |
:: aber die Frequenz muss bei jedem Paar von Kanälen, die zu demselben Timer         |
geleitet werden, gleich sein. Wenn mehr als 8 verschiedene PWM-Signale mit           |
unterschiedlichen Frequenzen benötigt werden, sollte man sich nach einer externen    |
Lösung umsehen (z.B. I2C PWM Controller IC).                                         |
:: HighSpeed-Kanäle :                       :: LowSpeed-Kanäle :                     |
+---+                                       +---+                  |
|   |   +-------+                           |   |   +-------+      |
+----------+   |   |-->| h_ch0 |            +----------+   |   |-->| l_ch1 |      |
| h_timer0 |-->|   |   +-------+            | l_timer0 |-->|   |   +-------+      |
+----------+   |   |-->| h_ch1 |            +----------+   |   |-->| h_ch2 |      |
|   |   +-------+                           |   |   +-------+      |
|   |                                       |   |                  |
|   |   +-------+                           |   |   +-------+      |
+----------+   |   |-->| h_ch2 |            +----------+   |   |-->| h_ch3 |      |
| h_timer1 |-->|   |   +-------+            | l_timer1 |-->|   |   +-------+      |
+----------+   |   |-->| h_ch3 |            +----------+   |   |-->| h_ch4 |      |
| M |   +-------+                           | M |   +-------+      |
| u |                                       | u |                  |
| x |   +-------+                           | x |   +-------+      |
+----------+   |   |-->| h_ch4 |            +----------+   |   |-->| h_ch5 |      |
| h_timer2 |-->|   |   +-------+            | l_timer2 |-->|   |   +-------+      |
+----------+   |   |-->| h_ch5 |            +----------+   |   |-->| h_ch6 |      |
|   |   +-------+                           |   |   +-------+      |
|   |                                       |   |                  |
|   |   +-------+                           |   |   +-------+      |
+----------+   |   |-->| h_ch6 |            +----------+   |   |-->| h_ch7 |      |
| h_timer3 |-->|   |   +-------+            | l_timer3 |-->|   |   +-------+      |
+----------+   |   |-->| h_ch7 |            +----------+   |   |-->| h_ch8 |      |
|   |   +-------+                           |   |   +-------+      |
+---+                                       +---+                  |
  |
* LED                                                                                   |
:: eine Leuchtdiode (Light-Emitting Diode) ist ein sogenanntes Halbleiter-           |
Bauelement, das Licht ausstrahlt, wenn elektrischer Strom in Durchlassrichtung       |
fließt. In Gegenrichtung sperrt die LED. Somit entsprechen die elektrischen          |
Eigenschaften der LED denjenigen einer Diode. Die Wellenlänge des emittierten        |
Lichts hängt vom Halbleitermaterial und der Dotierung der Diode ab:                  |
das Licht kann für das menschliche Auge sichtbar oder aber im Bereich von            |
Ultraviolettstrahlung sein.                                                          |
:: beim Anlöten ist darauf zu achten, den Lötkolben keinesfalls länger als           |
4- 5 sec pro Anschlussbeinchen an die LED zu halten. Durch Überhitzung der LED       |
mit dem Lötkolben geht diese entweder kaputt oder verliert an Leistung.              |
:: die Kathode (−) ist entweder durch das kürzere Anschluss(bein) oder durch die     |
Abflachung am Sockel des LED-Gehäuses deutlich gekennzeichnet.                       |
Eselsbrücke: (K)athode = (k)urz = (K)ante.                                           |
:: die simpelste Möglichkeit der Versorgung einer LED an einer Spannungsquelle       |
ist, in Reihe zu ihr einen Vorwiderstand zu schalten. Wird diese Anordnung mit       |
einer Spannungsquelle betrieben, deren Spannung unter Last bekannt ist, so           |
lässt sich der gewünschte Strom über die Wahl des Widerstandes einstellen :          |
  |
R    ^= Widerstand [Ohm],                                                            |
U0   ^= Spannung Spannungsquelle [Volt]                                              |
Uled ^= max.Spannung LED [Volt], wobei                                               |
infrarot:     1.2 – 1.8 V, typ. 1.3 V                                        |
rot:          1.6 – 2.2 V                                                    |
gelb, grün:   1.9 – 2.5 V                                                    |
blau, weiß:   2.7 – 3.5 V                                                    |
ultraviolett: 3.1 – 4.5 V, typ. 3.7 V                                        |
I    ^= Stromstärke [Ampère]                                                         |
  |
+----------------------------+                                                       |
|    R = (U0 - Uled) / I     |    <-- Vorwiderstand (Ohm)                            |
+----------------------------+                                                       |
|    P = U * I               |    <-- Leistung (Watt)                                |
+----------------------------+                                                       |
  |
Beispiel anhand einer roten LED :                                                    |
Spannungsquelle ESP32 GPIO :   3.3 V                                                 |
Nennspannung LED :             2.0 V                                                 |
Nennstromstärke LED :         25.0 mA (=> 0.025 A)                                   |
  |
Vorwiderstand (Ohm) :                                                                |
R = (3.3 - 2.0) / 0.025  =>   52 Ohm   =>  56 Ohm  (E12-Reihe)                       |
  |
Erfahrungswerte :                                                                    |
der ermittelte Wert ist 'lediglich' ein Hinweis auf den unteren Grenzwert; in        |
diesem Fall sollte der Wert des Widerstandes 56 Ohm nicht unterschreiten. Da         |
bei diesem Wert die LED zwar nicht durchbrennen würde, aber augenweh-grell           |
leuchten würde, nimmt man in der Regel Widerstandswerte zwischen 220 und 330 Ohm     |
Je höher der Wert, desto schwächer leuchtet die LED. (Dank an Bernd666)              |
  |
Leistung (Watt) :                                                                    |
P = 1.3 * 0.025  =>  0,0325 Watt                                                     |
  |
weitere Informationen :                                                              |
* https://arduino-hannover.de/2018/07/25/die-tuecken-der-esp32-stromversorgung       |
* https://www.electronicsplanet.ch/Widerstand/Widerstandsreihe-E12.htm               |
  |
+ --------------------------------------------------------------------------------------- +
|                    > > >   V O R G E H E N S W E I S E   < < <                          |
+ --------------------------------------------------------------------------------------- +
* Kommunikation :                                                                      |
:: einen Kanal auswählen, den man benutzen möchte => 0 // zwischen 0 und 15          |
  |
* Ansprechbarkeit :                                                                    |
:: PWM-Signalfrequenz setzen (LED: 5.000 Hz) => 5000   // zwischen 1000 und 5000     |
  |
* Kontrolle über die Helligkeit :                                                      |
:: Auflösung des sogenannten 'duty cycle'                                            |
festlegen (1-16 Bit) => 8                           // 10 ist auch ein guter Wert |
:: Kontrolle durch das Festlegen der                                                 |
LED-Helligkeitsstufe (0-255 <=> 8 Bit)              // 0 - 1024 (2^10) 10 Bit     |
  |
* Datenverarbeitung :                                                                  |
:: GPIO, über welchen Strom geliefert wird und die Signale                           |
verarbeitet werden sollen => 26 (DAC)               // oder 25 (DAC)              |
// oder alle anderen Ausgänge |
+ --------------------------------------------------------------------------------------- +
|                          > > >   V E R K A B E L U N G   < < <                          |
+ --------------------------------------------------------------------------------------- +
+--------------+----------------------------+----------------------+                   |
|    ESP 32    |            LED             |  WIDERSTAND 330 Ohm  |                   |
+--------------+----------------------------+----------------------+                   |
| GPIO 25      | Anode   (rot)              |                      |                   |
|              | Kathode (rot)              |           X          |                   |
| GND          |                            |           X          |                   |
+--------------+----------------------------+----------------------+                   |
| GPIO 26      | Anode   (gelb)             |                      |                   |
|              | Kathode (gelb)             |           X          |                   |
| GND          |                            |           X          |                   |
+--------------+----------------------------+----------------------+                   |
| GPIO 27      | Anode   (grün)             |                      |                   |
|              | Kathode (grün)             |           X          |                   |
| GND          |                            |           X          |                   |
+--------------+----------------------------+----------------------+                   |
  |
+ --------------------------------------------------------------------------------------- +
|                           > > >   S T Ü C K L I S T E   < < <                           |
+ --------------------------------------------------------------------------------------- +
1x   ESP 32                                                                            |
1x   microUSB-USB-Kabel                                                                |
1x   weltbestes Steckbrett (ELV 88532)                                                 |
1x   LED rot, gelb, grün                                                               |
3x   Widerstand 330 Ohm                                                                |
3x   Jumperkabel grün, gelb, rot, m-m                                                  |
3x   Jumperkabel schwarz, m-m                                                          |
+ ======================================================================================= +
|                                                                                         |
|                                 > > >   C O D E   < < <                                 |
|                                                                                         |
+ ======================================================================================= +
|                          > > >   D E K L A R A T I O N   < < <                          |
+ ---------------------------------------------------------------------------------------*/
/*const byte PWM_PIN_ROT = 25;                          // Steuerung rote LED
const byte PWM_PIN_GELB = 26;                         // Steuerung gelbe LED
const byte PWM_PIN_GRUEN = 27;                        // Steuerung grüne LED
const int PWM_KANAL = 0;                              // Standard-Kommunikationskanal
const int PWM_KANAL_GRUEN = 2;                        // zweiter Kommunikationskanal
const int PWM_FREQUENZ = 5000;                        // Hz
const byte PWM_AUFLOESUNG = 8;                        // Bit (=> 0 bis 255)
int pinHighTimeRot;                                   // PIN-Einschaltdauer (DutyCircle)
int pinHighTimeGelb;                                  //                .
int pinHighTimeGruen;  */
//                .
//   in Prozent im Verhältnis zum
//   Intervall des Taktes (Frequenz)
/*--------------------------------------------------------------------------------------- +
|                                > > >   S E T U P   < < <                                |
+ ---------------------------------------------------------------------------------------*/
/*void setup() {                                        // Die Setup-Funktion wird einmal
                                                      // ausgeführt, wenn auf dem Board die
                                                      // Reset-Taste gedrückt oder das Board
                                                      // eingeschaltet wird.
  ledcSetup(PWM_KANAL, PWM_FREQUENZ, PWM_AUFLOESUNG); // LED-PWM-Eigenschaften konfigurieren
  ledcSetup(PWM_KANAL_GRUEN, PWM_FREQUENZ, PWM_AUFLOESUNG);
  ledcAttachPin(PWM_PIN_ROT, PWM_KANAL);              // GPIO initialisieren
  ledcAttachPin(PWM_PIN_GELB, PWM_KANAL);             //
  ledcAttachPin(PWM_PIN_GRUEN, PWM_KANAL_GRUEN);      //
}*/
/*--------------------------------------------------------------------------------------- +
|                                 > > >   L O O P   < < <                                 |
+ ---------------------------------------------------------------------------------------*/
/*void loop() {*/ // die Loop-Funktion läuft
                  // fortwährend durch, solange der
                  // ESP32 eingeschaltet ist
/*--------------------- +
|   via 'ledcWrite()'   |
+ ---------------------*/
/*  ledcWrite(PWM_KANAL, 255);
  ledcWrite(PWM_KANAL_GRUEN, 0);
  delay(20000);
  ledcWrite(PWM_KANAL, 127);
  ledcWrite(PWM_KANAL_GRUEN, 255);
  delay(20000);
  ledcWrite(PWM_KANAL, 0);
  ledcWrite(PWM_KANAL_GRUEN, 127);
  delay(20000);*/
/*

  for(pinHighTimeRot = 0; pinHighTimeRot <= 255; pinHighTimeRot++) { // Helligkeit erhöhen
    ledcWrite(PWM_KANAL, pinHighTimeRot);                            // LED-Helligkeit ändern mittels PWM
    delay(15);
  }

  for(pinHighTimeRot = 255; pinHighTimeRot >= 0; pinHighTimeRot--) { // Helligkeit absenken
    ledcWrite(PWM_KANAL, pinHighTimeRot);
    delay(15);
  }
}*/
/*=================================================================================================================+
|                                                                                                                  |
|                                               H  I  N  W  E  I  S                                                |
|                                                                                                                  |
+==================================================================================================================+
ESP WROOM-32 :
:: der Befehl analogWrite() ist zwar im aktuellen Core 2.0.3 noch implementiert - wahrscheinlich aus Gründen der
   Abwärtskompatibilität, sollte jedoch nicht weiter verwendet werden.
:: stattdessen sollte man auf die LEDC-API zugreifen, um die Möglichkeiten des ESP32 besser ausschöpfen zu können.
:: LEDC-API (u.a.): ledcSetup(PWM_KANAL, PWM_FREQUENZ, PWM_AUFLOESUNG)
                    ledcAttachPin(PWM_PIN, PWM_KANAL)
                    ledcWrite(PWM_KANAL, pinHighTime)

*/

#pragma once

#include "Arduino.h"
#include <Ticker.h>

enum lamp_sweepers
{
  frontLight,     // lamp0: Scheinwerfer
  leftIndicator,  // lamp1: Blinker rechts
  rightIndicator, // lamp2: Blinker links
  rearLight       // lamp3: Rücklicht links
};
const uint8_t num_lamp_sweepers = 4;

const gpio_num_t channel2lamp_pins[num_lamp_sweepers] = {
    GPIO_NUM_35, // lamp0
    GPIO_NUM_37, // lamp1
    GPIO_NUM_36, // lamp2
    GPIO_NUM_34, // lamp3
};

enum blizzer_sweepers
{
  blizzerTopLeft,    // lamp4: Signallicht, oben links
  blizzerfrontLeft,  // lamp5: Signallicht, oben rechts
  blizzerFrontRight, // lamp6: Signallicht, vorne links
  blizzerTopRight,   // lamp7: Signallicht, vorne rechts
  LED_BUILTIN_OWN
};
const uint8_t num_blizzer_sweepers = 4;

const gpio_num_t channel2blizzer_pins[num_blizzer_sweepers] = {
    GPIO_NUM_9, // lamp4
    GPIO_NUM_8, // lamp5
    GPIO_NUM_7, // lamp6
    GPIO_NUM_6  // lamp7
};

const uint16_t note_c = 418;
const uint16_t note_g = 627;
const uint16_t note_hupe = 250;
const uint16_t no_sound = 0;

const gpio_num_t pin_ls = GPIO_NUM_5; // ls
const uint8_t channel_ls = 6; // 4 sind für die Lamps
const uint32_t freq_ls = 2000;
//void start_sound();
void make_sound(uint32_t f);

const gpio_num_t pin_motor = GPIO_NUM_1; // motorspeed
const uint8_t channel_motor = 7; // 4 sind für die Lamps
const uint16_t resolution = 0x08;
const uint32_t freq_motor = 1000;
const uint16_t no_speed = 0;
void start_speed();
void make_speed(uint16_t speed);

const uint32_t freq_lamps = 1000;

class Blizzer
{
public:
  void Attach_blizzer(blizzer_sweepers ch);
  void test_blizzer(blizzer_sweepers ch);
  void SetBrightness(uint8_t brightness);
private:
gpio_num_t pin;
};

class Lamp
{
public:
  // Voreinstellungen, LED-Nummer wird physikalisch mit
  // einer LED verbunden;
  void Attach_lamp(lamp_sweepers nbr);
  // Setzt die Helligkeit
  void SetBrightness(uint8_t brightness);
  void Update();
  void test_lamps();
  //
  uint8_t setIncrement(uint8_t p, uint8_t d)
  {
    if (p > d)
      return -1;
    else
      return 1;
  }

private:
  const uint8_t resolution = 8;
  uint8_t channel;
  gpio_num_t pin;
  uint8_t currBrightness; // current servo position
  uint8_t destBrightness; // servo position, where to go
};