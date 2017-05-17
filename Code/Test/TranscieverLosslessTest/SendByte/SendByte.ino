#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

const int CE_PIN = 9;
const int CSN_PIN = 10;
const byte slaveAddress[5] = {'x','R','A','T','x'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3, 15);
  radio.openWritingPipe(slaveAddress);
  Serial.println("Initialization successful.");
}

int data = 0;

void loop() {
  while (!sendData());
  data += 1;
  delay(1000);
}

bool sendData() {
  Serial.println("Message sent");
  return radio.write(&data, sizeof(data));
}

