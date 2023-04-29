#include <Arduino.h>
#include <LoRa_E220.h>

//Lora and TTN
//------------
#ifdef LORE_ENABLED
  #include "loraFunctions.h"
#endif

//Distance Sensors
//----------------
#ifdef SENSOR_TYPE_HCSR04
#include <HCSR04.h>

#define DISTANCE_MAX 400

UltraSonicDistanceSensor distanceSensor(SENSOR_TYPE_HCSR04_PIN_TRIGGER, SENSOR_TYPE_HCSR04_PIN_ECHO, DISTANCE_MAX);

#elif SENSOR_TYPE_VL53L1X
#include <Wire.h>
#include <VL53L1X.h>
VL53L1X sensor;
#else
#warning "Sensor type not selected. Add SENSOR_TYPE_HCSR04 or SENSOR_TYPE_VL53L1X in your environment build_plags in platformio.ini"
#endif

float distance = -1;
unsigned long last_print_time = 0;

#include <LoRa_E220.h>

// https://registry.platformio.org/libraries/xreef/EByte%20LoRa%20E220%20library/examples/01_getConfiguration/01_getConfiguration.ino
LoRa_E220 e220ttl(&Serial2, 15, 21, 19); //  RX AUX M0 M1

void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX);Serial.print(" ");Serial.print(configuration.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(configuration.LENGHT, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
	Serial.println(F(" "));
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
	Serial.println(F(" "));
	Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
	Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
	Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
	Serial.println(F(" "));
	Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
	Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
	Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
	Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


	Serial.println("----------------------------------------");
}


void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD: "));  Serial.print(moduleInformation.COMMAND, HEX);Serial.print(" ");Serial.print(moduleInformation.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(moduleInformation.LENGHT, DEC);

	Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");
}

void setup()
{

#ifdef WAIT_SERIAL
    while (!Serial)
    {
    }
#endif

    Serial.begin(115200); // We initialize serial connection so that we could print values from sensor.
    Serial.println("Starting...");

#ifdef SENSOR_TYPE_VL53L1X
    Wire.begin();
    Wire.setClock(400000); // use 400 kHz I2C

    sensor.setTimeout(500);
    if (!sensor.init())
    {
        Serial.println("Failed to detect and initialize VL53L1X sensor!");
        while (1)
            ;
    }
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(50);
  #endif

  #ifdef LORE_ENABLED
    loraSetup();
  #endif

  Serial.println("Started");


	delay(500);

	Serial.println();

	// Startup all pins and UART
	e220ttl.begin();

	Serial.println("after begin");

	ResponseStructContainer c;
	c = e220ttl.getConfiguration();
	
	Serial.println("after get config");

	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;

	Serial.println("after c.data");

	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);

	ResponseStructContainer cMi;
	cMi = e220ttl.getModuleInformation();
	// It's important get information pointer before all other operation
	ModuleInformation mi = *(ModuleInformation*)cMi.data;

	Serial.println(cMi.status.getResponseDescription());
	Serial.println(cMi.status.code);

	printModuleInformation(mi);

	c.close();
	cMi.close();
}

void loop()
{

// Every 500 miliseconds, do a measurement using the sensor and print the distance in centimeters.
#ifdef SENSOR_TYPE_HCSR04
    distance = distanceSensor.measureDistanceCm();
#elif SENSOR_TYPE_VL53L1X
    sensor.read();
    distance = sensor.ranging_data.range_mm / 10.0;
#endif

  #ifdef LORE_ENABLED
    if(distance <= 10){
      publish2TTN();
    }
    loraLoop();
  #endif

  // Print to serial every 500 miliseconds
  unsigned long current_time = millis();
  if (current_time - last_print_time >= 500) {
    last_print_time = current_time;
    Serial.printf("Distance: %f cm\n", distance);
  } 
}
