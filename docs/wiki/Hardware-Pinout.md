# Hardware Pinout & Architecture

The [Ulanzi TC001 Smart Pixel Clock](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882) (manufactured by Shenzhen Ulanzi Technology Co., Ltd.) is powered by an ESP32-WROOM-32 module.

## Pin Map

| Function | Pin | Hardware Type | Description |
|---|---|---|---|
| **LED Matrix Data** | `GPIO 32` | WS2812B DIN | 8 rows × 32 columns zigzag addressable LEDs |
| **I2C SDA** | `GPIO 21` | Sensirion SHT30 | Temperature & Relative Humidity |
| **I2C SCL** | `GPIO 22` | Sensirion SHT30 | Temperature & Relative Humidity |
| **Light Sensor (LDR)**| `GPIO 33` | ADC1 CH5 | Analog voltage divider for ambient brightness |
| **Battery Voltage** | `GPIO 34` | ADC1 CH6 | 1:1 resistive divider (3.3V – 4.2V range) |
| **Buzzer** | `GPIO 15` | Digital Out | Active/Passive buzzer driver |
| **Button Left** | `GPIO 26` | Input Pull-Up | Active LOW tactile button |
| **Button Center / OK**| `GPIO 27` | Input Pull-Up | Active LOW tactile button |
| **Button Right** | `GPIO 14` | Input Pull-Up | Active LOW tactile button |

---

## Power Management
- **Internal Battery**: 4400 mAh Li-ion cell.
- **Charging**: USB Type-C connector with battery charging controller.
- **Battery Curve**: Voltage read via GPIO 34 calibrated with linear approximation between 3.3V (0%) and 4.2V (100%).
