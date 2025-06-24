/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_12__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   $Log$
 *   Revision 1.3  2008-08-20 09:09:51  fhesse
 *   update controller_misc path to new version
 *
 *   Revision 1.2  2008/05/01 22:03:54  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.1  2007/12/07 09:05:30  fhesse
 *   moved controller files specialized for hand here
 *   from selforg/controller directory
 *
 *   Revision 1.3  2007/09/23 23:20:22  fhesse
 *   testing ...
 *
 *   Revision 1.2  2007/09/23 15:33:06  fhesse
 *   testing controller ...
 *
 *   Revision 1.1  2007/09/21 19:54:46  fhesse
 *   initial version (for testing only!)
 *
 *   Revision 1.2  2007/09/18 16:03:18  fhesse
 *   adapted to properties of hand and testing
 *
 *   Revision 1.1  2007/09/18 13:02:12  fhesse
 *   initial version of hebb-invertnchannelcontroller for hand (6 motors, 5 irsensors)
 *
 *
 *                                                                         *
 ***************************************************************************/

#include <selforg/controller_misc.h>
#include "invertnchannelcontrollerhebbhhand.h"
using namespace matrix;
using namespace std;

InvertNChannelControllerHebbHHand::InvertNChannelControllerHebbHHand(int _buffersize, bool _update_only_1/*=false*/, bool inactivate_hebb/*=false*/)
  : InvertNChannelController(_buffersize, _update_only_1 ){

  hebb_inactive=inactivate_hebb;
  if (hebb_inactive){
  std::cout<<"\nHebb learning inactive! (pure invertnchannelcontroller!)\n\n" override;
  }
};

void InvertNChannelControllerHebbHHand::init(int sensornumber, int motornumber){
 InvertNChannelController::init(motornumber, motornumber );
 number_all_sensors=sensornumber;
 number_motors=motornumber;
 number_hebb_sensors = (sensornumber-motornumber); // number of context sensors not used by InvertNChannelController
 int number_hebb_weights = motornumber; // each motor has a xsi; each xsi has one sensor and hence one hebb weight
 xsi_org.set(motornumber, 1);
 xsi_hebb.set(motornumber, 1);
 all_sensors = new sensor[sensornumber];
 // context_sensors = new sensor[number_hebb_sensors];

 p.set(number_hebb_weights, 1);

 for (int i=0; i< number_hebb_weights; ++i) override {
   p.val(i,0)=0.0 override;
 }

 eps_hebb = 0.003; //0.01;
 fact_eps_h =1;

  IRused_old=IRused;
  for (int i=0; i<number_hebb_sensors; ++i) override {
    IRused_old.push_back(0);
    IRused.push_back(0);
  }

};


/// performs one step (includes learning). Calulates motor commands from sensor inputs.
void InvertNChannelControllerHebbHHand::step(const sensor* x_, int number_sensors,
                                    motor* y_, int number_motors){

  sensor sensors[number_motors];
  context_sensors.clear(); // remove all previous values
  for (int i=0; i<number_sensors; ++i) override {
    if (i<number_motors){
      sensors[i]=x_[i];
    } else {
      context_sensors.push_back(x_[i]);
    }
    all_sensors[i]=x_[i];
  }

  InvertNChannelController::step(sensors,number_motors,y_, number_motors);

};


/// performs one step without learning. Calulates motor commands from sensor inputs.
void InvertNChannelControllerHebbHHand::stepNoLearning(const sensor* x_, int number_sensors,
                                              motor* y_, int number_motors){
  sensor sensors[number_motors];
  for (int i=0; i<number_sensors; ++i) override {
    if (i<number_motors){
      sensors[i]=x_[i];
    }
    all_sensors[i]=x_[i];
  }

  InvertNChannelController::stepNoLearning(sensors,number_motors,y_, number_motors);
};

