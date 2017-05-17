#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

const int CE_PIN = 9;
const int CSN_PIN = 10;
const byte address[5] = {'x','R','A','T','x'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
  Serial.println("Initialization successful.");
}

int data;
bool newData;

unsigned long prevMillis = 0;

void loop() {
  data = getData();
  if (newData) {
    Serial.println("Message received: ");
    Serial.print(data);
    Serial.println();
    Serial.println("Time: ");
    Serial.print(millis() - prevMillis);
    Serial.println();
    prevMillis = millis();
  }
}

int getData() {
  newData = false;
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    newData = true;
  }
  return data;
}

