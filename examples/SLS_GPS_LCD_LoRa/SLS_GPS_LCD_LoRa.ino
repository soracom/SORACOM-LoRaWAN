/*
 * SLS_GPS_LCD_LoRa
 * (c) 2017 SORACOM, INC.
 * https://blog.soracom.jp/blog/2017/05/18/sls-lcd-keypad/
 */

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <TinyGPS.h>

#include <StandardCplusplus.h>
#include <deque>

#include <lorawan_client.h>
#include <lorawan_client_al050.h>

//
// Hardware flags and pin settings
//

// Define to send data via LORAWAN
#define USE_LORAWAN
// If the device has been connected to a gw before, you do not need to connect every time.
//#define SKIP_LORAWAN_CONNECT

#define USE_GPS
// If you are using other pins, update accordingly
// Note that pin 11 and 12 are used by lorawan shield
#define GPS_TX_PIN  (14+2) // Arduino RX pin connecting to GPS module TX pin
#define GPS_RX_PIN  (14+3) // Arduino TX pin connecting to GPS module RX pin


#define USE_LCD
// Define LCD_RW, LCD_DB0, LCD_DB1, LCD_DB2 and LCD_DB3 if and only if you use these pins.
// See your LCD manual for details.
#define LCD_RS 8
// #define LCD_RW
#define LCD_ENABLE 9
//#define LCD_DB0 0
//#define LCD_DB1 1
//#define LCD_DB2 2
//#define LCD_DB3 3
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7
// If the screen is smaller than the default value, you probably need to update the code.
#define LCD_WIDTH 16
#define LCD_HEIGHT 2


#define USE_KEY
#define KEY_ANALOG_PIN 0
#define KEY_DEBOUNCE_DELAY_MSEC 50
// Analog Read Values - the values depend on the device
#define KEY_RIGHT_THRESHOLD 70
#define KEY_UP_THRESHOLD 240
#define KEY_DOWN_THRESHOLD 420
#define KEY_LEFT_THRESHOLD 620
#define KEY_SELECT_THRESHOLD 780
const int adc_key_val[] ={KEY_RIGHT_THRESHOLD, KEY_UP_THRESHOLD, KEY_DOWN_THRESHOLD, KEY_LEFT_THRESHOLD, KEY_SELECT_THRESHOLD};
enum KEY {KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_SELECT, KEY_NONE};

// Feature flags. If defined, print raw nmea values to serial
#define PRINT_NMEA
//#define PRINT_GOT_GPS

#define SERIAL_RATE 9600
#define GPS_SERIAL_RATE 9600

// interval msec to send gps data
#define SEND_INTERVAL 15000

// Sensor may return small different values at the same place
#define SIGMA_LONG_LATLNG 300
#define SIGMA_METER_ALT 50
#define LAT_LONG_MIN  -90000000
#define LAT_LONG_MAX   90000000
#define LNG_LONG_MIN -180000000
#define LNG_LONG_MAX  180000000
#define ALT_MIN -10000
#define ALT_MAX  10000

#define WIRELESS_LORA 1
#define SENSOR_GPS 1
#define LORAWAN_GPS_HEADER ((WIRELESS_LORA << 5) + SENSOR_GPS) // (LoRa=1) << 5 + (GPS=1) = 0x21

#ifdef USE_GPS
SoftwareSerial gpsSerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPS tinyGps;
#endif

// More history size requires more memory and some code update
#define GPS_HISTORY_SIZE 10

#ifdef USE_LCD
#ifdef LCD_DB0
  #ifdef LCD_RW
    LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_ENABLE, LCD_DB0, LC_DB1, LCD_DB2, LCD_DB3, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
  #else
    LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_DB0, LC_DB1, LCD_DB2, LCD_DB3, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
  #endif
#else
  #ifdef LCD_RW
    LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
  #else
    LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
  #endif
#endif
const char cursors[] = {'.', ' '};
char cursorIndex = 0;
#endif