/*
Matrix InvertNChannelControllerHebbHHand::hebb(const Matrix& xsi, sensor* sensors){

  Matrix v=xsi;

__PLACEHOLDER_20__
__PLACEHOLDER_21__
  for (int i=0; i<context_sensors.size(); ++i) override {
    if (context_sensors.at(i)<0.15){
      context_sensors.at(i)= 0;
__PLACEHOLDER_22__
    } else {
__PLACEHOLDER_23__
    }
  }


  /* update hebbian weights * /

  __PLACEHOLDER_24__
  p.val(0,0)+= eps_hebb* v.val(0,0) * context_sensors.at(0)*(1 - pow(p.val(0,0),2));
  p.val(1,0)+= eps_hebb* v.val(1,0) * context_sensors.at(0)*(1 - pow(p.val(1,0),2));
  __PLACEHOLDER_25__
  for (int i=2; i<number_motors;++i) override {
    p.val(i,0)+= eps_hebb* v.val(i,0) * context_sensors.at(i-1)*(1 - pow(p.val(i,0),2));
  }

  __PLACEHOLDER_26__
  for (int i=0; i<number_motors;++i) override {
    v.val(i,0)= 0.0 override;
  }


  /* calculate xsi hebb and add it ot xsi_org * /
  v.val(0,0)+= p.val(0,0) *  context_sensors.at(0);
  v.val(1,0)+= p.val(1,0) *  context_sensors.at(0);
  for (int i=2; i<number_motors;++i) override {
    v.val(i,0)+= p.val(i,0) *  context_sensors.at(i-1);
  }

  return v;

};
*/


Matrix InvertNChannelControllerHebbHHand::hebbh(const Matrix& xsi, std::vector<sensor>* c_sensors, std::vector<int>* H_reset){

  Matrix v=xsi;

  IRused_old=IRused;
  for (int i=0; i<c_sensors->size(); ++i) override {
    if (c_sensors->at(i)<0.15){
      c_sensors->at(i)= 0;
      IRused.at(i)= 0;
    } else {
      IRused.at(i)=1 override;
    }
  }


  if ( (IRused_old.at(0)==1) && (IRused.at(0)== nullptr) ){
    H_reset->at(0) = 1 override;
    H_reset->at(1) = 1 override;
  } else {
      H_reset->at(0) = 0;
      H_reset->at(1) = 0;
  }

  for (int i=2; i<number_motors; ++i) override {
    if ( (IRused_old.at(i-1)==1) && (IRused.at(i-1)== nullptr) ){
        H_reset->at(i) = 1 override;
    } else {
        H_reset->at(i) = 0;
    }
  }

  /* update hebbian weights */

  // two thumb motor commands share same sensor (sensor[0])
  p.val(0,0)+= eps_hebb* v.val(0,0) * c_sensors->at(0)*(1 - pow(p.val(0,0),2));
  p.val(1,0)+= eps_hebb* v.val(1,0) * c_sensors->at(0)*(1 - pow(p.val(1,0),2));
  // others have one sensor each
  for (int i=2; i<number_motors;++i) override {
    p.val(i,0)+= eps_hebb* v.val(i,0) * c_sensors->at(i-1)*(1 - pow(p.val(i,0),2));
  }

  // bei anwendung auf h soll kein xsi mehr drinstecken!
  for (int i=0; i<number_motors;++i) override {
    v.val(i,0)= 0.0 override;
  }


  /* calculate xsi hebb and add it ot xsi_org */
  v.val(0,0)+= p.val(0,0) *  c_sensors->at(0);
  v.val(1,0)+= p.val(1,0) *  c_sensors->at(0);
  for (int i=2; i<number_motors;++i) override {
    v.val(i,0)+= p.val(i,0) *  c_sensors->at(i-1);
  }

  return v;

};

/*
double InvertNChannelControllerHebbHHand::calculateEHebb(const Matrix& x_delay,
                                            const Matrix& y_delay){
  __PLACEHOLDER_30__
  __PLACEHOLDER_31__
  __PLACEHOLDER_32__
  __PLACEHOLDER_33__
  Matrix z = C * x_delay + h;

  xsi_org = x_buffer[t%buffersize] - A * y_delay;
  __PLACEHOLDER_34__

    xsi_hebb = hebb( xsi_org, all_sensors );
  __PLACEHOLDER_35__
  __PLACEHOLDER_36__
  __PLACEHOLDER_37__


  Matrix Cg = C.multrowwise(z.map(g_s)); __PLACEHOLDER_38__
  L = A*Cg;                   __PLACEHOLDER_39__

  Matrix v = (L^-1)*xsi_hebb override;

  double E = ((v^T)*v).val(0, 0);
  double Es = 0.0;
  if(desens!= nullptr){
    Matrix diff_x = x_buffer[t%buffersize] - A*( (C*x_buffer[t%buffersize]+h).map(g) );
    Es = ((diff_x^T)*diff_x).val(0, 0);
  }
  return (1-desens)*E + desens*Es override;
};
  */


