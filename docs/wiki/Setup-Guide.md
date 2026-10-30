# Quick Setup & Wi-Fi Provisioning

## First Boot & AP Mode
1. Power on your Ulanzi TC001 clock.
2. If no valid Wi-Fi credentials are stored, the device starts a local SoftAP:
   - **SSID**: `Ulanzi-Setup-XXXX` (where `XXXX` is the unique ESP32 Chip ID)
   - **IP Address**: `192.168.4.1`
3. The 8×32 matrix display scrolls the setup instructions: `AP: Ulanzi-Setup-XXXX  IP: 192.168.4.1`.
4. Connect your smartphone, laptop, or tablet to the Wi-Fi network `Ulanzi-Setup-XXXX`.
5. A captive portal popup will appear automatically. If not, open your browser and navigate to `http://192.168.4.1`.

## Wi-Fi Connection
1. Click **Scan Nearby Networks** in the portal to discover available 2.4 GHz Wi-Fi networks.
2. Select your SSID and enter the Wi-Fi password.
3. Configure your desired preferences (Day/Night brightness, City coordinates, Financial Tickers, Clock layout, Carousel order).
4. Click **Save & Apply Settings**.
5. The device connects to your Wi-Fi network and announces its assigned IP address on the LED matrix display: `Connected! IP: 192.168.x.x`.

## Manual Navigation
- Press **Right Button** to switch to the next screen along your custom carousel order.
- Press **Left Button** to switch to the previous screen along your custom carousel order.
- Press **OK Button**:
  - **Screensaver Screen**: Cycle retro screensaver styles (Matrix Rain, Fire, Pac-Man, Game of Life, Shuffle).
  - **Battery Screen**: Cycle display telemetry between Voltage (V) and Percentage (%).
  - **Crypto / Financial Screen**: Cycle through market asset presets (BTC, ETH, SOL, BNB, AAPL, TSLA, NVDA, SPX, GOLD, EUR, USD).
  - **Stopwatch & Timer Screen**: Start / Pause / Resume timer.
  - **Other Screens (Clock, Weather, Airspace, YouTube)**: Cycle manual brightness levels (10% / 35% / 80%).
- Long-press **OK Button** (>700ms): Toggle auto-rotation carousel on or off (on Timer screen: reset / change mode).
- In **Night Mode**, pressing any button temporarily wakes the display for 12 seconds at Day Brightness.

## 📱 Android Companion App Pairing
For a native handheld experience, you can control and configure your clock directly from your Android phone using the **[Ulanzi TC001 Companion App](Android-Companion-App)**:
1. Download and install the latest signed production APK from **[Companion Releases](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)**.
2. Ensure your phone is connected to the same 2.4 GHz Wi-Fi network as the clock.
3. Open the app — it automatically discovers your clock via mDNS (`ulanzi-tc001.local`) and connects instantly.
4. Enjoy real-time 32×8 LED matrix mirroring, interactive finger pixel art studio, Android notification forwarding relay, Spotify / YouTube Music playback broadcasting, remote chronometer/Pomodoro controls, and 100% hardware configuration parity.

