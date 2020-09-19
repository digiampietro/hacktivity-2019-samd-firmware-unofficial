/* Disobey badge 2019 */
#include "hw.h"
#include <Wire.h>
#include <Tone.h>

#define FSM_OFF      0x01 //USB is disconnected
#define FSM_ON       0x02 //USB is connected and either idle or active
#define FSM_SUSPEND  0x04 //L2 suspended
#define FSM_SLEEP    0x08 //L1 sleep
#define FSM_DNRESUME 0x10 //Down stream resume
#define FSM_UPRESUME 0x20 //Up stream resume
#define FSM_RESET    0x40 //USB lines reset


#define NVM_MEMORY ((volatile uint16_t *)0x000000)
#define APP_START 0x400


static inline bool nvmReady(void) {
        return NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY;
}

/*__attribute__ ((long_call, section (".ramfunc")))
static void selfdestruct() {
    __disable_irq();
    while (!nvmReady())
        ;
    NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
    NVMCTRL->ADDR.reg  = (uintptr_t)&NVM_MEMORY[APP_START / 4];
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER | NVMCTRL_CTRLA_CMDEX_KEY;
    while (!nvmReady())
        ;
    NVIC_SystemReset() ;
    while (true);
}*/

Touchcontrol *t;
LEDcontrol *l;
volatile uint8_t responsebuffer[2]={0};

void setup() {
  //Pin configuration
  pinMode(PIN_BTN,       INPUT_PULLUP);
  pinMode(PIN_ESP_GPIO0, OUTPUT);
  pinMode(PIN_ESP_RESET, OUTPUT);
  pinMode(PIN_LCD_RESET, OUTPUT);
  pinMode(PIN_LCD_BACKL, OUTPUT);
  pinMode(PIN_LED_AR,    OUTPUT);
  pinMode(PIN_LED_AG,    OUTPUT);
  pinMode(PIN_LED_AB,    OUTPUT);
  pinMode(PIN_LED_C1,    OUTPUT);
  pinMode(PIN_LED_C2,    OUTPUT);
  pinMode(PIN_LED_C3,    OUTPUT);
  pinMode(PIN_LED_C4,    OUTPUT);
  pinMode(PIN_LED_C5,    OUTPUT);
  pinMode(PIN_LED_C6,    OUTPUT);
  pinMode(PIN_I2C_INT,   OUTPUT);

  //Initial output values
  digitalWrite(PIN_ESP_GPIO0, HIGH); //Normal ESP32 boot
  digitalWrite(PIN_LCD_RESET, LOW);  //Disable LCD
  digitalWrite(PIN_ESP_RESET, LOW);  //Disable ESP32
  digitalWrite(PIN_I2C_INT,   LOW);  //No I2C interrupt

  //Start the USB-serial port and the UART connected to the ESP32
  Serial1.begin(115200);    // ESP32 Serial Interface
  Serial.begin(115200);     // USB Serial Interface

  //Small delay to let things settle
  delay(50);

  //Start the ESP32
  digitalWrite(PIN_LCD_RESET,HIGH); //Enable LCD
  digitalWrite(PIN_ESP_RESET,HIGH); //Enable ESP32

  analogReadResolution(8);
  
  responsebuffer[1] = analogRead(PIN_ADC_VBATT); //We do one normal, synced analog read to configure the pin correctly
    
  Wire.begin(0x30);
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  
  uint32_t mask;
  
  mask = PM_APBAMASK_WDT | PM_APBAMASK_PAC0 | PM_APBAMASK_EIC;
  PM->APBAMASK.reg &= ~mask;

  mask = PM_APBBMASK_PAC1 | PM_APBBMASK_DSU | PM_APBBMASK_DMAC | PM_APBBMASK_NVMCTRL;
  PM->APBBMASK.reg &= ~mask;
    
  mask = PM_APBCMASK_PAC2 | PM_APBCMASK_DAC | PM_APBCMASK_AC | PM_APBCMASK_SERCOM5 | \
         PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM2 | \
         PM_APBCMASK_EVSYS | PM_APBCMASK_I2S;
  PM->APBCMASK.reg &= ~mask;
  
  mask= PM_AHBMASK_DSU | PM_AHBMASK_DMAC;
  PM->AHBMASK.reg &= ~mask;
  
  t=new Touchcontrol(300);
  l=new LEDcontrol(10);
  // l=new LEDcontrol(10000);
  l->alloff();
  
  // digitalWrite(PIN_LED_AR, LOW);   //On
  // digitalWrite(PIN_LED_AG, HIGH);  //Off
  // digitalWrite(PIN_LED_AB, HIGH);  //Off
  // by Valerio
  // digitalWrite(PIN_LCD_BACKL, HIGH);
  // digitalWrite(PIN_LED_C1,    LOW);
  // digitalWrite(PIN_LED_C2,    LOW);
  // digitalWrite(PIN_LED_C3,    LOW);
  // digitalWrite(PIN_LED_C4,    LOW);
  // digitalWrite(PIN_LED_C5,    LOW);
  // digitalWrite(PIN_LED_C6,    LOW);
  
  // delay(500);
}


