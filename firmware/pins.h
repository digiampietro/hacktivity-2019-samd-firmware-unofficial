#ifndef _PINS_H_
#define _PINS_H_

#define PIN_BTN       23
#define PIN_ESP_GPIO0 11
#define PIN_ESP_RESET 12
#define PIN_LCD_RESET 15
#define PIN_LCD_BACKL 16
#define PIN_LED_AR     8
#define PIN_LED_AG     9
#define PIN_LED_AB    10
//#define PIN_LED_C1     7
//#define PIN_LED_C2     4
//#define PIN_LED_C3     6
//#define PIN_LED_C4     5
//#define PIN_LED_C5     3
//#define PIN_LED_C6     2

#define PIN_LED_C1     6
#define PIN_LED_C2     2
#define PIN_LED_C3     4
#define PIN_LED_C4     3
#define PIN_LED_C5     7
#define PIN_LED_C6     5


#define PIN_I2C_INT   20
#define PIN_ADC_VBATT A2
#define PIN_TOUCH_1   29
#define PIN_TOUCH_2   30
#define PIN_TOUCH_3   31
#define PIN_TOUCH_4   28
#define PIN_TOUCH_5   27
#define PIN_TOUCH_6   26

const int touch_pins[6] = {
  PIN_TOUCH_1,
  PIN_TOUCH_2,
  PIN_TOUCH_3,
  PIN_TOUCH_4,
  PIN_TOUCH_5,
  PIN_TOUCH_6
};

const int led_pins[6] = {
  PIN_LED_C1,
  PIN_LED_C2,
  PIN_LED_C3,
  PIN_LED_C4,
  PIN_LED_C5,
  PIN_LED_C6
};

const int rgb_pins[3] = {
  PIN_LED_AR,
  PIN_LED_AG,
  PIN_LED_AB
};

#endif
