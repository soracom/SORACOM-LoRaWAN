#include <lorawan_client.h>
#define INTERVAL 10000 // 10000 msec = 10 sec

LoRaWANClient client;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting example sketch 'Uptime'.\n");
  Serial.println("Connecting ... \n");
  if(! client.connect())
  {
    Serial.println(" failed to connect. Halt...");
    for(;;){};
  }
  Serial.println("\nConnected.\n");
}

void loop() {
  unsigned long t;
  t=millis();

  client.sendData(millis()/1000); // Arduino uptime in seconds.

  delay(t+INTERVAL-millis()); // Wait for next loop
}
