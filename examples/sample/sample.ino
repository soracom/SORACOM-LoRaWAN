#include <lorawan_client.h>

LoRaWANClient client;

void setup() {
  client.init();
}

void loop() {
  char *data="Hello! LoRa";
  client.sendData(data);
  delay(10000);
}
