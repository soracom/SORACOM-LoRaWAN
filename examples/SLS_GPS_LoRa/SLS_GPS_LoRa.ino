/*
 * SLS_GPS_LoRa
 * (c) 2017 SORACOM, INC.
 */

#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <lorawan_client.h>
#include <lorawan_client_al050.h>

// If you are using other pins, update accordingly
#define GPS_TX_PIN  8 // Arduino 8pin to GPS module TX
#define GPS_RX_PIN  9 // Arduino 9pin to GPS module RX

// Define to send data via LORAWAN
#define USE_LORAWAN

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

SoftwareSerial gpsSerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPS tinyGps;

#ifdef USE_LORAWAN
LoRaWANClientAL050 lorawanClient;
#endif

unsigned long updated_time = 0L;
unsigned long sent_time = 0L;

// The struct to sent data via LoRaWAN. Max 11 bytes
struct SLS_GPS {
  byte header; // LORAWAN_GPS_HEADER
  long lat;    // Can store as MILLIONTHs of a degree
  long lng;    // 180 is stored as 180,000,000
  short alt;   // -32,768 < alt < 32,767 (meter)
};
SLS_GPS sls_gps = {LORAWAN_GPS_HEADER, 0, 0, 0};
SLS_GPS sls_gps_prev = {LORAWAN_GPS_HEADER, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  Serial.println("Connecting ... ");
  gpsSerial.begin(9600);

#ifdef USE_LORAWAN
  if (!lorawanClient.connect()) {
    Serial.println(" failed to connect. Halt...");
    for(;;){};
  }
#endif
}

void loop() {
  const bool is_updated = get_location();
  if (is_updated)
    updated_time = millis();

  // Send data
  // Note that sending can happen in a different loop from the updated one.
  // Unsigned operation should work after millis overflow. 
  if (updated_time > sent_time && (sent_time == 0 || millis() - sent_time > SEND_INTERVAL)) {
    const bool is_sent = send_location();
    if (is_sent) {
      sent_time = millis();
      sls_gps_prev = sls_gps;
    }
  }
}

/**
 * Set updated data to sls_gps variable
 * 
 * @return true if the sensor detects the new location
 *   The new gps data is stored in sls_gps
 */
bool get_location() {
  // activate GPS serial
  gpsSerial.listen();
  while (gpsSerial.available()) {
    const int c = gpsSerial.read();
    // Serial.print((char)c);
    if (tinyGps.encode(c)) {
      // process new gps info
      // retrieves +/- lat/long in 100000ths of a degree
      tinyGps.get_position(&sls_gps.lat, &sls_gps.lng);
      sls_gps.alt = tinyGps.altitude() / 100;

      byte hour, minutes, second;
      tinyGps.crack_datetime(NULL, NULL, NULL, &hour, &minutes, &second, NULL);
      
      Serial.print("got data ");
      Serial.print("lat="); Serial.print(sls_gps.lat);
      Serial.print(", lng="); Serial.print(sls_gps.lng);
      Serial.print(", alt="); Serial.print(sls_gps.alt); Serial.print("(m)　");
      Serial.print(hour); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.print(second); Serial.println("UTC");

      if (is_valid(sls_gps) && !is_almost_same(sls_gps, sls_gps_prev))
        return true;
    }
  }
  
  return false;
}

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
  Serial.print("sending data ");
  Serial.print("lat="); Serial.print(sls_gps.lat / 1000000.0D, 9);
  Serial.print(", lng="); Serial.print(sls_gps.lng / 1000000.0D, 9);
  Serial.print(", alt="); Serial.print(sls_gps.alt); Serial.println("(m)");

#ifdef USE_LORAWAN
  // note the endian in the server side
  lorawanClient.sendBinary((byte *)&sls_gps, sizeof(sls_gps));
#endif

  return true;
}