// LCD view mode
enum VIEW_MODE {VIEW_MODE_GOT_GPS, VIEW_MODE_SEND_GPS_HISTORY, VIEW_MODE_NMEA};
const char* VIEW_MODE_DESC[] = {
 //1234567890123456 // Up to lcd column length
  "Show latest GPS",
  "Show sending GPS",
  "Show raw NMEA",
};
// You can change view mode with the keypad
VIEW_MODE view_mode = VIEW_MODE_GOT_GPS;


#ifdef USE_KEY
KEY last_key = KEY_NONE;
int key_last_get_time = -KEY_DEBOUNCE_DELAY_MSEC;
#endif


#ifdef USE_LORAWAN
LoRaWANClientAL050 lorawanClient;
#endif


unsigned long gps_updated_time = 0L;
unsigned long gps_sent_time = 0L;

// The struct to sent data via LoRaWAN. Max 11 bytes
struct SLS_GPS {
  byte header; // LORAWAN_GPS_HEADER
  long lat;    // Can store as MILLIONTHs of a degree
  long lng;    // 180 is stored as 180,000,000
  short alt;   // -32,768 < alt < 32,767 (meter)
};

class GPS {
public:
  // data to be sent via lorawan
  SLS_GPS sls_gps;
  
  // time
  byte hour;
  byte minutes;
  byte second;

  GPS() : sls_gps({LORAWAN_GPS_HEADER, 0, 0, 0}), hour(0), minutes(0), second(0) {}
};

// last got (not sent) gps
GPS gps;
// sent gps history
std::deque<GPS> gps_history;
// index to be shown
int gps_history_index = 0;


void setup() {
  // setup serial
  Serial.begin(SERIAL_RATE);
  Serial.println("Starting Sketch SLS_GPS_LCD_LoRa.");

#ifdef USE_LCD
  setup_lcd();
#endif

#ifdef USE_LORAWAN
  setup_lorawan();
#endif

#ifdef USE_GPS
  setup_gps();
#endif
}

#ifdef USE_LCD
void setup_lcd() {
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.print("Starting Sketch"); 
}
#endif

#ifdef USE_GPS
void setup_gps() {
  pinMode(GPS_TX_PIN, INPUT);
  pinMode(GPS_RX_PIN, OUTPUT);
  gpsSerial.begin(GPS_SERIAL_RATE);
  const GPS gps; // copied to the collection
  for (int i = 0; i < GPS_HISTORY_SIZE; ++i)
    gps_history.push_back(gps);
}
#endif

#ifdef USE_LORAWAN
void setup_lorawan() {
#ifdef SKIP_LORAWAN_CONNECT
  Serial.println("Skip connecting to lora GW by setting.");
#else
  // make sure to connect
  if (!lorawanClient.connect()) {
    Serial.println(" failed to connect to lora GW. Halt.");
#ifdef USE_LCD
    lcd.setCursor(0, 0);
    lcd.print("Halt. Failed to"); 
    lcd.setCursor(0, 1);
    lcd.print("  connect to GW");
#endif
    for(;;){};
  }
  Serial.println("Connected to lora GW.");
#ifdef USE_LCD
  lcd.setCursor(0, 0);
  lcd.print("Connected to    "); 
  lcd.setCursor(0, 1);
  lcd.print("    LoRa Gateway"); 
#endif  
#endif 
}
#endif


void loop() {
#ifdef USE_KEY
  // get key and set view mode
  const KEY key = get_key(last_key);
  if (key != last_key) {
    handle_key(key);
    last_key = key;
  }
#endif

#ifdef USE_GPS
  // get and send gps
  const bool is_updated = get_location();
  if (is_updated)
    gps_updated_time = millis();

  // Send location data
  // Note that sending can happen in a different loop from the updated one.
  // Unsigned operation should work after millis overflow. 
  if (gps_updated_time > gps_sent_time && (gps_sent_time == 0 || millis() - gps_sent_time > SEND_INTERVAL)) {
    const bool is_sent = send_location();
    if (is_sent)
      gps_sent_time = millis();
  }
#endif
}

