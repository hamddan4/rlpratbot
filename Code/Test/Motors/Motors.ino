#include <Servo.h>

Servo myservo;//create a object of servo,named as myservo

#define ENA_PIN   6 // ENA of DC motor driver module attach to pin5 of sunfounder uno board
#define ENB_PIN   5 // ENB of DC motor driver moduleattach to pin6 of sunfounder uno board
#define IN1 14 // left IN1 attach to pin8 
#define IN2 15 // left IN2 attach to pin9
#define IN3 16 //right  IN3 attach to pin10
#define IN4 17 //right IN4 attach to pin11

#define FORWARD 0  //define forward=0,car move forward
#define BACK    1 //define back=1,car move back
#define RIGHT 2  //define forward=0,car move forward
#define LEFT    3 //define back=1,car move back

#define SPEED_RIGHT 215 //define SPEED=180,it is the rotate speed of motor
#define SPEED_LEFT 255 //define SPEED=180,it is the rotate speed of motor
void setup()
{
    Serial.begin(19200);
    /* set below pins as OUTPUT */
    pinMode(ENA_PIN, OUTPUT);
    pinMode(ENB_PIN, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    Serial.println("Hello");
    CAR_move(FORWARD,SPEED_LEFT,SPEED_RIGHT);//car move forward with speed 180
    delay(10000);//delay 10 second
    CAR_move(FORWARD,0,0);//car move forward with speed 180
    delay(10000);//delay 5 second
    CAR_move(FORWARD,SPEED_LEFT,SPEED_RIGHT);//car move forward with speed 180
    delay(10000);//delay 2 second

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
        case 0: Serial.println("Forward");
                digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);//left motor clockwise rotation
		            digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);break;//right motor clockwise rotation
        //car move back with speed 180
	      case 1: Serial.println("Backwards");
	              digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);
		            digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);break;
        case 2: Serial.println("RIGHT");
                digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);//left motor clockwise rotation
                digitalWrite(IN3,LOW);digitalWrite(IN4,LOW);break;//right motor clockwise rotation
        //car move back with speed 180
        case 3: Serial.println("LEFT");
                digitalWrite(IN1,LOW);digitalWrite(IN2,LOW);
                digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);break;
        case 4: Serial.println("SUPER RIGHT");
                digitalWrite(IN1,LOW);digitalWrite(IN2,HIGH);//left motor clockwise rotation
                digitalWrite(IN3,HIGH);digitalWrite(IN4,LOW);break;//right motor clockwise rotation
        //car move back with speed 180
        case 5: Serial.println("SUPER LEFT");
                digitalWrite(IN1,HIGH);digitalWrite(IN2,LOW);
                digitalWrite(IN3,LOW);digitalWrite(IN4,HIGH);break;
                
	      default: break;
    }
    analogWrite(ENA_PIN,speed_left);//write speed_left to ENA_PIN,if speed_left is high,allow left motor rotate
    analogWrite(ENB_PIN,speed_right);//write speed_right to ENB_PIN,if speed_right is high,allow right motor rotate
}

