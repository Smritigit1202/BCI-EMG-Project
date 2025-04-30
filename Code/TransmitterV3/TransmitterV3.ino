#include <esp_now.h>
#include <WiFi.h>

#define SAMPLE_RATE 500
#define BAUD_RATE 115200
#define INPUT_PIN1 34
#define INPUT_PIN2 35
#define BUFFER_SIZE 4
#define COOLDOWN_PERIOD 2000
#define CALIBRATION_SAMPLES 100

int fwdThreshold = 300;
int stopThreshold = 150;
int leftThreshold = 300;
int rightThreshold = 150;

bool input_prev = false;
int env = 0;
bool trackingEnv1 = false;
bool trackingEnv2 = false;
double env1MAX = 0;
double env2MAX = 0;
double lastenv1 = 0;
double lastenv2 = 0;

int circular_buffer[BUFFER_SIZE] = {0};
int data_index = 0, sum = 0;
const int command[] = {0, 1, 2, 3};
/*
  0 -> stop
  1 -> forward
  2 -> left
  3 -> right
*/
bool isMoving = false;
bool isMoving2 = false;
unsigned long lastCmdTime = 0;

// MAC address of the ESP32 receiver
uint8_t receiverMAC[] = {0xD0,0xEF,0x76,0x44,0xCE,0xC4};

void setup() {
  Serial.begin(BAUD_RATE);
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("ESP-NOW sender");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (!esp_now_add_peer(&peerInfo)) {
    Serial.println("Peer added successfully");
    cmdExec(command[0]);
  } else {
    Serial.println("Failed to add peer");
  }
  delay(1000);
}

void loop() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = micros();

  static double sensor1_value = 0, signal1 = 0, abs_signal1 = 0, envelop1 = 0;
  static double sensor2_value = 0, signal2 = 0, abs_signal2 = 0, envelop2 = 0;

  static bool trackingPeak = false;

  if ((currentTime - lastTime) >= (1000000 / SAMPLE_RATE)) {
    lastTime = currentTime;

    if (input_prev) {
      sensor1_value = analogRead(INPUT_PIN1);
      signal1 = EMGFilter(sensor1_value);
      abs_signal1 = abs(signal1);
      envelop1 = getSmoothedEnvelope(abs_signal1);
    } else {
      sensor2_value = analogRead(INPUT_PIN2);
      signal2 = EMGFilter(sensor2_value);
      abs_signal2 = abs(signal2);
      envelop2 = getSmoothedEnvelope(abs_signal2);
    }

    input_prev = !input_prev;

    Serial.print("EMG1_envelope:"); Serial.print(envelop1); Serial.print(",");
    Serial.print("EMG2_envelope:"); Serial.print(envelop2); Serial.print(",");

    unsigned long now = millis();

    // Start tracking if either envelop crosses its small threshold
    if (!trackingPeak && (envelop1 >= stopThreshold || envelop2 >= rightThreshold)) {
      trackingPeak = true;
      env1MAX = envelop1;
      env2MAX = envelop2;
    }

    if (trackingPeak) {
      if (envelop1 > env1MAX) env1MAX = envelop1;
      if (envelop2 > env2MAX) env2MAX = envelop2;

      // New Fix: Only dominant envelope fall matters
      bool dominant1 = (env1MAX > env2MAX);
      bool dominant_falling = dominant1 ? (envelop1 < lastenv1) : (envelop2 < lastenv2);

      if (dominant_falling) {
        // PEAK COMPLETE
        trackingPeak = false;

        if (now - lastCmdTime > COOLDOWN_PERIOD) {
          // Find which signal had higher peak
          if (env1MAX > env2MAX) {
            // EMG1 was dominant
            if (env1MAX >= fwdThreshold && !isMoving) {
              cmdExec(command[1]);  // Forward
              isMoving = true;
              isMoving2 = false;
              lastCmdTime = now;
            } else if (env1MAX >= stopThreshold && env1MAX < fwdThreshold && isMoving) {
              cmdExec(command[0]);  // Stop
              isMoving = false;
              isMoving2 = false;
              lastCmdTime = now;
            }
          } else if (env2MAX > env1MAX) {
            // EMG2 was dominant
            if (env2MAX >= leftThreshold /*&& !isMoving2*/) {
              cmdExec(command[2]);  // Left
              isMoving = false;
              isMoving2 = true;
              lastCmdTime = now;
            } else if (env2MAX >= rightThreshold && env2MAX < leftThreshold /*&& isMoving2*/) {
              cmdExec(command[3]);  // Right
              isMoving = false;
              isMoving2 = false;
              lastCmdTime = now;
            }
          }
          // Reset peak values
          env1MAX = 0;
          env2MAX = 0;
        }
      }
    }

    lastenv1 = envelop1;
    lastenv2 = envelop2;

    Serial.print("trackingPeak_status:"); Serial.println(trackingPeak);
  }

  delay(50);
}


void calibrate() {
  Serial.println("Calibration started");
  delay(2000);
  double max_val= 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    double sensor_val = analogRead(INPUT_PIN1);
    double envelop_val = getSmoothedEnvelope(abs(EMGFilter(sensor_val)));
    if(envelop_val > max_val){
      max_val = envelop_val;
    }
    delay(100);
  }
  fwdThreshold = max_val - 300;   
  stopThreshold = fwdThreshold/2;  
  leftThreshold = fwdThreshold;
  rightThreshold = stopThreshold;
  Serial.println("Calibration ended");
  Serial.print("Forward Threshold: "); Serial.println(fwdThreshold);
  Serial.print("Stop Threshold: "); Serial.println(stopThreshold);
  Serial.print("Left Threshold: "); Serial.println(leftThreshold);
  Serial.print("Right Threshold: "); Serial.println(rightThreshold);
  delay(3000);
}

int getSmoothedEnvelope(int abs_emg) {
  sum -= circular_buffer[data_index];
  sum += abs_emg;
  circular_buffer[data_index] = abs_emg;
  data_index = (data_index + 1) % BUFFER_SIZE;
  return sum / BUFFER_SIZE;
}

float EMGFilter(float input) {
  float output = input;
  static float z1a, z2a, z1b, z2b, z1c, z2c, z1d, z2d;

  float x = output - 0.05159732 * z1a - 0.36347401 * z2a;
  output = 0.01856301 * x + 0.03712602 * z1a + 0.01856301 * z2a;
  z2a = z1a; z1a = x;

  x = output - -0.53945795 * z1b - 0.39764934 * z2b;
  output = 1.00000000 * x + -2.00000000 * z1b + 1.00000000 * z2b;
  z2b = z1b; z1b = x;

  x = output - 0.47319594 * z1c - 0.70744137 * z2c;
  output = 1.00000000 * x + 2.00000000 * z1c + 1.00000000 * z2c;
  z2c = z1c; z1c = x;

  x = output - -1.00211112 * z1d - 0.74520226 * z2d;
  output = 1.00000000 * x + -2.00000000 * z1d + 1.00000000 * z2d;
  z2d = z1d; z1d = x;

  return output;
}

void cmdExec(int cmd) {
  esp_now_send(receiverMAC, (uint8_t *)&cmd, sizeof(cmd));
}