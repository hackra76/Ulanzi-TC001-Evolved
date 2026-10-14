# Ulanzi TC001 Evolved 🚀 (Slovenská dokumentácia)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Kompatibiln%C3%A9-orange?logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue?logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/Licencia-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Evolved?color=brightgreen)](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases)

Pokročilý firmvér novej generácie pre inteligentné stolové pixelové hodiny **Ulanzi TC001 Smart Pixel Desktop Clock** bežiaci na mikrokontroléri ESP32 s **8×32 WS2812B RGB LED maticou**, senzorom Sensirion SHT30, adaptívnym jasom (LDR), sledovaním finančných trhov, akcií a kryptomien, živým leteckým radarom (ADS-B), predpoveďou počasia, YouTube štatistikami, prispôsobiteľným Awtrix rozložením hodín a duálnym OTA systémom aktualizácií.

🌐 **Jazyky:** [English (Predvolený)](README.md) • [English Zrkadlo](README_EN.md) • [Slovenčina](README_SK.md)

---

## ✨ Kľúčové funkcie

- **📱 Wi-Fi Captive Portal & Sprievodca pripojením:**
  - Automatický prechod do SoftAP režimu (`Ulanzi-Setup-XXXX`) s presmerovaním na captive portál na adrese `192.168.4.1`.
  - Neinvazívny skener okolitých Wi-Fi sietí s ukazovateľom sily signálu (RSSI).
  - Spoľahlivé ukladanie prístupových údajov do pamäte NVS (`Preferences`) s automatickým opätovným pripájaním.
- **📺 Živý stream displeja & Virtuálna matica:**
  - Zrkadlenie 32×8 NeoPixel displeja v reálnom čase priamo vo webovom portáli s automatickou obnovou každých 1,5s.
  - Export snímky displeja do BMP na jeden klik (`/api/display/screenshot`).
  - Textová ASCII art reprezentácia displeja (`/api/display/text`).
  - Tlačidlá na manuálne prepínanie obrazoviek priamo z prehliadača.
- **⏰ Awtrix-Style Hodiny & Dátum:**
  - 3 režimy zobrazenia: Iba čas, Kombinovaný čas a dátum (striedanie 5s/3s) alebo Iba dátum.
  - 7 stĺpcov dní v týždni pod časom s voliteľnými farbami pre aktívny a neaktívny deň, začiatok týždňa v pondelok (ISO 8601) alebo v nedeľu.
  - Formáty dátumu: `DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`.
  - Štýly ikon hodín: Dynamický kalendár (číslo dňa 1–31), Srdiečko, Ciferník alebo Plná šírka bez ikony.
  - Zobrazenie sekúnd: Plná šírka s vycentrovaným časom `HH:MM:SS` a dvojbodkami; pri zapnutých sekundách sa ikona automaticky skryje pre dokonalú čitateľnosť.
  - 3×5 proporcionálne písmo na riadku 1 a pásiky dní na riadku 7 (žiadny vertikálny dotyk s písmom).
- **✈️ Živý letecký radar (ADS-B):**
  - Sledovanie prelietavajúcich lietadiel v reálnom čase cez OpenSky Network v okruhu ~50 km okolo nastavených GPS súradníc počasia.
  - Automatické striedanie volacích znakov (Callsign) lietadiel každé 4 sekundy s tyrkysovou ikonou lietadla.
  - Zobrazenie zeleného nápisu `"CLEAR"`, ak je vzdušný priestor voľný.
- **⚡ Telemetria batérie a energie:**
  - Výrazná neónovo žltá a biela ikona blesku (`ICON_LIGHTNING`).
  - Voliteľný formát zobrazenia: Iba percentá (`93%`), Iba napätie (`4.12V`) alebo automatické striedanie každé 3 sekundy.
- **📈 Finančné trhy, akcie, komodity a krypto:**
  - Verejné trhové kanály bez nutnosti registrácie API kľúčov.
  - Kryptomeny: `BTC`, `ETH`, `SOL`, `BNB`.
  - Akcie a indexy: Apple (`AAPL`), Tesla (`TSLA`), Nvidia (`NVDA`), S&P 500 (`SPX`).
  - Forex a komodity: Zlato (`GOLD`), `EUR/USD`, `USD/EUR`.
  - Možnosť zadania vlastného burzového tickera s dynamickým farebným akcentom.
- **▶️ YouTube kanál & Odberatelia:**
  - Živý počet odberateľov z Google Data API v3.
  - Prepracovaná výrazná biela ikona prehrávania (play button) v červenej pilulke.
- **🌤️ Skutočné vonkajšie počasie:**
  - Živé meteorologické údaje z bezplatného Open-Meteo API (nevyžaduje žiadny API kľúč).
  - Mapovanie WMO kódov počasia na animované 8×8 ikony (Slnko, Oblaky, Dážď, Sneh, Búrka).
  - **Zoznam prednastavených miest:** Viac ako 25 prednastavených miest (Slovensko, Česko, Európa, Svet) s automatickým vyplnením GPS súradníc.
