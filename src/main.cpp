#include <iostream>
#include <string>
#include <Arduino.h>

// Lora and TTN
//------------
#ifdef LORA_ENABLED
#include "loraFunctions.h"
#endif

// Sensors
#if SENSOR_HCSR04
#include "sensors/sensor-hcsr04.h"
#else
#error "Sensor type not selected. Add SENSOR_HCSR04, SENSOR_DS18B20 or SENSOR_VL53L1X in your environment build_flags in platformio.ini"
#endif

using namespace std;

unsigned long last_print_time = 0;

void setup()
{

#ifdef WAIT_SERIAL
  while (!Serial)
  {
  }
#endif

  Serial.begin(115200); // We initialize serial connection so that we could print values from sensor.
  Serial.println("Starting...");

// Sensors
#if SENSOR_HCSR04
    Sensor::HCSR04::setup();
#endif

#ifdef LORA_ENABLED
  loraSetup();
#endif

  Serial.println("Started");
}

void loop()
{
  float distance = -1;
// Sensors
#if SENSOR_HCSR04
    distance = Sensor::HCSR04::measureDistanceCm();
    Sensor::HCSR04::loop();
#endif

  // Print to serial every 30 seconds
  unsigned long current_time = millis();
  if (current_time - last_print_time >= 30000)
  {
    last_print_time = current_time;
    if (distance > 0) 
    {
      Serial.printf("Distance: %f cm\n", distance);
    }
    #ifdef LORA_ENABLED
      if (distance >= 10)
      {
        std::string str = std::to_string(distance);
        uint8_t mydata[32];
        std::copy(str.begin(), str.end(), std::begin(mydata));
        publish2TTN(mydata);
        distance = 0;
      }
      loraLoop();
    #endif
  }
}
