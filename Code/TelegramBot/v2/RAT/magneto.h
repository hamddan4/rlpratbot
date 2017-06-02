/*H========================================================================
* FILENAME :        magneto.h
*
* DESCRIPTION :
*       All functions necessary for retrieve data from the magnetometer.
* 
* AUTHORS :    RLP Group 09-03
*                 - Daniel Azemar   dani.azemar@gmail.com
*                 - Jialuo Chen     
*                 - Sergi Pous
*                 - Adria Rico
*                 
* START DATE :    May 2017
* 
*========================================================================H*/

#include <Wire.h>
#include <HMC5883L.h>
HMC5883L compass;


SimpleKalmanFilter rotatingKalman(1, 10, 0.01); //When rotating, kalman has to be softer
SimpleKalmanFilter hardKalman(2, 10, 0.01);     //Sometimes is useful to have more filtrated data

const double TAU = 2*PI;

struct AXIS {
  float x;
  float y;
  float z;
};

void setup_magneto()
{
 Wire.begin();
 compass = HMC5883L();
 compass.SetScale(1.3);
 compass.SetMeasurementMode(Measurement_Continuous);
}

AXIS get_axis()
{
 MagnetometerScaled scaled = compass.ReadScaledAxis();
 float xAngle = atan2(scaled.YAxis, scaled.XAxis);
 float yAngle = atan2(scaled.ZAxis, scaled.XAxis);
 float zAngle = atan2(scaled.ZAxis, scaled.YAxis);
 AXIS orientation = {xAngle+PI, yAngle+PI, zAngle+PI};

 return orientation;
}

int get_yangle() {
 MagnetometerScaled scaled = compass.ReadScaledAxis();
 float yAngle = atan2(scaled.ZAxis, scaled.XAxis);

 return int(yAngle*(180/PI));
}

int get_yangle_rotating(int yangle){
  return rotatingKalman.updateEstimate(yangle);
}

int get_yangle_rotating_soft(int yangle){
  //More kalman means more hard filtering, that in return means more soft pics of data
  return hardKalman.updateEstimate(yangle);
}

