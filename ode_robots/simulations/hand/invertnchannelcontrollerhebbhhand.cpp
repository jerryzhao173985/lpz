/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http://creativecommons.org/licenses/by-nc-sa/2.5/ *
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
  std::cout<<"\nHebb learning inactive! (pure invertnchannelcontroller!)\n\n";
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

 for (int i=0; i< number_hebb_weights; i++){
   p.val(i,0)=0.0;
 }

 eps_hebb = 0.003; //0.01;
 fact_eps_h =1;

  IRused_old=IRused;
  for (int i=0; i<number_hebb_sensors; i++){
    IRused_old.push_back(0);
    IRused.push_back(0);
  }

};


/// performs one step (includes learning). Calulates motor commands from sensor inputs.
void InvertNChannelControllerHebbHHand::step(const sensor* x_, int number_sensors,
                                    motor* y_, int number_motors){

  sensor sensors[number_motors];
  context_sensors.clear(); // remove all previous values
  for (int i=0; i<number_sensors; i++){
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
  for (int i=0; i<number_sensors; i++){
    if (i<number_motors){
      sensors[i]=x_[i];
    }
    all_sensors[i]=x_[i];
  }

  InvertNChannelController::stepNoLearning(sensors,number_motors,y_, number_motors);
};

/*
Matrix InvertNChannelControllerHebbHHand::hebb(Matrix& xsi, sensor* sensors){

  Matrix v=xsi;

//  IRsensed_old.clear();
//  IRsensed_old=IRsensed;
  for (int i=0; i<context_sensors.size(); i++){
    if (context_sensors.at(i)<0.15){
      context_sensors.at(i)=0;
//      IRsensed.at(i)=false;
    } else {
//      IRsensed.at(i)=true;
    }
  }


  /* update hebbian weights * /

  // two thumb motor commands share same sensor (sensor[0])
  p.val(0,0)+= eps_hebb* v.val(0,0) * context_sensors.at(0)*(1 - pow(p.val(0,0),2));
  p.val(1,0)+= eps_hebb* v.val(1,0) * context_sensors.at(0)*(1 - pow(p.val(1,0),2));
  // others have one sensor each
  for (int i=2; i<number_motors;i++){
    p.val(i,0)+= eps_hebb* v.val(i,0) * context_sensors.at(i-1)*(1 - pow(p.val(i,0),2));
  }

  // bei anwendung auf h soll kein xsi mehr drinstecken!
  for (int i=0; i<number_motors;i++){
    v.val(i,0)= 0.0;
  }


  /* calculate xsi hebb and add it ot xsi_org * /
  v.val(0,0)+= p.val(0,0) *  context_sensors.at(0);
  v.val(1,0)+= p.val(1,0) *  context_sensors.at(0);
  for (int i=2; i<number_motors;i++){
    v.val(i,0)+= p.val(i,0) *  context_sensors.at(i-1);
  }

  return v;

};
*/


Matrix InvertNChannelControllerHebbHHand::hebbh(Matrix& xsi, std::vector<sensor>* c_sensors, std::vector<int>* H_reset){

  Matrix v=xsi;

  IRused_old=IRused;
  for (int i=0; i<c_sensors->size(); i++){
    if (c_sensors->at(i)<0.15){
      c_sensors->at(i)=0;
      IRused.at(i)=0;
    } else {
      IRused.at(i)=1;
    }
  }


  if ( (IRused_old.at(0)==1) && (IRused.at(0)==0) ){
    H_reset->at(0) = 1;
    H_reset->at(1) = 1;
  } else {
      H_reset->at(0) = 0;
      H_reset->at(1) = 0;
  }

  for (int i=2; i<number_motors; i++){
    if ( (IRused_old.at(i-1)==1) && (IRused.at(i-1)==0) ){
        H_reset->at(i) = 1;
    } else {
        H_reset->at(i) = 0;
    }
  }

  /* update hebbian weights */

  // two thumb motor commands share same sensor (sensor[0])
  p.val(0,0)+= eps_hebb* v.val(0,0) * c_sensors->at(0)*(1 - pow(p.val(0,0),2));
  p.val(1,0)+= eps_hebb* v.val(1,0) * c_sensors->at(0)*(1 - pow(p.val(1,0),2));
  // others have one sensor each
  for (int i=2; i<number_motors;i++){
    p.val(i,0)+= eps_hebb* v.val(i,0) * c_sensors->at(i-1)*(1 - pow(p.val(i,0),2));
  }

  // bei anwendung auf h soll kein xsi mehr drinstecken!
  for (int i=0; i<number_motors;i++){
    v.val(i,0)= 0.0;
  }


  /* calculate xsi hebb and add it ot xsi_org */
  v.val(0,0)+= p.val(0,0) *  c_sensors->at(0);
  v.val(1,0)+= p.val(1,0) *  c_sensors->at(0);
  for (int i=2; i<number_motors;i++){
    v.val(i,0)+= p.val(i,0) *  c_sensors->at(i-1);
  }

  return v;

};

/*
double InvertNChannelControllerHebbHHand::calculateEHebb(const Matrix& x_delay,
                                            const Matrix& y_delay){
  // Calculate z based on the delayed inputs since the present input x is
  // produced by the outputs tau time steps before
  // which on their hand are y = K(x_D)
  // due to the delay in the feed back loop.
  Matrix z = C * x_delay + h;

  xsi_org = x_buffer[t%buffersize] - A * y_delay;
  //Matrix xsi = x_buffer[t%buffersize] - A * z.map(g);

    xsi_hebb = hebb( xsi_org, all_sensors );
  //xsi_hebb = hebb_if( xsi_org, all_sensors );
  //if (fabs(xsi_hebb.val(0,0))>0.2)
  //  std::cout<<"calcE("<<t<<")  xsi_org="<<xsi_org.val(0,0)<<"    xsi_hebb="<<xsi_hebb.val(0,0)<<"\n";


  Matrix Cg = C.multrowwise(z.map(g_s)); // Cg_{ij} = g'_i * C_{ij}
  L = A*Cg;                   // L_{ij}  = \sum_k A_{ik} g'_k c_{kj}

  Matrix v = (L^-1)*xsi_hebb;

  double E = ((v^T)*v).val(0, 0);
  double Es = 0.0;
  if(desens!=0){
    Matrix diff_x = x_buffer[t%buffersize] - A*( (C*x_buffer[t%buffersize]+h).map(g) );
    Es = ((diff_x^T)*diff_x).val(0, 0);
  }
  return (1-desens)*E + desens*Es;
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
  for (unsigned int i = 0; i < number_motors; i++){
    h.val(i,0) += delta;
    h_update.val(i,0) = -eps * fact_eps_h * (calculateE(x_delay, y_delay) - E_0) / delta;
    //h_update[i] = -2*eps *eita[i]*eita[i]*g(y_delay[i]);
    h.val(i,0) -= delta;
  }
  /*
   *  now add (delta)h as predicted from hebb
   */
  xsi_org = x_buffer[t%buffersize] - A * y_delay;
  matrix::Matrix xsi_org_h = x_buffer[t%buffersize] - A * y_delay;
  std::vector<int> reset_h;
  for (int i=0; i<number_motors; i++){
    reset_h.push_back(0);
  }
  matrix::Matrix d_h= hebbh(xsi_org_h, &context_sensors, &reset_h);
//nur fuer gnuplot: noch umbenennen!!
xsi_hebb=d_h;
  for (unsigned int i = 0; i < number_motors; i++){
    if (fabs(p.val(i,0))>0.1){
      h.val(i,0) += d_h.val(i,0)*0.01;
    }
  }

  /**/
  /* giving changes of h also to neighbours* /
  for (unsigned int i = 3; i < number_motors; i++){
    if (fabs(p.val(i,0))>0.1){
      h.val(i,0) += d_h.val(i-1,0)*0.01*0.2;
    }
  }
  / * */

  // set h back to zero if hebb has no contribution any more
  if (reset_h.at(0)==1) {
    h.val(0,0)=0;
    h.val(1,0)=0;
  }
  for (unsigned int i = 2; i < number_motors; i++){
    if (reset_h.at(i-1)==1) {
      h.val(i,0)=0.0;
    }
  }
  // dont let grow h to much
  for (int i=0; i<number_motors; i++){
    if (h.val(i,0)>0.8){
      h.val(i,0)=0.8;
    }
    if (h.val(i,0)<-0.8){
      h.val(i,0)=-0.8;
    }
  }

  // only weights of one channel adapted in one time step
  unsigned int start=0;
  unsigned int end=number_channels;
  if(update_only_1) {
    start = t%number_channels;
    end = (t%number_channels) + 1;
  }
  for (unsigned int i = start; i < end; i++){
    for (unsigned int j = 0; j < number_motors; j++)
    // TEST!
    // Nur Diagonalelemente lernen!
    //if (i==j)
    {
      C.val(i,j) += delta;
      C_update.val(i,j)  = - eps *  (calculateE(x_delay, y_delay) - E_0) / delta ;
      C_update.val(i,j) -= damping_c*C.val(i,j) ;  // damping term
      C.val(i,j) -= delta;
      //A[i][j] += delta;
      //A_update[i][j] = -eps * (calculateE(x_delay, y_delay,eita) - E_0) / delta;
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
  //std::cout<<"getInternalParams("<<t<<")      xsi_hebb="<<xsi_hebb.val(0,0)<<"\n";
  l+=p.convertToList();
  return l;
}



