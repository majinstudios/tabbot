#include <Tabbot.h>


String Tabbot::readString(){
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

void Tabbot::init(){
	//US init
	ultrasounds_right_front.init(18,19);
	ultrasounds_right.init(16,17);

	ultrasounds_left_front.init(7,8);
	ultrasounds_left.init(12,13);

	//Buffer init
	newInfoReceived=false;

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

int * Tabbot::round_encoder_measure (int enc_left, int enc_right){
	int diff = abs(abs(enc_left)+abs(enc_right))/2;
	int enc_values []={0,0};
	if(enc_left > 0){
		enc_values[0] = diff;
	}
	else{
		enc_values[0] = -diff;
	}
	if(enc_right > 0){
		enc_values[1] = diff;
	}
	else{
		enc_values[1] = -diff;
	}
	return enc_values;
}

void Tabbot::move_control(){
  String incoming = "";
  int initEncoder [2]={0,0}; //left, right
  int finalEncoder [2]={0,0}; //left, right
  int movement[100];
  int count= 0;
	int enc_value = 0;
	int diff_left, diff_right, diff;
  while (1){
    if(Serial.available()) {
      incoming=readString();
      if (atoi(incoming.c_str()) == save){
        break;
      }
      switch (atoi(incoming.c_str())){
        case stop:
			//	default:
        move_robot_meters(stop, 0);
        finalEncoder[0] = degrees_left;
        finalEncoder[1] = degrees_right;
				diff_left= finalEncoder[0] - initEncoder[0]; //left
				diff_right= finalEncoder[1] - initEncoder[1]; //right
				diff = abs(abs(diff_left)+abs(diff_right))/2;
				if(diff_left > 0){
					movement[count]= diff;
				}
				else{
					movement[count] = -diff;
				}
				if(diff_right > 0){
					movement[count+1] = diff;
				}
				else{
					movement[count+1] = -diff;
				}


				// Serial.print(movement[count]);
				// Serial.print(" ");
				// Serial.print(movement[count+1]);
				// Serial.print(" ");
				count+=2;

				delay(50);
        break;

        case forwards:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot_meters(forwards, 0);
        break;

        case backwards:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot_meters(backwards, 0);
        break;

        case left:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot_meters(left, 0);
        break;

        case right:
        initEncoder[0] = degrees_left;
        initEncoder[1] = degrees_right;
        move_robot_meters(right, 0);
        break;
      }
    }
  }


  //Store encoder values
  EEPROM.write(0, count);
	Serial.println("EEPROM write (size, and then values): ");

	Serial.print(EEPROM.read(0));
	Serial.print(" ");
	Serial.print(count);
	Serial.println("");
	int mov_index= 0;
	int dummy = 0;
  for ( int i = 1; i < count*2+ 1; i+=2 ){
		EEPROM.write(i, movement[mov_index]>0);
    EEPROM.write(i+1, abs(movement[mov_index]));
		delay(200);
		dummy = EEPROM.read(i+1);
		if (EEPROM.read(i) == 0){
			dummy*=-1;
		}
		// Serial.print(dummy);
		// Serial.print(EEPROM.read(i));
		// Serial.print(EEPROM.read(i+1));
		// Serial.print(" ");
		Serial.print(movement[mov_index]);
		Serial.print(" ");
		mov_index ++;
  }
}

void Tabbot::get_saved_path(){
  int size = EEPROM.read(0);
	int movement[size];
	int mov_index = 0;
	bool flag=false;
	String message = "Movement_encoder path_encoder []={{";
  for (int i= 1; i<size*2+1; i+=2){
		movement[mov_index]= EEPROM.read(i+1);
		if(EEPROM.read(i)==0){
			movement[mov_index]*= -1;
		}

		message +=String(movement[mov_index]);
		if (flag && i!= size*2-1){
			message +=",\"\",0},{";
		}else if (flag && i== size*2-1){
			message +=",\"\",0}};";
		}else{
			message +=" , ";
		}
		flag = !flag;
		mov_index ++;
  }
	Serial.println(message);
}

void Tabbot::search_obstacles(){
  int d= 8;
  int US [4]={ultrasounds_right.read(),ultrasounds_right_front.read(),ultrasounds_left_front.read(),ultrasounds_left.read()};

  for(int i=0; i<sizeof(US)/sizeof(US[0]); i++){
    while (US[i]<d){ //If there is an object near the robot
      move_robot_meters(stop, 1);  //Stop the robot
      Serial.println("Please, move aside so I may continue the tour");  //Send message to tablet
      delay(2000);  //Wait & update the sensors
      US[0]=ultrasounds_right.read();
      US[1]=ultrasounds_right_front.read();
      US[2]=ultrasounds_left_front.read();
      US[3]=ultrasounds_left.read();
    }
  }
}

void Tabbot::move_path (struct Movement * path, int size){
  for (int i = 0; i < size; i ++){
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
            move_robot_meters(path[i].dir, d);
          }
      }else{
        move_robot_meters(path[i].dir, path[i].m);
      }
       if (i<(size) -1 &&( (path[i].dir==forwards && path[i+1].dir == backwards) || (path[i].dir==backwards && path[i+1].dir == forwards))){
         delay(500);
       }
  }
  }
}


