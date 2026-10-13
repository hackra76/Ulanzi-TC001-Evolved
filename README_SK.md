# Ulanzi TC001 Evolved 🚀 (Slovenská dokumentácia)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Kompatibiln%C3%A9-orange?logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue?logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/Licencia-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Evolved?color=brightgreen)](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases)

Pokročilý firmvér novej generácie pre inteligentné stolové pixelové hodiny **Ulanzi TC001 Smart Pixel Desktop Clock** bežiaci na mikrokontroléri ESP32 s **8×32 WS2812B RGB LED maticou**, senzorom Sensirion SHT30, adaptívnym jasom (LDR), živým krypto trackerom, predpoveďou počasia, YouTube štatistikami a duálnym OTA systémom aktualizácií.

🌐 **Jazyky:** [English (Predvolený)](README.md) • [English Zrkadlo](README_EN.md) • [Slovenčina](README_SK.md)

---

## ✨ Kľúčové funkcie

- **📱 Wi-Fi Captive Portal & Sprievodca pripojením:**
  - Automatický prechod do SoftAP režimu (`TC001-Setup-XXXX`) s presmerovaním na captive portál na adrese `192.168.4.1`.
  - Neinvazívny skener okolitých Wi-Fi sietí s ukazovateľom sily signálu (RSSI).
  - Spoľahlivé ukladanie prístupových údajov do pamäte NVS (`Preferences`) s automatickým opätovným pripájaním.
- **💡 Adaptívny jas & Nočný režim:**
  - Plynulý automatický jas riadený fotorezistorom (LDR) s filtrom exponenciálneho kĺzavého priemeru (EMA) a nelineárnou gama krivkou.
  - **Denný strop jasu:** Nastaviteľný posuvník (5% – 100%) vo webovom rozhraní.
  - **Harmonogram nočného režimu:** Konfigurovateľné tiché hodiny (napr. 23:00 až 07:00) s tlmenými červenými hodinami a polmesiacom.
  - **Nočný posuvník jasu:** Nastaviteľný posuvník (1% – 25%).
  - **Rýchle prebudenie:** Stlačenie ľubovoľného tlačidla v noci dočasne rozsvieti displej na 12 sekúnd na denný jas.
- **🌤️ Skutočné vonkajšie počasie:**
  - Živé meteorologické údaje z bezplatného Open-Meteo API (nevyžaduje žiadny API kľúč).
  - Mapovanie WMO kódov počasia na animované 8×8 ikony (Slnko, Oblaky, Dážď, Sneh, Búrka).
  - **Zoznam prednastavených miest:** Viac ako 25 prednastavených miest (Slovensko, Česko, Európa, Svet) s automatickým vyplnením GPS súradníc.
- **📈 Živý krypto ticker:**
  - Priame spojenie s verejným trhovým API burzy Binance.
  - Podpora pre `BTC`, `ETH`, `SOL` a `BNB` s kompaktným formátovaním cien (`68.5K`, `2.6K`).
- **▶️ YouTube kanál & Odberatelia:**
  - Živý počet odberateľov z Google Data API v3 s prehľadným zobrazením jednotiek (`1.4M`, `250K`).
- **🔄 Duálny Over-The-Air (OTA) aktualizačný systém:**
  - Priame nahrávanie `.bin` súboru priamo z webového prehliadača cez `/update` s ukazovateľom priebehu v percentách.
  - 1-Klik aktualizácia priamo z GitHub Releases (`/api/ota/check`, `/api/ota/install`).
  - Živé zobrazenie priebehu inštalácie v percentách priamo na 8×32 LED displeji (`OTA: XX%`).
- **⏱️ Dynamický kolotoč obrazoviek:**
  - Plynulý horizontálny rolovací engine s fixovanou 8×8 stavovou ikonou.
  - Nastaviteľné oneskorenie prepínania obrazoviek (3 až 120 sekúnd).
  - Možnosť manuálneho prepínania a pozastavenia rotácie pomocou fyzických tlačidiel.

---

## 🎛️ Fyzické ovládanie zariadenia

