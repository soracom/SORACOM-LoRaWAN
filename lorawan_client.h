#ifndef LoRaWANClient_h
#define LoRaWANClient_h

#include "Arduino.h"
#include <SoftwareSerial.h>

#define MAX_PAYLOAD_SIZE 11
#define DEFAULT_LOCAL_CMD_WAIT_TIME 1000
#define DEFAULT_NETWORK_CMD_WAIT_TIME 6000

typedef void (* CALLBACK)(char *, int);

enum DataRate { DR0 = 0, DR1 = 1, DR2 = 2, DR3 = 3, DR4 = 4, DR5 = 5, DR6 = 6, DR7 = 7 };
enum TxType { TX_TYPE_CONFIRMED, TX_TYPE_UNCONFIRMED };

/**
 * Abtract class to be extended by the each device classes
 */
class LoRaWANClient {
public:
  enum Device { DEVICE_AL050 };

  /**
   * factory method
   * 
   * @return null if the corresponding device object is not found.
   */
  static LoRaWANClient* create(Device device);

  /**
   * Send raw command to module
   * 
   * @return String response from gateway. Raw string including "tx_ok", "err" and "Ok"
   */
  virtual String sendLocalCmd(const String& cmd);
  virtual String sendNetworkCmd(const String& cmd);
  virtual String sendCmd(const String& cmd, int waitTime);
  
  /**
   * Send raw command to module
   * 
   * @param cmd command to transmit
   * @param waitStr text to search from the response
   * @return true if waitStr is found in the response
   */
  virtual bool sendLocalCmd(const String& cmd, const String& waitStr);
  virtual bool sendNetworkCmd(const String& cmd, const String& waitStr);
  virtual bool sendCmd(const String& cmd, const String& waitStr, int waitTime);
    
  /**
   * Connect to gateway
   */
  virtual bool connect(bool force_reconnect=true) = 0;

  /**
   * Send Binary data with data_size to gateway
   */
  virtual bool sendBinary(byte *data_pointer, int data_size, short port=1, CALLBACK p=NULL) = 0;
  
  /**
   * Send String or char* data to gateway
   */
  virtual bool sendData(const String& msg, short port=1, CALLBACK p=NULL) = 0;

  /**
   * Send long data to gateway
   */  
  virtual bool sendData(unsigned long, short port=1, CALLBACK p=NULL) = 0;

  void enableSerialPrint();
  void disableSerialPrint();

  void setLocalWaitTime(int waitTime);
  void setNetworkWaitTime(int waitTime);

protected:
  SoftwareSerial ss;
  bool serialPrint = true;
  int localWaitTime = DEFAULT_LOCAL_CMD_WAIT_TIME;
  int networkWaitTime = DEFAULT_NETWORK_CMD_WAIT_TIME;
  
  /**
   * Instead of calling the constructor directly,
   * call factory method create() or constructors of concrete objects
   */
  LoRaWANClient(SoftwareSerial serial);
};

inline String LoRaWANClient::sendLocalCmd(const String& cmd){
  return sendCmd(cmd, localWaitTime);
}

inline String LoRaWANClient::sendNetworkCmd(const String& cmd){
  return sendCmd(cmd, networkWaitTime);
}

inline bool LoRaWANClient::sendLocalCmd(const String& cmd, const String& waitStr){
  return sendCmd(cmd, waitStr, localWaitTime);
}

inline bool LoRaWANClient::sendNetworkCmd(const String& cmd, const String& waitStr){
  return sendCmd(cmd, waitStr, networkWaitTime);
}

inline void LoRaWANClient::enableSerialPrint() {
  serialPrint = true;
}

inline void LoRaWANClient::disableSerialPrint() {
  serialPrint = false;
}

inline void LoRaWANClient::setLocalWaitTime(int waitTime) {
  localWaitTime = waitTime;
}

inline void LoRaWANClient::setNetworkWaitTime(int waitTime) {
  networkWaitTime = waitTime;
}

#endif
