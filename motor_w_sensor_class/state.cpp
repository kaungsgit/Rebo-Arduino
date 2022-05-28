//#include <iostream>
#include <Arduino.h>
#include "state.h"

char stopped_state_name[] = "stopped_state";
Stopped stopped_state(stopped_state_name);

char inactive_state_name[] = "inactive_state";
Inactive inactive_state(inactive_state_name);

char active_state_name[] = "active_state";
Active active_state(active_state_name);

char moving_forward_state_name[] = "moving_forward_state";
MovingForward moving_forward_state(moving_forward_state_name);

char moving_backward_state_name[] = "moving_backward_state";
MovingBackward moving_backward_state(moving_backward_state_name);

char moving_left_state_name[] = "moving_left_state";
MovingLeft moving_left_state(moving_left_state_name);

char moving_right_state_name[] = "moving_right_state";
MovingRight moving_right_state(moving_right_state_name);

char spinning_state_name[] = "spinning_state";
Spinning spinning_state(spinning_state_name);

extern char NAME[];
extern char GO[];
extern char GO1[];
extern char STOP[];
extern char BACK[];
extern char LEFT[];
extern char RIGHT[];
extern char SPIN[];
extern char OFF[];

int strlength(char *s) {
  int c = 0;
  while (*s != '\0') {
    c++;
    s++;
  }
  return c;
}

State::State(char *name) {
  this->name = name;
}

void State::print_name(char *name) {
}

State* State::on_event(char *event) {
  return this;

}

Inactive::Inactive(char *name) :
  State(name) {
}

State* Inactive::on_event(char *event) {
  if (strcmp(event, NAME) == 0) {
    return &active_state;
  } else {
    return this;
  }

}
void Inactive::perform_action(int *motor_speeds_gbl) {
  //  cout << "Performing state: ";
  motor_speeds_gbl[0] = 0;
  motor_speeds_gbl[1] = 0;
  motor_speeds_gbl[2] = 0;
  motor_speeds_gbl[3] = 0;

  motor_speeds_gbl[4] = 0;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 0;


}


// Concrete states
Stopped::Stopped(char *name) :
  State(name) {
}

State* Stopped::on_event(char *event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void Stopped::perform_action(int *motor_speeds_gbl) {
  //  cout << "Performing state: ";
  motor_speeds_gbl[0] = 0;
  motor_speeds_gbl[1] = 0;
  motor_speeds_gbl[2] = 0;
  motor_speeds_gbl[3] = 0;

  motor_speeds_gbl[4] = 0;
  motor_speeds_gbl[5] = 1;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 0;

}


Active::Active(char* name) : State(name) {
}

State* Active::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void Active::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = 0;
  motor_speeds_gbl[1] = 0;
  motor_speeds_gbl[2] = 0;
  motor_speeds_gbl[3] = 0;

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 0;


}


MovingForward::MovingForward(char* name) : State(name) {
}

State* MovingForward::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void MovingForward::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = 30;
  motor_speeds_gbl[1] = 30;
  motor_speeds_gbl[2] = 30;
  motor_speeds_gbl[3] = 30;

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 1;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 0;

}

MovingBackward::MovingBackward(char* name) : State(name) {
}

State* MovingBackward::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
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

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 1;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 0;
}

MovingLeft::MovingLeft(char* name) : State(name) {
}

State* MovingLeft::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void MovingLeft::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = +30;
  motor_speeds_gbl[1] = -30;
  motor_speeds_gbl[2] = +30;
  motor_speeds_gbl[3] = -30;

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 1;
  motor_speeds_gbl[9] = 0;
}

MovingRight::MovingRight(char* name) : State(name) {
}

State* MovingRight::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void MovingRight::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = -30;
  motor_speeds_gbl[1] = +30;
  motor_speeds_gbl[2] = -30;
  motor_speeds_gbl[3] = +30;

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 0;
  motor_speeds_gbl[7] = 0;
  motor_speeds_gbl[8] = 0;
  motor_speeds_gbl[9] = 1;
}

Spinning::Spinning(char* name) : State(name) {
}

State* Spinning::on_event(char* event) {
  if (strcmp(event, STOP) == 0) {
    return &stopped_state;
  }
  else if ((strcmp(event, GO) == 0) || (strcmp(event, GO1) == 0)) {
    return &moving_forward_state;
  }
  else if (strcmp(event, BACK) == 0) {
    return &moving_backward_state;
  }
  else if (strcmp(event, LEFT) == 0) {
    return &moving_left_state;
  }
  else if (strcmp(event, RIGHT) == 0) {
    return &moving_right_state;
  }
  else if (strcmp(event, SPIN) == 0) {
    return &spinning_state;
  }
  else if (strcmp(event, OFF) == 0) {
    return &inactive_state;
  }
  else {
    return this;
  }

}
void Spinning::perform_action(int* motor_speeds_gbl) {
  //  cout << "Performing state: ";
  //  print_name(this->name);
  motor_speeds_gbl[0] = -30;
  motor_speeds_gbl[1] = +30;
  motor_speeds_gbl[2] = +30;
  motor_speeds_gbl[3] = -30;

  motor_speeds_gbl[4] = 1;
  motor_speeds_gbl[5] = 0;
  motor_speeds_gbl[6] = 1;
  motor_speeds_gbl[7] = 1;
  motor_speeds_gbl[8] = 1;
  motor_speeds_gbl[9] = 1;

}
