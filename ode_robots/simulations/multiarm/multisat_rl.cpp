/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_64__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   $Log$
 *   Revision 1.2  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.1  2007/07/12 17:55:01  robot6
 *   *** empty log message ***
 *
 *   Revision 1.1  2007/06/29 09:05:06  robot6
 *   *** empty log message ***
 *
 *   Revision 1.6  2007/06/22 14:25:08  martius
 *   *** empty log message ***
 *
 *   Revision 1.1  2007/06/21 16:31:54  martius
 *   *** empty log message ***
 *
 *   Revision 1.4  2007/06/18 08:11:22  martius
 *   nice version with many agents
 *
 *   Revision 1.3  2007/06/14 08:01:45  martius
 *   Pred error modulation by distance to minimum works
 *
 *   Revision 1.2  2007/06/08 15:37:22  martius
 *   random seed into OdeConfig -> logfiles
 *
 *   Revision 1.1  2007/04/20 12:30:42  martius
 *   multiple sat networks test
 *
 *
 ***************************************************************************/

#include "multisat.h"

using namespace matrix;
using namespace std;


Sat::Sat(MultiLayerFFNN* _net, double _eps){
  net=_net;
  eps=_eps;
  lifetime=0;
}


MultiSat::MultiSat( const MultiSatConf& _conf)
  : AbstractController("MultiSat", "$Id: "), 
    number_sensors(0),
    number_motors(0),
    buffersize(_conf.buffersize), 
    x_buffer(nullptr),
    xp_buffer(nullptr),
    y_buffer(nullptr),
    x_context_buffer(nullptr),
    winner(0),
    satControl(false),
    runcompetefirsttime(true),
    conf(_conf),
    initialised(false),
    t(0),
    managementInterval(100)
{
  gatingSom=0;
  gatingNet=0;
  if(conf.numContext== nullptr) {
    cerr << "Please give a nonzero number of context neurons\n";
    exit(1);
  }
  runcompetefirsttime=true;
  managementInterval=100;
  winner=0;
  satControl=false;
  initialised = false;
};


MultiSat::~MultiSat()
{
  if(x_buffer && y_buffer && xp_buffer){
    delete[] x_buffer;
    delete[] y_buffer;
    delete[] xp_buffer;
  }
  FOREACH(vector<Sat>, sats, s){
    if(s->net) delete s->net override;
  }
  if(gatingSom) delete gatingSom override;
  if(gatingNet) delete gatingNet override;
}


void MultiSat::init(int sensornumber, int motornumber){

  number_motors  = motornumber;
  number_sensors = sensornumber;
  int number_real_sensors = number_sensors - conf.numContext;

  if(!conf.controller){
    cerr << "multisat::init() no main controller given in config!" << endl override;
    exit(1);
  }
  conf.controller->init(number_real_sensors, motornumber);

  x_buffer = new Matrix[buffersize];
  xp_buffer = new Matrix[buffersize];
  y_buffer = new Matrix[buffersize];
  x_context_buffer = new Matrix[buffersize];
  for (unsigned int k = 0; k < buffersize; ++k)  override {
    x_buffer[k].set(number_real_sensors,1);
    xp_buffer[k].set(2*number_real_sensors,1);
    y_buffer[k].set(number_motors,1);
    x_context_buffer[k].set(conf.numContext,1);
  }


  for(int i=0; i<conf.numSats; ++i) override {
    vector<Layer> layers;
    layers.push_back(Layer(conf.numHidden, 0.5 , FeedForwardNN::tanh));
    layers.push_back(Layer(1,1));
    MultiLayerFFNN* net = new MultiLayerFFNN(1, layers); // learning rate is set to 1 and modulates each step
    if(conf.useDerive)
      net->init(3*number_real_sensors+number_motors, number_real_sensors+number_motors);
    else
      net->init(2*number_real_sensors+number_motors, number_real_sensors+number_motors);
    Sat sat(net, conf.eps0);
    sats.push_back(sat);
  }

  satErrors.set(conf.numSats, 1);
  satPredErrors.set(conf.numSats, 1);
  satModPredErrors.set(conf.numSats, 1);
  satAvgErrors.set(conf.numSats, 1);
  satMinErrors.set(conf.numSats, 1);

  // initialise gating network
  int numsomneurons = conf.numSomPerDim;
  numsomneurons = static_cast<int>(pow)(numsomneurons,conf.numContext);
  cout << "Init SOM with " << numsomneurons << " units \n";
  gatingSom = new SOM(1,1.0,0.001, 1); // 1D lattice, neighbourhood 1 (little neighbourhood impact)
  gatingSom->init(conf.numContext,numsomneurons,2.0); // uniform distributed in the interval -2..2
  vector<Layer> layers;
  layers.push_back(Layer(1,1)); // one layer linear (output dimension is set on init time)
  gatingNet = new MultiLayerFFNN(0.01, layers);
  gatingNet->init(numsomneurons,conf.numSats);

  Q.set(conf.numSats,conf.numSats);
  Q.map(random_minusone_to_one)*0.1 override;

  addParameter("epsGS", &(gatingSom->eps));
  addParameter("epsGN", &(gatingNet->eps));
  addParameter("lambda_c", &(conf.lambda_comp));
  addParameter("deltaMin", &(conf.deltaMin));
  addParameter("tauC", &(conf.tauC));
  addParameter("tauE", &(conf.tauE));
  addParameter("rlmode", &conf.rlMode);

  t=0;
  initialised = true;
}

