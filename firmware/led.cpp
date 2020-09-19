#include "hw.h"
#include "Arduino.h"
#include <Tone.h>

void timersetup(uint16_t counts);
volatile uint8_t led_channels[18]={0};
volatile int ledindex=0;
volatile int chindex=0;
LEDcontrol::LEDcontrol(uint16_t counts){
  timersetup(counts);
  
}
void LEDcontrol::setled(uint8_t id, uint8_t r, uint8_t g, uint8_t b){
  enabled=true;
  if(id<6){
    led_channels[3*id+0]=r;
    led_channels[3*id+1]=g;
    led_channels[3*id+2]=b;
  }
  if(id==6){
    for(int i=0;i<6;i++){
      led_channels[3*i+0]=r;
      led_channels[3*i+1]=g;
      led_channels[3*i+2]=b;
    }
  }
}
void LEDcontrol::setbl(uint8_t value){
  enabled=true;
  analogWrite(16,value);
}
void LEDcontrol::setbuzz(uint16_t freq, uint16_t duration){
  noTone(14);
  tone(14,freq,duration);
}
void LEDcontrol::alloff(){
  enabled=false;
  for(int i=0;i<18;i++)led_channels[i]=0;
  analogWrite(16,0);
  noTone(14);
}


void timersetup(uint16_t counts) {
  NVIC_DisableIRQ(TC4_IRQn);
  NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  while (GCLK->STATUS.bit.SYNCBUSY);
  uint32_t prescalerConfigBits;
  
  TC4->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY);
  
  // Reset TCx
  TC4->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY);
  while (TC4->COUNT16.CTRLA.bit.SWRST);
  
  prescalerConfigBits = TC_CTRLA_PRESCALER_DIV1024;

  uint16_t tmpReg = 0;
  tmpReg |= TC_CTRLA_MODE_COUNT16;  // Set Timer counter Mode to 16 bits
  tmpReg |= TC_CTRLA_WAVEGEN_MFRQ;  // Set TONE_TC mode as match frequency
  tmpReg |= prescalerConfigBits;
  TC4->COUNT16.CTRLA.reg |= tmpReg;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY);
  TC4->COUNT16.CC[0].reg = (uint16_t) counts;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY);
  TC4->COUNT16.INTENSET.bit.MC0 = 1;
  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY);
  NVIC_EnableIRQ(TC4_IRQn);
}


void TC4_Handler()                              // Interrupt Service Routine (ISR) for timer TC4
{
  int i;
  
  chindex+=1;
  if(chindex==4){
    chindex=0;
    ledindex+=1;
    if(ledindex==6){
      ledindex=0;
    }
    for(int i=0;i<6;i++){ 
      //- if(ledindex==i)digitalWrite(led_pins[i],HIGH);
      //- else digitalWrite(led_pins[i],LOW);
      if(ledindex==i)digitalWrite(led_pins[i],LOW);
      else digitalWrite(led_pins[i],HIGH);
    }
  }
  for(int i=0;i<3;i++){
    // led_channels[3*ledindex+chindex] = 254;
    if(i==chindex && led_channels[3*ledindex+chindex]>0)analogWrite(rgb_pins[i],255-led_channels[3*ledindex+chindex]);
    else {
      pinMode(rgb_pins[i],OUTPUT);
      //- digitalWrite(rgb_pins[i],HIGH);
      digitalWrite(rgb_pins[i],LOW);
    }
  }
  REG_TC4_INTFLAG = TC_INTFLAG_MC0;         // Clear the MC0 interrupt flag
}
