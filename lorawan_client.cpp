#include "Arduino.h"
#include <SoftwareSerial.h>

#include "lorawan_client.h"
#include "lorawan_client_al050.h"
#include "lorawan_util.h"

#define ECHO(str) if(serialPrint){Serial.print(str);}
#define ECHOLN(str) if(serialPrint){Serial.println(str);}

LoRaWANClient* LoRaWANClient::create(Device device) {
  switch (device) {
    case DEVICE_AL050:
      return new LoRaWANClientAL050();
      break;
    default:
      return NULL;
      break;
  }
}

LoRaWANClient::LoRaWANClient(SoftwareSerial softwareSerial) : ss(softwareSerial) {
}

bool LoRaWANClient::sendCmd(const String& cmd, const String& waitStr, int waitTime){
  const String str = sendCmd(cmd, waitTime);
  
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0) return true;

  return false;
}

String LoRaWANClient::sendCmd(const String& cmd, int waitTime){
  // send
  ECHO("sendCmd: ");
  ECHOLN(cmd);
  ss.listen();
  ss.print(cmd);
  ss.print('\r');
  delay(100);

  // receive response
  String str;
  const unsigned long tim = millis() + waitTime - 100;
  while (millis() < tim) {
    while (ss.available() > 0) {
      const char ch = ss.read();
      ECHO(ch);
      str += String(ch);
    }
  }

  return str;
}

