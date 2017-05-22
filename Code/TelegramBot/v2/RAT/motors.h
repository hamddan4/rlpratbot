

typedef enum {LFWD, FWD, RFWD, STOP, LBCK, BCK, RBCK, SPDN, SPUP, RROT, LROT} button;

#define ENA_PIN 5 // ENA of DC motor driver module attach to pin5 of sunfounder uno board
#define ENB_PIN 6 // ENB of DC motor driver moduleattach to pin6 of sunfounder uno board
#define IN1 14 // left IN1 attach to pin8
#define IN2 15 // left IN2 attach to pin9
#define IN3 16 //right  IN3 attach to pin10
#define IN4 17 //right IN4 attach to pin11

#define MIN_SPD 80
#define MAX_SPD 250

void setup_motors(){
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void moveRAT(button direction, unsigned char speed_left, unsigned char speed_right)
{
    switch(direction)
    {
        case FWD:  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH); break;
        case LFWD: digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);
                   digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH); break;
        case RFWD: digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW); break;
        case BCK:  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
                   digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW); break;
        case LBCK: digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);
                   digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW); break;
        case RBCK: digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
                   digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW); break;
        case RROT: digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,HIGH);  digitalWrite(IN4,LOW); break;
        case LROT: digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
                   digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); break;                   
        case STOP:
        default:   digitalWrite(IN1,HIGH);  digitalWrite(IN2,HIGH);
                   digitalWrite(IN3,HIGH);  digitalWrite(IN4,HIGH); break;
         
    }
    analogWrite(ENA_PIN,speed_left);//write speed_left to ENA_PIN,if speed_left is high,allow left motor rotate
    analogWrite(ENB_PIN,speed_right);//write speed_right to ENB_PIN,if speed_right is high,allow right motor rotate
}

