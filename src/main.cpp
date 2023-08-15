#include <Arduino.h>

//Lora and TTN
//------------
#ifdef LORA_ENABLED
  #include "loraFunctions.h"
#endif

unsigned long last_print_time = 0;
float distance = -1;

void setup () {

  #ifdef WAIT_SERIAL
  while (!Serial) {}
  #endif
  
  Serial.begin(115200);  // We initialize serial connection so that we could print values from sensor.
  Serial.println("Starting...");


  #ifdef LORA_ENABLED
    loraSetup();
  #endif

  Serial.println("Started");
}

void loop()
{
  #ifdef LORA_ENABLED
    if(distance >= 10){
      publish2TTN();
      distance = 0;
    }
    loraLoop();
  #endif

  // Print to serial every 500 miliseconds
  unsigned long current_time = millis();
  if (current_time - last_print_time >= 500) {
    last_print_time = current_time;
    Serial.printf("Distance: %f cm\n", distance);

    distance++;
  }
}
