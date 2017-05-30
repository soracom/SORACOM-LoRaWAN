#include "Arduino.h"
#include <SoftwareSerial.h>

#include "lorawan_client.h"
#include "lorawan_client_al050.h"
#include "lorawan_util.h"

#define SERIAL_BAUDRATE 9600
#define SERIAL_TIMEOUT 1000

#define JOIN_RETRY_MAX 0 // 0 = unlimited

#define ECHO(str) if(serialPrint){Serial.print(str);}
#define ECHOLN(str) if(serialPrint){Serial.println(str);}

const char LoRaWANClientAL050::RESPONSE_PREFIX[] = ">> ";
const char LoRaWANClientAL050::NEWLINE_STR[] = "\n\r";
const char LoRaWANClientAL050::PROMPT_STR[] = "\n\r> ";
const char LoRaWANClientAL050::SEND_CMD_RESPONSE_SEPARATOR[] = "\n";

const constexpr size_t RESPONSE_PREFIX_LENGTH = sizeof(LoRaWANClientAL050::RESPONSE_PREFIX) - 1;
const constexpr size_t PROMPT_STR_LENGTH = sizeof(LoRaWANClientAL050::PROMPT_STR) - 1;

LoRaWANClientAL050::LoRaWANClientAL050() : LoRaWANClient(SoftwareSerial(LoRa_RX_PIN,LoRa_TX_PIN)) {
  ss.begin(SERIAL_BAUDRATE);
  ss.setTimeout(SERIAL_TIMEOUT);
}

/**
 * Remove the prompt chars and return the response content itself
 * Multiple lines are delimited with the single char '\n'
 * e.g.
 * "res1\n"
 * "res2"
 * from
 * ">> res1\n\r"
 * ">> res2\n\r"
 * "> "
 */
String LoRaWANClientAL050::sendCmd(const String& cmd, int waitTime){
  String response = LoRaWANClient::sendCmd(cmd, waitTime);

  // Serial.println("#" + response + "#");
  int p = response.indexOf(RESPONSE_PREFIX);
  p = p < 0 ? 0 : p + RESPONSE_PREFIX_LENGTH;

  // Compute the prompt position
  // Note that some commands like "lorawan join" returns multiple PROMPT_STR.
  // Some commands like "mod reset" returns response after PROMPT_STR
  // We do not try too hard to parse these kind of response.
  int q = response.length();
  for (int q_next = response.indexOf(PROMPT_STR, p);
    q_next >= 0;
    q = q_next, q_next = response.indexOf(PROMPT_STR, q + 1))
  {
    /*
    // Consider an empty prompt line as the end of response
    // We may chop it, but keep as is to rescue the delayed response from the previous command
    if (response.charAt(q_next + PROMPT_STR_LENGTH) == NEWLINE_STR[0]) {
      q = q_next;
      break;
    }
    */
  }

  response = response.substring(p, q);

  // Deal with the multi line response like "lorawan join"
  response.replace(String(NEWLINE_STR) + RESPONSE_PREFIX, SEND_CMD_RESPONSE_SEPARATOR);
  response.replace(PROMPT_STR, SEND_CMD_RESPONSE_SEPARATOR);
  
  // Serial.println("!" + response + "!");
  return response;
}

