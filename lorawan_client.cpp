#include "lorawan_client.h"
#include "Arduino.h"
#define ECHO(str) if(echo){Serial.print(str);}
#define ECHOLN(str) if(echo){Serial.println(str);}

LoRaWANClient::LoRaWANClient() {
  ss.begin(9600);
  ss.setTimeout(SERIAL_WAIT_TIME);
}

bool LoRaWANClient::connect(){
  int waitTime=INIT_WAIT_TIME;
  String cmd;

  ss.listen();
  sendCmd("\n", "",false);
  while (ss.available() > 0) {
    char ch = ss.read();
    Serial.print(ch);
  }

  //
  // LoRa module status clear
  //
  if (!sendCmd("mod factory_reset", "Ok", false, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod set_echo off", "Ok", false, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod save", "Ok", false, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod reset", "", false, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }

  //
  // LoRa module various value get
  //

  if (!sendCmd("mod get_hw_model", "", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod get_ver", "", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("lorawan get_deveui", "", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("lorawan get_appkey", "", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }

  //
  // LoRa module initialize for Japan
  //
  if (!sendCmd("lorawan set_ch_plan AS923 3 1 1 0", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("lorawan set_appeui abcdef0123456789", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  // Set DataRate 5 (SF7/BW125kHz) for receive CFList to join (payload size 242bytes)
  if (!sendCmd("lorawan set_dr 5", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }

  if (!sendCmd("lorawan save", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }

  // LoRa module join to Network Server by OTAA
  //
  int retry=0;
  while (!sendCmd("lorawan join otaa", "accepted", true, NETWORK_WAIT_TIME)) {
    retry++;
    Serial.print("'lorawan join otaa' Failed (");
    Serial.print(retry);
    Serial.print("/");
    Serial.print(JOIN_RETRY_MAX);
    Serial.println(")");
    if(retry == JOIN_RETRY_MAX)
    {
      Serial.println("Exceeded JOIN_RETRY_MAX attempts.");
      return false;
    }
  }
  // Reset DataRate 2 (SF10/BW125kHz) before join (payload size 11bytes)
  if (!sendCmd("lorawan set_dr 2", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }

  return true;
}

bool LoRaWANClient::sendCmd(String cmd, String waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME){
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
      if (str.indexOf("\n> ") >= 0) break;
    }
  }
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0) return true;

  return false;
}

bool LoRaWANClient::sendData(char *data, short port=1, CALLBACK p=NULL, bool echo=true){
  char cmdLine[32];

  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHO(port);
  sprintf(cmdLine, "lorawan tx ucnf %d %s", port, data);
  if(sendCmd(cmdLine, "tx_ok", true, SERIAL_WAIT_TIME))
  {
    ECHOLN(" ... sent.");
    return true;
  }
  else
  {
    ECHOLN(" ... failed.");
    return false;
  }
}