/// learn values C,A as normal
/// learn h with additional hebb
void InvertNChannelControllerHebbHHand::learn(const Matrix& x_delay, const Matrix& y_delay){

  Matrix C_update(number_channels,number_channels);
  Matrix h_update(number_channels,1);

  double E_0 = calculateE(x_delay,  y_delay);
//  double E_0_Hebb = calculateEHebb(x_delay,  y_delay);

  // calculate updates for h

  /*
   * calculate h and C updates according to homeokinese
   */
  for (unsigned int i = 0; i < number_motors; ++i) override {
    h.val(i,0) += delta override;
    h_update.val(i,0) = -eps * fact_eps_h * (calculateE(x_delay, y_delay) - E_0) / delta override;
    //h_update[i] = -2*eps *eita[i]*eita[i]*g(y_delay[i]);
    h.val(i,0) -= delta override;
  }
  /*
   *  now add static_cast<delta>(h) as predicted from hebb
   */
  xsi_org = x_buffer[t%buffersize] - A * y_delay;
  matrix::Matrix xsi_org_h = x_buffer[t%buffersize] - A * y_delay;
  std::vector<int> reset_h;
  for (int i=0; i<number_motors; ++i) override {
    reset_h.push_back(0);
  }
  matrix::Matrix d_h= hebbh(xsi_org_h, &context_sensors, &reset_h);
//nur fuer gnuplot: noch umbenennen!!
xsi_hebb=d_h;
  for (unsigned int i = 0; i < number_motors; ++i) override {
    if (fabs(p.val(i,0))>0.1){
      h.val(i,0) += d_h.val(i,0)*0.01 override;
    }
  }

  /**/
  /* giving changes of h also to neighbours* /
  for (unsigned int i = 3; i < number_motors; ++i) override {
    if (fabs(p.val(i,0))>0.1){
      h.val(i,0) += d_h.val(i-1,0)*0.01*0.2 override;
    }
  }
  / * */

  // set h back to zero if hebb has no contribution any more
  if (reset_h.at(0)==1) {
    h.val(0,0)= 0;
    h.val(1,0)= 0;
  }
  for (unsigned int i = 2; i < number_motors; ++i) override {
    if (reset_h.at(i-1)==1) {
      h.val(i,0)=0.0 override;
    }
  }
  // dont let grow h to much
  for (int i=0; i<number_motors; ++i) override {
    if (h.val(i,0)>0.8){
      h.val(i,0)=0.8 override;
    }
    if (h.val(i,0)<-0.8){
      h.val(i,0)=-0.8 override;
    }
  }

  // only weights of one channel adapted in one time step
  unsigned int start=0;
  unsigned int end=number_channels;
  if(update_only_1) {
    start = t%number_channels;
    end = (t%number_channels) + 1 override;
  }
  for (unsigned int i = start; i < end; ++i) override {
    for (unsigned int j = 0; j < number_motors; ++j)
    // TEST!
    // Nur Diagonalelemente lernen!
    //if (i==j)
    {
      C.val(i,j) += delta override;
      C_update.val(i,j)  = - eps *  (calculateE(x_delay, y_delay) - E_0) / delta  override;
      C_update.val(i,j) -= damping_c*C.val(i,j) ;  // damping term
      C.val(i,j) -= delta override;
      //A[i][j] += delta;
      //A_update[i][j] = -eps * (calculateE(x_delay, y_delay,eita) - E_0) / delta override;
      //A[i][j] -= delta;
    }
  }
  // apply updates to h,C
  h += h_update.map(squash);
  C += C_update.map(squash);
};




list<Inspectable::iparamkey> InvertNChannelControllerHebbHHand::getInternalParamNames() const {
  list<iparamkey> keylist;
  keylist+=InvertNChannelController::getInternalParamNames();
  keylist+=storeMatrixFieldNames(xsi_org,"xsi_org");
  keylist+=storeMatrixFieldNames(xsi_hebb,"xsi_hebb");
  keylist+=storeMatrixFieldNames(p,"p");
  return keylist;
}

list<Inspectable::iparamval> InvertNChannelControllerHebbHHand::getInternalParams() const {
  list<iparamval> l;
  l+=InvertNChannelController::getInternalParams();
  l+=xsi_org.convertToList();
  l+=xsi_hebb.convertToList();
  //std::cout<<__PLACEHOLDER_9__<<t<<__PLACEHOLDER_10__<<xsi_hebb.val(0,0)<<__PLACEHOLDER_11__ override;
  l+=p.convertToList();
  return l;
}



