#include "sound.h"

#include "Marlin.h"

//#include <inttypes.h>
//#include <avr/eeprom.h>
//#include "eeprom.h"
#include "backlight.h"


//eSOUND_MODE eSoundMode=e_SOUND_MODE_LOUD;
// nema vyznam, pokud se bude volat Sound_Init (tzn. poc. hodnota je v EEPROM)
// !?! eSOUND_MODE eSoundMode; v ultraldc.cpp :: cd_settings_menu() se takto jevi jako lokalni promenna
eSOUND_MODE eSoundMode; //=e_SOUND_MODE_DEFAULT;


static void Sound_SaveMode(void);
static void Sound_DoSound_Echo(void);
static void Sound_DoSound_Prompt(void);
static void Sound_DoSound_Alert(bool bOnce);
static void Sound_DoSound_Encoder_Move(void);
static void Sound_DoSound_Blind_Alert(void);

void Sound_Init(void)
{
     SET_OUTPUT(BEEPER);
     eSoundMode = static_cast<eSOUND_MODE>(eeprom_init_default_byte((uint8_t*)EEPROM_SOUND_MODE, e_SOUND_MODE_DEFAULT));
}

void Sound_SaveMode(void)
{
eeprom_update_byte((uint8_t*)EEPROM_SOUND_MODE,(uint8_t)eSoundMode);
}

void Sound_CycleState(void)
{
switch(eSoundMode)
     {
     case e_SOUND_MODE_LOUD:
          eSoundMode=e_SOUND_MODE_ONCE;
          break;
     case e_SOUND_MODE_ONCE:
          eSoundMode=e_SOUND_MODE_SILENT;
          break;
     case e_SOUND_MODE_SILENT:
          eSoundMode=e_SOUND_MODE_BLIND;
          break;
     case e_SOUND_MODE_BLIND:
          eSoundMode=e_SOUND_MODE_LOUD;
          break;
     default:
          eSoundMode=e_SOUND_MODE_LOUD;
     }
Sound_SaveMode();
}

//if critical is true then silend and once mode is ignored
void __attribute__((noinline)) Sound_MakeCustom(uint16_t ms,uint16_t tone_,bool critical){
     if (critical || eSoundMode != e_SOUND_MODE_SILENT) {
          if(!tone_) {
               WRITE(BEEPER, HIGH);
               _delay(ms);
               WRITE(BEEPER, LOW);
          } else {
               _tone(BEEPER, tone_);
               _delay(ms);
               _noTone(BEEPER);
          }
     }
}

void Sound_MakeSound(eSOUND_TYPE eSoundType)
{
switch(eSoundMode)
     {
     case e_SOUND_MODE_LOUD:
          if(eSoundType==e_SOUND_TYPE_ButtonEcho)
               Sound_DoSound_Echo();
          if(eSoundType==e_SOUND_TYPE_StandardPrompt)
               Sound_DoSound_Prompt();
          if(eSoundType==e_SOUND_TYPE_StandardAlert)
               Sound_DoSound_Alert(false);
          break;
     case e_SOUND_MODE_ONCE:
          if(eSoundType==e_SOUND_TYPE_ButtonEcho)
              Sound_DoSound_Echo();
          if(eSoundType==e_SOUND_TYPE_StandardPrompt)
               Sound_DoSound_Prompt();
          if(eSoundType==e_SOUND_TYPE_StandardAlert)
               Sound_DoSound_Alert(true);
          break;
     case e_SOUND_MODE_SILENT:
          if(eSoundType==e_SOUND_TYPE_StandardAlert)
               Sound_DoSound_Alert(true);
          break;
     case e_SOUND_MODE_BLIND:
          if(eSoundType==e_SOUND_TYPE_ButtonEcho)
               Sound_DoSound_Echo();
          if(eSoundType==e_SOUND_TYPE_StandardPrompt)
               Sound_DoSound_Prompt();
          if(eSoundType==e_SOUND_TYPE_StandardAlert)
               Sound_DoSound_Alert(false);
          if(eSoundType==e_SOUND_TYPE_EncoderMove)
               Sound_DoSound_Encoder_Move();
          if(eSoundType==e_SOUND_TYPE_BlindAlert)
               Sound_DoSound_Blind_Alert();
          break;
     default:
          break;
     }
}

static void Sound_DoSound_Blind_Alert(void)
{
    backlight_wake(1);
     uint8_t nI;

     for(nI=0; nI<20; nI++)
     {
         WRITE(BEEPER,HIGH);
         delayMicroseconds(94);
         WRITE(BEEPER,LOW);
         delayMicroseconds(94);
     }
}

 static void Sound_DoSound_Encoder_Move(void)
{
uint8_t nI;

 for(nI=0;nI<5;nI++)
     {
     WRITE(BEEPER,HIGH);
     delayMicroseconds(75);
     WRITE(BEEPER,LOW);
     delayMicroseconds(75);
     }
}

static void Sound_DoSound_Echo(void)
{
uint8_t nI;

for(nI=0;nI<10;nI++)
     {
     WRITE(BEEPER,HIGH);
     delayMicroseconds(100);
     WRITE(BEEPER,LOW);
     delayMicroseconds(100);
     }
}

static void Sound_DoSound_Prompt(void)
{
    backlight_wake(2);
WRITE(BEEPER,HIGH);
_delay_ms(500);
WRITE(BEEPER,LOW);
}

static void Sound_DoSound_Alert(bool bOnce)
{
uint8_t nI,nMax;

nMax=bOnce?1:3;
for(nI=0;nI<nMax;nI++)
     {
     WRITE(BEEPER,HIGH);
     delayMicroseconds(200);
     WRITE(BEEPER,LOW);
     delayMicroseconds(500);
     }
}

static int16_t constexpr CONTINOUS_BEEP_PERIOD = 2000; // in ms
static ShortTimer beep_timer; // Timer to keep track of continous beeping
static bool bFirst; // true if the first beep has occurred, e_SOUND_MODE_ONCE

/// @brief Handles sound when waiting for user input
/// the function must be non-blocking. It is up to the caller
/// to call this function repeatedly.
/// Make sure to call sound_wait_for_user_reset() when the user has clicked the knob
///     Loud - should continuously beep
///     Silent - should be silent
///     Once - should beep once
///     Assist/Blind - as loud with beep and click on knob rotation and press
void sound_wait_for_user() {
#if BEEPER > 0
     if (eSoundMode == e_SOUND_MODE_SILENT) return;

     // Handle case where only one beep is needed
     if (eSoundMode == e_SOUND_MODE_ONCE) {
          if (bFirst) return;
          Sound_MakeCustom(80, 0, false);
          bFirst = true;
     }

     // Handle case where there should be continous beeps
     if (beep_timer.expired_cont(CONTINOUS_BEEP_PERIOD)) {
          beep_timer.start();
          if (eSoundMode == e_SOUND_MODE_LOUD) {
               Sound_MakeCustom(80, 0, false);
          } else {
               // Assist (lower volume sound)
               Sound_MakeSound(e_SOUND_TYPE_ButtonEcho);
          }
     }
#endif // BEEPER > 0
}

/// @brief Resets the global state of sound_wait_for_user()
void sound_wait_for_user_reset()
{
     beep_timer.stop();
     bFirst = false;
}
