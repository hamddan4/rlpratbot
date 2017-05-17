#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const int CE_PIN = 9;
const int CSN_PIN = 10;

const int trigPin = 3;
const int echoPin = 4;

const double c = 20; // temperature in Cº
const double soundSpeed = 331.3 + (0.6 * c); // approximate speed of sound (in m/s) at temperature c

const byte slaveAddress[5] = {'R','x','A','A','A'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup() {
  pinMode(trigPin, OUTPUT); // pin to send waves
  pinMode(echoPin, INPUT); // pin to recieve echo waves
  Serial.begin(9600);
  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setRetries(3,5); // delay, count
  radio.openWritingPipe(slaveAddress);
}

long t;
double d;

void loop() {
    d = readSonarDistance();
    Serial.println(d);
    sendData((byte*)&d, sizeof(d));
}

double readSonarDistance() {
    // resets the trig pin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // sets the trig pin to 1 for 100ms (must set for this long for the sonar to send waves)
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(100);
    digitalWrite(trigPin, LOW);
    // reads the travel time (in micro seconds) of the echo wave from the echo pin
    t = pulseIn(echoPin, HIGH);
    return (t * soundSpeed) / 20000;
}

bool sendData(byte *dataToSend, int len) {
    bool res;
    res = radio.write(dataToSend, len);
    // Always use sizeof() as it gives the size as the number of bytes.
    // For example if dataToSend was an int sizeof() would correctly return 2
    return res;
}