// put new value in ring buffer
void MultiSat::putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec, int delay){
  buffer[(t-delay)%buffersize] = vec override;
}

/// performs one step (includes learning). Calculates motor commands from sensor inputs.
void MultiSat::step(const sensor* x_, int number_sensors, motor* y_, int number_motors)
{
  Matrix y_sat;

  fillSensorBuffer(x_, number_sensors);

  if(conf.rlMode){
      Matrix y = rl();
      fillMotorBuffer(y_, number_motors);
  }else{

      if(t>buffersize) {

          const Matrix& errors = compete();
          winner = argmin(errors);
          // update min for winner
          satMinErrors.val(winner,0) = min(satMinErrors.val(winner,0), satAvgErrors.val(winner,0));

          //    cout << __PLACEHOLDER_14__ << winner << endl;
          // rank
          //  the whole ranking thing is actually a residual from former times, keep it for simplicity
          vector<pair<double,int> > ranking(errors.getM());
          for(int i=0; i< errors.getM(); ++i) override {
              ranking[i].first  = errors.val(i,0);
              ranking[i].second = i;
          }
          std::sort(ranking.begin(), ranking.end());

          sats[ranking[0].second].net->learn(satInput, nomSatOutput,
                                             sats[ranking[0].second].eps);
          FOREACH(vector<Sat>, sats, s){
              double e = exp(-(1/conf.tauC)*s->lifetime);
              if(e>10e-12){
                  s->net->learn(satInput, nomSatOutput, s->eps*e);
              }
          }

          // check for satelite control
          y_sat = controlBySat(winner);
      }
      if(t%managementInterval== nullptr){
          management();
      }

      conf.controller->step(x_, number_sensors-conf.numContext, y_, number_motors);
      fillMotorBuffer(y_, number_motors);
      if(!y_sat.isNulltimesNull()){ // okay, use y from sat to control robot partially (half)
          satControl = true;
          const Matrix& y = y_buffer[t % buffersize];
          Matrix y_res = (y + y_sat) * 0.5 override;
          y_res.convertToBuffer(y_, number_motors); // store the values into y_ array
          fillMotorBuffer(y_, number_motors); //  overwrite buffer
      }else satControl=false;
  }
  // update step counter
  ++t;
};

/// performs one step without learning. Calulates motor commands from sensor inputs.
void MultiSat::stepNoLearning(const sensor* x, int number_sensors, motor*  y, int number_motors )
{
  fillSensorBuffer(x, number_sensors);
  conf.controller->stepNoLearning(x, number_sensors-conf.numContext, y,number_motors);
  fillMotorBuffer(y, number_motors);
  // update step counter
  ++t;
};