void requestEvent()
{
  digitalWrite(PIN_I2C_INT, HIGH);
  Wire.write((char *)responsebuffer,2); // respond with message of 2 bytes
}

void receiveEvent(int howMany)
{
  if (Wire.available()) {
    int command = Wire.read();
    switch(command) {
      case 0x01: {
        uint8_t id = Wire.read();
        uint8_t r  = Wire.read();
        uint8_t g  = Wire.read();
        uint8_t b  = Wire.read();
        l->setled(id, r, g, b);
        break;
      }
      case 0x02: {
        l->setbl(Wire.read());
        break;
      }
      case 0x03: {
        uint8_t  freq_h = Wire.read();
        uint8_t  freq_l = Wire.read();
        uint8_t  dur_h  = Wire.read();
        uint8_t  dur_l  = Wire.read();
        uint16_t freq   = (freq_h<<8)|(freq_l);
        uint16_t dur    = (dur_h<<8)|(dur_l);
        l->setbuzz(freq,dur);
        break;
      }
      case 0x04:
        l->alloff();
        break;
    }
  }
}

uint8_t vbatt_adc_status = 0xFF; //Go to "default"
unsigned long previousVbattMillis = 0;
const unsigned long vbattInterval = 1000;
inline void update_vbatt_status()
{
  //responsebuffer[1] = analogRead(PIN_ADC_VBATT); //The code below replaces this function
  switch(vbatt_adc_status) {
    case 0:
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->CTRLA.bit.ENABLE = 1;
        vbatt_adc_status = 1;
      }
      break;
    case 1: //Waiting for sync
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[PIN_ADC_VBATT].ulADCChannelNumber;
        vbatt_adc_status = 2;
      }
      break;
    case 2:
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->CTRLA.bit.ENABLE = 0x01;
        vbatt_adc_status = 3;
      }
      break;
    case 3:
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->SWTRIG.bit.START = 1;
        ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
        vbatt_adc_status = 4;
      }
      break;
    case 4:
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->SWTRIG.bit.START = 1;
        vbatt_adc_status = 5;
      }
      break;
    case 5:
      if (ADC->INTFLAG.bit.RESRDY) {
        //Serial.println("ADC value "+String(ADC->RESULT.reg));
        responsebuffer[1] = ADC->RESULT.reg;
        vbatt_adc_status = 6;
      }
      break;
    case 6:
      if (!ADC->STATUS.bit.SYNCBUSY) {
        ADC->CTRLA.bit.ENABLE = 0x00;
        vbatt_adc_status = 7;
      }
      break;
    default:
      unsigned long currentVbattMillis = millis();
      if (currentVbattMillis - previousVbattMillis >= vbattInterval) {
        previousVbattMillis = currentVbattMillis;
        //Serial.println("ACTUAL FUNC "+String(analogRead(PIN_ADC_VBATT)));
        vbatt_adc_status = 0; //Start!
      }
  }
}

