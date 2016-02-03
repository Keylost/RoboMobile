#include <Servo.h>

#define upwm_pin 9 //пин для управления скоростью мотора. канал M1 на шилде
#define udir_pin 7 //пин для управления направлением вращения. канал M1 на шилде
#define uservo_pin 3 //пин куда подключен сервомотор
#define power_indicator_pin 13 //передние и задние фары
#define stop_indicator_pin 11 //стоп сигналы
#define left_indicator_pin 6 //левый поворотник
#define right_indicator_pin 5 //правый поворотник
#define deviation 10 //значение угла поворота сервомотора в градусах при котором будут включены поворотники


#define angle_range 35
#define angle_center 90
#define speed_min 450

enum directions
{
  FORWARD = 1,
  BACKWARD = 2
};

class Motor
{
  private:
  int PWM_PIN; //pin on motorshild to control pwm
  int DIR_PIN; //pin on motorshild to control direction
  public:
  Motor(int PWM_PIN,int DIR_PIN)
 {
   this->PWM_PIN = PWM_PIN;
   this->DIR_PIN = DIR_PIN;   
   pinMode(PWM_PIN, OUTPUT); 
   pinMode(DIR_PIN, OUTPUT);
 }
 void set_direction(directions dir)
 {
   switch(dir)
   {
    case FORWARD:
     {
        digitalWrite(DIR_PIN, HIGH);
        break;
     }
    case BACKWARD:
     {
        digitalWrite(DIR_PIN, LOW);
        break;        
     }
    default:
     {
        break;
     } 
   }
 }
 void set_speed_digit(int Speed)
 {
    if (Speed > 0)
    {
      if (Speed > 999) Speed = 999;
      digitalWrite(PWM_PIN, HIGH);
      delayMicroseconds(Speed);
      digitalWrite(PWM_PIN, LOW);
      delayMicroseconds(1000 - Speed);
    }
 }
 void set_speed(int Speed)
 {
   analogWrite(PWM_PIN, Speed);
 }
};

Motor motor1(upwm_pin,udir_pin);

void full_stop()
{
  for(int i=0;i<5;i++)
  {
  motor1.set_direction(BACKWARD);
  motor1.set_speed_digit(200);
  motor1.set_direction(FORWARD);
  motor1.set_speed_digit(200);
  }
}

Servo myservo;

char cur_state = 0;
int kod = 0;
unsigned long time;

void setup(void)
{
  Serial.begin(115200);
  Serial.setTimeout(20);
  myservo.attach(uservo_pin);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);  
  motor1.set_direction(FORWARD);
  pinMode(power_indicator_pin,OUTPUT);
  pinMode(stop_indicator_pin,OUTPUT);
  pinMode(left_indicator_pin,OUTPUT);
  pinMode(right_indicator_pin,OUTPUT);
  digitalWrite(power_indicator_pin, HIGH);
}

int Speed = 0,old_Speed=0; // Скорость
int DIR = 0;
int Corner = 90,old_Corner = 0; // угол поворота в градусах
//boolean stay = false;

void loop(void)
{
  if(Corner!=old_Corner) myservo.write(Corner);  //set up corner
  if((millis()-time)>1000)
  {
     Speed = 0;   
  }
  
  if(Speed == 0)
  {
    if(abs(Corner-90)>angle_range-5)
    {
      Speed = old_Speed;
      motor1.set_speed_digit(Speed);
    }
    else
    {
      digitalWrite(stop_indicator_pin, HIGH);
      digitalWrite(upwm_pin, LOW);
    }
    /*
    if(!stay)
    {
    motor1.set_direction(BACKWARD);
    for(int i=0;i<20;i++)
    {
      motor1.set_speed_digit(999);
    }
    digitalWrite(upwm_pin, LOW);
    motor1.set_direction(FORWARD);
    stay = true;
    }
    */
  }
  else
  {
    digitalWrite(stop_indicator_pin, LOW);
    motor1.set_speed_digit(Speed);
    //stay=false;
  }
  if(Corner>90+deviation)
  {
    digitalWrite(right_indicator_pin, HIGH);
  }
  else
  {
    digitalWrite(right_indicator_pin, LOW);
  }
  if(Corner<90-deviation)
  {
    digitalWrite(left_indicator_pin, HIGH);
  }
  else
  {
    digitalWrite(left_indicator_pin, LOW);
  }
  
 old_Speed = Speed; 
 old_Corner = Corner;
 if (Serial.available() > 0)
   {
       char c = Serial.read();
       time = millis();
       switch(cur_state)
       {
        case 1:
            if(c == 'P') cur_state = 2;
            else cur_state = 0;
            break;

        case 2:
            if(c == 'D')
            {
               Corner = Serial.parseInt();
               DIR = Serial.parseInt();
               Speed = Serial.parseInt();
               cur_state = 0;
            }
            else cur_state = 0;
            break;
        default:
            if(c == 'S') cur_state = 1;
            else cur_state = 0;
            break;
       }

   }
}

