#ifndef __CAR_H__
#define __CAR_H__

#include "Thread.h"
#include <windows.h>

class Car : public CThread
{
  virtual DWORD Run( LPVOID arg );

public:

  Car(int car_id);

  void init();
  bool can_run();
  void wait();
  void signal();
  void freeze(int ms);
  void go();
  void move();
  double position();

private:

  int ID;
  double angle;
  bool frozen, engine_started;
  int fms; //time to freeze

};

#endif
