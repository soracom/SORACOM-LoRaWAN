/*
 * Sample code of downlink callback
 */

#include <lorawan_client.h>
#include <lorawan_client_al050.h>

LoRaWANClientAL050* pLorawanClient = (LoRaWANClientAL050*)LoRaWANClient::create(LoRaWANClient::DEVICE_AL050);

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting ... ");

  if(!pLorawanClient->connect())
  {
    Serial.println(" failed to connect. Halt...");
    for(;;){};
  }

  Serial.println("Setting confirmed mode");
  pLorawanClient->setTxType(TX_TYPE_CONFIRMED);
}

/**
 * callback of downlink data
 */
void fn(char *data, int portnum) {
  Serial.println("got callback");
  Serial.print("data=");
  Serial.println(data);
  Serial.print("port=");
  Serial.println(portnum);
}

void loop() {
  char data[] = "cnf data";

  pLorawanClient->sendData(data, 1, &fn);

  delay(10000);
}