| Tlačidlo | Akcia | Funkcia |
|---|---|---|
| **Vľavo** (GPIO 26) | Krátke stlačenie | Prepnutie na predchádzajúcu obrazovku (pozastaví rotáciu) |
| **Vpravo** (GPIO 14) | Krátke stlačenie | Prepnutie na nasledujúcu obrazovku (pozastaví rotáciu) |
| **OK / Stred** (GPIO 27) | Krátke stlačenie | Cyklovanie manuálneho jasu (Nízky / Stredný / Vysoký) |
| **OK / Stred** (GPIO 27) | Dlhé stlačenie (>800ms) | Zapnutie / vypnutie automatického striedania obrazoviek |
| **Ľubovoľné tlačidlo** | Stlačenie v nočnom režime | Dočasné prebudenie displeja na 12 sekúnd na denný jas |

---

## 🌐 Webový konfiguračný portál

Po pripojení k Wi-Fi sieti otvorte vo webovom prehliadači adresu `http://<ip-adresa-zariadenia>/` (alebo `http://192.168.4.1/` v AP režime):

1. **Wi-Fi Sieť:** Skenovanie okolitých sietí a uloženie hesla.
2. **Obrazovka & Senzory:** Aktivácia adaptívneho jasu, nastavenie denného a nočného stropu jasu, konfigurácia nočných hodín a intervalu striedania obrazoviek.
3. **Predpoveď počasia:** Výber mesta zo zoznamu alebo zadanie vlastnej zemepisnej šírky a dĺžky.
4. **Finančný ticker:** Výber kryptomenového páru (`BTC`, `ETH`, `SOL`, `BNB`).
5. **YouTube kanál:** Zadanie ID kanála (alebo @handle) a Google API kľúča.
6. **Aktualizácia firmvéru (OTA):** Nahrávanie lokálneho `.bin` súboru alebo inštalácia z GitHubu na jedno kliknutie.

---

## 🔌 Hardvérová architektúra a pinout

| Komponent | Typ periférie | GPIO Pin | Popis |
|---|---|---|---|
| **Displej** | WS2812B NeoPixel | **GPIO 32** | 8×32 matica (256 LED diód, cik-cak stĺpce) |
| **Senzor (I2C)** | Sensirion SHT30 | **GPIO 21 (SDA), GPIO 22 (SCL)** | Meranie teploty a vlhkosti, I2C adresa `0x44` |
| **Senzor svetla** | Fotorezistor (LDR) | **GPIO 33** | ADC1 CH5 s EMA filtrom |
| **Monitor batérie** | Meranie napätia | **GPIO 34** | ADC1 CH6 s deličom napätia 1:1 |
| **Bzučiak** | Piezo menič | **GPIO 15** | Neinvazívna zvuková odozva a kliky |
| **Ľavé tlačidlo** | Mikrospínač | **GPIO 26** | Aktívne v stave LOW, interný pull-up |
| **Stredné tlačidlo** | Mikrospínač | **GPIO 27** | Aktívne v stave LOW, interný pull-up |
| **Pravé tlačidlo** | Mikrospínač | **GPIO 14** | Aktívne v stave LOW, interný pull-up |
| **Napájanie** | Li-ion akumulátor | — | Kapacita 4400 mAh, nabíjanie cez USB-C |

---

## 🛠️ Zostavenie a nahrávanie firmvéru

### Požiadavky
- [PlatformIO Core](https://platformio.org/) alebo rozšírenie pre VS Code

### Kompilácia projektu
```powershell
pio run -e ulanzi-tc001
```

### Nahrávanie cez USB (Windows PowerShell)
Vždy nastavte kódovanie konzoly na UTF-8 na zamedzenie pádov knižnice Python na Windows:
```powershell
$env:PYTHONIOENCODING="utf8"; pio run -e ulanzi-tc001 -t upload
```

### Sériový monitor
```powershell
pio device monitor -b 115200
```

---

## 📦 Zostavené balíčky (Releases)

Predkompilované binárne súbory pre každé vydanie sú dostupné v sekcii [Releases](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases):
- `firmware.bin`: OTA aktualizačný súbor pre webový portál a automatické online aktualizácie.
- `firmware.factory.bin`: Kompletný továrenský obraz pre prvotné nahrávanie cez USB kábel od adresy `0x0000`.

---

## 📄 Licencia
Tento projekt je zverejnený ako open-source pod licenciou MIT.
