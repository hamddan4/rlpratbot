#define echoPin 3
#define trigPin 4

const double c = 20; // temperature in Cº
const double soundSpeed = 331.3 + (0.6 * c); // approximate speed of sound (in m/s) at temperature c

void setup() {
  pinMode(trigPin, OUTPUT); // pin to send waves
  pinMode(echoPin, INPUT); // pin to recieve echo waves
  Serial.begin(9600);
}

long t;
double d;

double getSonarDistance() {
  // resets the trig pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // sets the trig pin to 1 for 100ms (must set for this long for the sonar to send waves)
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);
  // reads the travel time (in micro seconds) of the echo wave from the echo pin
  return (pulseIn(echoPin, HIGH) * soundSpeed) / 20000;
}

void loop() {
  d = getSonarDistance();
  Serial.println(d);
}

