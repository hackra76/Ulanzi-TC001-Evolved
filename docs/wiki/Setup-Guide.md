# Quick Setup & Wi-Fi Provisioning

## First Boot & AP Mode
1. Power on your Ulanzi TC001 clock.
2. If no valid Wi-Fi credentials are stored, the device starts a local SoftAP:
   - **SSID**: `TC001-Setup-XXXX` (where `XXXX` is the unique ESP32 Chip ID)
   - **IP Address**: `192.168.4.1`
3. The 8×32 matrix display will scroll: `AP: TC001-Setup-XXXX  IP: 192.168.4.1`.
4. Connect your smartphone, laptop, or tablet to the Wi-Fi network `TC001-Setup-XXXX`.
5. A captive portal popup will appear automatically. If not, open your browser and navigate to `http://192.168.4.1`.

## Wi-Fi Connection
1. Click **Scan Nearby Networks** in the portal to discover available 2.4 GHz Wi-Fi networks.
2. Select your SSID and enter the Wi-Fi password.
3. Configure your desired preferences (Day/Night brightness, City, Tickers).
4. Click **Save & Apply Settings**.
5. The device connects to your Wi-Fi network and announces its assigned IP address on the LED matrix display: `Connected! IP: 192.168.x.x`.

## Manual Navigation
- Press **Right Button** to switch to the next screen.
- Press **Left Button** to switch to the previous screen.
- Press **OK Button** to cycle manual brightness levels.
- Long-press **OK Button** to toggle auto-rotation carousel on or off.
