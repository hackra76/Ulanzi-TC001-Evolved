# ESP-Ulanzi-TC001 (Smart Pixel Desktop Clock)

Firmware for the **Ulanzi TC001 Smart Pixel Desktop Clock** running on the ESP32 with an **8×32 WS2812B RGB LED matrix**, Sensirion SHT30 temperature & humidity sensor, battery telemetry, ambient light sensor, and buzzer.

## Hardware Specifications
- **SoC**: ESP32-WROOM-32 (Dual-core Xtensa LX6 @ 240 MHz, 4MB Flash, 520 KB SRAM)
- **Display**: 8×32 WS2812B RGB NeoPixel Matrix (256 pixels, GPIO 32)
- **Sensors**:
  - Sensirion SHT30 (I2C: SDA GPIO 21, SCL GPIO 22, address 0x44)
  - Battery Monitor (ADC1 CH6 on GPIO 34, 1:1 voltage divider)
  - Ambient Light Sensor / LDR (ADC1 CH5 on GPIO 33)
- **Buttons**:
  - Left Button: GPIO 26
  - OK / Center Button: GPIO 27
  - Right Button: GPIO 14
- **Audio**: Active/Passive Buzzer (GPIO 15)
- **Power**: 4400 mAh Li-ion battery + USB-C charging

## Controls
- **Left Button**: Switch to Previous App
- **Right Button**: Switch to Next App
- **Center Button (Short Press)**: Cycle Display Brightness (Low / Medium / High)
- **Center Button (Long Press)**: Toggle Screen Auto-Rotation

## Compiling & Flashing
```powershell
pio run -e ulanzi-tc001
pio run -e ulanzi-tc001 -t upload
```
