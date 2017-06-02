/*H========================================================================
* FILENAME :        RAT.ino
*
* DESCRIPTION :
*       Main alogirthm that controls the behaviour of the RAT robot. 
* 
* AUTHORS :    RLP Group 09-03
*                 - Daniel Azemar   dani.azemar@gmail.com
*                 - Jialuo Chen     
*                 - Sergi Pous
*                 - Adria Rico
*                 
* START DATE :    Feb 2017
* 
*========================================================================H*/

#include <SimpleKalmanFilter.h>

#include "transceiver.h"//Transceiver
#include "magneto.h"    //Magnetometer
#include "sonar.h"      //Sonar
#include "motors.h"     //Motors
#include "GPS.h"        //GPS
#include "global.h"     //Global functions

//=======================================================
// When Debugging the sensors, use these parameters
// in order to print data through the serial
//=======================================================
const bool print_sonar = false;
const bool print_magneto = false;
const bool print_motors = false; //not implemented
const bool print_GPS_pos = false;
const bool print_GPS_dist = false; //this and above are the same, i think, dunno why there is two of them
const bool print_avoidance_stats = true;

bool enable_GPS = true;     //If you want to work with the GPS, activate these parameter


//=======================================================
// Global Variables to control the avoidance of an object
//=======================================================
typedef enum {CALIBRATE, BCKW,  ROTATE, FORWARD, CALIBRATE2, BACKROTATE, END} avoidance; //This are the steps of the avoidance of an object
typedef enum {ROTATING_RIGHT, ROTATING_LEFT} rotations_avoidance; //To control in which direction are we avoiding an object
boolean avoidance_started = false;      //Boolean that controls some stats
int avoidance_step = CALIBRATE;         //At witch point of the avoidance are we
float angle_calibrated = 0.0;           //Value that stores the angle when calibrated
float angle_rotated = 0.0;              //Value that stores the actual angle when rotation is done
boolean timer_set = false;              //Value that controls the setting of the timer
int actual_millis;                      //Value that stores the actual timing of arduino's clock
int timer_avoidance = 0;                //What?
int actual_rot = ROTATING_RIGHT;        //NOT IMPLEMENTED YET (When this value is changed, the rotations of avoidance change)
int diff_angle_rotated;                 //Difference of angle_calibrated and angle_rotated
const int speed_avoid = 160;            //Speed of the motors when rotating. Needs to be slower bc magnetometer has to move slowly in order to read more reliable values


int LAST_MOV = 0;                       //Deprecated variable, when you have time, delete it and test the code again


SimpleKalmanFilter simpleKalmanFilter(1, 10, 0.01); //For obtain a precise mean and approximation for sonar measures, we use Kalman Filtering

void setup() {
  Serial.begin(9600);
  setup_transceiver();
  setup_motors();
  setup_sonar();
  setup_magneto();
  if (enable_GPS) setup_GPS();
  moveRAT(STOP,0,0);
}

const int min_vel = 150;                //Max power of motor rotation
const int max_vel = 250;                //Min power of motor rotation
int spd = 200;                          //Arbitrary speed to rotation
int spdPlus = 10;                       //Arbitrary speed boost when necessary
button currentDir;                      //Actual order of the bot (see buttons in motors.h)
bool obstacle = false;                  //Boolean that determines if we are going to crash with an obstacle
bool stopped = false;                   
int angle_tol = 10; // angle error tolerance

bool automatic_for_ever = true;      // WHEN DEBUGGING AUTOMATIC MODE, THIS IS USEFUL
bool automatic = automatic_for_ever; // FALSE IF YOU DONT WANT TO START IN AUTOMATIC MODE
int dest_angle = 0;


// GPS values
double latitude_objective = 0.0;    //When automatic, an objective has to be set
double longitude_objective = 0.0;
double latitude_current = 0.0;     
double longitude_current = 0.0;

//SONAR Values 
float actual_dist = 0.0;
double distance_from_objective = 0.0;

