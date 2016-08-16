#include <Arduino.h>
#include <rotary.h>
#include <BitbloqUS.h>
#include <EEPROM.h>

/***   Global variables and function definition  ***/
US ultrasounds_right_front(A4,A5);
US ultrasounds_right(A2,A3);

US ultrasounds_left_front(7,8);
US ultrasounds_left(12,13);

bool newInfoReceived=false;

#define forwards 0
#define backwards 1
#define left 2
#define right 3
#define stop 4
#define save 5

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
   double m;
   String msg;
   double s;
};

struct Movement_encoder {
   int  enc_left;
   int enc_right;
   String msg;
   double s;
};
//Path init
// Movement path [6]={
//   {forwards,  0.3,"", 0},
//   {0, 0, "Hi! How are you?", 2},
//   {0, 0, "I am Tabbot, your guide for today's tour", 0},
//   {right, 0.1, "", 0},
//   {forwards, 0.3, "", 0},
//   {left, 0.5, "", 0}
// };

// Movement path [10]={
//   {forwards,  1.15,"", 0},
//   {left, 0.22, "", 0},
//   {forwards, 0.9, "", 0},
//   {backwards, 0.9, "", 0},
//   {right, 0.25, "", 0},
//   {forwards, 1.8, "", 0},
//   {left, 0.25, "", 0},
//   {forwards, 0.8, "", 0},
//   {right, 0.25, "", 0},
//   {forwards, 1.3, "", 0}
// };

Movement path []={
  {forwards,  1,"", 0},
  {stop, 1, "", 0}
};
Movement_encoder path_encoder []={
  {40,40,"",0},
  {0,0,"Hola hola",3},
  {-20,-20,"",0},
};
void move_path();
void move_robot(int dir, double distance);
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

String readString(){
	String inString ="";
	char inChar;
	while(Serial.available()>0){
		inChar =(char) Serial.read();
		inString+=inChar;
		delay(1);
	}
	newInfoReceived=true;
	return inString;
}

