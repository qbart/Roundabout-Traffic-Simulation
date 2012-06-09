#include "Env.h"

bool Env::cars[MAX];
double Env::pos[MAX];

Env::Env() {}
Env::~Env() {}

void Env::init()
{
  for (int i=0; i<MAX; i++)
    {
      wait(i);
      set_pos( i , ((double)360 / (double)MAX) * i ); //starting position
    }
}

void Env::signal(int ID)
{
  cars[_ID( ID )] = true;
}

void Env::wait(int ID)
{
  cars[_ID( ID )] = false;
}

double Env::position(int ID)
{
  return pos[_ID( ID )];
}

int Env::_ID(int ID)
{
  return (ID % MAX);
}

void Env::set_pos(int ID, double a)
{
  pos[_ID( ID )] = a;
}
