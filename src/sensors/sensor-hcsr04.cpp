#include <Arduino.h>

// Library for HCSR04 sensor
#include <HCSR04.h>

namespace Sensor
{
    namespace HCSR04
    {
        // Create the sensor object
        UltraSonicDistanceSensor distanceSensor(SENSOR_PIN_TRIGGER, SENSOR_PIN_ECHO, SENSOR_MAX_DISTANCE);

        unsigned long last_print_time = 0;

        /**
         * @brief Do a measurement using the sensor and print the distance in centimeters.
         *
         * @return float
         */
        float measureDistanceCm()
        {
            float distance = distanceSensor.measureDistanceCm();
            return distance;
        }

        /* @deprecated */
        void setup()
        {
            Serial.println("Setup: Sensor HCSR04");
        }
        void loop()
        {
            unsigned long current_time = millis();
            if (current_time - last_print_time >= 5000)
            {
                last_print_time = current_time;
                float distance = measureDistanceCm();
                if (distance > 0)
                {
                    Serial.print("HCSR04: ");
                    Serial.print(distance);
                    Serial.println(" cm");
                }
            }
        }
    }
} // namespace Sensor
