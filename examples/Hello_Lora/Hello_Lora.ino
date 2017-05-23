/*
 * Sample code to connect and say hello
 */

#include <lorawan_client.h>
#include <lorawan_client_al050.h>

LoRaWANClientAL050 client;

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting ... ");
  if(!client.connect())
  {
    Serial.println(" failed to connect. Halt...");
    for(;;){};
  }
}

void loop() {
  char data[] = "Hello! LoRa";

  Serial.print("Sending ... ");
  Serial.println(data);
  client.sendData(data);
  delay(10000);
}
