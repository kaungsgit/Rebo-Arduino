# include "TimerOne.h"


const byte MOTOR1 = 2;  // Motor 1 Interrupt Pin - INT 0
const byte MOTOR2 = 3;  // Motor 2 Interrupt Pin - INT 1

const byte MOTOR3 = 18;  // Motor 1 Interrupt Pin - INT 0
const byte MOTOR4 = 19;  // Motor 2 Interrupt Pin - INT 1


//// Integers for pulse counters
//unsigned int counter1 = 0;
//unsigned int counter2 = 0;

// Float for number of slots in encoder disk
float diskslots = 20.0;  // Change to match value of encoder disk

String joystick_data; // ps4 data via bluetooth from RPI

long prev_t = 0;

volatile int target_rpm = 200;

float sensed_rotation1;
float sensed_rotation2;

float m1_rpm;
float m1_rpm_filtered;

float motor_drive_signal;

float pid_loop_dur_us = 0.01e6;

const float alpha = 0.5;
float data_filtered[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int n = 7;

long curr_t = 0;
long prev_t1 = 0;
float delta_t = 0.0;
float data_filtered1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2 };
const int n1 = 7;

float m1_rpm_tag[] = { 0, 0, 1, 0, 0, 0, 1, 0 };
int n2 = 0;

bool everything_equal = true;


class Motor {
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
        speed_pct = speed_pct * 2.55;
        if (speed_pct > 255) speed_pct = 255;
        else if (speed_pct < -255) speed_pct = -255;

        if (speed_pct > 0) {
            digitalWrite(this->in1, LOW);
            digitalWrite(this->in2, HIGH);
            analogWrite(this->ena, abs(speed_pct));
        }
        else if (speed_pct < 0) {
            digitalWrite(this->in1, HIGH);
            digitalWrite(this->in2, LOW);
            analogWrite(this->ena, abs(speed_pct));
        }
        else {
            analogWrite(this->ena, 0);
        }
        delay(time_dur);
    }

    void forward(float speed_value, float time_dur) {
        digitalWrite(this->in1, LOW);
        digitalWrite(this->in2, HIGH);
        analogWrite(this->ena, abs(speed_value));
    }

    void stop() {
        analogWrite(this->ena, 0);
    }
};

// Motor pin assigments
//// uno pin assigment
//Motor motor1(3, 11, 10);
//Motor motor2(5, 2, 4);
//Motor motor3(6, 7, 8);
//Motor motor4(9, 12, 13);

// mega pin assigment
Motor motor1(4, 22, 23);
Motor motor2(5, 25, 24);
Motor motor3(6, 26, 27);
Motor motor4(7, 28, 29);

void rotate(float turn_speed) {

    turn_speed = turn_speed * 0.5;
    if (turn_speed > 255) turn_speed = 255;
    else if (turn_speed < -255) turn_speed = -255;

    motor1.move(turn_speed, 0);
    motor2.move(-turn_speed, 0);
    motor3.move(-turn_speed, 0);
    motor4.move(turn_speed, 0);
}


class PID {

private:
    float kp;
    float ki;
    float kd;
    float setpoint;
    float error;
    float integral_error;
    float derivative_error;
    float prev_error;
    float output;

public:
    PID(float kp, float ki, float kd) {

        this->kp = kp;
        this->ki = ki;
        this->kd = kd;

        this->error = 0;
        this->integral_error = 0;
        this->derivative_error = 0;
        this->prev_error = 0;
        this->output = 0;

    }

    float compute(float sensed_val, float target, float delta_t) {
        this->setpoint = target;
        this->error = this->setpoint - sensed_val;
        this->integral_error += this->error * delta_t;
        this->derivative_error = (this->error - this->prev_error) / delta_t;
        this->prev_error = this->error;
        this->output = this->kp * this->error + this->ki * this->integral_error + this->kd * this->derivative_error;

        if (this->output > 255.0) this->output = 255.0;
        else if (this->output < 0.0) this->output = 0.0;

        if (this->integral_error > 170.0) this->integral_error = 170.0;
        else if (this->integral_error < -170.0) this->integral_error = -170.0;

        Serial.print(sensed_val);
        Serial.print(',');

        Serial.print(this->setpoint);
        Serial.print(',');

        Serial.print(this->error);
        Serial.print(',');
        //      
        Serial.print(this->output);
        Serial.print(',');

        Serial.print(delta_t);
        Serial.print(',');

        Serial.print(this->integral_error);
        Serial.print(',');

//        Serial.print(this->derivative_error);
//        Serial.print(',');

        Serial.println(m1_rpm);

        return this->output;
    }
};


