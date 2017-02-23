#ifndef LoRaWANClient_h
#define LoRaWANClient_h
#include "SoftwareSerial.h"

#define LoRa_RX_PIN 11 // Arduino 11pin to LoRa module TX
#define LoRa_TX_PIN 12 // Arduino 12pin to LoRa module RX
#define INIT_WAIT_TIME 1000
#define SERIAL_WAIT_TIME 1000
#define NETWORK_WAIT_TIME 10000
#define JOIN_RETRY_MAX 10

typedef void (* CALLBACK)(char *, int);

class LoRaWANClient {
private:
  SoftwareSerial ss = SoftwareSerial(LoRa_RX_PIN,LoRa_TX_PIN);

public:
  LoRaWANClient();
  bool connect();
  bool sendCmd(String cmd, String waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  bool sendData(char *msg, short port=1, CALLBACK p=NULL, bool echo=true);
};

#endif
