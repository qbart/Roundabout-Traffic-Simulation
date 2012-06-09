#include "Car.h"
#include "Env.h"

DWORD Car::Run( LPVOID arg )
{
  while(1)
  {
    if (frozen)
      {
        wait();
        if (fms != 0) Sleep(fms);
        signal();
        frozen = false;
      }
    else
    if (Env::cars[ID])
      {
        if (can_run()) move(); else wait();
      }
    else
      go();

      Sleep(20);
  }
}

Car::Car(int car_id)
{
  ID = car_id;
  init();
}

void Car::init()
{
  engine_started = false;
  frozen = false;
  fms = 50;
  angle = ((double)360 / (double)MAX) * ID; //starting position
  wait();
  Env::set_pos(ID,angle);
}

bool Car::can_run()
{
  double A = angle; //current car
  double B = Env::position(ID+1); //next car

  double dist = B > A ? B - A : (360 - A) + B;

  return dist >= SAFE_DIST;
}

void Car::wait()
{
  Env::wait(ID);
}

void Car::signal()
{
  engine_started = true;
  Env::signal(ID);
}

void Car::freeze(int ms)
{
  fms = ms;
  frozen = true;
}

void Car::go()
{
  if (engine_started)
    {
      //person reaction
      int t = 500 + ( (rand() % 250 ) - 50 );
      //printf("%d\n", t);
      Sleep(t);
      signal();
    }
}

void Car::move()
{
  angle++;

  if (angle >= 360.0) angle = 0;

  Env::set_pos(ID,angle);
}

double Car::position()
{
  return angle;
}