- **💡 Adaptívny jas & Nočný režim:**
  - Plynulý automatický jas riadený fotorezistorom (LDR) s filtrom EMA a nelineárnou gama krivkou.
  - Denný strop jasu (5% – 100%) a Nočný jas (1% – 25%).
  - Konfigurovateľné tiché hodiny s tlmenými červenými hodinami a polmesiacom.
  - Rýchle prebudenie: Stlačenie ľubovoľného tlačidla v noci dočasne rozsvieti displej na 12 sekúnd na denný jas.
- **🎛️ Zoradenie obrazoviek kolotoča:**
  - Tlačidlá `▲` / `▼` vo webovom portáli na zmenu poradia obrazoviek podľa vlastného výberu.
  - Samostatné zaškrtávacie políčka na aktiváciu alebo deaktiváciu ktorejkoľvek obrazovky.
  - Vlastné poradie sa ukladá do NVS (`car_ord`) a riadi automatickú rotáciu aj tlačidlá.
  - Nastaviteľný interval rotácie (3 až 120 sekúnd).
  - Okamžité ukladanie cez AJAX s plávajúcou notifikáciou bez opustenia portálu.
- **🎬 Efekty prechodu obrazoviek:**
  - Výber z 8 plynulých animácií pri zmene obrazoviek vo webovom portáli:
    - **Žiadny (Okamžitý prepis)**: Bez animácie.
    - **Posun doľava (Slide Left)**: Plynulý horizontálny posun vľavo (krok 2 px, 16 fáz).
    - **Posun doprava (Slide Right)**: Plynulý horizontálny posun vpravo.
    - **Posun nahor (Slide Up)**: Vertikálny posun zdola nahor (8 fáz).
    - **Posun nadol (Slide Down)**: Vertikálny posun zhora nadol.
    - **Prelínanie (Crossfade / Dissolve)**: 10-krokové plynulé prelínanie farieb cez natívny 24-bitový GRB double-buffer.
    - **Opona (Curtain Split Wipe)**: Rozdelenie a odhalenie novej obrazovky od stredu k okrajom.
    - **Náhodný (Random)**: Automatický náhodný výber prechodového efektu pri každej zmene.
  - Priamy zápis do vyrovnávacej pamäte eliminuje preblikávanie a zachováva plný jas bez straty intenzity.
- **🏠 Home Assistant MQTT Auto-Discovery:**
  - Natívna integrácia bez manuálnej konfigurácie: automaticky vytvorí 12 synchronizovaných entít v Home Assistante po pripojení k MQTT brokeru.
  - Senzory: SHT30 Teplota (`°C`), Vlhkosť (`%`), Batéria (`%`), Napätie batérie (`V`), Osvetlenie, Aktívna obrazovka.
  - Ovládacie prvky: Svetlo matice (Zapnutie/Vypnutie a posuvník jasu 0–255), Prepínač automatického jasu, Prepínač automatického kolotoča, Výber obrazovky, Tlačidlo pípania bzučiaka, Tlačidlo reštartu.
  - Živý stav pripojenia a konfiguračné polia brokera priamo vo webovom portáli.
- **🎵 Custom App Push API & Spotify Integrácia:**
  - **Dynamické obrazovky**: Odosielanie vlastných obrazoviek cez MQTT (`<prefix>/custom/<nazov>`) alebo HTTP REST (`POST /api/custom?name=<nazov>`).
  - Prijaté obrazovky (napr. aktuálne prehrávaná skladba Spotify, stav 3D tlačiarne, časovač) sa dynamicky zaradia do rotácie kolotoča.
  - **Funkcie správ**: Text, 8×8 ikony (vstavané `spotify`, `bell`, `lightning`, `youtube` atď., alebo 64 farebných hodnôt RGB565), farby, 1-pixelový spodný progress bar (0–100%), vlastná dĺžka zobrazenia a automatické vypršanie (lifetime).
  - **Čistá typografia bez kolízií**: Text sa posúva na riadkoch 0–6 pri aktívnom progress bare na riadku 7 bez prekrývania.
  - **Pripnuté ikony**: Ľavá ikona zostáva fixne na vrchu s vyčistenými stĺpcami 0–8, takže rolujúci text plynule mizne za ňou.
  - **🚨 Prioritné notifikácie**: Okamžité výstrahy cez `POST /api/notify` alebo `<prefix>/notify`, ktoré prerušia aktívnu obrazovku so zvukovým signálom a po skončení vrátia pôvodný obsah.
- **🔄 Duálny Over-The-Air (OTA) aktualizačný systém:**
  - Priame nahrávanie `.bin` súboru priamo z webového prehliadača cez `/update` s ukazovateľom priebehu v percentách.
  - 1-Klik aktualizácia priamo z GitHub Releases (`/api/ota/check`, `/api/ota/install`).
  - Živé zobrazenie priebehu inštalácie v percentách priamo na 8×32 LED displeji (`OTA: XX%`).

---

## 🎛️ Fyzické ovládanie zariadenia

