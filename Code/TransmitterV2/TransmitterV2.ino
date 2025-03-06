#include <WiFi.h>
#define SAMPLE_RATE 500
#define BAUD_RATE 115200
#define INPUT_PIN1 32
#define INPUT_PIN2 33
#define BUFFER_SIZE 5
#define fwdThreshold 900
#define stopThreshold 500
#define fwd "forward"
#define stop "stop"
#define COOLDOWN_PERIOD 3000  

int circular_buffer[BUFFER_SIZE] = {0};
int data_index = 0, sum = 0;
const int command[] = {0,1,2,3};
/*
  0 -> stop
  1 -> forward
  2 -> left
  3 -> right
*/
bool isMoving = false;
unsigned long lastCmdTime = 0;

const char* ssid = "ABCD";     // CHANGE TO YOUR WIFI SSID
const char* password = "XYZ"; // CHANGE TO YOUR WIFI PASSWORD
const char* serverAddress = "192.168.76.11"; // CHANGE TO ESP32#2'S IP ADDRESS
const int serverPort = 3080;


WiFiClient TCPclient;


void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("ESP32: TCP CLIENT");
  // Connect to Wi-Fi
  connectToWiFi();
  // Connect to TCP server (Arduino #2)
  connectToServer();
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }


  if (!TCPclient.connected()) {
    connectToServer();
  }


  // Run timer and sampling at defined interval
  static unsigned long lastTime = 0;
  unsigned long currentTime = micros();


  if ((currentTime - lastTime) >= (1000000 / SAMPLE_RATE)) {
    lastTime = currentTime;


    double sensor1_value = analogRead(INPUT_PIN1);
    double signal1 = EMGFilter(sensor1_value);
    double abs_signal1 = abs(signal1);
    double envelop1 = getSmoothedEnvelope(abs_signal1);

    // int sensor2_value = analogRead(INPUT_PIN2);
    // int signal2 = EMGFilter(sensor2_value);
    // int abs_signal2 = abs(signal2);
    // int envelop2 = getSmoothedEnvelope(abs_signal2);


    Serial.print("Timestamp:");
    Serial.print(millis());  // Timestamp
    Serial.print(",");
    Serial.print("EMG1 sensor:");
    Serial.print(sensor1_value);  // Raw EMG
    Serial.print(",");
    Serial.print("EMG1 filtered:");
    Serial.print(abs_signal1);  // Filtered EMG
    Serial.print(",");
    Serial.print("EMG1 envelope:");
    Serial.println(envelop1);  // Smoothed Envelope
    
    // Serial.print("EMG2:");
    // Serial.print(sensor2_value);  // Raw EMG
    // Serial.print(",");
    // Serial.print(abs_signal2);  // Filtered EMG
    // Serial.print(",");
    // Serial.println(envelop2);  // Smoothed Envelope
    unsigned long now = millis();

    if (now - lastCmdTime > COOLDOWN_PERIOD) {  // Only update if cooldown expired
      if (envelop1 >= fwdThreshold && !isMoving) {
        cmdExec(command[1]);  // Forward
        isMoving = true;
        lastCmdTime = now;
      } 
      else if (envelop1 >= stopThreshold && envelop1 < fwdThreshold && isMoving) {
        cmdExec(command[0]);  // Stop
        isMoving = false;
        lastCmdTime = now;
      }
    }
    
    // if (envelop >= threshold && !prevExec) {
    //   cmdExec(true);
    //   prevExec = true;
    // } else if((envelop >= threshold) && prevExec) {
    //   cmdExec(false);
    //   prevExec = false;
    // }
  }
  delay(100);
}


// Envelop detection algorithm
int getSmoothedEnvelope(int abs_emg) {
  sum -= circular_buffer[data_index];
  sum += abs_emg;
  circular_buffer[data_index] = abs_emg;
  data_index = (data_index + 1) % BUFFER_SIZE;
  return sum / BUFFER_SIZE;
}


// Band-Pass IIR digital filter, generated using filter_gen.py
float EMGFilter(float input) {
  float output = input;
  {
    static float z1, z2;
    float x = output - 0.05159732 * z1 - 0.36347401 * z2;
    output = 0.01856301 * x + 0.03712602 * z1 + 0.01856301 * z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2;
    float x = output - -0.53945795 * z1 - 0.39764934 * z2;
    output = 1.00000000 * x + -2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2;
    float x = output - 0.47319594 * z1 - 0.70744137 * z2;
    output = 1.00000000 * x + 2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2;
    float x = output - -1.00211112 * z1 - 0.74520226 * z2;
    output = 1.00000000 * x + -2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;
  }
  return output;
}


void cmdExec(int cmd) {
    TCPclient.write(('0' + cmd));
    TCPclient.flush();
}


void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}


void connectToServer() {
  if (TCPclient.connect(serverAddress, serverPort)) {
    // Serial.println("Connected to TCP server");
  } else {
    Serial.println("Failed to connect to TCP server. Retrying...");
  }
}
