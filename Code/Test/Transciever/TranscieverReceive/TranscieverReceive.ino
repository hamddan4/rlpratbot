#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// SimpleRx - the slave or the receiver

#include <SPI.h>

const int CE_PIN = 9;
const int CSN_PIN = 10;

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};

RF24 radio(CE_PIN, CSN_PIN);

bool newData = false;
char dataReceived[10];

//===========

void setup() {

    Serial.begin(19200);
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
}

//=============

void loop() {
    getData();
    if (newData) {
        Serial.println("Retrieving data");
        Serial.println(dataReceived);
    }
}

//==============

void getData() {
    newData = false;
    if (radio.available()) {
        radio.read(&dataReceived, sizeof(dataReceived));
        newData = true;
    }
}


