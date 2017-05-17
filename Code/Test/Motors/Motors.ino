#include <Servo.h>

Servo myservo;//create a object of servo,named as myservo

#define ENA_PIN   4 // ENA of DC motor driver module attach to pin5 of sunfounder uno board
#define ENB_PIN   5 // ENB of DC motor driver moduleattach to pin6 of sunfounder uno board
#define IN1 3 // left IN1 attach to pin8 
#define IN2 2 // left IN2 attach to pin9
#define IN3 1 //right  IN3 attach to pin10
#define IN4 0 //right IN4 attach to pin11

#define FORWARD 0  //define forward=0,car move forward
#define BACK    1 //define back=1,car move back
#define SPEED 200 //define SPEED=180,it is the rotate speed of motor
void setup()
{
    /* set below pins as OUTPUT */
    pinMode(ENA_PIN, OUTPUT);
    pinMode(ENB_PIN, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    CAR_move(FORWARD,SPEED,SPEED);//car move forward with speed 180
    delay(2000);//delay 2 second
    CAR_move(BACK,SPEED,SPEED);
    delay(2000);
    CAR_move(BACK,0,0);//car stop    
}

void loop()
{
    //myservo.write(90); //servo ratote angle remain as 90 
}

void CAR_move(unsigned char direction, unsigned char speed_left, unsigned char speed_right)
{
    switch(direction)
    {
        //car move forward with speed 180
        case 0: digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);//left motor clockwise rotation
		digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);break;//right motor clockwise rotation
        //car move back with speed 180
	case 1: digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);
		digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);break;
	default: break;
    }
    analogWrite(ENA_PIN,speed_left);//write speed_left to ENA_PIN,if speed_left is high,allow left motor rotate
    analogWrite(ENB_PIN,speed_right);//write speed_right to ENB_PIN,if speed_right is high,allow right motor rotate
}

