#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <SimpleKalmanFilter.h>

#define CE_PIN 9
#define CSN_PIN 10

const byte address[5] = {'x','R','A','T','x'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

typedef enum {LFWD, FWD, RFWD, STOP, LBCK, BCK, RBCK, SPDN, SPUP} button;

#define ENA_PIN 5 // ENA of DC motor driver module attach to pin5 of sunfounder uno board
#define ENB_PIN 6 // ENB of DC motor driver moduleattach to pin6 of sunfounder uno board
#define IN1 14 // left IN1 attach to pin8
#define IN2 15 // left IN2 attach to pin9
#define IN3 16 //right  IN3 attach to pin10
#define IN4 17 //right IN4 attach to pin11

#define MIN_SPD 80
#define MAX_SPD 250

#define echoPin 3
#define trigPin 4

SimpleKalmanFilter simpleKalmanFilter(10, 10, 0.01);
// Serial output refresh time
const long SERIAL_REFRESH_TIME = 100;
long refresh_time;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  pinMode(trigPin, OUTPUT); // pin to send waves
  pinMode(echoPin, INPUT); // pin to recieve echo waves
  moveRAT(STOP,0,0);
}

bool newData = false;
int data;

int getData() {
  newData = false;
  Serial.print("hika");
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    Serial.print(data);
    newData = true;
  }
  return data;
}

void moveRAT(button direction, unsigned char speed_left, unsigned char speed_right)
{
    switch(direction)
    {
        case FWD:  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH); break;
        case LFWD: digitalWrite(IN1,HIGH);  digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH); break;
        case RFWD: digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,HIGH);  digitalWrite(IN4,HIGH); break;
        case BCK:  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
                   digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW); break;
        case LBCK: digitalWrite(IN1,HIGH);  digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW); break;
        case RBCK: digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
                   digitalWrite(IN3,HIGH);  digitalWrite(IN4,HIGH); break;
        case STOP:
        default:   digitalWrite(IN1,HIGH);  digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,HIGH);  digitalWrite(IN4,HIGH); break;
         
    }
    analogWrite(ENA_PIN,speed_left);//write speed_left to ENA_PIN,if speed_left is high,allow left motor rotate
    analogWrite(ENB_PIN,speed_right);//write speed_right to ENB_PIN,if speed_right is high,allow right motor rotate
}

const double c = 20; // temperature in Cº
const double soundSpeed = 331.3 + (0.6 * c); // approximate speed of sound (in m/s) at temperature c

double getSonarDistance() {
  // resets the trig pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // sets the trig pin to 1 for 100ms (must set for this long for the sonar to send waves)
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);
  // reads the travel time (in micro seconds) of the echo wave from the echo pin
  float distance = (pulseIn(echoPin, HIGH) * soundSpeed) / 20000;
  return distance;
}

int spd = 150;
int spdPlus = 10;
button currentDir;
bool obstacle = false;
bool stopped = false;

void loop() {
  float real_dist = getSonarDistance();
  float estimated_value = simpleKalmanFilter.updateEstimate(real_dist);
  
  obstacle = (estimated_value < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);
  if (!stopped && obstacle) {
    Serial.println("I'm stoppin.");
    moveRAT(STOP, 0, 0);
    stopped = true;
  }
  data = getData();
  if (newData) {
      if (data == SPUP) {
        spd += 10;
        if (spd > MAX_SPD) {
          spd = MAX_SPD;
        }
        Serial.println(spd);
      } else
      if (data == SPDN) {
        spd -= 10;
        if (spd < MIN_SPD) {
          spd = MIN_SPD;
        }
        Serial.println(spd);
      } else {
        currentDir = (button)data;
      }
      if (currentDir == FWD || currentDir == LFWD || currentDir == RFWD) {
        if (!obstacle) {
          moveRAT(currentDir, spd, spd);
          stopped = false;
        }
      } else {
        Serial.println("I'm movin backwards.");
        moveRAT(currentDir, spd, spd);
        stopped = false;
      }
  }
}
