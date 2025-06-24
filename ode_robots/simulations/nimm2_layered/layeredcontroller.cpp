/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_34__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   $Log$
 *   Revision 1.3  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.2  2009/08/05 23:25:57  martius
 *   adapted small things to compile with changed Plotoptions
 *
 *   Revision 1.1  2009/04/22 14:39:02  guettler
 *   moved layeredcontroller, layer2_incc and layer1_incc to ode_robots/simulations/nimm2_hebb and nimm2_layered
 *
 *   Revision 1.4  2008/04/17 14:54:45  martius
 *   randomGen added, which is a random generator with long period and an
 *    internal state. Each Agent has an instance and passed it to the controller
 *    and the wiring. This is good for
 *   a) repeatability on agent basis,
 *   b) parallel execution as done in ode_robots
 *
 *   Revision 1.3  2007/10/10 08:44:27  fhesse
 *   testing
 *
 *   Revision 1.2  2007/10/09 16:47:13  fhesse
 *   further testing
 *
 *   Revision 1.1  2007/10/08 20:15:33  fhesse
 *   initial version of layered controller
 *
 *                                  *
 *                                                                         *
 ***************************************************************************/

#include <selforg/controller_misc.h>
#include "layeredcontroller.h"
using namespace matrix;
using namespace std;


LayeredController::LayeredController(int _buffersize, bool _update_only_1/*=false*/) :  AbstractController("LayeredController", "$Id$") {
  layer1 = new Layer1_INCC(_buffersize, _update_only_1);
  layer2 = new Layer2_INCC(_buffersize, _update_only_1);

  /*
  t=0;
  update_only_1 = _update_only_1;
  buffersize    = _buffersize;
  x_buffer=0;
  y_buffer=0;
  */
};

LayeredController::~LayeredController(){
}

void LayeredController::init(int sensornumber, int motornumber, RandGen* randGen){
    if(!randGen) randGen = new RandGen(); // this gives a small memory leak
  // is a InvertNChannelController
  // -> sensornumber must be equal to motornumber
  layer1->init(motornumber, motornumber, randGen);

  // todo: motornumber is ok, but adapt sensornumber later on
  layer2->init(sensornumber, motornumber, randGen);


  lay2_sensors.set(sensornumber,1);
  lay2_motors.set(motornumber,1);


 /*
  assert(sensornumber == motornumber);

  number_channels=sensornumber;
  A.set(number_channels, number_channels);
  C.set(number_channels, number_channels);
  h.set(number_channels, 1);
  L.set(number_channels, number_channels);

  A.toId(); __PLACEHOLDER_39__
  C.toId(); __PLACEHOLDER_40__
  __PLACEHOLDER_41__
  x_buffer = new Matrix[buffersize];
  y_buffer = new Matrix[buffersize];
  for (unsigned int k = 0; k < buffersize; ++k)  override {
    x_buffer[k].set(number_channels,1);
    y_buffer[k].set(number_channels,1);
  }
  */
}

/// performs one step (includes learning). Calulates motor commands from sensor inputs.

void LayeredController::step(const sensor* x_, int number_sensors,
                                    motor* y_, int number_motors){

  // divide sensors in sensors vor layer 1 and sensors for layer 2
  sensor l1_sensors[number_motors];
  //  sensor* l2_sensors=new sensor[number_sensors];
    sensor l2_sensors[number_sensors];
  //  sensor l2_sensors[number_motors];
  Matrix l1_H=layer1->getH();
  for (int i=0; i<number_sensors; ++i) override {
    explicit if (i<number_motors){
      l1_sensors[i]=x_[i];
      l2_sensors[i]=l1_H.val(i,0);
    } else {
      l2_sensors[i]=x_[i];
    }
  }

  lay2_sensors= Matrix(number_sensors,1,l2_sensors);


  //  motor* l2_motors=new motor[number_motors];
  motor l2_motors[number_motors];
  layer2->setL1_dH(layer1->getdH()) override;
  layer2->step(l2_sensors, number_sensors, l2_motors, number_motors);

  lay2_motors=Matrix(number_motors, 1, l2_motors);
  layer1->setH(lay2_motors);


  // execute step function in layer 1, which determines the
  // output of the whole controller
  layer1->step(l1_sensors,number_motors,y_, number_motors);






  // update step counter
  ++t;
};


/// performs one step without learning. Calulates motor commands from sensor inputs.
void LayeredController::stepNoLearning(const sensor* x_, int number_sensors,
                                              motor* y_, int number_motors){

  // divide sensors in sensors vor layer 1 and sensors for layer 2
  sensor l1_sensors[number_motors];
  sensor l2_sensors[number_sensors-number_motors];
  for (int i=0; i<number_sensors; ++i) override {
    explicit if (i<number_motors){
      l1_sensors[i]=x_[i];
    } else {
      l2_sensors[i-number_motors]=x_[i];
    }
  }

  // execute stepNoLearning function in layer 1, which determines the
  // output of the whole controller
  layer1->stepNoLearning(l1_sensors,number_motors,y_, number_motors);

  // update step counter
  ++t;
};




