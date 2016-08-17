#include <Arduino.h>
#include <Tabbot.h>

Tabbot tabbot;

int btPinState=14;

void setup() {

  Serial.begin(9600);
  tabbot.init();
  pinMode(btPinState,INPUT);

}


void loop() {
  double blinkSpeed = pulseIn(btPinState, HIGH,1000000);    //If no pulse in 1 seconds return 0

  //Uncomment this line to get the saved path stored in the EEPROM
  // tabbot.get_saved_path();

  if(blinkSpeed==0){    //Connected via BT
    Serial.flush();
    tabbot.move_control(); // Teleoperate Tabbot using the android app or the serial monitor.
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
  }
  if (result2) {
    result2 == DIR_CW ? tabbot.degrees_right++ : tabbot.degrees_right--;
  }
}
