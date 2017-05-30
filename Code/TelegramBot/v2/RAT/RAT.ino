#include <SimpleKalmanFilter.h>

#include "transceiver.h"//Transceiver
#include "magneto.h"    //Magnetometer
#include "sonar.h"      //Sonar
#include "motors.h"     //Motors
#include "GPS.h"        //GPS
#include "global.h"     //Global functions

SimpleKalmanFilter simpleKalmanFilter(2, 4, 0.1);

const bool print_sonar = true;
const bool print_magneto = false; //not implemented
const bool print_motors = false; //not implemented
const bool print_GPS_pos = false;
const bool print_GPS_dist = false;

bool enable_GPS = true;

void setup() {
  Serial.begin(9600);
  
  setup_transceiver();

  setup_motors();
  
  setup_sonar();

  setup_magneto();

  if (enable_GPS) setup_GPS();

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

double latitude_objective = 0.0;
double longitude_objective = 0.0;

double latitude_current = 0.0;
double longitude_current = 0.0;

float actual_dist = 0.0;

double distance_from_objective = 0.0;
void loop() {

  if(enable_GPS){
    GPS_loop();
    latitude_current = get_latitude();
    longitude_current = get_longitude();
  }
  
  if (print_GPS_dist || print_GPS_pos){
    if(get_fix()){
      if(print_GPS_pos){
        Serial.print(latitude_current, 8);
        Serial.print(" ");
        Serial.print(longitude_current, 8);
        Serial.print(" ");
      }
      distance_from_objective = calc_dist(latitude_current,longitude_current,latitude_objective,longitude_objective);
      if(print_GPS_dist){
        Serial.print(distance_from_objective, 8);
        Serial.print(" ");
      }
    } else {
      Serial.print("Fixing position");
      Serial.print(" ");
    }
    
  }
  
  float detected_dist = getSonarDistance();
  
  if(detected_dist > 3.0){
    actual_dist = detected_dist;
  }
  
  float estimated_dist = simpleKalmanFilter.updateEstimate(actual_dist);
  
  if (print_sonar){
    Serial.print(actual_dist);
    Serial.print(" ");
    Serial.print(estimated_dist);
    Serial.print(" ");
  }
  
  obstacle = false;
  //(estimated_dist < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);
  
  if (obstacle) {
    moveRAT(STOP, 0, 0);
  }
  data = getData();

  if (newData) {
    if (data == SPUP) {
      spd += 10;
      if (spd > MAX_SPD) {
        spd = MAX_SPD;
      }
    } else
    if (data == SPDN) {
      spd -= 10;
      if (spd < MIN_SPD) {
        spd = MIN_SPD;
      }
    } else if (data == LOCATION) {
       
      automatic = true;
      latitude_objective = getDataFloat();
      longitude_objective = getDataFloat();
    
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

  if (automatic && get_fix() && (distance_from_objective > 3)){
    float angle = atan2(longitude_objective-longitude_current, latitude_objective-latitude_current);
    dest_angle = angle * 180/PI;
    dest_angle = mod(dest_angle, 360);
      
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
    }
    //Serial.println(mod(diff, 360));
    float rotationSpeed = min_vel + dist*(max_vel-min_vel)/180;
    //Serial.println(rotationSpeed);
//    Serial.print(curr_angle);
//    Serial.print(" ");
//    Serial.print(dest_angle);
//    Serial.print(" ");
//    Serial.println(dist);

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
 Serial.println(" ");
}

