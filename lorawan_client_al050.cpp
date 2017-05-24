#include "Arduino.h"
#include <SoftwareSerial.h>

#include "lorawan_client.h"
#include "lorawan_client_al050.h"

#define ECHO(str) if(echo){Serial.print(str);}
#define ECHOLN(str) if(echo){Serial.println(str);}

const char LoRaWANClientAL050::RESPONSE_PREFIX[] = ">> ";
const char LoRaWANClientAL050::PROMPT_STR[] = "\n\r> ";

LoRaWANClientAL050::LoRaWANClientAL050() : LoRaWANClient(SoftwareSerial(LoRa_RX_PIN,LoRa_TX_PIN)) {
  setPromptStr(PROMPT_STR);
}

String LoRaWANClientAL050::sendCmd(String cmd, bool echo, int waitTime){
  const String response = LoRaWANClient::sendCmd(cmd, echo, waitTime);

  // We currently do not support multi line response like "lorawan set_linkchk" and "lorawan tx".
  int p = response.indexOf(RESPONSE_PREFIX) + 3;
  if (p < 0) p = 0;
  
  int q = response.indexOf(PROMPT_STR, p);
  if (q < 0) q = response.length();
    
  return response.substring(p, q);
}

// You need to override all overloaded methods
bool LoRaWANClientAL050::sendCmd(String cmd, String waitStr, bool echo, int waitTime) {
  return LoRaWANClient::sendCmd(cmd, waitStr, echo, waitTime);
}

bool LoRaWANClientAL050::sendCmd(const char* cmd, const char* waitStr, bool echo, int waitTime) {
  return LoRaWANClient::sendCmd(cmd, waitStr, echo, waitTime);  
}

bool LoRaWANClientAL050::connect(bool force_reconnect){
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
  return true;
}

bool LoRaWANClientAL050::sendData(char *data, short port, CALLBACK p, bool echo){
  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHOLN(port);

  char payload[MAX_PAYLOAD_SIZE*2+1] = "";
  char tmp[3] = "";
  for(int i=0, j=0; i< MAX_PAYLOAD_SIZE ; i++, j+=2)
  {
    if(data[i] == 0x00)
    {
      payload[j] = 0x00;
      break;
    }
    sprintf(tmp,"%0x",data[i]);
    strcat(payload, tmp);
  }

  char cmdLine[32];
  sprintf(cmdLine, "lorawan tx %s %d %s", getTxTypeString(), port, payload);
  ECHOLN(cmdLine);

  return handleTx(cmdLine, p, echo);
}

bool LoRaWANClientAL050::sendData(unsigned long data, short port, CALLBACK p, bool echo){
  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHOLN(port);

  char cmdLine[32];
  sprintf(cmdLine, "lorawan tx %s %d %08lx", getTxTypeString(), port, data);  
  ECHOLN(cmdLine);
  
  return handleTx(cmdLine, p, echo);
}

bool LoRaWANClientAL050::sendBinary(byte *data_pointer, int data_size, short port, CALLBACK p, bool echo){
  if (data_size > MAX_PAYLOAD_SIZE)
  {
    ECHO("ERROR: size of data (");
    ECHO(data_size);
    ECHOLN(" bytes) too big. ");
    return false;
  }

  char cmdLine[MAX_PAYLOAD_SIZE*2+30];
  sprintf(cmdLine, "lorawan tx %s %d ", getTxTypeString(), port);

  byte *b=data_pointer;
  char tmp[]="00";
  for(int i=0; i<data_size; i++,b++)
  {
    sprintf(tmp, "%02x", *b );
    strcat(cmdLine, tmp);
  }
  
  ECHOLN(cmdLine);
  
  return handleTx(cmdLine, p, echo);
}

bool LoRaWANClientAL050::handleTx(char* cmdLine, CALLBACK p, bool echo){
  const String response = sendCmd(cmdLine, true, NETWORK_WAIT_TIME);
  const int posRx = response.indexOf("rx ");
  if (posRx >= 0)
  {
    ECHOLN(" ... received downlink data.");
    if (p != NULL) {
      // parse "rx portnum data\n\r>> tx_sent"
      const int posPrompt = response.indexOf("\n\r", posRx + 3);
      const String rx = response.substring(posRx + 3, posPrompt);

      int portnum;
      const int posDelim = rx.indexOf(" ");
      sscanf(rx.substring(0, posDelim).c_str(), "%d", &portnum);
      
      // You need to have a variable instance here.
      const String dataString = rx.substring(posDelim + 1, rx.length());
      char* data = const_cast<char*>(dataString.c_str());
      p(data, portnum);
    }
    // fall through. rx response also includes tx_ok
  }
  
  if (response.indexOf("tx_ok") >= 0)
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

DataRate LoRaWANClientAL050::getDataRate(){
  constexpr bool echo = true;
  constexpr int waitTime = NETWORK_WAIT_TIME;
  
  const String s = sendCmd("lorawan get_dr", echo, waitTime);

  int dr;
  sscanf(s.c_str(), "%d", &dr);
  
  return static_cast<DataRate>(dr);
}

bool LoRaWANClientAL050::setDataRate(DataRate dr){
  constexpr bool echo = true;
  constexpr int waitTime = NETWORK_WAIT_TIME;

//  if (dr == DR7) {
//    // not supported by AL-050
//    return false;
//  }
  
  const String cmd = String("lorawan set_dr ") + dr;
  return sendCmd(cmd, "Ok", echo, waitTime);
}

String LoRaWANClientAL050::getVersion(bool echo, int waitTime) {
  return sendCmd("mod get_ver", echo, waitTime);
}

String LoRaWANClientAL050::getHardwareDeveui(bool echo, int waitTime) {
  return sendCmd("mod get_hw_deveui", echo, waitTime);  
}

String LoRaWANClientAL050::getHardwareModel(bool echo, int waitTime) {
  return sendCmd("mod get_hw_model", echo, waitTime);
}

String LoRaWANClientAL050::getDeveui(bool echo, int waitTime) {
  return sendCmd("lorawan get_deveui", echo, waitTime);
}

void LoRaWANClientAL050::setTxType(TxType type){
  txType = type;
}

TxType LoRaWANClientAL050::getTxType(){
  return txType;
}

const char* LoRaWANClientAL050::getTxTypeString(){
  // string used in sendCmd
  return txType == TX_TYPE_CONFIRMED ? "cnf" : "ucnf";
}

