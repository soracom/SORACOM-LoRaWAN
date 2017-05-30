#ifndef LoRaWANClient_AL050_h
#define LoRaWANClient_AL050_h

#include "Arduino.h"

#include "lorawan_client.h"

#define LoRa_RX_PIN 11 // Arduino 11pin to LoRa module TX
#define LoRa_TX_PIN 12 // Arduino 12pin to LoRa module RX

/**
 * ABit AL-050
 */
class LoRaWANClientAL050 : public LoRaWANClient {
public:
  static const char RESPONSE_PREFIX[];
  static const char NEWLINE_STR[];
  static const char PROMPT_STR[];
  static const char SEND_CMD_RESPONSE_SEPARATOR[];
  
  LoRaWANClientAL050();

  /**
   * @override
   * @return String - delimited with SEND_CMD_RESPONSE_SEPARATOR if response has multiple lines
   */
  virtual String sendCmd(const String& cmd, int waitTime);
  
  /**
   * Connect to gateway
   */
  virtual bool connect(bool force_reconnect=true);

  /**
   * Send Binary data with data_size to gateway
   */
  virtual bool sendBinary(byte *data_pointer, int data_size, short port=1, CALLBACK pDownLinkCallback=NULL);
  
  /**
   * Send String or char* data to gateway
   */
  virtual bool sendData(const String& msg, short port=1, CALLBACK pDownLinkCallback=NULL);

  /**
   * Send long data to gateway
   */  
  virtual bool sendData(unsigned long data, short port=1, CALLBACK pDownLinkCallback=NULL);

  /** 2.2.2 get_ver */
  String getVersion();

  /** 2.2.3 get_hw_deveui */
  String getHardwareDeveui();

  /** 2.2.4 get_hw_model */
  String getHardwareModel();
  
  /** 2.3.5 set_dr */
  bool setDataRate(DataRate dr);

  /** 2.3.12 get_deveui */
  String getDeveui();
  
  /** 2.3.13 get_dr */
  DataRate getDataRate();

  
  // tx type used in sendData() and sendBinary()
  void setTxType(TxType txType);
  TxType getTxType();

protected:
  TxType txType = TX_TYPE_UNCONFIRMED;
  const char* getTxTypeString();
  bool handleTx(const String& cmdLine, CALLBACK p=NULL);
};

inline String LoRaWANClientAL050::getVersion() {
  return sendLocalCmd("mod get_ver");
}

inline String LoRaWANClientAL050::getHardwareDeveui() {
  return sendLocalCmd("mod get_hw_deveui");
}

inline String LoRaWANClientAL050::getHardwareModel() {
  return sendLocalCmd("mod get_hw_model");
}

inline String LoRaWANClientAL050::getDeveui() {
  return sendNetworkCmd("lorawan get_deveui");
}

inline bool LoRaWANClientAL050::setDataRate(DataRate dr){
//  if (dr == DR7) {
//    // not supported by AL-050
//    return false;
//  }
  
  const String cmd = String("lorawan set_dr ") + dr;
  return sendNetworkCmd(cmd, "Ok");
}

inline void LoRaWANClientAL050::setTxType(TxType type){
  txType = type;
}

inline TxType LoRaWANClientAL050::getTxType(){
  return txType;
}

inline const char* LoRaWANClientAL050::getTxTypeString(){
  // string used in sendCmd
  return txType == TX_TYPE_CONFIRMED ? "cnf" : "ucnf";
}

#endif

