#ifndef TABBOT_H
#define TABBOT_H

#define forwards 0
#define backwards 1
#define left 2
#define right 3
#define stop 4
#define save 5

#include <Arduino.h>
#include <rotary.h>
#include <BitbloqUS.h>
#include <EEPROM.h>

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

class Tabbot
{

  public:
    void init();
    void move_robot(int dir, double distance);
    void moveMotor (int * motor, int speed);
    void move_control();
    void get_saved_path();

    void move_path (struct Movement *, int size);
    void move_path (struct Movement_encoder * path_encoder, int size);
    void move_robot_meters(int , double );
    void move_robot_encoders(int , int , int );

    int * round_encoder_measure (int enc_left, int enc_right);
    Rotary r = Rotary(2, 3);
    Rotary r2 = Rotary(4, 5);
    int degrees_left, degrees_right;


    String readString();

  private:

    bool newInfoReceived;
    US ultrasounds_right_front;
    US ultrasounds_right;

    US ultrasounds_left_front;
    US ultrasounds_left;


    int motor_left [2] = {11, 10}; //forwards, backwards
    int motor_right [2] = {9, 6}; //forwards, backwards

    void search_obstacles();
};


#endif
