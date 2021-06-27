



class Motor{
  private:
    byte ena;
    byte in1; 
    byte in2;
  public:
    Motor(byte ena, byte in1, byte in2) {
    
      this->ena = ena;
      this->in1 = in1;
      this->in2 = in2;
      
      pinMode(this->ena, OUTPUT);
      pinMode(this->in1, OUTPUT);
      pinMode(this->in2, OUTPUT);
    }
  
  
    void move(float speed_pct, float time_dur) {
//      speed_pct = speed_pct*100;
      speed_pct = speed_pct*2.55;
      if (speed_pct>255) speed_pct=255;
      else if (speed_pct<-255) speed_pct=-255;
      
      if (speed_pct>0) {
        digitalWrite(this->in1, LOW);
        digitalWrite(this->in2, HIGH);
        analogWrite(this->ena, abs(speed_pct));
        
      }
      
      else if (speed_pct<0) {
        digitalWrite(this->in1, HIGH);
        digitalWrite(this->in2, LOW);
        analogWrite(this->ena, abs(speed_pct));
        
      }
      
      else {
        analogWrite(this->ena, 0); 
      }
      
      delay(time_dur);
    }
    
    void stop() {
      analogWrite(this->ena, 0); 
    }
  
  
};

Motor motor1(3, 11, 10);
Motor motor2(5, 2, 4);
Motor motor3(6, 7, 8);
Motor motor4(9, 12, 13);

void rotate(float turn_speed) {
  
  turn_speed = turn_speed*0.5;
  if (turn_speed>255) turn_speed=255;
  else if (turn_speed<-255) turn_speed=-255;
  
  motor1.move(turn_speed, 0);
  motor2.move(-turn_speed, 0);
  motor3.move(-turn_speed, 0);
  motor4.move(turn_speed, 0);
}

void setup() {
  Serial.begin(9600);

}

void loop() {
//  motor1.move(0.0, 0.0);
//  motor2.move(0.0, 0.0);
//  motor3.move(0.0, 0.0);
//  motor4.move(0.0, 0.0);
  
//  motor1.move(90, 0.0);
//  motor2.move(90, 0.0);
//  motor3.move(90, 0.0);
//  motor4.move(90, 0.0);


  String data = "None";
//  
////  while (Serial.available() > 0) {
//    
//    
  data = Serial.readStringUntil('\n');

//  Serial.print("You sent me: ");
//  Serial.println(data);

  String data1 = data.substring(0, 4);
  String data2 = data.substring(5, 9);
  String data3 = data.substring(10, 14);
//  String data4 = data.substring(15, 19);
  
  int vy = data1.toInt();
  int vx = data2.toInt();
  
  int turn = data3.toInt();
//  int right = data2.toInt();

  motor1.move(vy-vx, 0.0);
  motor2.move(vy+vx, 0.0);
  motor3.move(vy-vx, 0.0);
  motor4.move(vy+vx, 0.0);
  
  if (turn != 0) {
  
    rotate(turn);
  }

  
//  }
}
