# include "TimerOne.h"


const byte MOTOR1 = 2;  // Motor 1 Interrupt Pin
const byte MOTOR2 = 3;  // Motor 2 Interrupt Pin

const byte MOTOR3 = 18;  // Motor 3 Interrupt Pin
const byte MOTOR4 = 19;  // Motor 4 Interrupt Pin

// Float for number of slots in encoder disk
float diskslots = 20.0;  // Change to match value of encoder disk

String joystick_data; // ps4 data via bluetooth from RPI

long prev_t = 0;

volatile int target_rpm = 200;

volatile int target_input = 100;

volatile float m1_rpm;
volatile float m1_rpm_filtered;

volatile float m2_rpm;
volatile float m2_rpm_filtered;

volatile float m3_rpm;
volatile float m3_rpm_filtered;

volatile float m4_rpm;
volatile float m4_rpm_filtered;

volatile float motor1_drive_signal;
volatile float motor2_drive_signal;
volatile float motor3_drive_signal;
volatile float motor4_drive_signal;

float pid_loop_dur_us = 0.01e6;

int m1_speed;
int m2_speed;
int m3_speed;
int m4_speed;

volatile int mapped_m1_speed;
volatile int mapped_m2_speed;
volatile int mapped_m3_speed;
volatile int mapped_m4_speed;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;

const unsigned int MAX_MESSAGE_LENGTH = 20;
String global_message;
int global_int;


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

public:
    float kp;
    float ki;
    float kd;
    float setpoint;
    float error;
    float integral_error;
    float derivative_error;
    float prev_error;
    float output;

    float* int_err_history;
    uint8_t int_err_history_length;
    uint8_t int_err_history_index;
    bool curr_int_err_zero = true;

