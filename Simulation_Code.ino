float timeInSeconds = 0.0; // Tracks how much time has passed since the program started
float samplingInterval = 0.01; //100 Hz sampling rate
float baseline = 500; //DC Baseline
float lastPeakTime = 0.0; //Stores the time (in seconds) when the last valid peak was detected
float previousHighPassOutput = 0.0; //y[n−1] for the high-pass filter
float previousInput = 0.0; //x[n−1]. The previous raw signal value for high-pass filter
float previousLowPassOutput = 0.0; //y[n−1] for the low-pass filter

const int bufferSize = 100;
float bufferIR[bufferSize]; //Creating a Buffer
float bufferRed[bufferSize];//Creating a Buffer
int bufferIndex = 0;

float sumIRDC = 0;
float sumRDC = 0;

float previousHighPassOutputIR = 0.0;
float previousInputIR = 0.0;
float previousLowPassOutputIR = 0.0;

float previousHighPassOutputRed = 0.0;
float previousInputRed = 0.0;
float previousLowPassOutputRed = 0.0;

float maxIRAC = 0;
float maxRedAC = 0;
float minIRAC = 0;
float minRedAC = 0;

void setup() {
  Wire.begin(21, 22);  // Use 19, 18 if that's how your wiring is set
  Serial.begin(115200);

  if (!particleSensor.begin(Wire)) {
    Serial.println("MAX30102 not found. Please check wiring.");
    while (1);
  }

  particleSensor.setup(0x1F, 4, 411, 100, 2); // Sample settings
  particleSensor.setPulseAmplitudeIR(0x24);
  particleSensor.setPulseAmplitudeRed(0x24);
}

void loop() {
  float heartRate = 75.0; // bpm
  float frequency = heartRate / 60.0;
  float cardiacPhase = fmod(timeInSeconds * frequency, 1.0);
  float refractoryPeriod = timeInSeconds - lastPeakTime;

  float ppgSignalInfrared = baseline;
  float systolicPeakIR = 100.0 * exp(-50.0 * pow(cardiacPhase - 0.1, 2));
  float diastolicPeakIR = 30.0 * exp(-40.0 * pow(cardiacPhase - 0.4, 2));
  float dicroticNotchIR = -20.0 * exp(-80.0 * pow(cardiacPhase - 0.3, 2));
  float noiseIR = random(-5, 5);
  ppgSignalInfrared += noiseIR;
  ppgSignalInfrared += systolicPeakIR + diastolicPeakIR + dicroticNotchIR;

  float ppgSignalRed = baseline;
  float systolicPeakRed = 50.0 * exp(-50.0 * pow(cardiacPhase - 0.1, 2));
  float diastolicPeakRed = 30.0 * exp(-40.0 * pow(cardiacPhase - 0.4, 2));
  float dicroticNotchRed = -20.0 * exp(-80.0 * pow(cardiacPhase - 0.3, 2));
  float noiseRed = random(-10, 10);
  ppgSignalRed += noiseRed;
  ppgSignalRed += systolicPeakRed + diastolicPeakRed + dicroticNotchRed;

  float alphaLowPass = 0.239;
  float alphaHighPass = 0.0245;

  //Filter IR Signal
  float highPassFilteredSignalIR = alphaHighPass * (previousHighPassOutputIR + ppgSignalInfrared - previousInputIR);
  float lowPassFilteredSignalIR = alphaLowPass * highPassFilteredSignalIR + (1 - alphaLowPass) * previousLowPassOutputIR;
  previousInputIR = ppgSignalInfrared;
  previousHighPassOutputIR = highPassFilteredSignalIR;
  previousLowPassOutputIR = lowPassFilteredSignalIR;

  // Filter Red Signal
  float highPassFilteredSignalRed = alphaHighPass * (previousHighPassOutputRed + ppgSignalRed - previousInputRed);
  float lowPassFilteredSignalRed = alphaLowPass * highPassFilteredSignalRed + (1 - alphaLowPass) * previousLowPassOutputRed;
  previousInputRed = ppgSignalRed;
  previousHighPassOutputRed = highPassFilteredSignalRed;
  previousLowPassOutputRed = lowPassFilteredSignalRed;

  //Calculating DC
  bufferIR[bufferIndex] = lowPassFilteredSignalIR;
  bufferRed[bufferIndex] = lowPassFilteredSignalRed;
  bufferIndex++;
  if (bufferIndex >= bufferSize) {
    bufferIndex = 0; // Wrap around (circular)
  }

  sumIRDC = 0;
  sumRDC = 0;
  for (int i = 0; i < bufferSize; i++) {
    sumIRDC += bufferIR[i];
    sumRDC += bufferRed[i];
  }
  float DcIr = sumIRDC / bufferSize;
  float DcRed = sumRDC / bufferSize;

  maxIRAC = bufferIR[0];
  minIRAC = bufferIR[0];
  maxRedAC = bufferRed[0];
  minRedAC = bufferRed[0];
  for (int i = 0; i < bufferSize; i++) {
    if (bufferIR[i] > maxIRAC) {
      maxIRAC = bufferIR[i];
    }
    if (bufferIR[i] < minIRAC) {
      minIRAC = bufferIR[i];
    }
    if (bufferRed[i] > maxRedAC) {
      maxRedAC = bufferRed[i];
    }
    if (bufferRed[i] < minRedAC) {
      minRedAC = bufferRed[i];
    }
  }

  float ACIR = maxIRAC - minIRAC;
  float ACR = maxRedAC - minRedAC;
  float ratioRed = ACR / DcRed;
  float ratioIR = ACIR / DcIr;
  float R = ratioRed / ratioIR;
  float SpO2 = 110 - 25 * R;

  if (SpO2 > 100) {
    SpO2 = 100;
  }
  if (SpO2 < 0) {
    SpO2 = 0;
  }

  if (lowPassFilteredSignalIR > 575 && refractoryPeriod > 0.3) {
    lastPeakTime = timeInSeconds;
    float bpm = 60 / refractoryPeriod;
    Serial.print("Peak detected at ");
    Serial.print(lastPeakTime);
    Serial.print(" seconds → BPM: ");
    Serial.println(bpm);
  }

  // Print simulated signal
  Serial.print(ppgSignalInfrared);
  Serial.print(",");
  Serial.print(lowPassFilteredSignalIR);
  Serial.print(",");
  Serial.print(ppgSignalRed);
  Serial.print(",");
  Serial.println(lowPassFilteredSignalRed);

  // Update time
  timeInSeconds += samplingInterval;

  delay(10); // 10 ms delay = 100 Hz sample rate
}



  
