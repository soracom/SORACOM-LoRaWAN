/*
 * LoRa Gateway Monitor
 * 
 * Pushing SELECT button changes the view mode
 * - combo (sequence) mode
 * - ok rate mode
 * - count mode
 */
#include <LiquidCrystal.h>
#include <lorawan_client.h>
#include <lorawan_client_al050.h>

#include "SoracomLogoDrawer.h"
#include "Statistics.h"
#include "ViewManager.h"

// LCD settings
#define LCD_RS 8
#define LCD_ENABLE 9
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

#define USE_KEY
// Keypad settings - You may need to check the header file and update the pin numbers
#ifdef USE_KEY
#include "KeyManager.h"
#endif

LoRaWANClientAL050 lorawanClient;
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
SoracomLogoDrawer logoDrawer(lcd);

#define SERIAL_BAUDRATE 9600
// Must be more than 4000 msecs
#define LORAWAN_SEND_INTERVAL_MSEC 4500
#define LOGO_BLINK_INTERVAL_MSEC 750

Statistics stat;

#ifdef USE_KEY
KeyManager keyManager;
#endif

ViewManager viewManager(stat, lcd);

// last sent to lora gateway
int last_sent_time;
// last blinked soracom logo
int last_blinked_time;


void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  // init lcd things
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  logoDrawer.init();
  logoDrawer.drawLogo(0, 0);
  viewManager.showGreetings();

  // init lorawan
  lorawanClient.connect();
  lorawanClient.setTxType(TX_TYPE_CONFIRMED);

  const int current_time = millis();
  last_sent_time = current_time;
  last_blinked_time = current_time;

#ifdef USE_KEY
  keyManager.setKeyHandler(handleKey);
#endif
}


#ifdef USE_KEY
/**
 * Key handler
 */
void handleKey(KEY key) {
  Serial.print("handling key="); Serial.println(key);
  
  if (key == KEY_SELECT) {
    viewManager.rotateViewMode();
    viewManager.updateView();
  }
}
#endif


void loop() {
#ifdef USE_KEY
  // check key input and make callback request according to key input
  keyManager.checkKey();
#endif

  const int current_time = millis();

  // lorawan requires interval time between communications
  if (current_time > last_sent_time + LORAWAN_SEND_INTERVAL_MSEC) {
    char data[] = "sample data";
    const bool isOk = lorawanClient.sendData(data);
    last_sent_time = millis();    
    stat.addResult(isOk);
    viewManager.updateView();
  }

  // toggle logo
  if (current_time > last_blinked_time + LOGO_BLINK_INTERVAL_MSEC) {
    logoDrawer.blinkLogo();
    last_blinked_time = current_time;
  }
}

