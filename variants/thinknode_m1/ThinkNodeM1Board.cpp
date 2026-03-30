#include <Arduino.h>
#include <Wire.h>

#include "ThinkNodeM1Board.h"

#ifdef THINKNODE_M1

#ifdef NRF52_POWER_MANAGEMENT
const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};

void ThinkNodeM1Board::initiateShutdown(uint8_t reason) {
  bool enable_lpcomp = (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
                        reason == SHUTDOWN_REASON_BOOT_PROTECT);

#ifdef P_LORA_TX_LED
  digitalWrite(P_LORA_TX_LED, LOW);
#endif

  pinMode(SX126X_POWER_EN, OUTPUT);
  digitalWrite(SX126X_POWER_EN, LOW);

  pinMode(GPS_EN, OUTPUT);
  digitalWrite(GPS_EN, LOW);

  pinMode(PIN_GPS_RESET, OUTPUT);
  digitalWrite(PIN_GPS_RESET, LOW);

  if (enable_lpcomp) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }

  enterSystemOff(reason);
}
#endif

void ThinkNodeM1Board::begin() {
  NRF52Board::begin();

  Wire.begin();

#ifdef P_LORA_TX_LED
  pinMode(P_LORA_TX_LED, OUTPUT);
  digitalWrite(P_LORA_TX_LED, LOW);
#endif

  pinMode(BATTERY_PIN, INPUT);

  pinMode(SX126X_POWER_EN, OUTPUT);
  digitalWrite(SX126X_POWER_EN, HIGH);

#ifdef NRF52_POWER_MANAGEMENT
  checkBootVoltage(&power_config);
#endif

  delay(10); // give sx1262 some time to power up
}

uint16_t ThinkNodeM1Board::getBattMilliVolts() {
  int adcvalue = 0;

  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  delay(10);

  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  adcvalue = analogRead(PIN_VBAT_READ);
  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  return (uint16_t)((float)adcvalue * REAL_VBAT_MV_PER_LSB);
}
#endif