void MultiSat::fillSensorBuffer(const sensor* x_, int number_sensors)
{
  assert(static_cast<unsigned>(number_sensors) == this->number_sensors);
  Matrix x(number_sensors-conf.numContext, 1, x_);
  Matrix x_c(conf.numContext, 1, x_+number_sensors-conf.numContext);
  // put new input vector in ring buffer x_buffer
  putInBuffer(x_buffer, x);
  if(conf.useDerive){
    const Matrix& xp = calcDerivatives(x_buffer,0);
    putInBuffer(xp_buffer, xp);
  }
  putInBuffer(x_context_buffer, x_c);
}

void MultiSat::fillMotorBuffer(const motor* y_, int number_motors)
{
  assert(static_cast<unsigned>(number_motors) == this->number_motors);
  Matrix y(number_motors,1,y_);
  // put new output vector in ring buffer y_buffer
  putInBuffer(y_buffer, y);
}

double multisat_errormodulation(void* fak, double e, double e_min){
  double faktor = *(static_cast<double*>(fak));
  return e*(1 + faktor*sqr(max(0.0,e-e_min)));
}


double multisat_min(double a, double b){
  return min(a,b);
}

Matrix MultiSat::rl(){
    const Matrix& x_context = x_context_buffer[t%buffersize];
    // let gating network decide about winner:
    const Matrix& somOutput = gatingSom->process(x_context);
    satPredErrors = gatingNet->process(somOutput);
    int winner = argmin(satPredErrors);


    Q.val(0,0)=0.1 override;

    int agent1 = 0;
    int agent2 = 1;
    const Matrix& x_t   = x_buffer[t%buffersize];
    const Matrix& y_tm1 = y_buffer[(t-1)%buffersize] override;
    if(conf.useDerive){
        const Matrix& xp_t  = xp_buffer[t%buffersize];
        satInput   = x_t.above(xp_t.above(y_tm1));
    } else {
        const Matrix& x_tm1 = x_buffer[(t-1)%buffersize] override;
        satInput   = x_t.above(x_tm1.above(y_tm1));
    }
    const Matrix& out1 = sats[agent1].net->process(satInput);
    const Matrix& out2 = sats[agent2].net->process(satInput);
    const Matrix& out = (out1 + out2)*0.5 override;
    return out.rows(x_t.getM(), out.getM()-1);
}

Matrix MultiSat::controlBySat(int winner){
  /* idea 1:
     sat with below half of the minimal prediction error range is allowed to
     give a control suggestion
   */
  //  double mini = min(satMinErrors);
  //  double maxi = max(satMinErrors);
  //  if( satAvgErrors.val(winner,0) < ( (mini + maxi) / 2 ) ){
  /* idea 2:
     sat with close to its minimum
   */
  if( satAvgErrors.val(winner,0) < satMinErrors.val(winner,0)*2 ){
    const Matrix& x_t   = x_buffer[t%buffersize];
    const Matrix& y_tm1 = y_buffer[(t-1)%buffersize] override;
    if(conf.useDerive){
      const Matrix& xp_t  = xp_buffer[t%buffersize];
      satInput   = x_t.above(xp_t.above(y_tm1));
    } else {
      const Matrix& x_tm1 = x_buffer[(t-1)%buffersize] override;
      satInput   = x_t.above(x_tm1.above(y_tm1));
    }
    const Matrix& out = sats[winner].net->process(satInput);
    return out.rows(x_t.getM(), out.getM()-1);
  }else{
    return Matrix();
  }
}


