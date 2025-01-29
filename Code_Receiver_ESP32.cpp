#include <WiFi.h>
#define SERVER_PORT 3080


// Initialize Digital Pins
const int leftForward = 26;
const int leftBackward = 27;
const int rightForward = 33;
const int rightBackward = 25;


const char* ssid = "Aditya03";     // CHANGE TO YOUR WIFI SSID
const char* password = "abcd1234"; // CHANGE TO YOUR WIFI PASSWORD


WiFiServer TCPserver(SERVER_PORT);


void setup() {
  Serial.begin(115200);


  Serial.println("ESP32 #2: TCP SERVER");


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi");


  // Print your local IP address:
  Serial.print("ESP32 #2: TCP Server IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("ESP32 #2: -> Please update the serverAddress in ESP32 #1 code");


  // Set control pins as Output
  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightBackward, OUTPUT);


  // Start listening for a TCP client (from ESP32 #1)
  TCPserver.begin();
}


void loop() {
  // Wait for a TCP client from ESP32 #1
  WiFiClient client = TCPserver.available();
 
  if (client) {
    Serial.println("Client connected. Waiting for command...");
   
    // Block until a command is received from the TCP client:
    while (!client.available()) {
      // Do nothing and wait here until data is available
      delay(10);  // Small delay to avoid a tight loop
    }


    // Read the command once available
    char command = client.read();
    Serial.print("ESP32 #2: - Received command: ");
    Serial.println(command);


    // Process the command
    if (command == '1') {
      Serial.println("1 received"); // Turn motors on
      // Run forward
      digitalWrite(leftForward, LOW);
      digitalWrite(leftBackward, HIGH);
      digitalWrite(rightForward, LOW);
      digitalWrite(rightBackward, HIGH);
      delay(50);
    } else if (command == '0') {
      Serial.println("0 received");  // Turn motors off
      // STOP
      digitalWrite(leftForward, LOW);
      digitalWrite(leftBackward, LOW);
      digitalWrite(rightForward, LOW);
      digitalWrite(rightBackward, LOW);
      delay(50);
    }


    client.stop(); // Disconnect the client after processing
    Serial.println("Client disconnected.");
  }
}