bool LoRaWANClientAL050::connect(bool force_reconnect){
  ss.listen();

  sendLocalCmd("\n");
  while (ss.available() > 0) {
    char ch = ss.read();
    ECHO(ch);
  }

  if(!force_reconnect)
  {
    ECHOLN("Checking if already joined or not ... ");
    if (!sendLocalCmd("lorawan get_join_status", "unjoined")) {
      ECHOLN("already joined.");
      return true;
    }
    ECHOLN("unjoined.");
  }

  //
  // LoRa module status clear
  //
  if (!sendLocalCmd("mod factory_reset", "Ok")) {
    ECHOLN("Request Failed");
    return false;
  }
  if (!sendLocalCmd("mod set_echo off", "Ok")) {
    ECHOLN("Request Failed");
    return false;
  }
  if (!sendLocalCmd("mod save", "Ok")) {
    ECHOLN("Request Failed");
    return false;
  }
  if (!sendLocalCmd("mod reset")) {
    ECHOLN("Request Failed");
    return false;
  }

  //
  // LoRa module various value get
  //

  if (!sendLocalCmd("mod get_hw_model")) {
    ECHOLN("Request Failed");
    return false;
  }
  if (!sendLocalCmd("mod get_ver")) {
    ECHOLN("Request Failed");
    return false;
  }
  if (!sendLocalCmd("lorawan get_deveui")) {
    ECHOLN("Request Failed");
    return false;
  }

  // LoRa module join to Network Server by OTAA
  //
  int retry=0;
  while (!sendNetworkCmd("lorawan join otaa", "accepted")) {
    retry++;
    ECHO("'lorawan join otaa' Failed (");
    ECHO(retry);
    ECHO("/");
    ECHO(JOIN_RETRY_MAX);
    ECHOLN(")");
    if(retry == JOIN_RETRY_MAX)
    {
      ECHOLN("Exceeded JOIN_RETRY_MAX attempts.");
      return false;
    }
  }
  return true;
}

bool LoRaWANClientAL050::sendData(const String& msg, short port, CALLBACK p){
  ECHO("sending '");
  ECHO(msg);
  ECHO("' to port ");
  ECHOLN(port);
  
  byte* bp = (byte *)msg.c_str();
  const String payloadHex = bytesToHexString(bp, msg.length());
  const String cmdLine = String("lorawan tx ") + getTxTypeString() + ' ' + String(port) + ' ' + payloadHex;
  ECHOLN(cmdLine);
  return handleTx(cmdLine, p);
}

/*
bool LoRaWANClientAL050::sendData(char *msg, short port, CALLBACK p){
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

  return handleTx(cmdLine, p);

  const int data_size = strlen(data);
  const String payloadHex = bytesToHexString((byte *)data, data_size);
  const String cmdLine = String("lorawan tx ") + getTxTypeString() + ' ' + String(port) + ' ' + payloadHex;
  ECHOLN(cmdLine);
  return handleTx(cmdLine, p);
}
  */

bool LoRaWANClientAL050::sendData(unsigned long data, short port, CALLBACK p){
  ECHO("sending '");
  ECHO(data);
  ECHO("' to port ");
  ECHOLN(port);

  char cmdLine[32];
  sprintf(cmdLine, "lorawan tx %s %d %08lx", getTxTypeString(), port, data);  
  ECHOLN(cmdLine);
  
  return handleTx(cmdLine, p);
}

bool LoRaWANClientAL050::sendBinary(byte *data_pointer, int data_size, short port, CALLBACK pDownLinkCallback){
  if (data_size > MAX_PAYLOAD_SIZE)
  {
    ECHO("ERROR: size of data (");
    ECHO(data_size);
    ECHOLN(" bytes) too big. ");
    return false;
  }

  const String payloadHex = bytesToHexString(data_pointer, data_size);
  const String cmdLine = String("lorawan tx ") + getTxTypeString() + ' ' + String(port) + ' ' + payloadHex;
  ECHOLN(cmdLine);
  return handleTx(cmdLine, pDownLinkCallback);
}


bool LoRaWANClientAL050::handleTx(const String& cmdLine, CALLBACK pDownLinkCallback){
  const String response = sendNetworkCmd(cmdLine);
  
  const int posRx = response.indexOf("rx ");
  if (posRx >= 0)
  {
    // There are downlink data
    ECHOLN(" ... received downlink data.");
    if (pDownLinkCallback != NULL) {
      // parse "rx portnum data\n\r>> tx_sent"
      const int posPrompt = response.indexOf("\n\r", posRx + 3);
      const String rx = response.substring(posRx + 3, posPrompt);

      int portnum;
      const int posDelim = rx.indexOf(" ");
      sscanf(rx.substring(0, posDelim).c_str(), "%d", &portnum);
      
      // You need to have a variable instance here.
      const String dataString = rx.substring(posDelim + 1, rx.length());
      char* data = const_cast<char*>(dataString.c_str());
      
      pDownLinkCallback(data, portnum);
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
  const String s = sendLocalCmd("lorawan get_dr");

  int dr;
  sscanf(s.c_str(), "%d", &dr);
  
  return static_cast<DataRate>(dr);
}

