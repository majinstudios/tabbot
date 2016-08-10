#include <SoftwareSerial.h>

SoftwareSerial bt(2, 3);
bool newInfoReceived=false;
bool btConnected=false;

void setup(){
    bt.begin(9600);
    Serial.begin(19200);
}


String readString(){
	String inString ="";
	char inChar;
	while(bt.available()>0){
		inChar =(char) bt.read();
		inString+=inChar;
		delay(1);
	}
	newInfoReceived=true;
	return inString;
}


void loopMsgs(){
    bt.println("If you look at your left, you may find the kitchen. It is quite big. ");
    delay(5000);
    bt.println("Living Room");
    delay(1000);
    bt.println("Dorm");
    delay(1000);
    bt.println("Bathroom");
    delay(1000);
    bt.println("Second dorm");
    delay(1000);
    bt.println("Office");
    delay(1000);
}


void loop(){
    String incoming;
    if(bt.available()) incoming=readString();
    if (newInfoReceived==true){
        if(incoming=="Connect") {
            btConnected=true;
            loopMsgs();
        }
        newInfoReceived=false;
	}
}
