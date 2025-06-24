#include <cassert>
#include <cstdlib>
#include <iostream>

#include "robotlearncontrol.h"


template <int NUMBER_CHANNELS, int BUFFER_SIZE=1> class RobotLearnControlTarget :public RobotLearnControl<NUMBER_CHANNELS, BUFFER_SIZE>{


  protected:
  double eps_target; // learning rate for target learning

  virtual void learnTarget(double *x, double *y) override {
    double E[NUMBER_CHANNELS];

    E[0]=y[1]-y[0];   // Motorkommandos sollen den gleichen Wert haben
