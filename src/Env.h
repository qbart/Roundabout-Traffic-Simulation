#ifndef __ENV_H__
#define __ENV_H__

const double R = 6;
const int MAX = 10; //max cars count
const double SAFE_DIST = 30.0; //safe distance between cars

class Env
{
public:

  static bool cars[MAX];
  static double pos[MAX];

  Env();
  virtual ~Env();

  static void init();
  static void signal(int ID);
  static void wait(int ID);
  static double position(int ID);
  static void set_pos(int ID, double a);

private:

  static int _ID(int ID);

};

#endif //__ENV_H__