class FIR {
    // https://www.youtube.com/watch?v=uNNNj9AZisM&t=1307s&ab_channel=Phil%E2%80%99sLab
public:
    uint8_t fir_filter_length;
    float* fir_impulse_response;
    float* buf;
    uint8_t buf_index = 0;
    float out = 0.0f;

    FIR(uint8_t fir_filter_length, float* fir_impulse_response, float* buf) {
        this->fir_filter_length = fir_filter_length;
        this->fir_impulse_response = fir_impulse_response;
        this->buf = buf;

        for (uint8_t n = 0; n < this->fir_filter_length;n++) {
            this->buf[n] = 0.0f;
        }

        this->buf_index = 0;

        this->out = 0.0f;

    }

    float FIR_filter_update_c(float inp) {
        this->buf[this->buf_index] = inp;

        this->buf_index++;

        if (this->buf_index == this->fir_filter_length) {
            this->buf_index = 0;
        }

        this->out = 0.0f;
        uint8_t sum_index = this->buf_index;
        for (uint8_t n = 0; n < this->fir_filter_length; n++) {

            if (sum_index > 0) {
                sum_index--;
            }
            else {
                sum_index = this->fir_filter_length - 1;
            }

            this->out += this->fir_impulse_response[n] * this->buf[sum_index];
        }
        return this->out;

    }

};


//float fir_coeff1[] = {1,1,1,1, 1,1,1,1};
float fir_coeff1[]={0.0363355740541436573,0.0615978543451880037,0.0877507540054860286,0.110613759704731765,0.126230094818408495,0.1317764226854167,0.126230094818408495,0.110613759704731765,0.0877507540054860286,0.0615978543451880037,0.0363355740541436573};

//float fir_coeff1[] = { 1,1};
uint8_t size_fir1 = sizeof(fir_coeff1) / sizeof(float);

float* fir_buf1 = new float[size_fir1];
float* fir_buf2 = new float[size_fir1];
float* fir_buf3 = new float[size_fir1];

FIR fir1(size_fir1, fir_coeff1, fir_buf1);
FIR fir2(size_fir1, fir_coeff1, fir_buf2);
FIR fir3(size_fir1, fir_coeff1, fir_buf3);

//PID pid1(1, 0.9, 2.624);
//PID pid1(0.0296135794214367, 0.217772079012672, 0.00100674532075656);
//PID pid1(1.49354981921043, 11.8189671332204, 0.0471845601506406);
//PID pid1(0.733704837954405, 5.63837220103501, 0.0238687146770339);

PID pid1(0.260526977432438, 1.41893450525107, 0.0106458398336518);  // this is pretty good with the 10 coeffs lpf
//PID pid1(0.81339, 4.414, 0.01356);

//PID pid1(0.5029, 2.352, 0.02946);

//PID pid1(0.4064, 1.813, 0.02151); // this is pretty good as well

// Interrupt Service Routines
// Motor 1 pulse count ISR
void ISR_count1()
{
//    counter1++;  // increment Motor 1 counter value

    // time difference
    curr_t = micros();
    delta_t = ((float)(curr_t - prev_t1)) / (1.0e3);
    prev_t1 = curr_t;

    //  m1_rpm = (diskslots/delta_t);

    m1_rpm = (60.0 / (delta_t * diskslots)) * 1.0e3;

//    m1_rpm = fir2.FIR_filter_update_c(m1_rpm);

}

// Motor 2 pulse count ISR
void ISR_count2()
{
//    counter2++;  // increment Motor 2 counter value
}



