#ifndef LoRaWAN_UTIL_h
#define LoRaWAN_UTIL_h

#include "Arduino.h"
#include "lorawan_client.h"

/*
 * Utility functions to work with lorawan_client
 * You may want to call them directly
 */

/**
 * Create Json String to be sent via lorawan.
 * Note that there is a strict limitation of the data length, so key should be generally single char.
 * e.g. Minimum json data {"a":1} spend 7 bytes
 */
template <typename T> String createJson(const String& key, T value);
template <> String createJson(const String& key, const String& value);
template <> String createJson(const String& key, const char* p);
/**
 * Digit is adjusted automatically to best fit in the length limitation.
 * Note that the value is truncated rather than rounded.
 */
String createJson(const String& key, double value);


/**
 * String to hex string
 * "abcdefghij" => "6162636465666768696a"
 */
String stringToHexString(const String& data);

/**
 * byte* to hex string
 */
String bytesToHexString(const byte* data, int data_size);


/** verify if it can be sent via lorawan */
bool verifyPayloadLength(const String& payload);
/** called by createJson */
String escapeJsonString(const String& key);

/**
 * dtostrf char* length may be longer than width. This function guarantees chars is equal to or shorter than maxWholeWidth
 * note that the val is truncated rather than rounded as of dtostrf
 */
char* dtostrfMaxWidth(double val, unsigned int maxWholeWidth, unsigned int maxPrecWidth, char* s);


/*
 * Template function implementations
 */

template <typename T> String createJson(const String& key, T value){
  const String jsonString(String("{\"") + escapeJsonString(key) + "\":" + value + '}');
  verifyPayloadLength(jsonString);
  return jsonString;
}

#endif

