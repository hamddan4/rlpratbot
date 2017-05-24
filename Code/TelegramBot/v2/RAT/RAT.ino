

#include <SimpleKalmanFilter.h>

#include "transceiver.h"//Transceiver
#include "magneto.h"  //Magnetometer
#include "sonar.h"    //Sonar
#include "motors.h"   //Motors
#include "global.h"

SimpleKalmanFilter simpleKalmanFilter(20, 2, 0.01);

void setup() {
  Serial.begin(9600);
  
  setup_transceiver();

  setup_motors();
  
  setup_sonar();

  setup_magneto();

  moveRAT(STOP,0,0);
}

const int min_vel = 120;
const int max_vel = 250;
int spd = 200;
int spdPlus = 10;
button currentDir;
bool obstacle = false;
bool stopped = false;
int angle_tol = 10; // angle error tolerance

float latitude;
float longitude;

bool automatic = false;
int dest_angle = 0;

void loop() {
  float real_dist = getSonarDistance();
  float estimated_value = simpleKalmanFilter.updateEstimate(real_dist);
  
//  Serial.print(real_dist);
//  Serial.print(" ");
//  Serial.println(estimated_value);
  
  obstacle = (real_dist < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);
  
  if (obstacle) {
    Serial.println("I'm stoppin.");
    moveRAT(STOP, 0, 0);
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
    } else if (data == LOCATION) {
      automatic = true;
      float latitude_objective = getDataFloat();
      float longitude_objective = getDataFloat();
//      Serial.println("LOCATION");
//      Serial.print(latitude);
//      Serial.print(" ");
//      Serial.println(longitude);
      float latitude_current = 41.500450;
      float longitude_current = 2.111220;
      float angle = atan2(longitude_objective-longitude_current, latitude_objective-latitude_current);
      dest_angle = angle * 180/PI;
      dest_angle = mod(dest_angle, 360);
    } else {
      automatic = false;
      currentDir = (button)data;
    }
    if(!automatic){
      if (currentDir == FWD || currentDir == LFWD || currentDir == RFWD) {
        if (!obstacle) {
          moveRAT(currentDir, spd, spd);
        }
      } else {
        moveRAT(currentDir, spd, spd);
      }
    }
  }

  if (automatic){
    //TO THE NORTH
    int curr_angle = mod(get_yangle() + 90, 360);  
    int diff = dest_angle - curr_angle;
    int dir;
    int dist;
    if (abs(diff) < 180) {
      dir = sign(diff);
      dist = abs(diff);
    } else {
      dir = -sign(diff);
      dist = 360 - abs(diff);
      Serial.println("Bigger than 180");
    }
    //Serial.println(mod(diff, 360));
    float rotationSpeed = min_vel + dist*(max_vel-min_vel)/180;
    //Serial.println(rotationSpeed);
    Serial.print(curr_angle);
    Serial.print(" ");
    Serial.print(dest_angle);
    Serial.print(" ");
    Serial.println(dist);
    if (dist > angle_tol) {
      //Serial.print("Entro ");
      if (dir == -1) {
        //Serial.println("DRETA");
        moveRAT(LROT, rotationSpeed, rotationSpeed);
      } else {
        //Serial.println("ESQUERRA");
        moveRAT(RROT, rotationSpeed, rotationSpeed);
      }
    } else {
      if (!obstacle) {
        currentDir = FWD;
        moveRAT(FWD,spd,spd);
      }
    }
  }

  /*if (abs(dest_angle - curr_angle) > angle_tol) {
    moveRAT(RROT, spd, spd);
  } else {
    moveRAT(STOP, 0, 0);
  }*/
  
 //=================
 
  
// Serial.print(orientation.x);
// Serial.print(" ");
// Serial.print(orientation.y);
// Serial.print(" ");
// Serial.print(orientation.z);
// Serial.println(" ");
}

