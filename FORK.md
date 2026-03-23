## Fork changes in this branch

To make MeshCore comaptible with [HarukiToreda's Pager Upgrade kit](https://www.etsy.com/listing/4359661489/heltec-v3-pager-upgrade-shield-for) for the Heltec V3, this fork enables gps, joystick navigation, buzzer, and vibration.

## Companion Behavior

- Onboard button is Back
- Joystick Center is Enter
- Joystick CCW is Left
- Joystick CW is Right
- Buzzer and vibration menus
- GPS enabled

## Build Notes

1. Check out the heltec_v3_pager_upgrade_shield branch.
1. Enable PlatformIO env: Heltec_v3_pager_upgrade_shield_companion_radio_ble_
1. Build:

```bash
pio run -e Heltec_v3_pager_upgrade_shield_companion_radio_ble_
```

### Heltec V3 Pager Upgrade Wiring

| PIN | Function       | Description                |
| --- | -------------- | -------------------------- |
| 47  | PIN_NAV_ENTER  | Navigation press input     |
| 34  | JOYSTICK_LEFT  | Navigation left input, CCW |
| 33  | JOYSTICK_RIGHT | Navigation right input, CW |
| 0   | PIN_BACK_BTN   | Onboard Heltec button      |
| 7   | PIN_VIBRATION  | Vibration motor            |
| 6   | PIN_BUZZER     | Piezo buzzer               |
| 5   | PIN_GPS_RX     | To GPS module RX pin       |
| 4   | PIN_GPS_TX     | To GPS module TX pin       |
| 3   | PIN_GPS_EN     | GPS enable control         |