| Tlačidlo | Akcia | Funkcia |
|---|---|---|
| **Vľavo** (GPIO 26) | Krátke stlačenie | Prepnutie na predchádzajúcu obrazovku podľa vlastného poradia (pozastaví rotáciu) |
| **Vpravo** (GPIO 14) | Krátke stlačenie | Prepnutie na nasledujúcu obrazovku podľa vlastného poradia (pozastaví rotáciu) |
| **OK / Stred** (GPIO 27) | Krátke stlačenie | Cyklická zmena manuálneho jasu (Nízky / Stredný / Vysoký) |
| **OK / Stred** (GPIO 27) | Dlhé stlačenie (>700ms) | Zapnutie / vypnutie automatického kolotoča obrazoviek |
| **Ľubovoľné tlačidlo** | Stlačenie v nočnom režime | Dočasné prebudenie displeja na 12 sekúnd na denný jas |

---

## 🌐 Webové konfiguračné rozhranie

Portál je dostupný na adrese `http://<ip-adresa-zariadenia>/` (alebo `http://192.168.4.1/` v režime nastavenia):

1. **Živý náhľad displeja:** Reálny 32×8 canvas stream a tlačidlo na stiahnutie BMP snímky.
2. **Wi-Fi Sieť:** Skenovanie okolitých sietí a ukladanie prístupových údajov.
3. **Svetelný senzor & Obrazovka:** Automatický jas, denný a nočný strop, tiché nočné hodiny.
4. **Kolotoč obrazoviek & Prechody:** Zmena poradia pomocou tlačidiel `▲`/`▼`, výber aktívnych obrazoviek a výber z 8 prechodových efektov (Posun, Prelínanie, Opona, Náhodný).
5. **Formát batérie:** Percentá, Napätie alebo Striedanie každé 3 sekundy.
6. **Rozloženie hodín a dátumu:** Režim hodín, zobrazenie sekúnd, stĺpce dní, farby, formát dátumu a ikona.
7. **Predpoveď počasia:** Výber mesta zo zoznamu alebo vlastné GPS súradnice.
8. **Finančné trhy a krypto:** Výber akcie, indexu, komodity alebo kryptomeny, prípadne vlastný ticker.
9. **YouTube Štatistiky:** ID kanála alebo @handle a Google Cloud API kľúč.
10. **Home Assistant & MQTT:** Nastavenie IP brokera, portu, prístupových údajov a predpony témy so živým indikátorom pripojenia.
11. **Aktualizácia firmvéru (OTA):** Flash lokálneho `.bin` súboru alebo kontrola nových verzií na GitHube.

---

## 🔌 Hardvérová architektúra a zapojenie pinov

| Komponent | Súčiastka | GPIO Pin | Popis |
|---|---|---|---|
| **Displej** | WS2812B NeoPixel | **GPIO 32** | Matica 8×32 (256 LED, cik-cak zapojenie stĺpcov) |
| **Senzor (I2C)** | Sensirion SHT30 | **GPIO 21 (SDA), GPIO 22 (SCL)** | Teplota a vlhkosť, I2C adresa `0x44` |
| **Svetelný senzor** | LDR fotorezistor | **GPIO 33** | ADC1 CH5 s EMA filtrom |
| **Snímanie batérie** | Li-ion telemetria | **GPIO 34** | ADC1 CH6 s odporovým deličom 1:1 |
| **Bzučiak** | Piezo menič | **GPIO 15** | Tónová signalizácia bez blokovania slučky |
| **Ľavé tlačidlo** | Taktilný spínač | **GPIO 26** | Aktívne v nule (LOW), interný pull-up |
| **Stredné tlačidlo** | Taktilný spínač | **GPIO 27** | Aktívne v nule (LOW), interný pull-up |
| **Pravé tlačidlo** | Taktilný spínač | **GPIO 14** | Aktívne v nule (LOW), interný pull-up |
| **Napájanie** | Li-ion akumulátor | — | Kapacita 4400 mAh s nabíjaním cez USB-C |

---

## 🛠️ Zostavenie a nahratie firmvéru

### Požiadavky
- [PlatformIO Core](https://platformio.org/) alebo PlatformIO IDE (rozšírenie pre VS Code)

### Kompilácia firmvéru
```powershell
pio run -e ulanzi-tc001
```

### Nahratie cez USB kábel (Windows PowerShell)
Vždy nastavte kódovanie konzoly na UTF-8 na ochranu pred pádmi Pythonu:
```powershell
$env:PYTHONIOENCODING="utf8"; pio run -e ulanzi-tc001 -t upload
```

### Sledovanie sériového portu (Monitor)
```powershell
pio device monitor -b 115200
```

---

## 📦 Zostavené vydania (Releases)

Predkompilované binárne súbory sú k dispozícii v sekcii [Releases](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases):
- `firmware.bin`: Súbor pre OTA aktualizáciu cez webové rozhranie alebo GitHub 1-klik aktualizáciu.
- `firmware.factory.bin`: Kompletný obraz pamäte pre prvotné nahratie cez USB od adresy `0x0000`.

---

## 📄 Licencia
Tento projekt je otvorený softvér pod licenciou MIT.
