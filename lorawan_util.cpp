#include "Arduino.h"
#include "lorawan_client.h"
#include "lorawan_util.h"

/*
 * Utility functions to work with lorawan_client
 */

template <> String createJson(const String& key, const String& value){
  const String jsonString(String("{\"") + escapeJsonString(key) + "\":\"" + escapeJsonString(value) + "\"}");
  verifyPayloadLength(jsonString);
  return jsonString;
}

template <> String createJson(const String& key, const char* p){
  const String s(p);
  const String jsonString(createJson<const String&>(key, s));
  verifyPayloadLength(jsonString);
  return jsonString;
}

template <> String createJson(const String& key, double value){
  const String pre("{\"");
  const String key_esc(escapeJsonString(key));
  const String delim("\":");
  const String post('}');

  // if 1 is chosen, verifyJson will return false
  const int valueMaxWidth = max(1, MAX_PAYLOAD_SIZE - pre.length() - key_esc.length() - delim.length() - post.length());
  char s[valueMaxWidth + 1];
  dtostrfMaxWidth(value, valueMaxWidth, valueMaxWidth, s);
  
  const String jsonString(pre + key_esc + delim + s + post);
  verifyPayloadLength(jsonString);
  return jsonString;
}

String stringToHexString(const String& data) {
  byte* bp = (byte *)data.c_str();
  return bytesToHexString(bp, data.length());
}

String bytesToHexString(const byte* data, int data_size) {
  String payload;
  for (int i = 0; i < data_size; ++i) {
    // %02x
    const String s = String(data[i], 16);
    payload += data[i] >= 10 ? s : String('0') + s;
  }
  return payload;
}


bool verifyPayloadLength(const String& payload){
  if (payload.length() > MAX_PAYLOAD_SIZE) {
    Serial.println("json exceeds lorawan max payload and will be truncated: " + payload);
    return false;
  }
  return true;
}

String escapeJsonString(const String& key){
  String esc(key); // need non const instance to call replace
  esc.replace("\"", "\\\"");
  return esc;
}

char* dtostrfMaxWidth(double val, unsigned int maxWholeWidth, unsigned int maxPrecWidth, char* s) {
  const int signWidth = val < 0 ? 1 : 0;
  
  const double absVal = abs(val);
  int intWidth = 1;
  for (double cmp = 10.0; cmp < absVal; ++intWidth, cmp *= 10)
    ;

  // 1 is dot in 1.2
  const int prec = min(maxPrecWidth, (unsigned)max(0, (int)maxWholeWidth - signWidth - intWidth - 1));
  
  const int width = signWidth + intWidth + 1 + prec;
  // Serial.println(String("!") + maxWholeWidth + ">" + width + "=" + signWidth + ":" + intWidth + "." + prec);
  
  dtostrf(val, width, prec, s);
  return s;
}