void loop() {
  //====================================================================
  // RETRIEVE SENSORS DATA
  //====================================================================
  //-- GPS -- (latitude and longitude)
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

  //-- MAGNETOMETER -- (angle)
  int curr_angle = mod(get_yangle() + 90, 360);  
  float e_rotation_angle = get_yangle_rotating(curr_angle);
  float e_rotation_soft_angle = get_yangle_rotating_soft(curr_angle);
  
  if(print_magneto){
      Serial.print(curr_angle);
      Serial.print(" ");
      Serial.print(e_rotation_angle);
      Serial.print(" ");
  }
    
  //-- SONAR -- (distance from an object)
  float detected_dist = getSonarDistance();
  
  if(detected_dist > 3.0){ //this IF is used to correct a possible sonar malfunction when working with GPS
    actual_dist = detected_dist;
  }
  float distance_from_object = simpleKalmanFilter.updateEstimate(actual_dist);
  if (print_sonar){
    Serial.print(actual_dist);
    Serial.print(" ");
    Serial.print(distance_from_object);
    Serial.print(" ");
  }

  //========================================================================================
  //  OBJECT DETECTION AND AVOIDANCE
  //========================================================================================
  obstacle = (distance_from_object < 16) && (currentDir == FWD || currentDir == LFWD || currentDir == RFWD);
  
  if ((obstacle || avoidance_started) && automatic) {
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
        // 1. CALIBRATING MAGNETOMETER (2s wait)
        //=================
        if (not(timer_set)){
          if(print_avoidance_stats){
            Serial.print("RESETING TIMER ");
          }
          timer_avoidance = millis();
          timer_set = true;
        }
        if(print_avoidance_stats){
            Serial.print("CALIBRATING");
            Serial.print(" ");
        }
        actual_millis = millis();
    
        if ((actual_millis - timer_avoidance) < 2000){
          moveRAT(STOP, 0, 0);
          Serial.print("STOP");
          Serial.print(" ");
        } else {
          angle_calibrated = e_rotation_soft_angle;
          avoidance_step = BCKW;
          timer_set = false;
          if(print_avoidance_stats){
            Serial.print(actual_millis);
            Serial.print(" ");
            Serial.print(timer_avoidance);
            Serial.print(" ");
            Serial.print(angle_calibrated);
            Serial.print(" ");
          }
        }
        
        break;
      case BCKW:
        //=================
        // 2. Safety backwards walk
        //=================
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        if(print_avoidance_stats){
              Serial.print("BACKWARDS");
              Serial.print(" ");
         }
         
        actual_millis = millis();

        if(print_avoidance_stats){
          Serial.print(actual_millis);
          Serial.print(" ");
          Serial.print(timer_avoidance);
          Serial.print(" ");
        }
        if ((actual_millis - timer_avoidance) < 500){
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
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        
        if(actual_rot == ROTATING_RIGHT){
            moveRAT(RROT, speed_avoid, speed_avoid);
          } else{
            moveRAT(LROT, speed_avoid, speed_avoid);
        }
          
        diff_angle_rotated = mod(e_rotation_soft_angle - angle_calibrated, 360);

        actual_millis = millis();
        if((diff_angle_rotated > 80 && diff_angle_rotated < 280) && ((actual_millis - timer_avoidance) > 300)){
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
        //=================
        // 4. MOVING FORWARD
        //=================
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        actual_millis = millis();

        if(print_avoidance_stats){
          Serial.print(actual_millis);
          Serial.print(" ");
          Serial.print(timer_avoidance);
          Serial.print(" ");
        }
        if ((actual_millis - timer_avoidance) < 1000){
          moveRAT(FWD, spd, spd);
        } else {
          avoidance_step = CALIBRATE2;
          timer_set = false;
        }
        if(obstacle){
          moveRAT(STOP, 0, 0);
          //DO SOMETHING MORE BRO!
          /* Explanation for future developers:
           *    We wanted for our robot that it needed to avoid obstacles not only in one direction
           *    (as it turns right at every time) and if it finded an obstacle in this direction,
           *    change the rotation to the other side (with the variable "actual_rot").
           *    
           *    This will be useful if someones improves the code with that modification! 
           *    (Harder than you tink, it needs to check that the 90degrees rotation has been
           *    done correctly, etc etc).
          */ 
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
        // 5. CALIBRATING MAGNETOMETER (2s wait)
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

        actual_millis = millis();

        if(print_avoidance_stats){
          Serial.print(actual_millis);
          Serial.print(" ");
          Serial.print(timer_avoidance);
          Serial.print(" ");
        }
  
        if (((actual_millis - timer_avoidance) < 2000)){
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
        // 6. ROTATING 90 degrees
        //=================
        if (not(timer_set)){
          Serial.print("RESETING TIMER ");
          timer_avoidance = millis();
          timer_set = true;
        }
        if(actual_rot == ROTATING_RIGHT){
            moveRAT(LROT, speed_avoid, speed_avoid);
          } else{
            moveRAT(RROT, speed_avoid, speed_avoid);
        }

        actual_millis = millis();
        
        diff_angle_rotated = mod(e_rotation_angle - angle_calibrated, 360);
        if(diff_angle_rotated > 80 && diff_angle_rotated < 280 && ((actual_millis - timer_avoidance) > 300)){
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
        //=================
        // 7. FINISING PROCESS (Just reseting all variables in case we detect another obstacle).
        //=================
        if(print_avoidance_stats){
            Serial.print("COMPLETED AVOIDANCE");
            Serial.print(" ");
         }
        avoidance_started = false; 
        avoidance_step = CALIBRATE;
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
    
  } else if (obstacle){
    //Just STOP if we are in remote control mode
    moveRAT(STOP, 0, 0);
    if(avoidance_started){
      avoidance_started = false; 
      avoidance_step = CALIBRATE;
      angle_calibrated = 0.0;
      angle_rotated = 0.0;
      timer_set = false; 
      timer_avoidance = 0;
      //int timer_difference = 0;
      actual_rot = ROTATING_RIGHT;
    }
  }

  //===========================================================================================
  //  (NEW ORDER) DATA RETRIEVE AND ACTUALIZATION
  //===========================================================================================
  data = getData();

  if (newData) {
    
    if (data == SPUP) {
      spd += 10;
      if (spd > MAX_SPD) {
        spd = MAX_SPD;
      }
    } else if (data == SPDN) {
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
        automatic = automatic_for_ever; // FALSE IF YOU DONT WANT TO BE IN AUTOMATIC FOR EVER
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
  
  //if (automatic && get_fix() && (distance_from_objective > 3) && !avoidance_started){
  
  bool activate_autonomous = false;
  
  if(automatic_for_ever){
    activate_autonomous = (automatic && get_fix() && (distance_from_objective > 3) && !avoidance_started);
  } else {
    activate_autonomous = !avoidance_started;
  }
  
  if(activate_autonomous){ 
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
  
 //=================
 Serial.println(" ");
}

