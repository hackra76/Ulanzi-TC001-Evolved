#include "Buzzer.h"
#include "Config.h"
#include <ctype.h>

struct ToneNote {
  uint16_t freq;
  uint16_t dur;
  uint16_t pause;
};

#define MAX_TONE_QUEUE 64
static ToneNote s_queue[MAX_TONE_QUEUE];
static uint8_t s_qHead = 0;
static uint8_t s_qTail = 0;
static uint8_t s_qCount = 0;

static bool s_isPlaying = false;
static bool s_inPause = false;
static uint32_t s_noteEndMs = 0;
static uint32_t s_pauseEndMs = 0;
static uint16_t s_currentPause = 0;
static bool s_ledcAttached = false;

static void startHardwareTone(uint16_t freqHz) {
  if (freqHz == 0) {
    if (s_ledcAttached) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
      ledcWriteTone(PIN_BUZZER, 0);
      ledcDetach(PIN_BUZZER);
#else
      ledcWriteTone(BUZZER_LEDC_CH, 0);
      ledcDetachPin(PIN_BUZZER);
#endif
      s_ledcAttached = false;
    }
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    return;
  }

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  if (!s_ledcAttached) {
    ledcAttach(PIN_BUZZER, freqHz, BUZZER_LEDC_RES);
    s_ledcAttached = true;
  }
  ledcWriteTone(PIN_BUZZER, freqHz);
#else
  if (!s_ledcAttached) {
    ledcSetup(BUZZER_LEDC_CH, freqHz, BUZZER_LEDC_RES);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CH);
    s_ledcAttached = true;
  }
  ledcWriteTone(BUZZER_LEDC_CH, freqHz);
#endif
}

static void stopHardwareTone() {
  if (s_ledcAttached) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWriteTone(PIN_BUZZER, 0);
    ledcDetach(PIN_BUZZER);
#else
    ledcWriteTone(BUZZER_LEDC_CH, 0);
    ledcDetachPin(PIN_BUZZER);
#endif
    s_ledcAttached = false;
  }
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

static void playNextNote() {
  if (s_qCount == 0) {
    s_isPlaying = false;
    s_inPause = false;
    stopHardwareTone();
    return;
  }

  ToneNote n = s_queue[s_qHead];
  s_qHead = (s_qHead + 1) % MAX_TONE_QUEUE;
  s_qCount--;

  s_isPlaying = true;
  s_inPause = false;
  s_currentPause = n.pause;
  s_noteEndMs = millis() + n.dur;

  startHardwareTone(n.freq);
}

static void enqueueNote(uint16_t freq, uint16_t dur, uint16_t pause = 0) {
  if (s_qCount >= MAX_TONE_QUEUE)
    return;

  s_queue[s_qTail] = {freq, dur, pause};
  s_qTail = (s_qTail + 1) % MAX_TONE_QUEUE;
  s_qCount++;

  if (!s_isPlaying) {
    playNextNote();
  }
}

void Buzzer_Init() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  Buzzer_Stop();
}

void Buzzer_Stop() {
  s_qHead = 0;
  s_qTail = 0;
  s_qCount = 0;
  s_isPlaying = false;
  s_inPause = false;
  stopHardwareTone();
}

bool Buzzer_IsPlaying() {
  return s_isPlaying;
}

void Buzzer_Poll() {
  if (!s_isPlaying)
    return;
  uint32_t now = millis();

  if (!s_inPause) {
    if (now >= s_noteEndMs) {
      stopHardwareTone();
      if (s_currentPause > 0) {
        s_inPause = true;
        s_pauseEndMs = now + s_currentPause;
      } else {
        playNextNote();
      }
    }
  } else {
    if (now >= s_pauseEndMs) {
      s_inPause = false;
      playNextNote();
    }
  }
}

void Buzzer_Beep(uint16_t freqHz, uint16_t durationMs) {
  enqueueNote(freqHz, durationMs, 0);
}

void Buzzer_Click() { enqueueNote(2400, 15, 0); }

void Buzzer_Alert() {
  enqueueNote(3200, 70, 30);
  enqueueNote(3200, 70, 0);
}

void Buzzer_Success() {
  // Upward arpeggio: C5 -> E5 -> G5 -> C6
  enqueueNote(523, 50, 15);
  enqueueNote(659, 50, 15);
  enqueueNote(784, 50, 15);
  enqueueNote(1046, 100, 0);
}

void Buzzer_Notification() {
  // Dual soft bell chime: A5 -> D6
  enqueueNote(880, 60, 25);
  enqueueNote(1175, 110, 0);
}

