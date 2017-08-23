#include <lorawan_client.h>
#define LED_PIN 13

LoRaWANClient client;

bool ledOn = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.print("Connecting ... ");
  if(! client.connect())
  {
    Serial.println(" failed to connect. Halt...");
    for(;;){};
  }
}

void loop() {
  char *data="Hello! LoRa";

  Serial.print("Sending ... ");
  Serial.println(data);
  client.sendData(data, 1, cb);

  if (ledOn) {
    digitalWrite(LED_PIN, HIGH);  
  }
  delay(10000);
  
  digitalWrite(LED_PIN, LOW);
}

void cb(String s) {
  Serial.print("Received: '");
  Serial.print(s);
  Serial.println("'");
  ledOn = true;
}