unsigned long previousResetMillis = 0;
const unsigned long resetInterval = 100;
extern uint8_t getLineState();
uint8_t prevLineState = 0xFF;
int resetTrigger = -1;
inline void update_reset_status()
{
  unsigned long currentResetMillis = millis();
  if (currentResetMillis - previousResetMillis >= resetInterval) {
    previousResetMillis = currentResetMillis;
    uint8_t lineState = getLineState();
    if (lineState!=prevLineState) {
        if (lineState == 0x02) resetTrigger = 20;         // RTS ?     
        if (lineState == 0x01 && resetTrigger > 0) {      // DTR ?      
          resetTrigger = 0;
          //To bootloader mode
          digitalWrite(PIN_ESP_GPIO0,LOW);
          digitalWrite(PIN_ESP_RESET,LOW);
          delay(100);
          digitalWrite(PIN_ESP_RESET,HIGH);
          delay(100);
          digitalWrite(PIN_ESP_GPIO0,HIGH);
        }
      prevLineState = lineState;
    }
    if (resetTrigger > 0) {
      if (resetTrigger == 1) {
        //To normal mode
        digitalWrite(PIN_ESP_GPIO0,HIGH);
        digitalWrite(PIN_ESP_RESET,LOW);
        delay(100);
        digitalWrite(PIN_ESP_RESET,HIGH);
      }
      resetTrigger--;
    }
  }
}

inline void update_touch_status()
{
  for (int i=0;i<6;i++) {
    if(t->buttons[i]){
      responsebuffer[0]|=(1<<i);
    }else{
      responsebuffer[0]&=~(1<<i);
    }
  }
}

inline void update_usb_status()
{
  switch(USB->DEVICE.FSMSTATUS.reg)
  {
    case FSM_OFF:
      responsebuffer[0]|=(1<<6);
      break;
    default: //USB is in any other state than OFF
      responsebuffer[0]&=~(1<<6);
  }
}

#define BUFLEN 512

inline void my_update_serial()
{
  int n0=0;
  int m0=0;;
  int n1=0;
  int m1=0;
  int min0=0;
  int min1=0;
  char buf[BUFLEN];

  while (( ((n0=Serial.available())  > 0)  and   ((m0=Serial1.availableForWrite()) > 0 ) )
     or  ( ((n1=Serial1.available()) > 0)  and   ((m1=Serial.availableForWrite())  > 0 ) )) {

    if (n0 > 0) {
      min0=min(n0,m0);
      min0=min(min0,BUFLEN);    
      if (min0 > 0) {
            Serial.readBytes(buf,min0);
            Serial1.write(buf,min0);
      }
    }

    if (n1 > 0) {
      min1=min(n1,m1);  
      min1=min(min1,BUFLEN);  
      if (min1 > 0) {
            Serial1.readBytes(buf,min1);
            Serial.write(buf,min1);
      }
    }
    n0=0;
    m0=0;
    n1=0;
    m1=0;
 }
}

inline void update_serial()
{
  while (Serial.available() or Serial1.available()) {
    if (Serial.available()) {
      Serial1.write(Serial.read());
    }
    if (Serial1.available()) {
      Serial.write(Serial1.read());
    }
  }
}

void loop()
{
  my_update_serial();
  
  //responsebuffer[0] ^= (-digitalRead(PIN_BTN) ^ responsebuffer[0]) & (1UL << 7);

  if (!digitalRead(PIN_BTN)) {
      digitalWrite(PIN_ESP_RESET,LOW);
      if (bitRead(responsebuffer[0],5) == 1) {
        digitalWrite(PIN_ESP_GPIO0,LOW);
      } else {
        digitalWrite(PIN_ESP_GPIO0,HIGH);
      }
      delay(100);
      digitalWrite(PIN_ESP_RESET,HIGH);
  }
  
  update_vbatt_status();
  my_update_serial();

  update_reset_status();
  my_update_serial();
  
  update_usb_status();
  
  update_touch_status();
  my_update_serial();
 
  t->tick();

  // TC4_Handler();
}