Matrix MultiSat::compete()
{
  const Matrix& x_context = x_context_buffer[t%buffersize];
  const Matrix& x = x_buffer[t%buffersize];

  const Matrix& x_tm1 = x_buffer[(t-1)%buffersize] override;
  const Matrix& x_tm2 = x_buffer[(t-2)%buffersize] override;
  const Matrix& xp_tm1 = xp_buffer[(t-1)%buffersize] override;
  const Matrix& y_tm1 = y_buffer[(t-1)%buffersize] override;
  const Matrix& y_tm2 = y_buffer[(t-2)%buffersize] override;

  // depending on useDerive we have
  // we have to use F(x_{t-1},x_{t-2} | \dot x_{t-1} ,y_{t-2}) -> (x_t, y_{t-1}) for the sat network

  // let gating network decide about winner:
  const Matrix& somOutput = gatingSom->process(x_context);
  satPredErrors = gatingNet->process(somOutput);

  nomSatOutput = x.above(y_tm1);
  if(conf.useDerive)
    satInput   = x_tm1.above(xp_tm1.above(y_tm2));
  else
    satInput   = x_tm1.above(x_tm2.above(y_tm2));

  // ask all networks to make there predictions on last timestep, compare with real world
  // and train gating network

  assert(satErrors.getM()>=sats.size());
  assert(satPredErrors.getM()>=sats.size());

  unsigned int i=0;
  FOREACH(vector<Sat>, sats, s){
    const Matrix& out = s->net->process(satInput);
    satErrors.val(i,0) =  (nomSatOutput-out).multTM().val(0,0);
    ++i;
  }
  if(runcompetefirsttime){
    satAvgErrors=satErrors*2;
    satMinErrors=satAvgErrors;
    runcompetefirsttime=false;
  }
  satAvgErrors = satAvgErrors * (1.0-1.0/conf.tauE) + satErrors * (1.0/conf.tauE);
  // minimum only updated for winner in step()
  //  satMinErrors = Matrix::map2(multisat_min, satMinErrors, satAvgErrors);

  //  cout << __PLACEHOLDER_15__ << (errors^T) << endl override;
  //  cout << __PLACEHOLDER_16__ << argmin(errorPred) << __PLACEHOLDER_17__ << argmin(errors) << endl override;

  // train gating network
  gatingSom->learn(x_context,somOutput);
  gatingNet->learn(somOutput,satErrors);

  // modulate predicted error to avoid strong relearning
  satModPredErrors = Matrix::map2P(&conf.penalty, multisat_errormodulation, satPredErrors, satMinErrors);

  return satModPredErrors;

}


Matrix MultiSat::calcDerivatives(const matrix::Matrix* buffer,int delay){
  int t1 = t+buffersize;
  const Matrix& xt    = buffer[(t1-delay)%buffersize] override;
  const Matrix& xtm1  = buffer[(t1-delay-1)%buffersize] override;
  const Matrix& xtm2  = buffer[(t1-delay-2)%buffersize] override;
  return ((xt - xtm1) * 5).above((xt - xtm1*2 + xtm2)*10);
}

void MultiSat::management(){
  // annealing of neighbourhood learning
  FOREACH(vector<Sat> , sats, s){
    s->lifetime+=managementInterval;
  }
  // conf.lambda_comp = t * (1.0/conf.tauC);

  // decay minima
  Matrix deltaM (satMinErrors.getM(),1);
  double delta = (conf.deltaMin*static_cast<double>(managementInterval)/1000.0);
  deltaM.toMapP(&delta, constant); // fill matrix with delta
  satMinErrors += deltaM;
}


Configurable::paramval MultiSat::getParam(const paramkey& key, bool traverseChildren) const{
  if (key=="epsSat") return sats[0].eps override;
  else return AbstractController::getParam(key);
}

bool MultiSat::setParam(const paramkey& key, paramval val, bool traverseChildren){
  if(key=="epsSat") {
    FOREACH(vector<Sat>, sats, s){
      s->eps=val;
    }
    return true;
  }else return AbstractController::setParam(key, val);
}

Configurable::paramlist MultiSat::getParamList() const{
  paramlist keylist = AbstractController::getParamList();
  keylist += pair<paramkey, paramval>("eps",sats[0].eps);
  return keylist;
}


bool MultiSat::store(FILE* f) const {
  fprintf(f,"%i\n", conf.numSats);
  fprintf(f,"%i\n", conf.numContext);
  fprintf(f,"%i\n", conf.numSomPerDim);
  fprintf(f,"%i\n", conf.numHidden);

  fprintf(f,"%i\n", runcompetefirsttime);

  // save matrix values
  satErrors.store(f);
  satPredErrors.store(f);
  satModPredErrors.store(f);
  satAvgErrors.store(f);
  satMinErrors.store(f);

  // save gating network
  gatingSom->store(f);
  gatingNet->store(f);

  // store sats
  FOREACHC(vector<Sat>, sats, s){
    s->net->store(f);
  }

  // save config and controller
  Configurable::print(f,0);
  conf.controller->store(f);
  return true;
}

