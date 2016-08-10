#include <Arduino.h>
#include <rotary.h>

#define forwards 0
#define backwards 1

Rotary r = Rotary(2, 3);
Rotary r2 = Rotary(4, 5);
int degrees01, degrees02;

int maxdegrees;
bool next = false;
int motor_right [2] = {10, 11}; //forwards, backwards
int motor_left [2] = {6, 9}; //forwards, backwards

struct Movement {
   String  dir;
   String msg; 
   float m;
};
Movement path [2];

void setup() {
  //Path init
  path[0].dir = "forward";
  path[0].m = 0.5;
  path[1].dir = "forward";
  path[1].m = 0.5;
  
  Serial.begin(115200);
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

}

void loop() {

  move_path();
  
  moveMotor(motor_right, 0);
  moveMotor(motor_left, 0);
  while (1){
  }
 // moveMotor(motor_right, 255);
  //moveMotor(motor_left, 255);
  // delay(1000);
  // moveMotor(motor_right, -100);
  // delay(1000);
}

ISR(PCINT2_vect) {
  char result = r.process();
  char result2 = r2.process();
  if (result) {
    result == DIR_CW ? degrees01++ : degrees01--;
    //Serial.println("Encoder 1: "+String(degrees01));
  }
  if (result2) {
    result2 == DIR_CW ? degrees02++ : degrees02--;
    //Serial.println("Encoder 2: "+String(degrees02));
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

void move_path (){
  Serial.println("sizeof(path)   "+String( sizeof(path)/sizeof(path[0])));
  for (int i = 0; i < sizeof(path)/sizeof(path[0]); i ++){
    Serial.println("Estamos en el momento"+String(i));
    Serial.println("Degrees 01: "+String(degrees01));

    if (path[i].dir =="forward"){
     // for (int j = 0; j < path[i].m; j+=0.1){
        //move forwards
        int enc = degrees01; 
        moveMotor(motor_right, 255);
        moveMotor(motor_left, 255);
        while (abs(enc - degrees01) < path[i].m * 100){
          //enc = degrees01;
          Serial.println(String(abs(enc - degrees01))+"   "+String(path[i].m *100));
          delay(20);
        }
        moveMotor(motor_right, 0);
        moveMotor(motor_left, 0);       
        
     // }
    }
  }
}
