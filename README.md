# LED Group Controller (WS2812 / NeoPixel)

This project lets you control **one or more WS2812/NeoPixel LED strips** with rich group-based animations, live LED toggling, and a simple interface.

---

## Features

- 🌈 Fade different **LED groups independently**
- ✨ Groups defined with **human-friendly ranges** (`"1-5, 8, 10-12"`)
- 🖥️ Live control: **turn individual LEDs on/off via Serial**
- 🔌 Support for **multiple LED strips on different pins**
- 🔧 Just update a few lines — no deep code editing needed

---

## Setup

### 1. Hardware Required

- Arduino (e.g., Nano, Uno, Mega)
- 1 or more WS2812/NeoPixel LED strips
- Power source (⚠️ power separately for >30 LEDs)
- Common ground between strips and Arduino

---

### 2. Software Setup

In your Arduino sketch, update the following areas:

---

### ① Define Your LED Strips

```cpp
#define NUM_STRIPS 2  // number of strips you're using

Adafruit_NeoPixel strips[NUM_STRIPS] = {
  Adafruit_NeoPixel(60, 2, NEO_GRB + NEO_KHZ800),  // Strip 0: 60 LEDs on pin 2
  Adafruit_NeoPixel(30, 3, NEO_GRB + NEO_KHZ800)   // Strip 1: 30 LEDs on pin 3
};

#define TOTAL_LEDS 90  // Total of all strips (e.g., 60 + 30)
```

You can keep using `strips[0]` for now — multi-strip logic will need minor expansion if you want **per-strip animation**.

---

### ② Define Your LED Groups

Use **`groupDefs[]`** with human-friendly strings to define animation groups:

```cpp
const char* groupDefs[] = {
  "1-5",          // Group 0
  "8,10-12",      // Group 1
  "20-25,30"      // Group 2
};
```

No need to calculate sizes — the code does it automatically.

---

### ③ (Optional) Adjust Settings

```cpp
float groupSpeed = 0.01;   // How fast color cycles
uint8_t brightness = 150;  // LED brightness (0–255)
```

---

## Serial Command Interface

Use the Serial Monitor (baud 115200) to:

### Turn off an LED:
```
off 6
```

### Turn on an LED:
```
on 6
```

### Show current group/LED info:
```
show
```

---

## Advanced: Add a New LED Strip

If you want to animate multiple **independent** strips:

1. Add it to the `strips[]` array  
2. Update `TOTAL_LEDS`  
3. Extend logic in `loop()` if you want each strip to have different patterns  
   Example:
   ```cpp
   // For second strip (strips[1]):
   strips[1].setPixelColor(i, color);
   strips[1].show();
   ```

Currently the logic applies to `strips[0]` only. Expand it to handle each strip individually as needed.

---

## Example

```cpp
const char* groupDefs[] = {
  "0-4",         // Group 0
  "5-9",         // Group 1
  "10-14,16,18,20-60"  // Group 2
};
```

You can add/remove/edit group lines freely — just make sure they refer to valid LED indices within `TOTAL_LEDS`.

---

## Persistence

Changes made via Serial (`off`, `on`) are **not saved** after reboot. This is by design — manually update your `groupDefs[]` if you want to make changes permanent.

---

## Summary

| Feature         | How to use                         |
|----------------|------------------------------------|
| Add LED strip   | Add line to `strips[]` array       |
| Add group       | Add line to `groupDefs[]`          |
| Control LED     | Use Serial: `off <id>`, `on <id>`  |
| See info        | Type `show` in Serial Monitor      |
| Adjust brightness | Change `brightness` variable (0–255) |
| Adjust animation speed | Change `groupSpeed` variable |

---

## Notes

- LED indices are **zero-based** (LED 0 = first pixel).
- Each group cycles through hues independently, based on its index.
- Serial `off`/`on` commands affect only visual output (not group definitions).
- If you power your strips separately, always **connect GND** to the Arduino’s GND.

---