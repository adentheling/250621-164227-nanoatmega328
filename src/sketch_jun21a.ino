#include <Adafruit_NeoPixel.h>

#define NUM_STRIPS 1
#define LED_PIN    2
#define TOTAL_LEDS 60

Adafruit_NeoPixel strips[NUM_STRIPS] = {
  Adafruit_NeoPixel(TOTAL_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800)
};

// ====== USER DEFINED GROUPS IN EASY FORMAT ======
const char* groupDefs[] = {
  "1-5",
  "8,10-12",
  "20-25,,29,30,30-60"
};
const uint8_t groupCount = sizeof(groupDefs) / sizeof(groupDefs[0]);
// ================================================

// Holds actual parsed LED IDs per group
#define MAX_GROUP_SIZE 50
uint8_t groups[groupCount][MAX_GROUP_SIZE];
uint8_t groupSizes[groupCount];  // auto-filled

// State
float groupPhase[groupCount];
float groupSpeed = 0.001;
uint8_t brightness = 10;
bool isLedOff[TOTAL_LEDS];

void setup() {
  Serial.begin(115200);
  Serial.println("LED Controller with Range Parser");

  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].begin();
    strips[i].show();
  }

  parseAllGroups();

  for (int i = 0; i < groupCount; i++) {
    groupPhase[i] = (float)i / groupCount;
  }

  for (int i = 0; i < TOTAL_LEDS; i++) {
    isLedOff[i] = false;
  }
}

void loop() {
  handleSerialCommands();

  for (int g = 0; g < groupCount; g++) {
    groupPhase[g] += groupSpeed;
    if (groupPhase[g] > 1.0) groupPhase[g] -= 1.0;

    uint16_t hue = (uint16_t)(groupPhase[g] * 65535);
    uint32_t fullColor = Adafruit_NeoPixel::ColorHSV(hue, 255, 255);

    uint8_t r = (fullColor >> 16) & 0xFF;
    uint8_t gVal = (fullColor >> 8) & 0xFF;
    uint8_t b = fullColor & 0xFF;
    r = (r * brightness) / 255;
    gVal = (gVal * brightness) / 255;
    b = (b * brightness) / 255;

    uint32_t color = strips[0].Color(r, gVal, b);

    for (int i = 0; i < groupSizes[g]; i++) {
      uint8_t led = groups[g][i];
      if (led < TOTAL_LEDS && !isLedOff[led]) {
        strips[0].setPixelColor(led, color);
      }
    }
  }

  for (int i = 0; i < TOTAL_LEDS; i++) {
    if (isLedOff[i]) {
      strips[0].setPixelColor(i, 0);
    }
  }

  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].show();
  }

  delay(20);
}

// ====== STRING PARSING FUNCTIONS ======

void parseAllGroups() {
  for (int g = 0; g < groupCount; g++) {
    groupSizes[g] = parseGroup(groupDefs[g], groups[g], MAX_GROUP_SIZE);
  }
}

int parseGroup(const char* str, uint8_t* buffer, int maxSize) {
  int count = 0;
  String input = String(str);
  input.replace(" ", "");
  while (input.length() > 0 && count < maxSize) {
    int commaIndex = input.indexOf(',');
    String token = (commaIndex >= 0) ? input.substring(0, commaIndex) : input;

    int dashIndex = token.indexOf('-');
    if (dashIndex >= 0) {
      int start = token.substring(0, dashIndex).toInt();
      int end = token.substring(dashIndex + 1).toInt();
      for (int i = start; i <= end && count < maxSize; i++) {
        buffer[count++] = i;
      }
    } else {
      buffer[count++] = token.toInt();
    }

    input = (commaIndex >= 0) ? input.substring(commaIndex + 1) : "";
  }
  return count;
}

// ====== SERIAL CONTROL ======

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("off")) {
      int ledNum = command.substring(3).toInt();
      if (ledNum >= 0 && ledNum < TOTAL_LEDS) {
        isLedOff[ledNum] = true;
        Serial.print("Turned OFF LED ");
        Serial.println(ledNum);
      }
    } else if (command.startsWith("on")) {
      int ledNum = command.substring(2).toInt();
      if (ledNum >= 0 && ledNum < TOTAL_LEDS) {
        isLedOff[ledNum] = false;
        Serial.print("Turned ON LED ");
        Serial.println(ledNum);
      }
    } else if (command.equals("show")) {
      Serial.println("Groups:");
      for (int g = 0; g < groupCount; g++) {
        Serial.print("Group ");
        Serial.print(g);
        Serial.print(" (");
        Serial.print(groupSizes[g]);
        Serial.print("): ");
        for (int i = 0; i < groupSizes[g]; i++) {
          Serial.print(groups[g][i]);
          Serial.print(" ");
        }
        Serial.println();
      }

      Serial.println("LEDs turned OFF:");
      for (int i = 0; i < TOTAL_LEDS; i++) {
        if (isLedOff[i]) {
          Serial.print(i);
          Serial.print(" ");
        }
      }
      Serial.println();
    } else {
      Serial.println("Commands: off <led>, on <led>, show");
    }
  }
}
