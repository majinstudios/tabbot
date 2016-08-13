#include <Arduino.h>
#include <rotary.h>

bool newInfoReceived=false;

#define forwards 1
#define backwards 2
#define left 3
#define right 4

Rotary r = Rotary(2, 3);
Rotary r2 = Rotary(4, 5);
int degrees_left, degrees_right;

int maxdegrees;
bool next = false;
int motor_left [2] = {11, 10}; //forwards, backwards
int motor_right [2] = {9, 6}; //forwards, backwards
int btPinState=14;

struct Movement {
   int  dir;
   float m;
   String msg;
   float s;
};
//Path init
Movement path [6]={{forwards,  0.3,"", 0}, {0, 0, "Hi! How are you?", 2}, {0, 0, "I am Tabbot, your guide for today's tour", 0}, {right, 0.1, "", 0}, {forwards, 0.3, "", 0}, {left, 0.5, "", 0}};
void move_path();
void move_robot(int dir, float distance);
void moveMotor (int * motor, int speed);

void setup() {

  Serial.begin(9600);
  //Serial.begin(19200);

  //Encoders interrupts init
  PCICR |= (1 << PCIE2) | (1 << PCIE1);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20) | (1 << PCINT21);
  sei();

  for (int motor=0; motor<=sizeof(motor_right); motor ++){
    pinMode(motor_right[motor], OUTPUT);
  }
  for (int motor=0; motor<=sizeof(motor_left); motor ++){
    pinMode(motor_left[motor], OUTPUT);
  }

  pinMode(btPinState,INPUT);
}


void loop() {
  double blinkSpeed = pulseIn(btPinState, HIGH,1000000);    //If no pulse in 1 seconds return 0
  if(blinkSpeed==0){    //Connected
      Serial.flush();
      move_path(/*path*/);
      Serial.print("\n");
      Serial.println("Disconnect");
      Serial.flush();
  }
}

ISR(PCINT2_vect) {
  char result = r.process();
  char result2 = r2.process();
  if (result) {
    result == DIR_CW ? degrees_left++ : degrees_left--;
    //Serial.println("Encoder 1: "+String(degrees_left));
  }
  if (result2) {
    result2 == DIR_CW ? degrees_right++ : degrees_right--;
    //Serial.println("Encoder 2: "+String(degrees_right));
  }
}


void move_path (/*struct Movement * path*/){
  //Serial.println("sizeof(path)   "+String( sizeof(path)/sizeof(path[0])));
  for (int i = 0; i < sizeof(path)/sizeof(path[0]); i ++){
    //Serial.println("Movimiento #: "+String(i));
    //Serial.println("Direccion: "+String(path[i].dir));
    if (path[i].msg != ""){
      //Serial.println("Message: "+ path[i].msg);
      Serial.print("\n");
      Serial.println(path[i].msg);
      Serial.flush();

      delay(path[i].s*1000);
    }
    else{
      for (float j = 0; j < path[i].m; j+=0.1){
          //move robot 0.1 m
          move_robot(path[i].dir, 0.1);
          //search for obstacles
       }
       if (i<(sizeof(path)/sizeof(path[0])) -1 &&( (path[i].dir==forwards && path[i+1].dir == backwards) || (path[i].dir==backwards && path[i+1].dir == forwards))){
         delay(500);
       }
  }
  }
}


void move_robot(int dir, float distance){
  int enc = 0;
  int t=20;
  String msg="";
  switch (dir){
    case forwards:
      enc = degrees_left;
      moveMotor(motor_right, 255);
      moveMotor(motor_left, 255);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "FORWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case backwards:
      enc = degrees_left;
      moveMotor(motor_right, -255);
      moveMotor(motor_left, -255);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
      msg = "BACKWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case left:
      enc = degrees_left;
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 255);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "LEFT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case right:
      enc = degrees_right;
      moveMotor(motor_right, 255);
      moveMotor(motor_left, 0);
      while (abs(enc - degrees_right) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "RIGHT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    default:
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 0);
    break;
  }
  moveMotor(motor_right, 0);
  moveMotor(motor_left, 0);
}

void moveMotor (int * motor, int speed){
  if (speed >=0){ //If the speed is positive, we want to go forward
    analogWrite(motor[forwards], speed);
    analogWrite(motor[backwards], 0);
  } else{ //If the speed is negative, we want to go backwards
    analogWrite(motor[backwards], -speed);
    analogWrite(motor[forwards], 0);
  }
}
