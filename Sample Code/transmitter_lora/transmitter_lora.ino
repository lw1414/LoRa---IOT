#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// =====================================================
// ESP32 -> SX1262 PINOUT
// =====================================================

#define LORA_SCK    18
#define LORA_MISO   19
#define LORA_MOSI   23
#define LORA_NSS     5

#define LORA_NRST   14
#define LORA_DIO1   26
#define LORA_BUSY   27

// External antenna switch
#define LORA_ANT_SW 25


// =====================================================
// SX1262 RADIO OBJECT
// =====================================================

SX1262 radio = new Module(
  LORA_NSS,
  LORA_DIO1,
  LORA_NRST,
  LORA_BUSY
);


// =====================================================
// PACKET COUNTER
// =====================================================

uint32_t packetCounter = 0;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       SX1262 915 MHz TRANSMITTER");
  Serial.println("========================================");

  // -----------------------------------------
  // SPI
  // -----------------------------------------

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_NSS
  );

  // -----------------------------------------
  // External antenna switch
  // -----------------------------------------

  pinMode(LORA_ANT_SW, OUTPUT);

  // Keep LOW initially.
  // We will verify the correct polarity for
  // your module later.
  digitalWrite(LORA_ANT_SW, LOW);


  // =================================================
  // TCXO CONFIGURATION
  // =================================================

  // RadioLib current API uses an enum here.
  //
  // This assumes your SX1262 module uses a 1.6 V TCXO.
  //
  radio.tcxoVoltage =
    RadioLibTCXOVoltage_t::Voltage1V6;


  // =================================================
  // LoRa CONFIGURATION
  // =================================================

  ConfigLoRa_t config;

  config.frequency =
    RADIOLIB_UNIT_MEGA(915);

  config.bandwidth =
    RADIOLIB_UNIT_KILO(125);

  config.spreadingFactor = 9;

  config.codingRate = 5;

  config.syncWord = 0x12;

  config.power = 17;

  config.preambleLength = 8;


  // =================================================
  // INITIALIZE SX1262
  // =================================================

  Serial.println();
  Serial.println("Initializing SX1262...");

  int16_t state = radio.begin(config);


  // =================================================
  // CHECK INITIALIZATION
  // =================================================

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("SX1262 DETECTED");
    Serial.println("Initialization SUCCESS");
    Serial.println("----------------------------------------");

    Serial.println();
    Serial.println("LoRa Settings:");
    Serial.println("Frequency        : 915 MHz");
    Serial.println("Bandwidth        : 125 kHz");
    Serial.println("Spreading Factor : SF9");
    Serial.println("Coding Rate      : 4/5");
    Serial.println("Sync Word        : 0x12");
    Serial.println("TX Power         : 17 dBm");

    Serial.println();
    Serial.println("TX READY");

  }
  else {

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("SX1262 INITIALIZATION FAILED");
    Serial.print("RadioLib Error Code: ");
    Serial.println(state);
    Serial.println("----------------------------------------");

    while (true) {
      delay(1000);
    }
  }
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  packetCounter++;

  String message =
    "Hello from ESP32 TX - Packet " +
    String(packetCounter);

  Serial.println();
  Serial.println("========================================");
  Serial.print("TRANSMITTING PACKET #");
  Serial.println(packetCounter);
  Serial.println("========================================");

  Serial.print("Message: ");
  Serial.println(message);

  Serial.println("Sending...");


  // =================================================
  // TRANSMIT
  // =================================================

  int16_t state =
    radio.transmit(message);


  // =================================================
  // RESULT
  // =================================================

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("TX SUCCESS");

    Serial.print("Packet: ");
    Serial.println(packetCounter);

    Serial.println("915 MHz transmission complete.");

  }
  else {

    Serial.println();
    Serial.println("TX FAILED");

    Serial.print("Error code: ");
    Serial.println(state);

  }

  delay(2000);
}