void handle_key(KEY key) {
#ifdef USE_LCD
  if (key == KEY_UP) {
    if (view_mode != VIEW_MODE_SEND_GPS_HISTORY) {
      view_mode = VIEW_MODE_SEND_GPS_HISTORY;
      gps_history_index = 0;
    } else {
      // scroll history to older direction
      gps_history_index = std::min(gps_history_index + 1, GPS_HISTORY_SIZE - 1);
    }
    // show the history
    show_gps_history(gps_history_index);
  } else if (key == KEY_DOWN) {
    if (view_mode != VIEW_MODE_SEND_GPS_HISTORY) {
      view_mode = VIEW_MODE_SEND_GPS_HISTORY;
      gps_history_index = 0;
    } else {
      // scroll history to newer direction
      gps_history_index = std::max(gps_history_index - 1, 0);
    }
    // show the history
    show_gps_history(gps_history_index);
  } else if (key == KEY_SELECT || key == KEY_LEFT || key == KEY_RIGHT) {
    // change view mode
    view_mode = get_view_mode(key);
    show_new_view_mode(view_mode);
  } else {
    // KEY_NONE
  }
#endif
}

#ifdef USE_LCD
void show_new_view_mode(VIEW_MODE view_mode) {
  lcd.clear();
  lcd.print("New view_mode=");
  lcd.print(view_mode);
  lcd.setCursor(0, 1);
  lcd.print(VIEW_MODE_DESC[view_mode]);    
}
#endif

/**
 * Get view mode according to the key
 */
VIEW_MODE get_view_mode(KEY key) {
  if (key == KEY_SELECT)
    return VIEW_MODE_GOT_GPS;
  else if (key == KEY_LEFT || key == KEY_UP || key == KEY_DOWN)
    return VIEW_MODE_SEND_GPS_HISTORY;
  else if (key == KEY_RIGHT)
    return VIEW_MODE_NMEA;
}

/**
 * Set updated data to sls_gps variable
 * 
 * @return true if the sensor detects the new location
 *   The new gps data is stored in sls_gps
 */
#ifdef USE_GPS
bool get_location() {
  // activate GPS serial
  gpsSerial.listen();
  while (gpsSerial.available()) {
    const int c = gpsSerial.read();
#ifdef PRINT_NMEA
    Serial.print((char)c);
#endif
#ifdef USE_LCD
    if (view_mode == VIEW_MODE_NMEA) {
      lcd.print((char)c);
    }
#endif
    
    if (!tinyGps.encode(c))
      continue;
    
    // process new gps info
    // retrieves +/- lat/long in 100000ths of a degree
    tinyGps.get_position(&gps.sls_gps.lat, &gps.sls_gps.lng);
    gps.sls_gps.alt = tinyGps.altitude() / 100;

    tinyGps.crack_datetime(NULL, NULL, NULL, &gps.hour, &gps.minutes, &gps.second, NULL);

#ifdef PRINT_GOT_GPS
    Serial.print("got data ");
    Serial.print("lat="); Serial.print(gps.sls_gps.lat);
    Serial.print(", lng="); Serial.print(gps.sls_gps.lng);
    Serial.print(", alt="); Serial.print(gps.sls_gps.alt); Serial.print("(m)　");
    Serial.print(gps.hour); Serial.print(":"); Serial.print(gps.minutes); Serial.print(":"); Serial.print(gps.second); Serial.println("UTC");
#endif

#ifdef USE_LCD
    if (view_mode == VIEW_MODE_GOT_GPS) {
      // show latest value
      // note that arduino sprintf does not understand float
      // +1 is for null
      // 1234567890123456
      //   32012345 12.34
      //  123012345 +0009
      char lines[][LCD_WIDTH + 1] = { {}, {} }; // inited by null
      snprintf(lines[0], LCD_WIDTH + 1, "%10ld %02d%c%02d", gps.sls_gps.lat, gps.minutes, cursors[cursorIndex], gps.second);
      snprintf(lines[1], LCD_WIDTH + 1, "%10ld %+05d", gps.sls_gps.lng, gps.sls_gps.alt);
      lcd.setCursor(0, 0);
      lcd.print(lines[0]);
      lcd.setCursor(0, 1);
      lcd.print(lines[1]);
      
      if (++cursorIndex >= sizeof(cursors)/sizeof(cursors[0])) cursorIndex = 0;
    }
#endif

    if (is_valid(gps.sls_gps) && !is_almost_same(gps.sls_gps, gps_history[0].sls_gps)) {
      gps_history.push_front(gps);
      gps_history.resize(GPS_HISTORY_SIZE);
      return true;
    }
  }
  
  return false;
}
#endif

