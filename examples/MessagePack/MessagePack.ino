#include <lorawan_client.h>
#include <lorawan_client_al050.h>
#include <lorawan_util.h>

// Download and install the messagepack library according to README at https://github.com/HEADS-project/arduino_msgpack
#include "msgpck.h"

#include "ByteStream.h"

/*
 * Send data in MessagePack format (http://msgpack.org/)
 */

LoRaWANClientAL050 lorawanClient;

void setup() {
  Serial.begin(9600);
  delay(100);

  Serial.println("Starting MessagePack example");
  
  lorawanClient.connect();
}

byte buffer[100];

int i = 0;

void loop() {
  ByteStream stream(buffer);
  
  switch (i) {
    case 0: {
      // {"a":1,"b":2,"c":3}
      // 83 a1 61 01 a1 62 02 a1 63 03
      msgpck_write_map_header(&stream, 3);
      msgpck_write_string(&stream, (char*)"a", 1);
      msgpck_write_integer(&stream, 1);
      msgpck_write_string(&stream, (char*)"b", 1);
      msgpck_write_integer(&stream, 2);
      msgpck_write_string(&stream, (char*)"c", 1);
      msgpck_write_integer(&stream, 3);
      Serial.println(stream.available());
      Serial.println(bytesToHexString(buffer, stream.available()));
      break;
    }
      
    case 1:
      // fixnums
      // {"v":[null,false,true,0,1,127,-32]}
      // 81 a1 76 97 c0 c2 c3 00 01 7f e0
      msgpck_write_map_header(&stream, 1);
      msgpck_write_string(&stream, (char*)"v", 1);
      msgpck_write_array_header(&stream, 7);
      msgpck_write_nil(&stream);
      msgpck_write_bool(&stream, false);
      msgpck_write_bool(&stream, true);
      msgpck_write_integer(&stream, 0);
      msgpck_write_integer(&stream, 1);
      msgpck_write_integer(&stream, 127);
      msgpck_write_integer(&stream, -32);
      Serial.println(stream.available());
      Serial.println(bytesToHexString(buffer, stream.available()));
      break;

    case 2: {
      // float
      // {"f":1.5}
      // 81 a1 66 ca 3f 00 00 00
      float f = 0.5;
      msgpck_write_map_header(&stream, 1);
      msgpck_write_string(&stream, (char*)"f", 1);
      msgpck_write_float(&stream, f);
      Serial.println(stream.available());
      Serial.println(bytesToHexString(buffer, stream.available()));
      break;
    }
      
    case 3: {
      // uint8_t or int8_t
      // {"8_t":[255,-128]}
      // 81 a3 38 5f 74 92 cc ff d1 ff 80
      uint8_t u8 = 255;
      int8_t i8 = -128;
      msgpck_write_map_header(&stream, 1);
      msgpck_write_string(&stream, (char*)"8_t", 3);
      msgpck_write_array_header(&stream, 2);
      msgpck_write_integer(&stream, u8);
      msgpck_write_integer(&stream, i8);
      Serial.println(stream.available());
      Serial.println(bytesToHexString(buffer, stream.available()));
      break;
    }

    case 4:
    default: {
      // uint32_t
      // {"32_t":4294967295}
      // 81 a4 33 32 5f 74 ce ff ff ff ff
      uint32_t u32 = 4294967295;
      msgpck_write_map_header(&stream, 1);
      msgpck_write_string(&stream, (char*)"32_t", 4);
      msgpck_write_integer(&stream, u32);
      Serial.println(stream.available());
      Serial.println(bytesToHexString(buffer, stream.available()));
      break;
    }
  }

  // send and delay
  lorawanClient.sendBinary(buffer, stream.available());
  if (++i >= 5) i = 0;
  delay(5000);
}
