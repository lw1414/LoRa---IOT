#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// =====================================================
// ESP32 -> SX1262 PINOUT
// =====================================================

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_NSS    5

#define LORA_NRST  14
#define LORA_DIO1  26
#define LORA_BUSY  27

// ANT_SW is intentionally not used yet.
// #define LORA_ANT_SW 25


// =====================================================
// SX1262
// =====================================================

SX1262 radio = new Module(
  LORA_NSS,
  LORA_DIO1,
  LORA_NRST,
  LORA_BUSY
);


// =====================================================
// PACKET FLAG
// =====================================================

volatile bool packetReceived = false;


// =====================================================
// DIO1 INTERRUPT
// =====================================================

void IRAM_ATTR dio1ISR() {
  packetReceived = true;
}


// =====================================================
// SIGNAL QUALITY
// =====================================================

int rssiQuality(float rssi) {

  if (rssi >= -50.0)
    return 100;

  if (rssi <= -120.0)
    return 0;

  return (int)((rssi + 120.0) * 100.0 / 70.0);
}


int snrQuality(float snr) {

  if (snr >= 10.0)
    return 100;

  if (snr <= -20.0)
    return 0;

  return (int)((snr + 20.0) * 100.0 / 30.0);
}


void printSignalQuality(float rssi, float snr) {

  int rq = rssiQuality(rssi);
  int sq = snrQuality(snr);

  int quality = (rq + sq) / 2;

  Serial.println();
  Serial.println("--------------------------------");

  Serial.print("RSSI: ");
  Serial.print(rssi, 1);
  Serial.println(" dBm");

  Serial.print("SNR : ");
  Serial.print(snr, 1);
  Serial.println(" dB");

  Serial.print("Signal Quality: ");
  Serial.print(quality);
  Serial.println("%");

  Serial.print("Status: ");

  if (quality >= 80) {
    Serial.println("EXCELLENT");
  }
  else if (quality >= 60) {
    Serial.println("GOOD");
  }
  else if (quality >= 40) {
    Serial.println("FAIR");
  }
  else if (quality >= 20) {
    Serial.println("WEAK");
  }
  else {
    Serial.println("VERY WEAK");
  }

  Serial.println("--------------------------------");
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println();
  Serial.println("========================================");
  Serial.println("       SX1262 915 MHz RECEIVER");
  Serial.println("========================================");


  // ---------------------------------------------------
  // PIN INFORMATION
  // ---------------------------------------------------

  Serial.println();
  Serial.println("SX1262 PIN CONFIGURATION");

  Serial.println("SCK  -> GPIO18");
  Serial.println("MISO -> GPIO19");
  Serial.println("MOSI -> GPIO23");
  Serial.println("NSS  -> GPIO5");
  Serial.println("NRST -> GPIO14");
  Serial.println("DIO1 -> GPIO26");
  Serial.println("BUSY -> GPIO27");


  // ---------------------------------------------------
  // SPI
  // ---------------------------------------------------

  Serial.println();
  Serial.println("Initializing SPI...");

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_NSS
  );

  delay(100);

  Serial.println("SPI initialized.");


  // ---------------------------------------------------
  // OSCILLATOR
  //
  // VoltageNone = XTAL / 0 V TCXO
  //
  // Your module successfully initialized with this
  // configuration.
  // ---------------------------------------------------

  radio.tcxoVoltage =
    RadioLibTCXOVoltage_t::VoltageNone;


  // ===================================================
  // LORA CONFIGURATION
  // ===================================================

  ConfigLoRa_t config;

  config.frequency = RADIOLIB_UNIT_MEGA(915);

  config.bandwidth = RADIOLIB_UNIT_KILO(125);

  config.spreadingFactor = 9;

  config.codingRate = 5;

  config.syncWord = 0x12;

  config.power = 17;

  config.preambleLength = 8;


  // ===================================================
  // INITIALIZE RADIO
  // ===================================================

  Serial.println();
  Serial.println("Initializing SX1262...");

  int16_t state = radio.begin(config);


  if (state != RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("========================================");
    Serial.println("SX1262 INITIALIZATION FAILED");
    Serial.println("========================================");

    Serial.print("RadioLib Error Code: ");
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }


  // ===================================================
  // INITIALIZATION SUCCESS
  // ===================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("SX1262 INITIALIZATION SUCCESS");
  Serial.println("========================================");

  Serial.println();
  Serial.println("Frequency : 915 MHz");
  Serial.println("Bandwidth : 125 kHz");
  Serial.println("Spreading : SF9");
  Serial.println("Coding    : 4/5");
  Serial.println("Sync Word : 0x12");
  Serial.println("Power     : 17 dBm");
  Serial.println("Oscillator: XTAL");


  // ===================================================
  // DIO1 INTERRUPT
  // ===================================================

  radio.setDio1Action(dio1ISR);


  // ===================================================
  // START RECEIVING
  // ===================================================

  Serial.println();
  Serial.println("Starting receiver...");

  state = radio.startReceive();


  if (state != RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("FAILED TO START RECEIVER");

    Serial.print("RadioLib Error Code: ");
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }


  Serial.println();
  Serial.println("========================================");
  Serial.println("         RECEIVER READY");
  Serial.println("========================================");

  Serial.println();
  Serial.println("Waiting for packets...");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ---------------------------------------------------
  // Check for packet
  // ---------------------------------------------------

  if (packetReceived) {

    // Clear interrupt flag
    packetReceived = false;


    // -------------------------------------------------
    // Read received packet
    // -------------------------------------------------

    String receivedMessage;

    int16_t state = radio.readData(receivedMessage);


    // =================================================
    // PACKET RECEIVED
    // =================================================

    if (state == RADIOLIB_ERR_NONE) {

      float rssi = radio.getRSSI();

      float snr = radio.getSNR();


      Serial.println();
      Serial.println("========================================");
      Serial.println("           PACKET RECEIVED");
      Serial.println("========================================");

      Serial.print("Message : ");
      Serial.println(receivedMessage);

      Serial.print("Length  : ");
      Serial.print(receivedMessage.length());
      Serial.println(" bytes");


      printSignalQuality(rssi, snr);


      Serial.println();
      Serial.println("Packet processing complete.");
      Serial.println();


      // ------------------------------------------------
      // Restart receiver
      // ------------------------------------------------

      state = radio.startReceive();

      if (state != RADIOLIB_ERR_NONE) {

        Serial.println("Failed to restart receiver.");

        Serial.print("Error Code: ");
        Serial.println(state);

        delay(1000);
      }
    }


    // =================================================
    // CRC ERROR
    // =================================================

    else if (state == RADIOLIB_ERR_CRC_MISMATCH) {

      Serial.println();
      Serial.println("CRC ERROR");
      Serial.println("Packet was corrupted.");

      radio.startReceive();
    }


    // =================================================
    // OTHER ERROR
    // =================================================

    else {

      Serial.println();
      Serial.println("READ DATA FAILED");

      Serial.print("RadioLib Error Code: ");
      Serial.println(state);

      delay(100);

      radio.startReceive();
    }
  }


  // Keep CPU free
  delay(5);
}