public:
    PID(float kp, float ki, float kd, float* int_err_history, uint8_t int_err_history_length) {

        this->kp = kp;
        this->ki = ki;
        this->kd = kd;

        this->error = 0;
        this->integral_error = 0;
        this->derivative_error = 0;
        this->prev_error = 0;
        this->output = 0;

        this->int_err_history = int_err_history;
        this->int_err_history_length = int_err_history_length;


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
        else if (this->integral_error < 0) this->integral_error = 0.0;
        else if (this->detect_zero_int_error()) this->integral_error = 0.0;
        //Serial.print(sensed_val);
        //Serial.print(',');

        //Serial.print(this->setpoint);
        //Serial.print(',');

        //Serial.print(this->error);
        //Serial.print(',');
        ////      
        //Serial.print(this->output);
        //Serial.print(',');

        //Serial.print(delta_t);
        //Serial.print(',');

        //Serial.print(this->integral_error);
        //Serial.print(',');

        //        Serial.print(this->derivative_error);
        //        Serial.print(',');

//        Serial.println(m1_rpm);

        return this->output;
    }


    bool detect_zero_int_error() {
        // ISR timerone setting int_err to 0 when wheel is stopped method
        this->curr_int_err_zero = true;
        // set int_err to 0 when constant int err is observed
        if (this->int_err_history_index > (this->int_err_history_length - 1)) {
            this->int_err_history_index = 0;
        }

        this->int_err_history[this->int_err_history_index] = this->integral_error;
        this->int_err_history_index++;

        for (unsigned i = 0; i < this->int_err_history_length - 1; i++) {
            if (this->int_err_history[i] != this->int_err_history[0]) {
                this->curr_int_err_zero = false;
            }
        }

        return this->curr_int_err_zero;

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

class SpeedSensor {
public:

    uint8_t int_pin;
    void (*ISR_func)();

    long curr_t, prev_t = 0;
    float delta_t = 0.0;
    float diskslots;

    float* speed_history;
    uint8_t speed_history_length;
    uint8_t speed_history_index;
    bool curr_speed_zero = true;

    float curr_speed = 0.0;

    SpeedSensor(uint8_t int_pin, void ISR_func(), float diskslots, float* speed_history, uint8_t speed_history_length) {
        this->int_pin = int_pin;
        this->ISR_func = ISR_func;
        this->attach_int();

        this->diskslots = diskslots;

        this->speed_history = speed_history;
        this->speed_history_length = speed_history_length;

    }

    void detach_int() {
        detachInterrupt(digitalPinToInterrupt(this->int_pin));
    }

    void attach_int() {
        attachInterrupt(digitalPinToInterrupt(this->int_pin), this->ISR_func, RISING);
    }

    float calculate_speed() {
        // ISR speed calculation method

        // time difference      
        this->curr_t = micros();
        this->delta_t = ((float)(this->curr_t - this->prev_t)) / (1.0e3);
        this->prev_t = this->curr_t;

        //  m1_rpm = (diskslots/delta_t);

        this->curr_speed = (60.0 / (this->delta_t * this->diskslots)) * 1.0e3;

        return this->curr_speed;

    }

    bool detect_zero_speed() {
        // ISR timerone setting speed to 0 when wheel is stopped method
        this->curr_speed_zero = true;
        // set speed to 0 when wheel is no longer rotating
        if (this->speed_history_index > (this->speed_history_length - 1)) {
            this->speed_history_index = 0;
        }

        this->speed_history[this->speed_history_index] = this->curr_speed;
        this->speed_history_index++;

        for (unsigned i = 0; i < this->speed_history_length - 1; i++) {
            if (this->speed_history[i] != this->speed_history[0]) {
                this->curr_speed_zero = false;
            }
        }

        return this->curr_speed_zero;

    }

};


//float fir_coeff1[] = {1,1,1,1, 1,1,1,1};
float fir_coeff1[] = { 0.0363355740541436573,0.0615978543451880037,0.0877507540054860286,0.110613759704731765,0.126230094818408495,0.1317764226854167,0.126230094818408495,0.110613759704731765,0.0877507540054860286,0.0615978543451880037,0.0363355740541436573 };

//float fir_coeff1[] = { 1,1};
uint8_t size_fir1 = sizeof(fir_coeff1) / sizeof(float);

float* fir_buf1 = new float[size_fir1];
float* fir_buf2 = new float[size_fir1];
float* fir_buf3 = new float[size_fir1];
float* fir_buf4 = new float[size_fir1];

FIR fir1(size_fir1, fir_coeff1, fir_buf1);
FIR fir2(size_fir1, fir_coeff1, fir_buf2);
FIR fir3(size_fir1, fir_coeff1, fir_buf3);
FIR fir4(size_fir1, fir_coeff1, fir_buf4);

float s1_int_err_his[] = { 0,0,0,0,0,0,0,0 };
float s2_int_err_his[] = { 0,0,0,0,0,0,0,0 };
float s3_int_err_his[] = { 0,0,0,0,0,0,0,0 };
float s4_int_err_his[] = { 0,0,0,0,0,0,0,0 };


//PID pid1(1, 0.9, 2.624);
//PID pid1(0.0296135794214367, 0.217772079012672, 0.00100674532075656);
//PID pid1(1.49354981921043, 11.8189671332204, 0.0471845601506406);
//PID pid1(0.733704837954405, 5.63837220103501, 0.0238687146770339);

PID pid1(0.260526977432438, 1.41893450525107, 0.0106458398336518, s1_int_err_his, 8);  // this is pretty good with the 10 coeffs lpf
PID pid2(0.260526977432438, 1.41893450525107, 0.0106458398336518, s2_int_err_his, 8);  // this is pretty good with the 10 coeffs lpf
PID pid3(0.260526977432438, 1.41893450525107, 0.0106458398336518, s3_int_err_his, 8);  // this is pretty good with the 10 coeffs lpf
PID pid4(0.260526977432438, 1.41893450525107, 0.0106458398336518, s4_int_err_his, 8);  // this is pretty good with the 10 coeffs lpf

//PID pid1(0.81339, 4.414, 0.01356);

//PID pid1(0.5029, 2.352, 0.02946);

//PID pid1(0.4064, 1.813, 0.02151); // this is pretty good as well

void ISR_count1();
void ISR_count2();
void ISR_count3();
void ISR_count4();

float s1_spd_his[] = { 0,0,0,0,0,0,0,0 };
float s2_spd_his[] = { 0,0,0,0,0,0,0,0 };
float s3_spd_his[] = { 0,0,0,0,0,0,0,0 };
float s4_spd_his[] = { 0,0,0,0,0,0,0,0 };

SpeedSensor sensor1(2, ISR_count1, 20.0, s1_spd_his, 8);
SpeedSensor sensor2(3, ISR_count2, 20.0, s2_spd_his, 8);
SpeedSensor sensor3(18, ISR_count3, 20.0, s3_spd_his, 8);
SpeedSensor sensor4(19, ISR_count4, 20.0, s4_spd_his, 8);

// Interrupt Service Routines
void ISR_count1()
{
    m1_rpm = sensor1.calculate_speed();
}

// Motor 2 pulse count ISR
void ISR_count2()
{
    m2_rpm = sensor2.calculate_speed();
}

// Motor 3 pulse count ISR
void ISR_count3()
{
    m3_rpm = sensor3.calculate_speed();
}

// Motor 4 pulse count ISR
void ISR_count4()
{
    m4_rpm = sensor4.calculate_speed();
}


// TimerOne ISR
void ISR_timerone()
{
    //    Timer1.detachInterrupt();  // Stop the timer
    // this ISR is doing a lot of stuff! Need to move this to main loop
    m1_rpm_filtered = fir1.FIR_filter_update_c(m1_rpm);
    m2_rpm_filtered = fir2.FIR_filter_update_c(m2_rpm);
    m3_rpm_filtered = fir3.FIR_filter_update_c(m3_rpm);
    m4_rpm_filtered = fir4.FIR_filter_update_c(m4_rpm);

    //    m1_rpm_filtered = sensor1.curr_speed;
    //    m1_rpm_filtered = fir2.FIR_filter_update_c(sensor1.curr_speed);   // sensor1.curr_speed becomes 0 every 4 or so samples, seems that accessing the object's attribute doesn't work too well. Temp solution is to use global var

    if (sensor1.detect_zero_speed()) {
        m1_rpm = 0.0;
    }
    if (sensor2.detect_zero_speed()) {
        m2_rpm = 0.0;
    }
    if (sensor3.detect_zero_speed()) {
        m3_rpm = 0.0;
    }
    if (sensor4.detect_zero_speed()) {
        m4_rpm = 0.0;
    }

    // time difference
    long curr_t = micros();
    float delta_t = ((float)(curr_t - prev_t)) / (1.0e6);
    prev_t = curr_t;

    mapped_m1_speed = map(constrain(abs(m1_speed), 0, 100), 0, 100, 0, 300);
    mapped_m2_speed = map(constrain(abs(m2_speed), 0, 100), 0, 100, 0, 300);
    mapped_m3_speed = map(constrain(abs(m3_speed), 0, 100), 0, 100, 0, 300);
    mapped_m4_speed = map(constrain(abs(m4_speed), 0, 100), 0, 100, 0, 300);

    motor1_drive_signal = pid1.compute(m1_rpm_filtered, mapped_m1_speed, delta_t);
    motor2_drive_signal = pid2.compute(m2_rpm_filtered, mapped_m2_speed, delta_t);
    motor3_drive_signal = pid3.compute(m3_rpm_filtered, mapped_m3_speed, delta_t);
    motor4_drive_signal = pid4.compute(m4_rpm_filtered, mapped_m4_speed, delta_t);

    if (m1_speed == 0) motor1_drive_signal = 0.0 * motor1_drive_signal;
    if (m2_speed == 0) motor2_drive_signal = 0.0 * motor2_drive_signal;
    if (m3_speed == 0) motor3_drive_signal = 0.0 * motor3_drive_signal;
    if (m4_speed == 0) motor4_drive_signal = 0.0 * motor4_drive_signal;

    if (m1_speed < 0) motor1_drive_signal = -1.0 * motor1_drive_signal;
    if (m2_speed < 0) motor2_drive_signal = -1.0 * motor2_drive_signal;
    if (m3_speed < 0) motor3_drive_signal = -1.0 * motor3_drive_signal;
    if (m4_speed < 0) motor4_drive_signal = -1.0 * motor4_drive_signal;

    //    motor1.move(motor1_drive_signal, 0.0);
    //    motor2.move(motor2_drive_signal, 0.0);
    //    motor3.move(motor3_drive_signal, 0.0);
    //    motor4.move(motor4_drive_signal, 0.0);

    //    motor1_drive_signal = pid1.compute(m1_rpm_filtered, target_rpm, delta_t);
    //    motor2_drive_signal = pid2.compute(m2_rpm_filtered, target_rpm, delta_t);
    //    motor3_drive_signal = pid3.compute(m3_rpm_filtered, target_rpm, delta_t);
    //    motor4_drive_signal = pid4.compute(m4_rpm_filtered, target_rpm, delta_t);
    //
    //    motor1.forward(motor1_drive_signal, 0.0);
    //    motor2.forward(motor2_drive_signal, 0.0);
    //    motor3.forward(motor3_drive_signal, 0.0);
    //    motor4.forward(motor4_drive_signal, 0.0);

    //    Timer1.attachInterrupt(ISR_timerone);  // Enable the timer
}


void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        //Serial.print("This just in ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}

void setup() {
    Serial.begin(115200);

    //    // for moving average filter
    //    for (uint8_t n = 0; n < size_fir1; n++) {
    //        fir_coeff1[n] = fir_coeff1[n] / size_fir1;
    //    }

    Timer1.initialize(pid_loop_dur_us); // set timer for 1sec

    Timer1.attachInterrupt(ISR_timerone); // Enable the timer

    //Serial.println("Target MotorDrive Current");
    //Serial.println("SenseVal,SetPoint,Error,Output,deltaT,m1_rpm");
    //Serial.println("SenseVal,SetPoint,Error,Output,deltaT,intError,m1_rpm");
    //Serial.println("SenseVal,SetPoint,deltaT,m1_rpm");
    //Serial.println("m1,m2,m3,m4");
}


void read_data_from_RPI() {
    while (Serial.available()){
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    char inByte = Serial.read();

    if (inByte != '\n') {
      // do something
      message[message_pos] = inByte;
      message_pos++;
    }
    else {
      // do something else
      message[message_pos] = '\0';
      Serial.println(message);
      message_pos = 0;
//      global_message = message;
      joystick_data = message;
//      Serial.println(global_message);
    }
  }
}

void loop() {

    //    while (Serial.available()) {
    //        target_input = Serial.readStringUntil('\n').toInt();
    //    }

    //    Serial.print(m1_rpm_filtered);
    ////    Serial.print(',');
    ////    Serial.print(pid1.error);
    ////    Serial.println();
    //    Serial.print(',');
    //    Serial.print(m2_rpm_filtered);
    //    Serial.print(',');
    //    Serial.print(m3_rpm_filtered);
    //    Serial.print(',');
    //    Serial.print(m4_rpm_filtered);
    //
    //    Serial.print(',');
    //    Serial.print(pid1.output);
    //    Serial.print(',');
    //    Serial.print(pid2.output);
    //    Serial.print(',');
    //    Serial.print(pid3.output);
    //    Serial.print(',');
    //    Serial.print(pid4.output);
    //    Serial.println();

//    // ***************************** uncomment here for normal operation  ***************************** 
//
//    sensor1.detach_int();
//    sensor2.detach_int();
//    sensor3.detach_int();
//    sensor4.detach_int();
//    Timer1.detachInterrupt();  // Stop the timer
//
//    joystick_data = Serial.readStringUntil('\n');
//    //  Serial.print("You sent me");
//    Serial.println(joystick_data);
//
////    recvWithEndMarker();
////    showNewData();
//
////    joystick_data = receivedChars;
//
//    sensor1.attach_int();
//    sensor2.attach_int();
//    sensor3.attach_int();
//    sensor4.attach_int();
//    Timer1.attachInterrupt(ISR_timerone);  // Enable the timer
//
//    String data1 = joystick_data.substring(0, 4);
//    String data2 = joystick_data.substring(5, 9);
//    String data3 = joystick_data.substring(10, 14);
//    //  String data4 = joystick_data.substring(15, 19);
//
//    int vy = data1.toInt();
//    int vx = data2.toInt();
//
//    int turn = data3.toInt();
//
//    motor1.move(vy - vx, 0.0);
//    motor2.move(vy + vx, 0.0);
//    motor3.move(vy - vx, 0.0);
//    motor4.move(vy + vx, 0.0);
//
//    if (turn != 0) {
//
//        rotate(turn);
//    }
//
//    // ***************************** uncomment here for normal operation  ***************************** 

    //***************************** uncomment here for normal operation*****************************

    sensor1.detach_int();
    sensor2.detach_int();
    sensor3.detach_int();
    sensor4.detach_int();
    Timer1.detachInterrupt();  // Stop the timer
    
    joystick_data = Serial.readStringUntil('\n');
    //  Serial.print("You sent me");
    Serial.println(joystick_data);

//    recvWithStartEndMarkers();
//    showNewData();
//    joystick_data = receivedChars;


//    recvWithEndMarker();
//    showNewData();
//    joystick_data = receivedChars;

//    read_data_from_RPI();
    
    sensor1.attach_int();
    sensor2.attach_int();
    sensor3.attach_int();
    sensor4.attach_int();
    Timer1.attachInterrupt(ISR_timerone);  // Enable the timer
    
    
    String data1 = joystick_data.substring(0, 4);
    String data2 = joystick_data.substring(5, 9);
    String data3 = joystick_data.substring(10, 14);
    String data4 = joystick_data.substring(15, 19);

    int vy = data1.toInt();
    int vx = data2.toInt();
  
    int turn = data3.toInt();

    int all_forward = data4.toInt();

    noInterrupts();

//    Serial.print(mapped_m1_speed);
//    Serial.print(',');
//    Serial.print(mapped_m2_speed);
//    Serial.print(',');
//    Serial.print(mapped_m3_speed);
//    Serial.print(',');
//    Serial.print(mapped_m4_speed);
//    Serial.println();

    
      m1_speed = vy - vx;
      m2_speed = vy + vx;
      m3_speed = vy - vx;
      m4_speed = vy + vx;

//    m1_speed = target_input;
//    m2_speed = target_input;
//    m3_speed = target_input;
//    m4_speed = target_input;
    
    if (all_forward != 0) {
      m1_speed = all_forward;
      m2_speed = all_forward;
      m3_speed = all_forward;
      m4_speed = all_forward;
    }
    
    motor1.move(motor1_drive_signal, 0.0);
    motor2.move(motor2_drive_signal, 0.0);
    motor3.move(motor3_drive_signal, 0.0);
    motor4.move(motor4_drive_signal, 0.0);

//    motor1.move(target_input, 0.0);
//    motor2.move(target_input, 0.0);
//    motor3.move(target_input, 0.0);
//    motor4.move(target_input, 0.0);

    interrupts();



    if (turn != 0) {

        rotate(turn);
    }

    // ***************************** uncomment here for normal operation  ***************************** 


}
