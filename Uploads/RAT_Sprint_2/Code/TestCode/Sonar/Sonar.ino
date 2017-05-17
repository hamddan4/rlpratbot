const int trigPin = 12;
const int echoPin = 13;

const double c = 20; // temperature in Cº
const double soundSpeed = 331.3 + (0.6 * c); // approximate speed of sound (in m/s) at temperature c

void setup() {
  pinMode(trigPin, OUTPUT); // pin to send waves
  pinMode(echoPin, INPUT); // pin to recieve echo waves
  Serial.begin(9600);
}

long t;
double d;

void loop() {
  // resets the trig pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // sets the trig pin to 1 for 100ms (must set for this long for the sonar to send waves)
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);
  // reads the travel time (in micro seconds) of the echo wave from the echo pin
  t = pulseIn(echoPin, HIGH);
  /* Approximate distance calculation:
      1. Travel time must be divided by 2 since the echo wave has traveled forwards, bounced on the object, then back to the sonar
      2. Time is converted from micro seconds to seconds
      3. The speed of sound is used to convert from time passed to distance traveled
      4. Distance is converted from meters to centimeters
  */
  d = (t * soundSpeed) / 20000;
  Serial.println(d);
}

