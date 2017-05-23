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
  static const char PROMPT_STR[];
  
  LoRaWANClientAL050();

  virtual String sendCmd(String cmd, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  virtual bool sendCmd(String cmd, String waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  virtual bool sendCmd(const char* cmd, const char* waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  
  /**
   * Connect to gateway
   */
  virtual bool connect(bool force_reconnect=true);

  /**
   * Send Binary data with data_size to gateway
   */
  virtual bool sendBinary(byte *data_pointer, int data_size, short port=1, CALLBACK p=NULL, bool echo=true);
  
  /**
   * Send char* data to gateway
   */
  virtual bool sendData(char *msg, short port=1, CALLBACK p=NULL, bool echo=true);

  /**
   * Send long data to gateway
   */  
  virtual bool sendData(unsigned long, short port=1, CALLBACK p=NULL, bool echo=true);


  /** 2.2.2 get_ver */
  String getVersion(bool echo=true, int waitTime=SERIAL_WAIT_TIME);

  /** 2.2.3 get_hw_deveui */
  String getHardwareDeveui(bool echo=true, int waitTime=SERIAL_WAIT_TIME);

  /** 2.2.4 get_hw_model */
  String getHardwareModel(bool echo=true, int waitTime=SERIAL_WAIT_TIME);

  /** 2.3.5 set_dr */
  bool setDataRate(DataRate dr);

  /** 2.3.12 get_deveui */
  String getDeveui(bool echo=true, int waitTime=NETWORK_WAIT_TIME);
  
  /** 2.3.13 get_dr */
  DataRate getDataRate();

  
  // tx type used in sendData() and sendBinary()
  void setTxType(TxType txType);
  TxType getTxType();

private:
  TxType txType = TX_TYPE_UNCONFIRMED;
  const char* getTxTypeString();
};

#endif

