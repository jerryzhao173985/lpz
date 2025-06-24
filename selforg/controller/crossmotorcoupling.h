/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_7__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/

#ifndef __CROSSMOTORCOUPLING_H
#define __CROSSMOTORCOUPLING_H

#include "abstractcontrolleradapter.h"
#include "teachable.h"

#include <list>
#include <vector>

/**
   Adjacency lists representing the connection graph.
   CMC[i] contains the list of indices of motors,
   which are used as teaching signals for motor i.
*/
using CMC = std::vector<std::list<int>>;

/**
 * This is an adapter for a teachable controller to implement a
 * cross motor coupling, see dissertation of Georg Martius
 *
 */
class CrossMotorCoupling : public AbstractControllerAdapter {
public:
  /**
     @param controller actual controller
     @param teachable also pointer to the controller, must be equal to controller.
       This trick is used to ensure that the controller is both __PLACEHOLDER_2__ and
     __PLACEHOLDER_3__.
     @param threshold value below which (absolute) no cross motor teaching is done
      (avoids suppression of activity)
   */
  CrossMotorCoupling(AbstractController* controller, Teachable* teachable, double threshold = 0.4)
    : AbstractControllerAdapter(controller, "CrossMotorCoupling", "$ID$")
    , teachable(teachable)
    , threshold(threshold) {
    // We check whether controller and teachable are equally the same thing.
    // the pure pointer comparison does not work, because the type case
    //  also moves the pointer for the other vtable (tricky!)
    // That is why we need dynamic_cast here
    Teachable* t2 = dynamic_cast<Teachable*>(controller);
    (void)(t2); // this is to avoid a warning in -DNDEBUG mode
    assert(static_cast<void*>(t2) == static_cast<void*>(teachable));
  }

  virtual void step(const sensor* sensors,
                    int sensornumber,
                    motor* motors,
                    int motornumber);

  virtual void setCMC(const CMC& cmc);
  virtual CMC getCMC() const;

  /**** TEACHABLE Interface pass through ****/

  virtual void setMotorTeaching(const matrix::Matrix& teaching) {
    teachable->setMotorTeaching(teaching);
  }

  virtual void setSensorTeaching(const matrix::Matrix& teaching) {
    teachable->setSensorTeaching(teaching);
  }
  virtual matrix::Matrix getLastMotorValues() {
    return teachable->getLastMotorValues();
  }

  virtual matrix::Matrix getLastSensorValues() {
    return teachable->getLastSensorValues();
  }

  /**
      creates a permutation cross motor coupling, where for each motor
      we define one cross motor connection.
      @param permutation permutation[i]=j means that motor i receives from motor j
   */
  static CMC getPermutationCMC(const std::list<int>& permutation);

protected:
  CMC cmc;
  Teachable* teachable;
  double threshold = 0; ///< treshhold below which not cmc-teaching is done
};

#endif
