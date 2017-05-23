#ifndef LoRaWANClient_h
#define LoRaWANClient_h

#include "Arduino.h"
#include <SoftwareSerial.h>

#define INIT_WAIT_TIME 1000
#define SERIAL_WAIT_TIME 1000
#define NETWORK_WAIT_TIME 5000
#define JOIN_RETRY_INTERVAL 5000
#define JOIN_RETRY_MAX 0 // 0 = unlimited
#define MAX_PAYLOAD_SIZE 11

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
   * Send raw command to gateway
   * 
   * @return String response from gateway. Raw string including "tx_ok", "err" and "Ok"
   */
  virtual String sendCmd(String cmd, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  
  /**
   * Send raw command to gateway
   * 
   * @param cmd command to transmit
   * @param waitStr text to search from the response
   * @return true if waitStr is found in the response
   */
  virtual bool sendCmd(String cmd, String waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME);
  
  virtual inline bool sendCmd(const char* cmd, const char* waitStr, bool echo=true, int waitTime=SERIAL_WAIT_TIME) {
    // this function is defined to avoid ambiguous overload when you pass "const char[]" only
    return sendCmd(String(cmd), String(waitStr), echo, waitTime);
  }

  
  /**
   * Connect to gateway
   */
  virtual bool connect(bool force_reconnect=true) = 0;

  /**
   * Send Binary data with data_size to gateway
   */
  virtual bool sendBinary(byte *data_pointer, int data_size, short port=1, CALLBACK p=NULL, bool echo=true) = 0;
  
  /**
   * Send char* data to gateway
   */
  virtual bool sendData(char *msg, short port=1, CALLBACK p=NULL, bool echo=true) = 0;

  /**
   * Send long data to gateway
   */  
  virtual bool sendData(unsigned long, short port=1, CALLBACK p=NULL, bool echo=true) = 0;
  
protected:
  SoftwareSerial ss;
  String promptStr;
  
  /**
   * Instead of calling the constructor directly,
   * call factory method create() or constructors of concrete objects
   */
  LoRaWANClient(SoftwareSerial serial);

  /**
   * Set the prompt string which is used as a stop string in sendCmd()
   */
  void setPromptStr(const String& prompt);
};

#endif
