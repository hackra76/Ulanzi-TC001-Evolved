# Dual OTA Firmware Update System

Ulanzi TC001 Evolved features a dual Over-The-Air (OTA) update architecture, eliminating the need to connect USB cables for firmware upgrades.

## Partition Layout
The device uses a customized dual-OTA partition scheme (`min_spiffs.csv`):
- `otadata`: Partition tracking active boot slot (`0xe000`, 8 KB)
- `app0`: First OTA slot (`0x10000`, 1920 KB)
- `app1`: Second OTA slot (`0x1f0000`, 1920 KB)
- `spiffs`: Data storage (`0x3d0000`, 192 KB)

When flashing, the new binary is written to the inactive slot and validated with checksums before marking the boot flag.

---

## Method 1: Local Web Browser Upload (`/update`)
1. Download the latest `firmware.bin` release asset.
2. In the Web Configuration Portal, scroll down to **Firmware Update (OTA)**.
3. Choose the downloaded `firmware.bin` file.
4. Click **Flash Firmware File**.
5. The web page displays an upload and flashing progress bar (0% – 100%), while the LED matrix shows `OTA: XX%`.
6. Upon completion, the clock automatically restarts into the updated firmware.

---

## Method 2: GitHub 1-Click Cloud Update
1. Open the Web Configuration Portal.
2. Under **Firmware Update (OTA)**, click **Check GitHub Releases**.
3. The device queries `https://api.github.com/repos/hackra76/Ulanzi-TC001-Evolved/releases/latest`.
4. If a newer release exists, click **Install vX.Y.Z Now**.
5. The device streams the binary asset over HTTPS directly into flash memory, showing real-time percentage progress on the 8×32 matrix.
6. The clock reboots automatically.
