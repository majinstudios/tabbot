#include <Arduino.h>
#include <Tabbot.h>

Tabbot tabbot;
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
// Movement_encoder path_encoder []={
//   {47,-47,"",0},
//   {44,44,"",0},
//   {-43, 43,"",0},
//   {-79, -79 ,"",0}
// };

Movement_encoder path_encoder []={{104 , -104,"",0},{-3 , -3,"",0},{-3 , -3,"",0},{168 , -168,"",0},{39 , 39,"",0},{2 , 2,"",0},{73 , -73,"",0},{-1 , -1,"",0},{-1 , -1,"",0},{80 , -80,"",0},{-1 , -1,"",0},{0 , 0,"",0},{98 , -98,"",0}};

int btPinState=14;

void setup() {

  Serial.begin(9600);
  tabbot.init();
  pinMode(btPinState,INPUT);

}


void loop() {
  double blinkSpeed = pulseIn(btPinState, HIGH,1000000);    //If no pulse in 1 seconds return 0
  String incoming="";

  tabbot.move_path(path_encoder, sizeof(path_encoder)/sizeof(path_encoder[0]));
  // tabbot.get_saved_path();

  if(blinkSpeed==0){    //Connected
    Serial.flush();
    // tabbot.move_control();
    Serial.print("\n");
    Serial.println("Disconnect");
    Serial.flush();
    while (1){}
    }
}

ISR(PCINT2_vect) {
  char result = tabbot.r.process();
  char result2 = tabbot.r2.process();
  if (result) {
    result == DIR_CW ? tabbot.degrees_left++ : tabbot.degrees_left--;
    //Serial.println("Encoder 1: "+String(degrees_left));
  }
  if (result2) {
    result2 == DIR_CW ? tabbot.degrees_right++ : tabbot.degrees_right--;
    //Serial.println("Encoder 2: "+String(degrees_right));
  }
}
