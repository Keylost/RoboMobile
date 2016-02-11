#include <Servo.h>

//#define orange_car

#define upwm_pin 9 //пин для управления скоростью мотора. канал M1 на шилде
#define udir_pin 7 //пин для управления направлением вращения. канал M1 на шилде
#define uservo_pin 3 //пин куда подключен сервомотор

#ifndef orange_car
#define head_light_pin A0 //передние фары
#define left_indicator_pin A1 //левый поворотник
#define right_indicator_pin A2 //правый поворотник
#define stop_indicator_pin A3 //стоп сигналы
#define rear_light_pin A4 // задние фары
#define gnd_analog_pin A5 //масса
#else //в этом блоке неверный первоначальный вариант распиновки. только для авто с оранжевым кузовом
#define stop_indicator_pin A0 //стоп сигналы
#define head_light_pin A1 //передние фары
#define rear_light_pin A1 // задние фары
#define right_indicator_pin A3 //правый поворотник
#define left_indicator_pin A4 //левый поворотник
#define gnd_analog_pin A5 //масса
#endif

#define turn_signal_freq 500 //частота моргания поворотников в миллисекундах
#define deviation 10 //значение угла поворота сервомотора в градусах при котором будут включены поворотники
#define angle_range 35
#define angle_center 90
#define speed_min 450

int Speed = 0,old_Speed=0; // Скорость
int DIR = 0;
int Corner = 90,old_Corner = 0; // угол поворота в градусах
Servo myservo;
char cur_state = 0;
int kod = 0;
unsigned long time;
unsigned long right_time_indicator;
unsigned long left_time_indicator;
unsigned long time_current;
boolean turn_right_light;
boolean turn_left_light;


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

void setup(void)
{
  Serial.begin(115200);
  Serial.setTimeout(20);
  myservo.attach(uservo_pin);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);  
  motor1.set_direction(FORWARD);
  pinMode(gnd_analog_pin,OUTPUT);
  pinMode(rear_light_pin,OUTPUT);
  pinMode(head_light_pin,OUTPUT);
  pinMode(stop_indicator_pin,OUTPUT);
  pinMode(left_indicator_pin,OUTPUT);
  pinMode(right_indicator_pin,OUTPUT);
  digitalWrite(rear_light_pin, HIGH);
  digitalWrite(head_light_pin, HIGH);
  digitalWrite(gnd_analog_pin,LOW);
  digitalWrite(left_indicator_pin,LOW);
  digitalWrite(right_indicator_pin,LOW);
  digitalWrite(stop_indicator_pin,LOW);
  time = 0;
  right_time_indicator = 0;
  left_time_indicator = 0;
  time_current = 0;
  turn_right_light = false;
  turn_left_light = false;
  //OCR0A = 0xAF;
  //TIMSK0 |= _BV(OCIE0A);
  
}

void turnsignal_illumination();
/*
boolean state = false;

SIGNAL(TIMER0_COMPA_vect) 
{
 if(state)
 {
   state = false;
 }
 else
 {
   state = true;
 }
}
*/

void loop(void)
{
  time_current = millis();
  
  if(Corner!=old_Corner) myservo.write(Corner);  //set up corner
  if((time_current-time)>1000)
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
  }
  else
  {
    digitalWrite(stop_indicator_pin, LOW);
    motor1.set_speed_digit(Speed);
  }
  
  turnsignal_illumination(); //управляет мерцанием поворотников

 old_Speed = Speed; 
 old_Corner = Corner;
 if (Serial.available() > 0)
   {
       char c = Serial.read();
       time = time_current;
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

void turnsignal_illumination()
{
  if(Corner>90+deviation)
  {
    if(!turn_right_light)
    {
      if(right_time_indicator==0)
      {
        turn_right_light = true;
        digitalWrite(right_indicator_pin, HIGH);
        right_time_indicator = time_current;
      }
      else if(time_current - right_time_indicator > 2*turn_signal_freq)
      {
        turn_right_light = false;
        right_time_indicator = 0;
        digitalWrite(right_indicator_pin, LOW);
      }
    }
    else
    {
      if(time_current - right_time_indicator > turn_signal_freq)
      {
        turn_right_light = false;
        digitalWrite(right_indicator_pin, LOW);       
      }
    }
  }
  else
  {
    turn_right_light = false;
    right_time_indicator = 0;
    digitalWrite(right_indicator_pin, LOW);
  }
  
  if(Corner<90-deviation)
  {
    if(!turn_left_light)
    {
      if(left_time_indicator==0)
      {
        turn_left_light = true;
        digitalWrite(left_indicator_pin, HIGH);
        left_time_indicator = time_current;
      }
      else if(time_current - left_time_indicator > 2*turn_signal_freq)
      {
        turn_left_light = false;
        left_time_indicator = 0;
        digitalWrite(left_indicator_pin, LOW);
      }
    }
    else
    {
      if(time_current - left_time_indicator > turn_signal_freq)
      {
        turn_left_light = false;
        digitalWrite(left_indicator_pin, LOW);       
      }
    }
  }
  else
  {
    turn_left_light = false;
    left_time_indicator = 0;
    digitalWrite(left_indicator_pin, LOW);
  }
}