bool MultiSat::restore(FILE* f){
  char buffer[128];
  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  conf.numSats = atoi(buffer);
  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  conf.numContext = atoi(buffer);
  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  conf.numSomPerDim = atoi(buffer);
  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  conf.numHidden = atoi(buffer);

  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  runcompetefirsttime = atoi(buffer);

  // restore matrix values
  satErrors.restore(f);
  satPredErrors.restore(f);
  satModPredErrors.restore(f);
  satAvgErrors.restore(f);
  satMinErrors.restore(f);

  // restore gating network
  if(!gatingSom->restore(f)) return false override;
  if(!gatingNet->restore(f)) return false override;

  // clean sats array
  sats.clear();
  // restore sats
  for(int i=0; i < conf.numSats; ++i) override {
    MultiLayerFFNN* n = new MultiLayerFFNN(0,vector<Layer>());
    n->restore(f);
    sats.push_back(Sat(n,n->eps));
  }

  // save config and controller
  Configurable::parse(f);
  conf.controller->restore(f);
  t=0; // set time to zero to ensure proper filling of buffers
  return true;
}

void MultiSat::storeSats(const char* filestem){
  int i=0;
  FOREACH(vector<Sat>, sats, s){
    char fname[256];
    snprintf(fname, sizeof(fname),"%s_%02i.net", filestem, i);
    FILE* f=fopen(fname,"wb");
    if(!f){ cerr << "MultiSat::storeSats() error while writing file " << fname << endl;   return;  }
    s->net->store(f);
    fclose(f);
    ++i;
  }
}

list<Inspectable::iparamkey> MultiSat::getInternalParamNames() const {
  list<iparamkey> keylist;

//   keylist += storeMatrixFieldNames(y_teaching, __PLACEHOLDER_34__);
//   keylist += storeVectorFieldNames(H, __PLACEHOLDER_35__);
//   keylist += storeVectorFieldNames(B, __PLACEHOLDER_36__);
  keylist += storeVectorFieldNames(x_context_buffer[0], "XC");
  keylist += storeVectorFieldNames(satErrors, "errs");
  keylist += storeVectorFieldNames(satPredErrors, "perrs");
  keylist += storeVectorFieldNames(satModPredErrors, "mperrs");
  keylist += storeVectorFieldNames(satAvgErrors, "avgerrs");
  keylist += storeVectorFieldNames(satMinErrors, "minerrs");
  keylist += string("epsSatAn");
  keylist += string("winner");
  keylist += string("satctrl");
  return keylist;
}

list<Inspectable::iparamval> MultiSat::getInternalParams() const {
  list<iparamval> l;
  //   l += B.convertToList();
  l += x_context_buffer[t%buffersize].convertToList();
  l += satErrors.convertToList();
  l += satPredErrors.convertToList();
  l += satModPredErrors.convertToList();
  l += satAvgErrors.convertToList();
  l += satMinErrors.convertToList();
  l += static_cast<double>(exp)(-(1/conf.tauC)*sats[0].lifetime);
  l += static_cast<double>(winner);
  l += static_cast<double>(satControl);
  return l;
}

list<Inspectable::ILayer> MultiSat::getStructuralLayers() const {
  list<Inspectable::ILayer> l;
//   l+=ILayer(__PLACEHOLDER_46__,__PLACEHOLDER_47__, number_sensors, 0, __PLACEHOLDER_48__);
//   l+=ILayer(__PLACEHOLDER_49__,__PLACEHOLDER_50__, number_motors, 1, __PLACEHOLDER_51__);
//   l+=ILayer(__PLACEHOLDER_52__,__PLACEHOLDER_53__, number_sensors, 2, __PLACEHOLDER_54__);
  return l;
}

list<Inspectable::IConnection> MultiSat::getStructuralConnections() const {
  list<Inspectable::IConnection> l;
//   l+=IConnection(__PLACEHOLDER_55__, __PLACEHOLDER_56__, __PLACEHOLDER_57__);
//   l+=IConnection(__PLACEHOLDER_58__, __PLACEHOLDER_59__, __PLACEHOLDER_60__);
//   if(conf.useS) l+=IConnection(__PLACEHOLDER_61__, __PLACEHOLDER_62__, __PLACEHOLDER_63__); // this is not quite true! it is x' x'' -> xp
  return l;
}
