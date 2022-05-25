//#include <iostream>
#include <Arduino.h>
#include "state.h"

//using namespace std;
# define STR_LENGTH 20

char stopped_state_name[] = "stopped_state";
Stopped stopped_state(stopped_state_name);

char active_state_name[] = "active_state";
Active active_state(active_state_name);

char moving_forward_state_name[] = "moving_forward_state";
MovingForward moving_forward_state(moving_forward_state_name);

char moving_backward_state_name[] = "moving_backward_state";
MovingBackward moving_backward_state(moving_backward_state_name);

//extern int m1_speed;
//extern int m2_speed;
//extern int m3_speed;
//extern int m4_speed;


extern char NAME[];
extern char GO[];
extern char STOP[];
extern char BACK[];

//String NAME_s = String(NAME);
//String GO_s = String(GO);
//String STOP_s = String(STOP);
//String BACK_s = String(BACK);

int strlength(char* s) {
  int c = 0;
  while (*s != '\0')
  {
    c++;
    s++;
  }
  return c;
}



State::State(char* name) {
  this->name = name;
  //	cout << "Created state: ";
  //	print_name(this->name);
}

void State::print_name(char* name) {
  //name = this->name;
  //	for (int i = 0; i < strlength(name); ++i)
  //	{
  //		cout << name[i];
  //	}
  //	cout << endl;
}

//void State::perform_action() {
//	cout << "Performing state: ";
//	print_name(this->name);
//
//}

State* State::on_event(char* event) {
  //	cout << "Handling event: ";
  //	print_name(event);
  return this;

}
//void State::perform_action(void) {
//
//}


// Derived class

Stopped::Stopped(char* name) : State(name) {
}

State* Stopped::on_event(char* event) {
  if (strcmp(event, NAME) == 0) {
    //		cout << "return active state\n";

    return &active_state;
  }
  else {
    return this;
  }

}
void Stopped::perform_action(int* motor_speeds_gbl) {
  //	cout << "Performing state: ";
  motor_speeds_gbl[0] = 0;
  motor_speeds_gbl[1] = 0;
  motor_speeds_gbl[2] = 0;
  motor_speeds_gbl[3] = 0;

}


Active::Active(char* name) : State(name) {
}

State* Active::on_event(char* event) {
  if (strcmp(event, GO) == 0) {
    //		cout << "return moving forward state\n";
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    //    cout << "return moving forward state\n";
    return &moving_backward_state;
  }
  else {
    return this;
  }

}
void Active::perform_action(int* motor_speeds_gbl) {
  //	cout << "Performing state: ";
  //	print_name(this->name);
  motor_speeds_gbl[0] = 0;
  motor_speeds_gbl[1] = 0;
  motor_speeds_gbl[2] = 0;
  motor_speeds_gbl[3] = 0;

}


MovingForward::MovingForward(char* name) : State(name) {
}

State* MovingForward::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else {
    return this;
  }

}
void MovingForward::perform_action(int* motor_speeds_gbl) {
  //	cout << "Performing state: ";
  //	print_name(this->name);
  motor_speeds_gbl[0] = 30;
  motor_speeds_gbl[1] = 30;
  motor_speeds_gbl[2] = 30;
  motor_speeds_gbl[3] = 30;
}

MovingBackward::MovingBackward(char* name) : State(name) {
}

State* MovingBackward::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if (strcmp(event, GO) == 0) {
    return &moving_forward_state;
  }
  else {
    return this;
  }

}
void MovingBackward::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = -30;
  motor_speeds_gbl[1] = -30;
  motor_speeds_gbl[2] = -30;
  motor_speeds_gbl[3] = -30;
}
