#include "lorawan_client.h"
#define ECHO(str) if(echo){Serial.print(str);}
#define ECHOLN(str) if(echo){Serial.println(str);}

LoRaWANClient::LoRaWANClient() {
  ss.begin(9600);
  ss.setTimeout(SERIAL_WAIT_TIME);
}

bool LoRaWANClient::connect(bool force_reconnect=true){
  int waitTime=INIT_WAIT_TIME;
  String cmd;

  ss.listen();

  sendCmd("\n", "",false);
  while (ss.available() > 0) {
    char ch = ss.read();
    Serial.print(ch);
  }

  if(!force_reconnect)
  {
    Serial.println("Checking if already joined or not ... ");
    if (!sendCmd("lorawan get_join_status", "unjoined", true, waitTime)) {
      Serial.println("already joined.");
      return true;
    }
    Serial.println("unjoined.");
  }

  //
  // LoRa module status clear
  //
  if (!sendCmd("mod factory_reset", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod set_echo off", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod save", "Ok", true, waitTime)) {
    Serial.println("Request Failed");
    return false;
  }
  if (!sendCmd("mod reset", "", true, waitTime)) {
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

  //
  // LoRa module initialize for Japan
  //
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
  while (!sendCmd("lorawan join otaa", "accepted", true, JOIN_RETRY_INTERVAL)) {
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
  int i,j;
  char cmdLine[32];
  char payload[MAX_PAYLOAD_SIZE*2+1]="";
  char tmp[3]="";

  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHOLN(port);

  for(i=0, j=0; i< MAX_PAYLOAD_SIZE ; i++, j+=2)
  {
    if(data[i] == 0x00)
    {
      payload[j] = 0x00;
      break;
    }
    sprintf(tmp,"%0x",data[i]);
    strcat(payload, tmp);
  }

  sprintf(cmdLine, "lorawan tx ucnf %d %s", port, payload);
  ECHOLN(cmdLine);

  if(sendCmd(cmdLine, "tx_ok", true, NETWORK_WAIT_TIME))
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

bool LoRaWANClient::sendData(unsigned long data, short port=1, CALLBACK p=NULL, bool echo=true){
  int i,j;
  char cmdLine[32];
  char tmp[3]="";

  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHOLN(port);

  sprintf(cmdLine, "lorawan tx ucnf %d %08lx", port, data);
//  ECHOLN(cmdLine);

  if(sendCmd(cmdLine, "tx_ok", true, NETWORK_WAIT_TIME))
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

bool LoRaWANClient::sendBinary(byte *data_pointer, int data_size, short port=1, CALLBACK p=NULL, bool echo=true){
  char cmdLine[MAX_PAYLOAD_SIZE*2+30], tmp[]="00";
  int i;
  byte *b;
  b=data_pointer;

  sprintf(cmdLine, "lorawan tx ucnf %d ", port);
  for(i=0;i<data_size;i++,b++)
  {
    sprintf(tmp, "%02x", *b );
    strcat(cmdLine, tmp);
  }
  ECHOLN(cmdLine);
  if(sendCmd(cmdLine, "tx_ok", true, NETWORK_WAIT_TIME))
  {
    ECHOLN(" ... sent.");
    return true;
  }
  else
  {
    ECHOLN(" ... failed.");
    return false;
  }

  return true;
}
