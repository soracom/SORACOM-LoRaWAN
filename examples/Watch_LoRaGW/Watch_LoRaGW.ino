/*
 * LoRa Gateway Monitor
 * 
 * Pushing SELECT button changes the view mode
 * - combo (sequence) mode
 * - ok rate mode
 * - count mode
 * 
 * Note that data are sent in JSON and you can see the history using SORACOM harvest.
 */
#include <LiquidCrystal.h>
#include <lorawan_client.h>
#include <lorawan_client_al050.h>
#include <lorawan_util.h>

#include "SoracomLogoDrawer.h"
#include "Statistics.h"
#include "ViewManager.h"

// LCD settings
#define USE_LCD
#ifdef USE_LCD
#define LCD_RS 8
#define LCD_ENABLE 9
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#endif

#define USE_KEY
// Keypad settings - You may need to check the header file and update the pin numbers
#ifdef USE_KEY
#include "KeyManager.h"
#endif

LoRaWANClientAL050 lorawanClient;
#ifdef USE_LCD
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
SoracomLogoDrawer logoDrawer(lcd);
#endif

#define SERIAL_BAUDRATE 9600
// Must be more than 4000 msecs
#define LORAWAN_SEND_INTERVAL_MSEC 4500
#define LOGO_BLINK_INTERVAL_MSEC 750

const String JSON_KEY_INIT("init");
const String JSON_KEY_OK("o");
const String JSON_KEY_ERR("e");

Statistics stat;

#ifdef USE_KEY
KeyManager keyManager;
#endif

#ifdef USE_LCD
ViewManager viewManager(stat, lcd);
#endif

// last sent to lora gateway
unsigned long last_sent_time;
// last blinked soracom logo
unsigned long last_blinked_time;


void setup() {
  Serial.begin(SERIAL_BAUDRATE);

#ifdef USE_LCD
  // init lcd things
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  logoDrawer.init();
  logoDrawer.drawLogo(0, 0);
  viewManager.showGreetings();
#endif

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

#ifdef USE_LCD  
  if (key == KEY_SELECT) {
    viewManager.rotateViewMode();
    viewManager.updateView();
  }
#endif
}
#endif

String createPayload(const Statistics& stat) {
  // since INT_MAX is 32767, payload should always fits within 11 bytes
  String data;
  if (stat.getTotalCount() == 0) {
    data = createJson(JSON_KEY_INIT, 1);
  } else if (stat.getLastOk()) {
    // o stands for tx_ok
    data = createJson(JSON_KEY_OK, stat.getSequenceCount() + 1);
  } else {
    // e stands for err
    data = createJson(JSON_KEY_ERR, stat.getSequenceCount());
  }
  return data;
}

void updateSerialView() {
  if (stat.getLastOk()) {
    Serial.print("tx_ok. combo=");
    Serial.println(stat.getSequenceCount());      
  } else {
    Serial.print("err. combo=");
    Serial.println(stat.getSequenceCount());      
  }
  Serial.print("ok=");
  Serial.println(stat.getOkCount());
  Serial.print("err=");
  Serial.println(stat.getErrCount());
  Serial.print("ok rate=");
  Serial.println(stat.getOkRate());  
  Serial.print("max err(min)=");
  Serial.println(stat.getMaxErrMins());  
}

void loop() {
#ifdef USE_KEY
  // check key input and make callback request according to key input
  keyManager.checkKey();
#endif

  const unsigned long current_time = millis();

  // lorawan requires interval time between communications
  if (current_time > last_sent_time + LORAWAN_SEND_INTERVAL_MSEC) {
    const String txData = createPayload(stat);
    verifyPayloadLength(txData);
    
    const bool isOk = lorawanClient.sendData(txData);
    stat.addResult(isOk);
    
    last_sent_time = millis();    
#ifdef USE_LCD
    viewManager.updateView();
#else
    updateSerialView();
#endif

    // reset to send stat whole data to harvest
    // @see setDataToSend
    if (stat.getTotalCount() > 60000) {
      Serial.println("Resetting stat.");
      stat.reset();
    }
  }

#ifdef USE_LCD
  // toggle logo
  if (current_time > last_blinked_time + LOGO_BLINK_INTERVAL_MSEC) {
    logoDrawer.blinkLogo();
    last_blinked_time = current_time;
  }
#endif
}

