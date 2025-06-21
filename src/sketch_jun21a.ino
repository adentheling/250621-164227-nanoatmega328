#include <Adafruit_NeoPixel.h>

// ======= USER CONFIGURATION ========

// Number of LED strips connected
#define NUM_STRIPS 1

// First strip: 60 LEDs on pin 2
Adafruit_NeoPixel strips[NUM_STRIPS] = {
  Adafruit_NeoPixel(60, 2, NEO_GRB + NEO_KHZ800)
};

// Total number of LEDs (used in Serial control)
#define TOTAL_LEDS 60

// Group definitions (specify which LEDs are grouped for shared color animation)
const uint8_t groupCount = 3;

// Each inner array defines one group (up to 20 LEDs per group — you can increase this limit)
const uint8_t groups[][20] = {
  {1, 2, 3, 4, 5},                                 // Group 0: LEDs 1–5
  {8, 9,10,11,12,13,14,15,16,17,18,19},            // Group 1: LEDs 8–19
  {20,21,22,23,24,25}                              // Group 2: LEDs 20–25
};

// Number of LEDs per group (must match the groups above)
const uint8_t groupSizes[] = {5, 12, 6};

// Global brightness (0–255)
uint8_t brightness = 150;

// Speed of color cycling
float groupSpeed = 0.01;

// ========== END USER CONFIG ==========

// Phase value for each group (controls color position)
float groupPhase[groupCount];

// Stores which LEDs are turned off via Serial (volatile, not saved)
bool isLedOff[TOTAL_LEDS];  // false by default

void setup() {
  Serial.begin(115200);
  Serial.println("LED controller ready. Type 'off 5', 'on 5', or 'show'.");

  // Initialize strips
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].begin();
    strips[i].show();
  }

  // Initialize group color phases
  for (int i = 0; i < groupCount; i++) {
    groupPhase[i] = (float)i / groupCount;
  }

  // Clear off flags
  for (int i = 0; i < TOTAL_LEDS; i++) {
    isLedOff[i] = false;
  }
}

void loop() {
  handleSerialCommands();

  // Animate each group with its own hue
  for (int g = 0; g < groupCount; g++) {
    groupPhase[g] += groupSpeed;
    if (groupPhase[g] > 1.0) groupPhase[g] -= 1.0;

    uint16_t hue = (uint16_t)(groupPhase[g] * 65535);
    uint32_t fullColor = Adafruit_NeoPixel::ColorHSV(hue, 255, 255);

    // Scale brightness manually (NeoPixel doesn't support global brightness for HSV)
    uint8_t r = (fullColor >> 16) & 0xFF;
    uint8_t gVal = (fullColor >> 8) & 0xFF;
    uint8_t b = fullColor & 0xFF;

    r = (r * brightness) / 255;
    gVal = (gVal * brightness) / 255;
    b = (b * brightness) / 255;

    uint32_t color = strips[0].Color(r, gVal, b);

    // Apply color to each LED in the group
    for (int i = 0; i < groupSizes[g]; i++) {
      uint8_t led = groups[g][i];
      if (led < TOTAL_LEDS && !isLedOff[led]) {
        strips[0].setPixelColor(led, color);
      }
    }
  }

  // Turn off any LEDs manually disabled
  for (int i = 0; i < TOTAL_LEDS; i++) {
    if (isLedOff[i]) {
      strips[0].setPixelColor(i, 0);  // Off
    }
  }

  // Update LED strip
  for (int i = 0; i < NUM_STRIPS; i++) {
    strips[i].show();
  }

  delay(20);  // animation speed
}

// ===================== SERIAL COMMAND HANDLER ======================

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
        Serial.print(": ");
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
      Serial.println("Unknown command. Use: off <led>, on <led>, show");
    }
  }
}
