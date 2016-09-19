#include <Arduino.h>
#include <Tabbot.h>

Tabbot tabbot;
//Path init
// Movement path []={
//   {forwards,  1,"", 0},
//   {stop, 1, "", 0}
// };
// Movement_encoder path_encoder []={{0, 0, "I am Tabbot, your robotic guide. Please, follow me.", 3},{102 , -102,"",0},{-25 , -25,"",0},{0, 0, "If you look to your left, you can see the kitchen.", 5},{16 , 16,"",0},{157 , -157,"",0},{-24 , -24,"",0},{-2 , -2,"",0},{0, 0, "Please follow me into the living room.", 0},{50 , -50,"",0},{45 , -45,"",0},{18 , 18,"",0},{153 , -153,"",0},{45 , 45,"",0}};
Movement_encoder path_encoder []={{0, 0, "11111111111", 1},{39 , -39,"",0},{0, 0, "22222222222", 3},{-10 , -10,"",0},{19 , 19,"",0},{-8 , -8,"",0},{0, 0, "33333333333", 1},{-39 , 39,"",0}};



int btPinState=14;

void setup() {
  Serial.begin(9600);
  tabbot.init();
  pinMode(btPinState,INPUT);
}

void loop() {
  double blinkSpeed = pulseIn(btPinState, HIGH,1000000);    //If no pulse in 1 seconds return 0
  if(blinkSpeed==0){    // Connected via BT
    Serial.flush();
    tabbot.move_path(path_encoder, sizeof(path_encoder)/sizeof(path_encoder[0])); // Move the robot following the path defined in the variable path_encoder
    Serial.print("\n");
    Serial.println("Disconnect");
    Serial.flush();
    delay(2000);
  }
  delay(200);
}

ISR(PCINT2_vect) {
  char result = tabbot.r.process();
  char result2 = tabbot.r2.process();
  if (result) {
    result == DIR_CW ? tabbot.degrees_left++ : tabbot.degrees_left--;
  }
  if (result2) {
    result2 == DIR_CW ? tabbot.degrees_right++ : tabbot.degrees_right--;
  }
}
