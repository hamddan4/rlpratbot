#include <Wire.h>
#include <HMC5883L.h>
HMC5883L compass;

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

