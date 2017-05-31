#include <SimpleKalmanFilter.h>

#include "transceiver.h"//Transceiver
#include "magneto.h"    //Magnetometer
#include "sonar.h"      //Sonar
#include "motors.h"     //Motors
#include "GPS.h"        //GPS
#include "global.h"     //Global functions

SimpleKalmanFilter simpleKalmanFilter(1, 10, 0.01);

const bool print_sonar = false;
const bool print_magneto = false; //not implemented
const bool print_motors = false; //not implemented
const bool print_GPS_pos = false;
const bool print_GPS_dist = false;
const bool print_avoidance_stats = true;

bool enable_GPS = true;

typedef enum {CALIBRATE, BCKW,  ROTATE, FORWARD, CALIBRATE2, BACKROTATE, END} avoidance;
typedef enum {ROTATING_RIGHT, ROTATING_LEFT} rotations_avoidance;
boolean avoidance_started = false; 
int avoidance_step = 0;
float angle_calibrated = 0.0;
float angle_rotated = 0.0;
boolean timer_set = false; 
int timer_avoidance = 0;
//int timer_difference = 0;
int actual_rot = ROTATING_RIGHT; //NOT IMPLEMENTED YET
int diff_angle_rotated;
const int speed_avoid = 160;

int LAST_MOV = 0;
void setup() {
  Serial.begin(9600);
  
  setup_transceiver();

  setup_motors();
  
  setup_sonar();

  setup_magneto();

  if (enable_GPS) setup_GPS();

  moveRAT(STOP,0,0);
}