/**
 * @return true if location is roughly valid
 */
bool is_valid(SLS_GPS gps) {
  return gps.lat >= LAT_LONG_MIN && gps.lat <= LAT_LONG_MAX 
    && gps.lng >= LNG_LONG_MIN && gps.lng <= LNG_LONG_MAX 
    && gps.alt >= ALT_MIN && gps.alt <= ALT_MAX;
}

/**
 * @return true if both gps are same location
 */
bool is_almost_same(SLS_GPS lhs, SLS_GPS rhs) {
  return abs(lhs.lat - rhs.lat) < SIGMA_LONG_LATLNG
    && abs(lhs.lng - rhs.lng) < SIGMA_LONG_LATLNG
    && abs(lhs.alt - rhs.alt) < SIGMA_METER_ALT;
}

/**
 * Send gps data sls_gps via lorawan
 */
bool send_location() {
  SLS_GPS sls_gps = gps.sls_gps;
  Serial.print("sending data ");
  Serial.print("lat="); Serial.print(gps.sls_gps.lat);
  Serial.print(", lng="); Serial.print(gps.sls_gps.lng);
  Serial.print(", alt="); Serial.print(gps.sls_gps.alt); Serial.print("(m)　");
  Serial.print(gps.hour); Serial.print(":"); Serial.print(gps.minutes); Serial.print(":"); Serial.print(gps.second); Serial.println("UTC");

#ifdef USE_LCD
  if (view_mode == VIEW_MODE_SEND_GPS_HISTORY) {
    // jump to the latest
    gps_history_index = 0;
    show_gps_history(gps_history_index);
  }
#endif

#ifdef USE_LORAWAN
  // note the endian in the server side
  lorawanClient.sendBinary((byte *)&sls_gps, sizeof(sls_gps));
#endif

  return true;
}

#ifdef USE_LCD
void show_gps_history(int index) {
  const GPS gps = gps_history[index];

  // index may overwrite alt
  // 1234567890123456
  //  320123456 12.34
  // 1230123456 -12#9
  char lines[][LCD_WIDTH + 1] = { {}, {} }; // inited by null
  snprintf(lines[0], LCD_WIDTH + 1, "%10ld %02d%c%02d", gps.sls_gps.lat, gps.minutes, '.', gps.second);
  snprintf(lines[1], LCD_WIDTH + 1, "%10ld %-5d", gps.sls_gps.lng, gps.sls_gps.alt);
  lcd.setCursor(0, 0);
  lcd.print(lines[0]);
  lcd.setCursor(0, 1);
  lcd.print(lines[1]);
  lcd.setCursor(LCD_WIDTH - 2, 1);
  lcd.print('#');
  lcd.print(index);
}
#endif

#ifdef USE_KEY
/** 
 * @return KEY
 */ 
KEY get_key(KEY last_key) {
  const int adc_key_in = analogRead(KEY_ANALOG_PIN);
  // Serial.println(adc_key_in);

  // check debounce
  if (millis() - key_last_get_time > KEY_DEBOUNCE_DELAY_MSEC) {
    const KEY key = get_current_key(adc_key_in);
    key_last_get_time = millis();
    // Serial.println(key);
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
      return (KEY)k;
  }
  return KEY_NONE;
}
#endif