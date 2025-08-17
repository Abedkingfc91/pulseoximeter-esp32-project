#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

float timeInSeconds = 0.0;
float samplingInterval = 0.01;
float lastPeakTime = 0.0;

const int bufferSize = 100;
float bufferIR[bufferSize], bufferRed[bufferSize];
int bufferIndex = 0;

float previousInputIR = 0, previousHPF_IR = 0, previousLPF_IR = 0;
float previousInputRed = 0, previousHPF_Red = 0, previousLPF_Red = 0;

void setup() {
  Wire.begin(21, 22); // ESP32 I2C pins: SDA=21, SCL=22

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MAX30102 not found!");
    display.display();
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeIR(0x24);
  particleSensor.setPulseAmplitudeRed(0x24);
}

void loop() {
  float rawIR = (float)particleSensor.getIR();
  float rawRed = (float)particleSensor.getRed();

  const float alphaHPF = 0.0245;
  const float alphaLPF = 0.239;

  // IR filter
  float hpfIR = alphaHPF * (previousHPF_IR + rawIR - previousInputIR);
  float lpfIR = alphaLPF * hpfIR + (1 - alphaLPF) * previousLPF_IR;
  previousInputIR = rawIR;
  previousHPF_IR = hpfIR;
  previousLPF_IR = lpfIR;

  // Red filter
  float hpfRed = alphaHPF * (previousHPF_Red + rawRed - previousInputRed);
  float lpfRed = alphaLPF * hpfRed + (1 - alphaLPF) * previousLPF_Red;
  previousInputRed = rawRed;
  previousHPF_Red = hpfRed;
  previousLPF_Red = lpfRed;

  bufferIR[bufferIndex] = lpfIR;
  bufferRed[bufferIndex] = lpfRed;
  bufferIndex = (bufferIndex + 1) % bufferSize;

  float sumIR = 0, sumRed = 0;
  for (int i = 0; i < bufferSize; i++) {
    sumIR += bufferIR[i];
    sumRed += bufferRed[i];
  }
  float dcIR = sumIR / bufferSize;
  float dcRed = sumRed / bufferSize;

  float minIR = bufferIR[0], maxIR = bufferIR[0];
  float minRed = bufferRed[0], maxRed = bufferRed[0];
  for (int i = 0; i < bufferSize; i++) {
    if (bufferIR[i] > maxIR) maxIR = bufferIR[i];
    if (bufferIR[i] < minIR) minIR = bufferIR[i];
    if (bufferRed[i] > maxRed) maxRed = bufferRed[i];
    if (bufferRed[i] < minRed) minRed = bufferRed[i];
  }

  float acIR = maxIR - minIR;
  float acRed = maxRed - minRed;
  float R = (acRed / dcRed) / (acIR / dcIR);
  float SpO2 = 110 - 25 * R;
  SpO2 = constrain(SpO2, 0, 100);

  float bpm = 0;
  float refractoryPeriod = timeInSeconds - lastPeakTime;
  if (lpfIR > (dcIR + acIR * 0.6) && refractoryPeriod > 0.3) {
    lastPeakTime = timeInSeconds;
    bpm = 60.0 / refractoryPeriod;
  }

  // Update OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Pulse Oximeter");
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print("SpO2: ");
  display.print(SpO2, 1);
  display.println(" %");

  display.setCursor(0, 42);
  display.print("BPM: ");
  if (bpm > 0) {
    display.print(bpm, 1);
  } else {
    display.print("--");
  }

  display.display();

  timeInSeconds += samplingInterval;
  delay(10);
}

