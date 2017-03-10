/*
  Sample code to send temperature to LoRaWAN

  Requirement: 
    https://github.com/PaulStoffregen/OneWire
    https://github.com/milesburton/Arduino-Temperature-Control-Library

  Circuit:
    DS18B20
      VCC to Arduino DIGITAL 9
      DQ  to Arduino DIGITAL 10 , 
      GND to Arduino GND
      
    Resistor(4.7k ohm)
      between DS18B20 VCC and DQ
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <lorawan_client.h>

#define POWER_PIN     9   // Power supply for sensor
#define SENSOR_PIN   10   // For sensor data
#define SENSER_BIT   12   // Sensor Resolution

#define INTERVAL   60000  // Interval between sending data(ms)
#define WAKEUP_WAIT 1000  // Time to wait for sensor(ms)

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

// LoRaWAN Client
LoRaWANClient client;

void setup(void) {
  Serial.begin(9600);
  sensors.setResolution(SENSER_BIT);
  pinMode(POWER_PIN, OUTPUT);

  // Join
  client.connect();
}

void loop(void) {
  float s;
  unsigned long start_time=millis();
  char t[10];
  char payload[12];
  
  Serial.print("loop started at ");
  Serial.println(start_time);

  // Wake up sensor
  digitalWrite(POWER_PIN, HIGH);
  delay(WAKEUP_WAIT);

  // Request data and convert to string
  sensors.requestTemperatures();
  s=sensors.getTempCByIndex(0);
  dtostrf(s, -1, 1, t);

  // Build JSON payload to send, then send
  sprintf(payload, "{\"t\":%s}", t);
  Serial.print("temperature: ");
  Serial.print(s);
  Serial.print("(c) -> payload = ");
  Serial.println(payload);
  client.sendData(payload);
  
  // Sleep sensor
  digitalWrite(POWER_PIN, LOW);
  Serial.print("loop ended at ");
  Serial.println(millis());
  Serial.println("sleeping. zzz...\n");

  // Wait for next window
  delay(start_time + INTERVAL - millis());
}