void loop() {
  double blinkSpeed = pulseIn(btPinState, HIGH,1000000);    //If no pulse in 1 seconds return 0
  String incoming="";
  if(blinkSpeed==0){    //Connected
    Serial.flush();
    // move_path(path);
    // move_path(/*path_encoder*/);
    move_control();
    Serial.print("\n");
    Serial.println("Disconnect");
    Serial.flush();
    while (1){}
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
void move_control(){
  String incoming = "";
  int initEncoder [2]={0,0}; //left, right
  int finalEncoder [2]={0,0}; //left, right
  int movement[100];
  int count= 0;
  int last_dir=stop; //last direction
  while (1){
    if(Serial.available()) {
      incoming=readString();
      if (atoi(incoming.c_str()) == save){
        break;
      }
      switch (atoi(incoming.c_str())){
        case stop:
        move_robot(stop, 0);
        finalEncoder[0] = degrees_left;
        finalEncoder[1] = degrees_right;
        break;

        case forwards:
        if (last_dir == backwards){
          delay(500);
        }
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot(forwards, 0);
        break;

        case backwards:
        if (last_dir == forwards){
          delay(500);
        }
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot(backwards, 0);
        break;

        case left:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot(left, 0);
        break;

        case right:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot(right, 0);
        break;
      }
      movement[count]= finalEncoder[0] - initEncoder[0]; //left
      movement[count +1]= finalEncoder[1] - initEncoder[1]; //right
      count+=2;
      last_dir = atoi(incoming.c_str()); 
    }
  }
  //Store encoder values
  EEPROM.write(0, sizeof(movement)/sizeof(movement[0]));
  for ( int i = 1; i < sizeof(movement)/sizeof(movement[0])+ 1; i++ ){
    EEPROM.write(i, movement[i-1]);
  }
}

int * get_saved_path(){
  int movement[100];
  int size = EEPROM.read(0);
  for (int i= 1; i<size+1; i++){
    movement[i-1]= EEPROM.read(i);
  }
  return movement;
}

void search_obstacles(){
  int d= 8;
  int US [4]={ultrasounds_right.read(),ultrasounds_right_front.read(),ultrasounds_left_front.read(),ultrasounds_left.read()};

  for(int i=0; i<sizeof(US)/sizeof(US[0]); i++){
    while (US[i]<d){ //If there is an object near the robot
      move_robot(stop, 1);  //Stop the robot
      Serial.println("Please, move aside so I may continue the tour");  //Send message to tablet
      delay(2000);  //Wait & update the sensors
      US[0]=ultrasounds_right.read();
      US[1]=ultrasounds_right_front.read();
      US[2]=ultrasounds_left_front.read();
      US[3]=ultrasounds_left.read();
    }
  }
}

void move_path (struct Movement * path){
  for (int i = 0; i < sizeof(path)/sizeof(path[0]); i ++){
    if (path[i].msg != ""){
      Serial.print("\n");
      Serial.println(path[i].msg);
      Serial.flush();

      delay(path[i].s*1000);
    }
    else{
      double d= 0.1;
      if(path[i].dir != stop){
          for (double j = 0; j < path[i].m; j+=d){
            //search for obstacles
            search_obstacles();
            //move robot d m
            move_robot(path[i].dir, d);
          }
      }else{
        move_robot(path[i].dir, path[i].m);
      }
       if (i<(sizeof(path)/sizeof(path[0])) -1 &&( (path[i].dir==forwards && path[i+1].dir == backwards) || (path[i].dir==backwards && path[i+1].dir == forwards))){
         delay(500);
       }
  }
  }
}


void move_path (/*struct Movement_encoder path []*/){
  Serial.println("Moving path! Size: "+String(sizeof(path_encoder)/sizeof(path_encoder[0])));
  for (int i = 0; i < sizeof(path_encoder)/sizeof(path_encoder[0]); i ++){
    Serial.println("i: "+String(i));
    if (path_encoder[i].msg != ""){
      Serial.print("\n");
      Serial.println(path_encoder[i].msg);
      Serial.flush();

      delay(path_encoder[i].s*1000);
    }
    else{
      int d= 10; //distance to check for obstacles
      if(path_encoder[i].enc_left == 0 && path_encoder[i].enc_right == 0){ //if both encoders values are 0, stop the robot.
        move_robot(0,0, stop);
      }
      else{
          for (double j = 0; j <= abs(path_encoder[i].enc_left); j+=d){
            Serial.print("J: ");
            Serial.println(j);
            //search for obstacles
            search_obstacles();
            //move robot d m
            if (path_encoder[i].enc_left == path_encoder[i].enc_right && path_encoder[i].enc_left > 0){
              move_robot(d,d, forwards);
            }
            else if (path_encoder[i].enc_left == path_encoder[i].enc_right && path_encoder[i].enc_left < 0){
              move_robot(-d,-d, backwards);
            }
            else if (path_encoder[i].enc_left != path_encoder[i].enc_right && path_encoder[i].enc_left < 0){
              move_robot(-d,d, right);
            }
            else if (path_encoder[i].enc_left != path_encoder[i].enc_right && path_encoder[i].enc_left > 0){
              move_robot(d,-d, left);
            }
          }
      }
      move_robot(0,0, stop);
      if (i<(sizeof(path_encoder)/sizeof(path_encoder[0])) &&( (path_encoder[i].enc_left>0 && path_encoder[i].enc_right>0 && path_encoder[i+1].enc_left<0 && path_encoder[i+1].enc_right<0) || (path_encoder[i].enc_left<0 && path_encoder[i].enc_right<0 && path_encoder[i+1].enc_left>0 && path_encoder[i+1].enc_right>0))){
         delay(500);
      }
    }
  }
}


void move_robot(int dir, double distance=1){
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
    case right:
      enc = degrees_left;
      moveMotor(motor_right, -255);
      moveMotor(motor_left, 255);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "RIGHT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case left:
      enc = degrees_right;
      moveMotor(motor_right, 255);
      moveMotor(motor_left, -255);
      while (abs(enc - degrees_right) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "LEFT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case stop:
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 0);
    break;
    default:
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 0);
    break;
  }

}


void move_robot(int enc_left, int enc_right, int dir){
  int enc = 0;
  int t=20;
  String msg="";
  switch (dir){
    case forwards:
      enc = degrees_left;
      moveMotor(motor_right, 255);
      moveMotor(motor_left, 255);
      while (abs(enc - degrees_left) < enc_left){ //Move until the total ticks number is equal to the one given
        // msg = "FORWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case backwards:
      enc = degrees_left;
      moveMotor(motor_right, -255);
      moveMotor(motor_left, -255);
      while (abs(enc - degrees_left) < enc_left){ //Move until the total ticks number is equal to the one given
      // msg = "BACKWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case right:
      enc = degrees_left;
      moveMotor(motor_right, -255);
      moveMotor(motor_left, 255);
      while (abs(enc - degrees_left) < enc_left){ //Move until the total ticks number is equal to the one given
        // msg = "RIGHT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case left:
      enc = degrees_right;
      moveMotor(motor_right, 255);
      moveMotor(motor_left, -255);
      while (abs(enc - degrees_right) < enc_right){ ///Move until the total ticks number is equal to the one given
        // msg = "LEFT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case stop:
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 0);
    break;
    default:
      moveMotor(motor_right, 0);
      moveMotor(motor_left, 0);
    break;
  }
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
