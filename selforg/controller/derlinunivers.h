/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   $Log$
 *   Revision 1.1  2008-06-26 14:04:51  guettler
 *   controller with linear inversion and multilayer FFNN
 *
 *   Revision 1.3  2008/04/17 14:54:45  martius
 *   randomGen added, which is a random generator with long period and an
 *    internal state. Each Agent has an instance and passed it to the controller
 *    and the wiring. This is good for
 *   a) repeatability on agent basis,
 *   b) parallel execution as done in ode_robots
 *
 *   Revision 1.2  2007/12/13 16:04:01  martius
 *   store and restore
 *   stuctural output for neuronviz
 *
 *   Revision 1.1  2007/12/11 14:22:09  martius
 *   new (old) controller update only with backprop steps
 *   and complex nets
 *
 *                                            *
 *                                                                         *
 ***************************************************************************/
#ifndef __DERLINUNIVERS_H
#define __DERLINUNIVERS_H


#include <stdio.h>
#include <vector>
#include "abstractcontroller.h"
#include "elman.h"
#include "matrix.h"

typedef struct _DerLinUniversConf {
  unsigned int buffersize;  ///< buffersize size of the time-buffer for x,y,v
  double init;    ///<  init size of the matrices of the network.
  double squashsize; ///< update squashing
  bool someInternalParams;  ///< someInternalParams if true only some internal parameters are exported, all otherwise

  Elman* net; ///< entire contoller network (should have at least 2 layers)
  int motorlayer; ///< index of motor layer in the network (if -1 then one but last layer)
} DerLinUniversConf;


/**
 * class for robot control with sine and cosine
 *
 *
 */
class DerLinUnivers : public AbstractController {
public:

  DerLinUnivers(const DerLinUniversConf& conf = getDefaultConf());
  virtual ~DerLinUnivers();

  static DerLinUniversConf getDefaultConf(){
    DerLinUniversConf c;
    c.buffersize = 50;
    c.init       = 1;
    c.squashsize = 0.05;
    c.someInternalParams = true;
    c.net        = 0;
    c.motorlayer = -1;
    return c;
  }

  static DerLinUniversConf getDefaultNetConf(){
    DerLinUniversConf c = getDefaultConf();
    std::vector<Layer> layers;
    //   layers.push_back(Layer(20,0.5,FeedForwardNN::tanh)); // hidden layer
    layers.push_back(Layer(0,1,FeedForwardNN::tanhr)); // motor layer
    // size of output layer is automatically set
    layers.push_back(Layer(1,0,FeedForwardNN::linear));

    Elman* e = new Elman(1,layers,false, false, false);
    c.net = e;
    return c;
  }

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0);

  virtual int getSensorNumber() const {return number_sensors;}

  virtual int getMotorNumber() const {return number_motors;}

  virtual void step(const sensor* sensors, int sensornumber,
                    motor* motors, int motornumber);

  virtual void stepNoLearning(const sensor* , int number_sensors,
                              motor* , int number_motors);

protected:
  /** puts the sensors in the ringbuffer,
      generate controller values (by activating the network) and put them in the
      ringbuffer as well */
  void fillBuffersAndControl(const sensor* x_, int number_sensors,
                                     motor* y_, int number_motors);

  /// calculate time-smoothed values
  matrix::Matrix calculateSmoothValues(const matrix::Matrix* buffer,
                                               int number_steps_for_averaging_);

  /// calculate controller outputs (and activates inputs)
  matrix::Matrix calculateControllerValues(const matrix::Matrix& x);

  // put new value in ring buffer
  void putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec, int delay=0){
    buffer[(t-delay)%conf.buffersize] = vec;
  }

  /** calculates the error_factor for
      1: square (E=sqrt(e^t*e)) error;
      2: logarithmic (E=ln(e^T*e)) or 0 for normal
   */
  static double calcErrorFactor(const matrix::Matrix& e, int Enorm);


public:
  /********* INSPECTABLE INTERFACE ******/
  virtual std::list<iparamkey> getInternalParamNames() const;
  virtual std::list<iparamval> getInternalParams() const;
  virtual ilayerlist getStructuralLayers() const;
  virtual iconnectionlist getStructuralConnections() const;

  /********* STORABLE INTERFACE ******/
  virtual bool store(FILE* f) const;
  virtual bool restore(FILE* f);

protected:

  unsigned int t;
  unsigned int number_sensors;
  unsigned int number_motors;
  bool initialised;

  DerLinUniversConf conf;
  matrix::Matrix* x_buffer;
  matrix::Matrix* y_buffer;
  matrix::Matrix* v_buffer;

  matrix::Matrix v;
  matrix::Matrix J;
  matrix::Matrix xsi_smooth;

  paramval eps;
  paramval epsM;
  paramval epsV;
  paramval lambda;
  paramval s4avg;
  paramval s4del;
  paramval Enorm;
  paramval epsDyn;
};

#endif