void Tabbot::move_path (struct Movement_encoder * path_encoder, int size){
  for (int i = 0; i < size; i ++){
    if (path_encoder[i].msg != ""){
      Serial.print("\n");
      Serial.println(path_encoder[i].msg);
      Serial.flush();

      delay(path_encoder[i].s*1000);
    }
    else{
      int d= 1; //distance to check for obstacles
      if(path_encoder[i].enc_left == 0 && path_encoder[i].enc_right == 0){ //if both encoders values are 0, stop the robot.
        move_robot_encoders(0,0, stop);
      }
      else{
	      for (double j = 0; j <= abs(path_encoder[i].enc_left); j+=d){
	        //search for obstacles
					if (d%10==0){
						search_obstacles();
					}
	        //move robot d m
	        if (path_encoder[i].enc_left == path_encoder[i].enc_right && path_encoder[i].enc_left > 0){
	          move_robot_encoders(d,d, right);
	        }
	        else if (path_encoder[i].enc_left == path_encoder[i].enc_right && path_encoder[i].enc_left < 0){
	          move_robot_encoders(-d,-d, left);
	        }
	        else if (path_encoder[i].enc_left != path_encoder[i].enc_right && path_encoder[i].enc_left < 0){
	          move_robot_encoders(-d,d, backwards);
	        }
	        else if (path_encoder[i].enc_left != path_encoder[i].enc_right && path_encoder[i].enc_left > 0){
	          move_robot_encoders(d,-d, forwards);
	        }
	      }
      }
      move_robot_encoders(0,0, stop);
      // if (i<size &&( (path_encoder[i].enc_left>0 && path_encoder[i].enc_right>0 && path_encoder[i+1].enc_left<0 && path_encoder[i+1].enc_right<0) || (path_encoder[i].enc_left<0 && path_encoder[i].enc_right<0 && path_encoder[i+1].enc_left>0 && path_encoder[i+1].enc_right>0))){
			// 	Serial.println("in delay!!");
			// 	 delay(500);
      // }
    }
  }
}


void Tabbot::move_robot_meters(int dir, double distance=1){
  int enc = 0;
  int t=20;
  String msg="";
	int speed_right = 200, speed_left = 180;
  switch (dir){
    case forwards:
      enc = degrees_left;
      moveMotor(motor_right, speed_right);
      moveMotor(motor_left, speed_left);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "FORWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case backwards:
      enc = degrees_left;
      moveMotor(motor_right, -speed_right);
      moveMotor(motor_left, -speed_left);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
      	msg = "BACKWARDS:"+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case right:
      enc = degrees_left;
      moveMotor(motor_right, -speed_right);
      moveMotor(motor_left, speed_left);
      while (abs(enc - degrees_left) < distance * 100){ //distance [m] * 100 [encoder ticks / m]
        msg = "RIGHT: "+String(abs(enc - degrees_left))+"   "+String(distance * 100);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case left:
      enc = degrees_right;
      moveMotor(motor_right, speed_right);
      moveMotor(motor_left, -speed_left);
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


void Tabbot::move_robot_encoders(int enc_left, int enc_right, int dir){
  int enc = 0;
  int t=20;
  String msg="";
	int speed_right = 200, speed_left = 163;

  switch (dir){
    case forwards:
      enc = degrees_left;
      moveMotor(motor_right, speed_right);
      moveMotor(motor_left, speed_left);
      while (abs(enc - degrees_left) < abs(enc_left)){ //Move until the total ticks number is equal to the one given
        msg = "FORWARDS:"+String(abs(enc - degrees_left))+"   "+String(enc_left);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case backwards:
      enc = degrees_left;
      moveMotor(motor_right, -speed_right);
      moveMotor(motor_left, -speed_left);
      while (abs(enc - degrees_left) < abs(enc_left)){ //Move until the total ticks number is equal to the one given
      msg = "BACKWARDS:"+String(abs(enc - degrees_left))+"   "+String(enc_left);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case right:
      enc = degrees_left;
      moveMotor(motor_right, -speed_right);
      moveMotor(motor_left, speed_left);
      while (abs(enc - degrees_left) < abs(enc_left)){ //Move until the total ticks number is equal to the one given
        msg = "RIGHT: "+String(abs(enc - degrees_left))+"   "+String(enc_left);
        //Serial.println(msg);
        delay(t);
      }
    break;
    case left:
      enc = degrees_right;
      moveMotor(motor_right, speed_right);
      moveMotor(motor_left, -speed_left);
      while (abs(enc - degrees_right) < abs(enc_right)){ ///Move until the total ticks number is equal to the one given
        msg = "LEFT: "+String(abs(enc - degrees_left))+"   "+String(enc_right);
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

void Tabbot::moveMotor (int * motor, int speed){
  if (speed >=0){ //If the speed is positive, we want to go forward
    analogWrite(motor[forwards], speed);
    analogWrite(motor[backwards], 0);
  } else{ //If the speed is negative, we want to go backwards
    analogWrite(motor[backwards], -speed);
    analogWrite(motor[forwards], 0);
  }
}
