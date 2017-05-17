/**
 * This program makes the motors react to the sonar input.
 * The closer an object is, the more excited the motors get (they go faster).
 */

#define ENA_PIN   5 // ENA of DC motor driver module attach to pin5 of sunfounder uno board
#define ENB_PIN   6 // ENB of DC motor driver moduleattach to pin6 of sunfounder uno board
#define IN1 8 // left IN1 attach to pin8 
#define IN2 9 // left IN2 attach to pin9
#define IN3 10 //right  IN3 attach to pin10
#define IN4 11 //right IN4 attach to pin11

#define FORWARD 0  //define forward=0,car move forward
#define BACK    1 //define back=1,car move back

const int trigPin = 3;
const int echoPin = 4;

const double c = 20; // temperature in Cº
const double soundSpeed = 331.3 + (0.6 * c); // approximate speed of sound (in m/s) at temperature c

void setup()
{
    // motor controller
    pinMode(ENA_PIN, OUTPUT);
    pinMode(ENB_PIN, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    // sonar
    pinMode(trigPin, OUTPUT); // pin to send waves
    pinMode(echoPin, INPUT); // pin to recieve echo waves
    // inits serial
    Serial.begin(9600);
}

int SPEED; // speed ranges from 120-240
long t;
double d, spd;

void loop()
{
    // resets the trig pin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // sets the trig pin to 1 for 100ms (must set for this long for the sonar to send waves)
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(100);
    digitalWrite(trigPin, LOW);
    // reads the travel time (in micro seconds) of the echo wave from the echo pin
    t = pulseIn(echoPin, HIGH);
    d = (t * soundSpeed) / 20000;
    if (d > 160) {
      //continue;
    } else {
      Serial.println(d);
      spd = 240 + d*(120-240)/30;
      CAR_move(FORWARD,spd,spd);
      //Serial.println(spd);
    }
    /*
    delay(2000); //delay 2 second
    SPEED = 130;
    CAR_move(BACK,SPEED,SPEED);
    Serial.println(SPEED);
    delay(2000);
    SPEED = 160;
    CAR_move(FORWARD,SPEED,SPEED);
    Serial.println(SPEED);
    delay(2000); //delay 2 second
    SPEED = 180;
    CAR_move(BACK,SPEED,SPEED);
    Serial.println(SPEED);
    delay(2000);
    CAR_move(BACK,0,0); //car stop
    */
}

void CAR_move(unsigned char direction, unsigned char speed_left, unsigned char speed_right)
{
    switch(direction)
    {
        //car move forward with speed 180
        case 0:
          digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);//left motor clockwise rotation
		      digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);
		      break;//right motor clockwise rotation
	      case 1:
	        digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);
		      digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);
		      break;
	      default:
	        break;
    }
    analogWrite(ENA_PIN, speed_left);//write speed_left to ENA_PIN,if speed_left is high,allow left motor rotate
    analogWrite(ENB_PIN, speed_right);//write speed_right to ENB_PIN,if speed_right is high,allow right motor rotate
}

