# Pulse Oximeter ESP32 Project

A functional pulse oximeter system built using the ESP32 microcontroller and MAX30102 sensor. This project demonstrates real-time measurement of blood oxygen saturation (SpO₂) and heart rate (BPM), displayed on a 128x64 OLED screen. A second version simulates photoplethysmography (PPG) signals, enabling full signal processing pipeline testing without hardware.

## Why This Project Matters

Designed as a showcase of self-taught applied engineering in MedTech, this project reflects my ability to:
- Translate medical sensing theory into working embedded systems
- Apply signal processing techniques to extract physiological metrics
- Work across hardware, firmware, and algorithm development
- Communicate results clearly via UI (OLED) and documentation

As a incoming graduate student in Human Systems Engineering concentraing in Health Sysems with a background in Health Technology, this hands-on build sharpens my technical credibility while reinforcing my user-centric design foundation.

---

## 🛠️ System Overview

| Component         | Description                                      |
|------------------|--------------------------------------------------|
| Microcontroller  | ESP32 DOIT DevKit V1                             |
| Sensor           | MAX30102 (IR + Red LED, photodetector)           |
| Display          | SSD1306 OLED (128x64, I2C)                        |
| Output Metrics   | SpO₂, BPM                                         |
| Modes            | Hardware mode + Simulated data mode              |
| Communication    | I2C for sensor + display                         |
| Platform         | Arduino IDE                                      |

---

## ⚙️ How It Works – Step-by-Step

### 1. **Signal Acquisition**
- **Hardware Mode**: The MAX30102 emits IR and Red light into tissue and captures reflected light to generate raw PPG signals.
- **Simulated Mode**: Gaussian pulses simulate systolic/diastolic peaks + noise to mimic real PPG waveforms.

### 2. **Filtering**
- **High-Pass Filter** removes baseline wander (DC drift).
- **Low-Pass Filter** smooths high-frequency noise and stabilizes AC signals.

### 3. **AC/DC Extraction**
- AC component = pulsatile flow (from heartbeat)
- DC component = constant light absorption by tissue
- Calculated for both IR and Red signals over a circular buffer.

### 4. **SpO₂ Calculation**
Formula used: R = (AC_Red / DC_Red) ÷ (AC_IR / DC_IR)
SpO₂ = 110 - 25 × R

This ratio-of-ratios method approximates blood oxygen saturation.

### 5. **Heart Rate Detection**
- Peaks in the IR waveform are detected using amplitude thresholds.
- Time between peaks determines BPM:  
  `BPM = 60 / (time between peaks)`

### 6. **Display Output**
- Values are shown on the OLED in real-time.
- Simulated mode outputs signals to Serial Plotter.

---

## Modes

### Hardware Version
- Uses real data from MAX30102
- Outputs live BPM/SpO₂ to OLED

### Simulated Version
- No sensor required
- Generates synthetic Red/IR PPG signals
- Follows same filtering and calculation logic
- Outputs to Serial Plotter for visualization

---

## 📁 Files Included

- `hardware_version.ino`  
  Final implementation for real-time BPM + SpO₂ output using actual hardware.

- `simulated_version.ino`  
  Simulated PPG generator + full signal processing pipeline, with detailed inline comments.

- `README.md`  
  Full documentation and system architecture.

---

## Setup Instructions

### Requirements
- **Libraries**:
  - `SparkFun MAX3010x`
  - `Adafruit GFX`
  - `Adafruit SSD1306`

### Hardware Wiring (I2C):
| ESP32 Pin | MAX30102 | OLED Display |
|-----------|-----------|---------------|
| GPIO 21   | SDA       | SDA           |
| GPIO 22   | SCL       | SCL           |
| 3.3V      | VIN       | VCC           |
| GND       | GND       | GND           |

### Uploading
1. Open Arduino IDE
2. Select board: **ESP32 Dev Module**
3. Install required libraries via Library Manager
4. Upload `hardware_version.ino` or `simulated_version.ino`
5. Use OLED or Serial Monitor/Plotter for output

---

## Skills Demonstrated

- Embedded firmware design
- Digital signal processing
- Biomedical sensing fundamentals
- I2C integration
- Real-time systems debugging
- Simulation-driven development

---

## 📷 Demo Media

_Add OLED photos, GIFs, or Serial Plotter screenshots if available._

---

## 📌 License

This project is open-source under the MIT License.

