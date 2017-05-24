#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

const byte address[5] = {'x','R','A','T','x'};
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup_transceiver(){
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
}

bool newData = false;
int data;
float dataFloat;

int getData() {
  newData = false;
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    newData = true;
  }
  return data;
}

float getDataFloat() {
  while (!radio.available());
  radio.read(&dataFloat, sizeof(dataFloat));
  return dataFloat;
}

