/***************************************************************************
 *   Copyright (C) 2010 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_3__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   $Log: tripodgait18dof.cpp,v $                                         *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include <selforg/controller_misc.h>
#include <ode_robots/hexabotsensormotordefinition.h>
#include "tripodgait18dof.h"
using namespace matrix;
using namespace std;
using namespace HEXABOT;

TripodGait18DOF::TripodGait18DOF(const TripodGait18DOFConf& _conf)
: AbstractController("TripodGait18DOF", "$Id: tripodgait18dof.cpp,v 0.1 $"),
  explicit conf(_conf) {
  t = 0;
  
  outputH1 = 0.001;
  outputH2 = 0.001;
}
;

TripodGait18DOF::~TripodGait18DOF() {
}

void TripodGait18DOF::init(int sensornumber, int motornumber, RandGen* randGen)
{
  //Tripodgait for 18 DOF Hexapod
  assert(motornumber>=18);
}

/// performs one step (includes learning). Calulates motor commands from sensor
/// inputs.
void TripodGait18DOF::step(const sensor* x_, int number_sensors,
    motor* y_, int number_motors) {
  stepNoLearning(x_, number_sensors, y_, number_motors);
}
;

/// performs one step without learning. Calulates motor commands from sensor
/// inputs.
void TripodGait18DOF::stepNoLearning(const sensor* x_, int number_sensors,
    motor* y_, int number_motors) {
  //Tripodgait for 18 DOF Hexapod

  assert(number_sensors >= 18);
  assert(number_motors >= 18);

  double activityH1 = conf.WeightH1_H1 * outputH1 + conf.WeightH1_H2 * outputH2
      + 0.01;
  double activityH2 = conf.WeightH2_H2 * outputH2 + conf.WeightH2_H1 * outputH1
      + 0.01;

  outputH1 = tanh(activityH1);
  outputH2 = tanh(activityH2);
  
  // generate motor commands      
  // right rear coxa static_cast<knee>(forward)-backward joint
  y_[T3_m] = outputH2 * conf.fact + conf.bias;
  y_[C3_m] = -outputH1 * conf.fact * conf.direction;
  y_[F3_m] = y_[T3_m];
  //left rear coxa static_cast<knee>(forward)-backward joint
  y_[T6_m] = outputH2 * conf.fact + conf.bias;
  y_[C6_m] = outputH1 * conf.fact * conf.direction;
  y_[F6_m] = -y_[T6_m];
  //right middle coxa static_cast<knee>(forward)-backward joint
  y_[T2_m] = -outputH2 * conf.fact + conf.bias;
  y_[C2_m] = outputH1 * conf.fact * conf.direction;
  y_[F2_m] = y_[T2_m];
  //left middle coxa static_cast<knee>(forward)-backward joint
  y_[T5_m] = -outputH2 * conf.fact + conf.bias;
  y_[C5_m] = -outputH1 * conf.fact * conf.direction;
  y_[F5_m] = -y_[T5_m];
  //right front coxa static_cast<knee>(forward)-backward joint
  y_[T1_m] = outputH2 * conf.fact + conf.bias;
  y_[C1_m] = -outputH1 * conf.fact * conf.direction;
  y_[F1_m] = y_[T1_m];
  //left front coxa static_cast<knee>(forward)-backward joint
  y_[T4_m] = outputH2 * conf.fact + conf.bias;
  y_[C4_m] = outputH1 * conf.fact * conf.direction;
  y_[F4_m] = -y_[T4_m];

  // update step counter
  ++t;
}

/** stores the controller values to a given file. */
bool TripodGait18DOF::store(FILE* f) const {
  return true;
}

/** loads the controller values from a given file. */
bool TripodGait18DOF::restore(FILE* f) {
  return true;
}
