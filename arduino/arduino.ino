#include <Servo.h>

//#define orange_car
//пины для энкодера
//(!!! ардуино уно один из пинов обязательно на второй или третий пин
//так как они с прерываниями, второй - любой
//второй пин можно не использовать так как мы знаем направление движения
//и важно определять только скорость
#define encoder0PinA  2
//#define encoder0PinB  5

//выход на 5 вольт
#define addition_vdd 11

#define upwm_pin 6 //пин для управления скоростью мотора. канал M1 на шилде
#define udir_pin 7 //пин для управления направлением вращения. канал M1 на шилде
#define uservo_pin 9 //пин куда подключен сервомотор

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

int Speed = 0,old_Speed=0; // Скорость в условных единицах
float real_speed = 0; //реальная скорость в сантиметрах в секунду
int DIR = 0;
int Corner = 90,old_Corner = 0; // угол поворота в градусах
Servo myservo;
char cur_state = 0;
int kod = 0;
unsigned long time;
unsigned long right_time_indicator;
unsigned long left_time_indicator;
unsigned long time_current;
unsigned long last_speed_update =0;
boolean turn_right_light;
boolean turn_left_light;
volatile unsigned int encoder0Pos = 0;

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
  pinMode(addition_vdd,OUTPUT);
  digitalWrite(addition_vdd, HIGH);
  
  Serial.begin(115200);
  Serial.setTimeout(20);
  myservo.attach(uservo_pin);  
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
  
  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH); //включить нагрузочный резистор
  attachInterrupt(0, doEncoder, CHANGE); // encoder pin on interrupt 0 - pin 2 
}

void turnsignal_illumination();
void serial_get_data();

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
      motor1.set_speed(Speed/4);
    }
    else
    {
      digitalWrite(upwm_pin, LOW);
    }
  }
  else
  {
    motor1.set_speed(Speed/4);
  }
  
  turnsignal_illumination(); //управляет мерцанием поворотников

  old_Speed = Speed; 
  old_Corner = Corner;
  serial_get_data(); //получить данные по последовательному порту
  if(time_current-last_speed_update>1000)
  {
    serial_send_data();
  }
}










void serial_send_data()
{
  //отправить последовательность байт по COM порту
  //отправить реальную скорость
  //unsigned int - 2 байта
  real_speed = encoder0Pos*1.3;
  encoder0Pos=0;
  last_speed_update = millis();
   Serial.print('F');
   Serial.print(real_speed);
   Serial.print('E');
}


void serial_get_data()
{
   if (Serial.available() > 0)
   {
       char c = Serial.read();
       
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
               time = time_current;
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
  if(Speed == 0)
  {
   digitalWrite(stop_indicator_pin, HIGH); 
  }
  else
  {
    digitalWrite(stop_indicator_pin, LOW);
  }
  
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


void doEncoder()
{
  encoder0Pos++;
}