// TimerOne ISR
void ISR_timerone()
{
    Timer1.detachInterrupt();  // Stop the timer

//    sensed_rotation1 = (counter1 / diskslots) * 60.00 * (1.0e6 / pid_loop_dur_us);  // calculate RPM for Motor 1
//    counter1 = 0;  //  reset counter to zero


//    sensed_rotation1 = fir1.FIR_filter_update_c(sensed_rotation1);

    //  sensed_rotation2 = (counter2 / diskslots) * 60.00 * (1.0e6/pid_loop_dur_us);  // calculate RPM for Motor 2
    //  counter2 = 0;  //  reset counter to zero

    m1_rpm_filtered = fir2.FIR_filter_update_c(m1_rpm);

    
    if (n2 > (((sizeof(m1_rpm_tag)) / 4) - 1)) {
        n2 = 0;
    }

    m1_rpm_tag[n2] = m1_rpm;
    n2++;

    everything_equal = true;
    for (unsigned i = 0; i < (sizeof(m1_rpm_tag) / 4) - 1; i++) {
        if (m1_rpm_tag[i] != m1_rpm_tag[0]) {
            everything_equal = false;
        }
    }


    if (everything_equal) m1_rpm = 0;



    // time difference
    long curr_t = micros();
    float delta_t = ((float)(curr_t - prev_t)) / (1.0e6);
    prev_t = curr_t;

    //  motor_drive_signal = map(pid1.compute(sensed_rotation1, target_rpm, delta_t), 0, 350, 0, 255);

    motor_drive_signal = pid1.compute(m1_rpm_filtered, target_rpm, delta_t);

//    motor_drive_signal = fir3.FIR_filter_update_c(motor_drive_signal);
    
    motor1.forward(motor_drive_signal, 0.0);

    //  motor1.move(0, 0);

    Timer1.attachInterrupt(ISR_timerone);  // Enable the timer
}

void setup() {
    Serial.begin(115200);
    //  TCCR1A = 0;
    //  TCCR1B = bit(CS10);

//    // for moving average filter
//    for (uint8_t n = 0; n < size_fir1; n++) {
//        fir_coeff1[n] = fir_coeff1[n] / size_fir1;
//    }

    Timer1.initialize(pid_loop_dur_us); // set timer for 1sec
    attachInterrupt(digitalPinToInterrupt(MOTOR1), ISR_count1, RISING);  // Increase counter 1 when speed sensor pin goes High
    attachInterrupt(digitalPinToInterrupt(MOTOR2), ISR_count2, RISING);  // Increase counter 2 when speed sensor pin goes High
    Timer1.attachInterrupt(ISR_timerone); // Enable the timer

  //  Serial.println("Target MotorDrive Current");
//    Serial.println("SenseVal,SetPoint,Error,Output,deltaT,m1_rpm");
    Serial.println("SenseVal,SetPoint,Error,Output,deltaT,intError,m1_rpm");
//    Serial.println("SenseVal,SetPoint,deltaT,m1_rpm");
}

void loop() {
    
    while (Serial.available()) {
        target_rpm = Serial.readStringUntil('\n').toInt();
    }

//    motor1.forward(target_rpm, 0.0);

//    target_rpm = target_rpm + 1;
//    delay(50);
    
    
//  Serial.print(target_rpm);
//  Serial.print(',');
//  Serial.print(motor_drive_signal);
//  Serial.print(',');
//  Serial.println(sensed_rotation1);

//  motor1.move(0.0, 0.0);
//  motor2.move(0.0, 0.0);
//  motor3.move(0.0, 0.0);
//  motor4.move(0.0, 0.0);

//  motor1.move(90, 0.0);
//  motor2.move(90, 0.0);
//  motor3.move(90, 0.0);
//  motor4.move(90, 0.0);

//// ***************************** uncomment here for normal operation  ***************************** 
//  joystick_data = Serial.readStringUntil('\n');
//  
////  Serial.print("You sent me");
//  Serial.println(joystick_data);
//  
//  String data1 = joystick_data.substring(0, 4);
//  String data2 = joystick_data.substring(5, 9);
//  String data3 = joystick_data.substring(10, 14);
////  String data4 = joystick_data.substring(15, 19);
//  
//  int vy = data1.toInt();
//  int vx = data2.toInt();
//  
//  int turn = data3.toInt();
//
//  motor1.move(vy-vx, 0.0);
//  motor2.move(vy+vx, 0.0);
//  motor3.move(vy-vx, 0.0);
//  motor4.move(vy+vx, 0.0);
//  
//  if (turn != 0) {
//  
//    rotate(turn);
//  }
//
//// ***************************** uncomment here for normal operation  ***************************** 

}