const int min_vel = 150;
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
  //RETRIEVE GPS
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

  //RETIEVE ANGLE
  int curr_angle = mod(get_yangle() + 90, 360);  
  float e_rotation_angle = get_yangle_rotating(curr_angle);
  float e_rotation_soft_angle = get_yangle_rotating_soft(curr_angle);
  
  if(print_magneto){
      Serial.print(curr_angle);
      Serial.print(" ");
      Serial.print(e_rotation_angle);
      Serial.print(" ");
  }
    
  //RETRIEVE DISTANCE
  float detected_dist = getSonarDistance();
  
  if(detected_dist > 3.0){
    actual_dist = detected_dist;
  }
  
  float distance_from_object = simpleKalmanFilter.updateEstimate(actual_dist);
  
  if (print_sonar){
    Serial.print(actual_dist);
    Serial.print(" ");
    Serial.print(distance_from_object);
    Serial.print(" ");
  }

  //=========================================================================================================
  //  OBJECT DETECTION AND AVOIDANCE
  //=========================================================================================================
  
  obstacle = (distance_from_object < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);

  if (obstacle || avoidance_started) {
    if(print_avoidance_stats){
            Serial.print("OBJECT");
            Serial.print(" ");
    }
    
    if (not(avoidance_started)){
      avoidance_step = CALIBRATE;
      avoidance_started = true;
    }
    switch(avoidance_step){
      case CALIBRATE:
        //=================
        // 1. CALIBRATING MAGNETOMETER (1s wait)
        //=================
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        if(print_avoidance_stats){
            Serial.print("CALIBRATING");
            Serial.print(" ");
        }
  
        if (((millis() - timer_avoidance) < 1000)){
          moveRAT(STOP, 0, 0);
          Serial.print("STOP");
          Serial.print(" ");
        } else {
          angle_calibrated = e_rotation_angle;
          avoidance_step = BCKW;
          timer_set = false;
          if(print_avoidance_stats){
            Serial.print(angle_calibrated);
            Serial.print(" ");
          }
        }
        
        break;
        Serial.print("STOP THER EYOU MONSTER");
      case BCKW:
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        if(print_avoidance_stats){
              Serial.print("BACKWARDS");
              Serial.print(" ");
         }
        //=================
        // 2. Safety backwards walk
        //=================
        if ((millis() - timer_avoidance) < 500){
          moveRAT(BCK, spd, spd);
        } else {
          avoidance_step = ROTATE;
          timer_set = false;
          if(print_avoidance_stats){
            Serial.print("DONE");
            Serial.print(" ");
          }
        }
        break;
      case ROTATE:
        //=================
        // 3. ROTATING 90 degrees
        //=================
        if(actual_rot == ROTATING_RIGHT){
            moveRAT(RROT, speed_avoid, speed_avoid);
          } else{
            moveRAT(LROT, speed_avoid, speed_avoid);
        }
          
        diff_angle_rotated = mod(e_rotation_angle - angle_calibrated, 360);
        if(diff_angle_rotated > 80 && diff_angle_rotated < 280){
          avoidance_step = FORWARD;
          timer_set = false;
        }
        if(print_avoidance_stats){
            Serial.print("ROTATE");
            Serial.print(" ");
            Serial.print(diff_angle_rotated);
            Serial.print(" ");
         }
        break;
      case FORWARD:
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        //=================
        // 4. MOVING FORWARD
        //=================
        if ((millis() - timer_avoidance) < 1000 && not(obstacle)){
          moveRAT(FWD, spd, spd);
        } else {
          avoidance_step = CALIBRATE2;
          timer_set = false;
        }
        if(obstacle){
          moveRAT(STOP, 0, 0);
          //DO SOMETHING MORE BRO!
        }
        if(print_avoidance_stats){
            Serial.print("FORWARD");
            Serial.print(" obsacle? -> ");
            Serial.print(obstacle);
            Serial.print(" ");
         }
        break;
      case CALIBRATE2:
         //=================
        // 4.1 CALIBRATING MAGNETOMETER (1s wait)
        //=================
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        if(print_avoidance_stats){
            Serial.print("CALIBRATING2");
            Serial.print(" ");
        }
  
        if (((millis() - timer_avoidance) < 1000)){
          moveRAT(STOP, 0, 0);
          Serial.print("STOP");
          Serial.print(" ");
        } else {
          angle_calibrated = e_rotation_angle;
          avoidance_step = BACKROTATE;
          timer_set = false;
          if(print_avoidance_stats){
            Serial.print(angle_calibrated);
            Serial.print(" ");
          }
        }
        break;
      case BACKROTATE:
        //=================
        // 3. ROTATING 90 degrees
        //=================
        if(actual_rot == ROTATING_RIGHT){
            moveRAT(LROT, speed_avoid, speed_avoid);
          } else{
            moveRAT(RROT, speed_avoid, speed_avoid);
        }
          
        diff_angle_rotated = mod(e_rotation_angle - angle_calibrated, 360);
        if(diff_angle_rotated > 80 && diff_angle_rotated < 280){
          avoidance_step = END;
          timer_set = false;
        }
        if(print_avoidance_stats){
            Serial.print("BACKROTATE");
            Serial.print(" ");
            Serial.print(diff_angle_rotated);
            Serial.print(" ");
         }
        break;
      case END:
        if(print_avoidance_stats){
            Serial.print("COMPLETED AVOIDANCE");
            Serial.print(" ");
         }
        //=================
        // 2. FINISING PROCESS
        //=================
        avoidance_started = false; 
        avoidance_step = 0;
        angle_calibrated = 0.0;
        angle_rotated = 0.0;
        timer_set = false; 
        timer_avoidance = 0;
        //int timer_difference = 0;
        actual_rot = ROTATING_RIGHT;
        break;
      default: 
        break;
  }
    //moveRAT(STOP, 0, 0);
  }

  //=========================================================================================================
  //  DATA RETRIEVE AND ACTUALIZATION
  //=========================================================================================================
  
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
    } else { 
      avoidance_started = false;
      if (data == LOCATION) {
        avoidance_started = false;
        automatic = true;
        latitude_objective = getDataFloat();
        longitude_objective = getDataFloat();
      } else {
        automatic = false;
        currentDir = (button)data;
      }
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

  //=========================================================================================================
  //  AUTOMATION OF THE ROBOT
  //=========================================================================================================
  
  if (automatic && get_fix() && (distance_from_objective > 3) && !avoidance_started){
    float angle = atan2(longitude_objective-longitude_current, latitude_objective-latitude_current);
    dest_angle = angle * 180/PI;
    dest_angle = mod(dest_angle, 360);
    
    int diff = dest_angle - e_rotation_angle;
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
        LAST_MOV = RROT;
        moveRAT(LROT, rotationSpeed, rotationSpeed);
      } else {
        //Serial.println("ESQUERRA");
        LAST_MOV = LROT;
        moveRAT(RROT, rotationSpeed, rotationSpeed);
      }
    } else {
      LAST_MOV = FWD;
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

