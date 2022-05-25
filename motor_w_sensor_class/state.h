#pragma once
#ifndef STATE_H
#define STATE_H

class State {
public:
  char *name;
  State(char *name);
  void print_name(char *name);
  virtual State* on_event(char *event); // this still has default function definition.
  virtual void perform_action(int *motor_speeds_gbl) = 0; // this is an interface now, no need for definition in cpp file.
};

class Stopped: public State {
public:
  Stopped(char *name);
  State* on_event(char *event);
  void perform_action(int *motor_speeds_gbl);
};

class Active: public State {
public:
  Active(char *name);
  State* on_event(char *event);
  void perform_action(int *motor_speeds_gbl);
};

class MovingForward: public State {
public:
  MovingForward(char *name);
  State* on_event(char *event);
  void perform_action(int *motor_speeds_gbl);
};

class MovingBackward: public State {
public:
  MovingBackward(char *name);
  State* on_event(char *event);
  void perform_action(int *motor_speeds_gbl);
};

#endif
