#include <WiFi.h>
#define SAMPLE_RATE 500
#define BAUD_RATE 115200
#define INPUT_PIN 39
#define BUFFER_SIZE 5
#define threshold 1000
int circular_buffer[BUFFER_SIZE] = {0};
int data_index = 0, sum = 0;
bool command = false;
bool prevExec = false;


const char* ssid = "Aditya03";     // CHANGE TO YOUR WIFI SSID
const char* password = "abcd1234"; // CHANGE TO YOUR WIFI PASSWORD
const char* serverAddress = "192.168.121.11"; // CHANGE TO ESP32#2'S IP ADDRESS
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


    int sensor_value = analogRead(INPUT_PIN);
    int signal = EMGFilter(sensor_value);
    int abs_signal = abs(signal);
    int envelop = getSmoothedEnvelope(abs_signal);
    // Serial.print("EMG Signal ");
    Serial.print(signal);
    Serial.print(" , ");
    Serial.println(envelop);


    if (envelop >= threshold && !prevExec) {
      cmdExec(true);
      prevExec = true;
    } else if((envelop >= threshold) && prevExec) {
      cmdExec(false);
      prevExec = false;
    }
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


void cmdExec(bool command) {
  if (command) {
    TCPclient.write('1');
    TCPclient.flush();
    // Serial.println("- sent command: 1");
  } else {
    TCPclient.write('0');
    TCPclient.flush();
    // Serial.println("- sent command: 0");
  }
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
    Serial.println("Connected to TCP server");
  } else {
    Serial.println("Failed to connect to TCP server. Retrying...");
  }
}