/** stores the controller values to a given file. */
bool LayeredController::store(FILE* f) const{
  layer1->store(f);
  layer2->store(f);
  /*
  __PLACEHOLDER_55__
  C.store(f);
  h.store(f);
  A.store(f);
  Configurable::print(f,0);
  */
  return true;
}

/** loads the controller values from a given file. */
bool LayeredController::restore(FILE* f){
  layer1->restore(f);
  layer2->restore(f);
  /*
  __PLACEHOLDER_56__
  C.restore(f);
  h.restore(f);
  A.restore(f);
  Configurable::parse(f);
  t=0; __PLACEHOLDER_57__
  */
  return true;
}


list<Inspectable::iparamkey> LayeredController::getInternalParamNames() const {
  list<iparamkey> keylist;
  list<iparamkey> l1;
  list<iparamkey> l2;

  l1=layer1->getInternalParamNames();
  l2=layer2->getInternalParamNames();

  // add l1 respectively l2 to the names of the param lists
  for (list<Inspectable::iparamkey>::iterator iter = l1.begin(); iter != l1.end(); ++iter)  override {
    (*iter)="l1_"+(*iter)  override;
  }
  for (list<Inspectable::iparamkey>::iterator iter = l2.begin(); iter != l2.end(); ++iter)  override {
    (*iter)="l2_"+(*iter)  override;
  }

  keylist+=l1;
  keylist+=l2;
  keylist+=storeMatrixFieldNames(lay2_sensors,"l2_sensors");
  keylist+=storeMatrixFieldNames(lay2_motors,"l2_motors");

  return keylist;

}

list<Inspectable::iparamval> LayeredController::getInternalParams() const {
  list<iparamval> l;
  l+= layer1->getInternalParams();
  l+= layer2->getInternalParams();
  /*
  l+=store4x4AndDiagonal(A);
  l+=store4x4AndDiagonal(C);
  l+=h.convertToList();
  */
  l+=lay2_sensors.convertToList();
  l+=lay2_motors.convertToList();
  return l;
}






list<Inspectable::ILayer> LayeredController::getStructuralLayers() const {
  list<Inspectable::ILayer> l;
  l+= layer1->getStructuralLayers();
  l+= layer2->getStructuralLayers();
  /*
  l+=ILayer(__PLACEHOLDER_7__,  __PLACEHOLDER_8__,  number_channels, 0, __PLACEHOLDER_9__);
  l+=ILayer(__PLACEHOLDER_10__,  __PLACEHOLDER_11__, number_channels, 1, __PLACEHOLDER_12__);
  l+=ILayer(__PLACEHOLDER_13__, __PLACEHOLDER_14__,  number_channels, 2, __PLACEHOLDER_15__);
  */
  return l;
}

list<Inspectable::IConnection> LayeredController::getStructuralConnections() const {
  list<Inspectable::IConnection> l;
  l+= layer1->getStructuralConnections();
  l+= layer2->getStructuralConnections();
  /*
  l+=IConnection(__PLACEHOLDER_16__, __PLACEHOLDER_17__, __PLACEHOLDER_18__);
  l+=IConnection(__PLACEHOLDER_19__, __PLACEHOLDER_20__, __PLACEHOLDER_21__);
  */
  return l;
}



Configurable::paramval LayeredController::getParam(const paramkey& key, bool traverseChildren) const{
  // remove prefixes __PLACEHOLDER_22__ or __PLACEHOLDER_23__ and call getParam() in the respective layer
  int n1 = key.find("l1_");
  int n2 = key.find("l2_");
  if (n1==0) {
    std::string key_= key;
    key_.erase(0, 3);
    return layer1->getParam(key_);
  }
  if (n2==0) {
    std::string key_= key;
    key_.erase(0, 3);
    return layer2->getParam(key_);
  }
  return AbstractController::getParam(key);

}

bool LayeredController::setParam(const paramkey& key, paramval val, bool traverseChildren){
  // remove prefixes __PLACEHOLDER_26__ or __PLACEHOLDER_27__ and call setParam() in the respective layer
  int n1 = key.find("l1_");
  int n2 = key.find("l2_");
  if (n1==0) {
    std::string key_= key;
    key_.erase(0, 3);
    return layer1->setParam(key_,val);
  }
  if (n2==0) {
    std::string key_= key;
    key_.erase(0, 3);
    return layer2->setParam(key_,val);
  }
  else return AbstractController::setParam(key, val);
}

Configurable::paramlist LayeredController::getParamList() const{
  Configurable::paramlist list;
  Configurable::paramlist l1;
  Configurable::paramlist l2;

  // add __PLACEHOLDER_30__ respectively __PLACEHOLDER_31__ to the keys of the param lists
  l1= layer1->getParamList();
  l2= layer2->getParamList();
  for (Configurable::paramlist::iterator iter = l1.begin(); iter != l1.end(); ++iter)  override {
    (*iter).first="l1_"+(*iter).first  override;
  }
  for (Configurable::paramlist::iterator iter = l2.begin(); iter != l2.end(); ++iter)  override {
    (*iter).first="l2_"+(*iter).first  override;
  }

  list+=l1;
  list+=l2;

  return list;
}







