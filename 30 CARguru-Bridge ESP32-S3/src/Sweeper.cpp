
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <gustavwostrack@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <Sweeper.h>
#include <Ticker.h>
#include "UMS3.h"

// SOUND ////////////////////////////////////////////////////////////

Ticker sgnlTimer;

void signalTimer()
{
  static uint8_t hilo = 0;
  if (hilo == 0)
    hilo++;
  else
    hilo--;
  digitalWrite(pin_ls, hilo);
}

void make_sound(uint32_t f)
{
  if (f == no_sound)
    sgnlTimer.detach();
  else
  {
    pinMode(pin_ls, OUTPUT);
    sgnlTimer.attach_ms(1000 / f, signalTimer);
  }
}

/*


void start_sound()
{
  ledcAttachPin(pin_ls, channel_ls);
  ledcSetup(channel_ls, freq_ls, resolution);
  ledcWrite(channel_ls, 0x80);
  ledcWriteTone(channel_ls, no_sound);
}

void make_sound(uint32_t f)
{
//  ledcWriteTone(channel_ls, f);
}
*/
// MOTOR ////////////////////////////////////////////////////////////
void start_speed()
{
  ledcAttachPin(pin_motor, channel_motor);
  ledcSetup(channel_motor, freq_motor, resolution);
  ledcWrite(channel_motor, no_speed);
}

void make_speed(uint16_t speed)
{
  ledcWrite(channel_motor, speed);
}

// BLIZZER ////////////////////////////////////////////////////////////
void Blizzer::Attach_blizzer(blizzer_sweepers ch)
{
  pin = channel2blizzer_pins[ch];
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void Blizzer::test_blizzer(blizzer_sweepers ch)
{
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
}

// Setzt die Zielposition
void Blizzer::SetBrightness(uint8_t brightness)
{
  digitalWrite(pin, brightness);
}

// LAMP ////////////////////////////////////////////////////////////
// Voreinstellungen, LED-Nummer wird physikalisch mit
// einer LED verbunden
void Lamp::Attach_lamp(lamp_sweepers ch)
{
  channel = ch;
  pin = channel2lamp_pins[channel];
  currBrightness = dark;
  ledcAttachPin(pin, channel);
  ledcSetup(channel, freq_lamps, resolution);
  ledcWrite(channel, currBrightness);
  SetBrightness(currBrightness);
}

// Setzt die Zielposition
void Lamp::SetBrightness(uint8_t brightness)
{
  destBrightness = brightness;
}

// Überprüft periodisch, ob die Zielposition (Helligkeit) erreicht ist
void Lamp::Update()
{
  // Überprüfung, ob die Zielposition bereits erreicht wurde
  if (currBrightness != destBrightness)
  {
    // time to update
    currBrightness += setIncrement(currBrightness, destBrightness);
    ledcWrite(channel, currBrightness);
  }
}

void Lamp::test_lamps()
{
  ledcWrite(channel, smallBright);
  delay(200);
  ledcWrite(channel, dark);
}