void Buzzer_Alarm() {
  // High-urgency triple pulse for timer expiration
  enqueueNote(2800, 80, 30);
  enqueueNote(2800, 80, 30);
  enqueueNote(2800, 120, 0);
}

// Compact RTTTL (Ring Tone Text Transfer Language) Parser
static const uint16_t kNoteFrequenciesOctave4[] = {
    262, // C4
    277, // C#4
    294, // D4
    311, // D#4
    330, // E4
    349, // F4
    370, // F#4
    392, // G4
    415, // G#4
    440, // A4
    466, // A#4
    494  // B4
};

void Buzzer_PlayRTTTL(const char *rtttl) {
  if (!rtttl)
    return;
  Buzzer_Stop();

  const char *p = rtttl;
  // Skip title to first colon
  while (*p && *p != ':')
    p++;
  if (!*p)
    return;
  p++; // past ':'

  uint8_t defaultDuration = 4;
  uint8_t defaultOctave = 5;
  uint16_t bpm = 120;

  // Parse defaults section (e.g. "d=4,o=5,b=140:")
  while (*p && *p != ':') {
    while (*p == ' ' || *p == ',')
      p++;
    if (*p == 'd' && *(p + 1) == '=') {
      p += 2;
      defaultDuration = (uint8_t)atoi(p);
      while (isdigit(*p))
        p++;
    } else if (*p == 'o' && *(p + 1) == '=') {
      p += 2;
      defaultOctave = (uint8_t)atoi(p);
      while (isdigit(*p))
        p++;
    } else if (*p == 'b' && *(p + 1) == '=') {
      p += 2;
      bpm = (uint16_t)atoi(p);
      while (isdigit(*p))
        p++;
    } else {
      p++;
    }
  }
  if (*p == ':')
    p++;

  if (bpm == 0)
    bpm = 120;
  uint32_t wholenote = (60000 * 4) / bpm;

  // Parse notes section
  while (*p) {
    while (*p == ' ' || *p == ',')
      p++;
    if (!*p)
      break;

    // 1. Duration prefix
    uint8_t duration = 0;
    if (isdigit(*p)) {
      duration = (uint8_t)atoi(p);
      while (isdigit(*p))
        p++;
    }
    if (duration == 0)
      duration = defaultDuration;

    // 2. Note letter
    char noteLetter = tolower(*p);
    p++;

    // 3. Sharp symbol '#'
    bool isSharp = false;
    if (*p == '#' || *p == '_') {
      isSharp = true;
      p++;
    }

    // 4. Dot modifier '.' (1.5x length)
    bool isDotted = false;
    if (*p == '.') {
      isDotted = true;
      p++;
    }

    // 5. Octave
    uint8_t octave = defaultOctave;
    if (isdigit(*p)) {
      octave = (uint8_t)(*p - '0');
      p++;
    }

    // Dot modifier could also appear after octave
    if (*p == '.') {
      isDotted = true;
      p++;
    }

    uint32_t noteDur = wholenote / duration;
    if (isDotted)
      noteDur = (noteDur * 3) / 2;

    uint16_t freq = 0;
    if (noteLetter != 'p') {
      int8_t noteIdx = -1;
      switch (noteLetter) {
      case 'c':
        noteIdx = 0;
        break;
      case 'd':
        noteIdx = 2;
        break;
      case 'e':
        noteIdx = 4;
        break;
      case 'f':
        noteIdx = 5;
        break;
      case 'g':
        noteIdx = 7;
        break;
      case 'a':
        noteIdx = 9;
        break;
      case 'b':
        noteIdx = 11;
        break;
      default:
        break;
      }
      if (noteIdx >= 0) {
        if (isSharp)
          noteIdx++;
        if (noteIdx >= 12)
          noteIdx = 0;

        freq = kNoteFrequenciesOctave4[noteIdx];
        if (octave > 4) {
          freq = freq << (octave - 4);
        } else if (octave < 4) {
          freq = freq >> (4 - octave);
        }
      }
    }

    // Keep 15% articulation gap between notes
    uint16_t activeDur =
        (freq > 0) ? (uint16_t)((noteDur * 85) / 100) : (uint16_t)noteDur;
    uint16_t pauseDur = (freq > 0) ? (uint16_t)(noteDur - activeDur) : 0;

    enqueueNote(freq, activeDur, pauseDur);
    if (s_qCount >= MAX_TONE_QUEUE - 2)
      break;
  }
}
