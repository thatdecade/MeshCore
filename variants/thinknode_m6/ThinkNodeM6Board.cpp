#include "ThinkNodeM6Board.h"
#include <Arduino.h>

#ifdef THINKNODE_M6

#include <Wire.h>

#ifdef NRF52_POWER_MANAGEMENT
const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};

void ThinkNodeM6Board::initiateShutdown(uint8_t reason) {
  bool enable_lpcomp = (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
                        reason == SHUTDOWN_REASON_BOOT_PROTECT);

#ifdef P_LORA_TX_LED
  digitalWrite(P_LORA_TX_LED, LOW);
#endif
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_BLUE, LOW);

  pinMode(PIN_ADC_CTRL, OUTPUT);
  digitalWrite(PIN_ADC_CTRL, enable_lpcomp ? HIGH : LOW);

  pinMode(PIN_GPS_EN, OUTPUT);
  digitalWrite(PIN_GPS_EN, LOW);

  pinMode(PIN_GPS_RESET, OUTPUT);
  digitalWrite(PIN_GPS_RESET, LOW);

  pinMode(QSPI_FLASH_EN, OUTPUT);
  digitalWrite(QSPI_FLASH_EN, LOW);

  if (enable_lpcomp) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }

  enterSystemOff(reason);
}
#endif

void ThinkNodeM6Board::begin() {
  NRF52BoardDCDC::begin();

  pinMode(BATTERY_PIN, INPUT);
  pinMode(PIN_ADC_CTRL, OUTPUT);
  digitalWrite(PIN_ADC_CTRL, LOW);
  analogReadResolution(12);
  analogReference(AR_INTERNAL_3_0);
  delay(50);

  pinMode(PIN_USER_BTN, INPUT_PULLUP);

  Wire.begin();

#ifdef P_LORA_TX_LED
  pinMode(P_LORA_TX_LED, OUTPUT);
  digitalWrite(P_LORA_TX_LED, LOW);
#endif

#ifdef NRF52_POWER_MANAGEMENT
  checkBootVoltage(&power_config);
#endif

  delay(10); // give sx1262 some time to power up
}

uint16_t ThinkNodeM6Board::getBattMilliVolts() {
  int adcvalue = 0;

  digitalWrite(PIN_ADC_CTRL, HIGH);
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  delay(10);

  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  adcvalue = analogRead(PIN_VBAT_READ);
  digitalWrite(PIN_ADC_CTRL, LOW);
  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  return (uint16_t)((float)adcvalue * REAL_VBAT_MV_PER_LSB);
}
#endif
