#include <WiFi.h>
#include <esp_now.h>

// Motor Pins
const int leftForward = 26;
const int leftBackward = 27;
const int rightForward = 33;
const int rightBackward = 25;

// Received data
int command = -1;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 #2 (Receiver): Using ESP-NOW");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(onDataReceive);

  // Motor pins as outputs
  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightBackward, OUTPUT);
}

void loop() {
  if (command != -1) {
    Serial.print("Received Command: ");
    Serial.println(command);

    switch (command) {
      case 0:
        Serial.println("Stopping");
        digitalWrite(leftForward, LOW);
        digitalWrite(leftBackward, LOW);
        digitalWrite(rightForward, LOW);
        digitalWrite(rightBackward, LOW);
        break;
      case 1:
        Serial.println("Moving Forward");
        digitalWrite(leftForward, LOW);
        digitalWrite(leftBackward, HIGH);
        digitalWrite(rightForward, LOW);
        digitalWrite(rightBackward, HIGH);
        break;
      case 2:
        Serial.println("Turning Left");
        digitalWrite(leftForward, LOW);
        digitalWrite(leftBackward, HIGH);
        digitalWrite(rightForward, HIGH);
        digitalWrite(rightBackward, LOW);
        delay(400);
        // Stop after executing
        digitalWrite(leftForward, LOW);
        digitalWrite(leftBackward, LOW);
        digitalWrite(rightForward, LOW);
        digitalWrite(rightBackward, LOW);
        break;
      case 3:
        Serial.println("Turning Right");
        digitalWrite(leftForward, HIGH);
        digitalWrite(leftBackward, LOW);
        digitalWrite(rightForward, LOW);
        digitalWrite(rightBackward, HIGH);
        delay(400);
        // Stop after executing
        digitalWrite(leftForward, LOW);
        digitalWrite(leftBackward, LOW);
        digitalWrite(rightForward, LOW);
        digitalWrite(rightBackward, LOW);
        break;
      default:
        Serial.println("Unknown Command");
        break;
    }

    command = -1;  // Reset command
  }
  delay(50);
}

void onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&command, incomingData, sizeof(command));
}