#ifndef KEY_MANAGER_h
#define KEY_MANAGER_h

#define KEY_ANALOG_PIN 0
#define KEY_DEBOUNCE_DELAY_MSEC 50
// Analog Read Values - the values depend on the device
#define KEY_RIGHT_THRESHOLD 70
#define KEY_UP_THRESHOLD 240
#define KEY_DOWN_THRESHOLD 420
#define KEY_LEFT_THRESHOLD 620
#define KEY_SELECT_THRESHOLD 780
const unsigned int adc_key_val[] ={KEY_RIGHT_THRESHOLD, KEY_UP_THRESHOLD, KEY_DOWN_THRESHOLD, KEY_LEFT_THRESHOLD, KEY_SELECT_THRESHOLD};
enum KEY {KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_SELECT, KEY_NONE};
typedef void (* KEY_HANDLER)(KEY);

class KeyManager {
public:
  /** 
   * @return KEY
   * You should not call me directly. Use get_key() instead. 
   */ 
  KEY getKey() {
    const int adc_key_in = analogRead(KEY_ANALOG_PIN);
    // Serial.print("adc_key_in="); Serial.println(adc_key_in);
  
    // check debounce
    if (millis() - key_last_get_time > KEY_DEBOUNCE_DELAY_MSEC) {
      KEY key = get_current_key(adc_key_in);
      key_last_get_time = millis();
      // Serial.print("key="); Serial.println(key);
      return key;
    }
  
    // return the last key during bounce
    return last_key;
  }
  
  /** 
   * Convert ADC value to key enum
   */
  KEY get_current_key(unsigned int input) {
    const int len = sizeof(adc_key_val) / sizeof(adc_key_val[0]);
    for (int k = 0; k < len; k++) {
      if (input < adc_key_val[k])
        return static_cast<KEY>(k);
    }
    return KEY_NONE;
  }

  void setKeyHandler(KEY_HANDLER keyHandler) {
    handler = keyHandler;
  }

  void checkKey() {
    const KEY key = getKey();
    
    if (key != last_key) {
      if (handler != NULL) {
        handler(key);
      }
      last_key = key;
    }    
  }
  
private:
  KEY last_key = KEY_NONE;
  int key_last_get_time = -KEY_DEBOUNCE_DELAY_MSEC;
  KEY_HANDLER handler = NULL;
};

#endif


