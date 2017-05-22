#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <SimpleKalmanFilter.h>

#include "magneto.h"  //Magnetometer
#include "sonar.h"    //Sonar
#include "motors.h"   //Motors

#define CE_PIN 9
#define CSN_PIN 10

const byte address[5] = {'x','R','A','T','x'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

SimpleKalmanFilter simpleKalmanFilter(20, 2, 0.01);


int mod(int x, int n) {
  return (x < 0) ? ((x % n) + n) else (x % n);
}

void setup() {
  Serial.begin(9600);
  
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();

  setup_motors();
  
  setup_sonar();

  setup_magneto();

  moveRAT(STOP,0,0);
}

bool newData = false;
int data;

int getData() {
  newData = false;
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    Serial.print(data);
    newData = true;
  }
  return data;
}


int spd = 150;
int spdPlus = 10;
button currentDir;
bool obstacle = false;
bool stopped = false;
float angle_tol = 5*PI/180; // angle error tolerance

void loop() {
  float real_dist = getSonarDistance();
  float estimated_value = simpleKalmanFilter.updateEstimate(real_dist);
  
  Serial.print(real_dist);
  Serial.print(" ");
  Serial.println(estimated_value);
  
  obstacle = (estimated_value < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);
  
  if (!stopped && obstacle) {
    Serial.println("I'm stoppin.");
    moveRAT(STOP, 0, 0);
    stopped = true;
  }
  data = getData();
  float curr_angle = get_yangle();
  float dest_angle = 0;
  //Serial.println(curr_angle);
  diff = dest_angle - curr_angle;
  if (abs(diff) < 0.5) {
    dir = diff >= 0;
  } else {
    dir = diff < 0;
  }
  if (abs(diff) > angle_tol) {
    if (dir) {
      moveRAT(RROT, spd, spd);
    } else {
      moveRAT(LROT, spd, spd);
    }
  }
 
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
// Serial.print(orientation.x);
// Serial.print(" ");
// Serial.print(orientation.y);
// Serial.print(" ");
// Serial.print(orientation.z);
// Serial.println(" ");
}

