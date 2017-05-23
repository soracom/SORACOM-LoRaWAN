#include "Arduino.h"
#include <SoftwareSerial.h>

#include "lorawan_client.h"
#include "lorawan_client_al050.h"

#define ECHO(str) if(echo){Serial.print(str);}
#define ECHOLN(str) if(echo){Serial.println(str);}

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
  ss.begin(9600);
  ss.setTimeout(SERIAL_WAIT_TIME);
}

bool LoRaWANClient::sendCmd(String cmd, String waitStr, bool echo, int waitTime){
  const String str = sendCmd(cmd, echo, waitTime);
  
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0) return true;

  return false;
}

String LoRaWANClient::sendCmd(String cmd, bool echo, int waitTime){
  unsigned long tim;
  String str;

  ECHO("sendCmd: ");
  ECHOLN(cmd);
  ss.listen();
  ss.print(cmd);
  ss.print('\r');
  delay(100);
  tim = millis() + waitTime - 100;
  while (millis() < tim) {
    if(ss.available() > 0) {
      char ch = ss.read();
      ECHO(ch);
      str += String(ch);
      if (str.indexOf(promptStr) >= 0) break;
    }
  }

  return str;
}

void LoRaWANClient::setPromptStr(const String& prompt){
  promptStr = prompt;
}

