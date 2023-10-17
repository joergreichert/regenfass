#ifndef LoraFunctions_H
#define LoraFunctions_H
#pragma once

#include "Arduino.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <algorithm>
#include <keyhandler.h>

#define DEVICE_SIMPLE

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
// static const u1_t PROGMEM APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }

// This should also be in little endian format, see above.
// static const u1_t PROGMEM DEVEUI[8] = {0x7A, 0xCD, 0x05, 0x00, 0x00, 0x00, 0x99, 0x70};
// void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// static const u1_t PROGMEM APPKEY[16] = {0x3F, 0x5D, 0xB5, 0xEC, 0x00, 0xF3, 0x44, 0x00, 0x00, 0xB6, 0xF9, 0x00, 0xB8, 0xB9, 0xA7, 0x75};
// void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

static uint8_t mydata[] = "Hello, world!";
uint8_t payload[34];

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

char TTN_response[30];

// class MyHalConfig_t : public Arduino_LMIC::HalConfiguration_t
// {

// public:
//     MyHalConfig_t(){};

//     // set SPI pins to board configuration, pins may come from pins_arduino.h
//     virtual void begin(void) override
//     {
//         SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_NSS_PIN);
//     }
// };

// static MyHalConfig_t myHalConfig{};
static const lmic_pinmap myPinmap = {
    .nss = LORA_NSS_PIN,
    .prepare_antenna_tx = nullptr,
    .rst = LORA_RESET_PIN,
    // .dio = {/*dio0*/ LORA_DIO0_PIN, /*dio1*/ LORA_DIO1_PIN, /*dio2*/ LORA_DIO2_PIN},
    // .dio = {/*dio0*/ LMIC_UNUSED_PIN, /*dio1*/ LORA_DIO1_PIN, /*dio2*/ LMIC_UNUSED_PIN},
    .dio = {/*dio0*/ LMIC_UNUSED_PIN, /*dio1*/ LORA_DIO1_PIN},
    // .rxtx_rx_active = LMIC_UNUSED_PIN,
    // .rssi_cal = 10,
    // .spi_freq = 8000000, // 8MHz
    // .pConfig = &myHalConfig};
};

RadioSx1262 radio(myPinmap, ImageCalibrationBand::band_863_870);
LmicEu868 LMIC{radio};

void printHex2(unsigned v)
{
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void loraSetup()
{
    log_d("Setting up lora...");
    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC.reset();

    // Disable link check validation
    // Disable link-check mode and ADR, because ADR tends to complicate testing.
    LMIC.setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    // LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink
    // Set the data rate to Spreading Factor 7.  This is the fastest supported rate for 125 kHz channels, and it
    // minimizes air time and battery power. Set the transmission power to 14 dBi (25 mW).
    // LMIC_setDrTxpow(DR_SF7,14);

    // Start job (sending automatically starts OTAA too)
    // do_send(&sendjob);

    log_d("Setting up lora done");
}

void loraLoop()
{
    auto freeTimeBeforeNextCall = LMIC.run();
    if (freeTimeBeforeNextCall < OsDeltaTime::from_ms(100))
        return;

    if (LMIC.getOpMode().test(OpState::TXRXPEND))
    {
        auto waittime = freeTimeBeforeNextCall.to_ms() - 100;
        PRINT_DEBUG(1, F("Delay TXRXPEND %dms"), waittime);
        delay(waittime);
        return;
    }
}

void publish2TTN(void)
{

    // Check if there is not a current TX/RX job running
    if (LMIC.getOpMode().test(OpState::TXRXPEND))
    {
        Serial.printf("*** OP_TXRXPEND, not sending\n");
        return;
    }

    // Prepare upstream data transmission at the next possible time.

    LMIC.setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    Serial.println(F("Packet queued"));
    // Next TX is scheduled after TX_COMPLETE event.
}

#endif
