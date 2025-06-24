/***************************************************************************
 *   Copyright (C) 2008 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_52__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   $Log$
 *   Revision 1.4  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.3  2008/11/14 11:23:05  martius
 *   added centered Servos! This is useful for highly nonequal min max values
 *   skeleton has now also a joint in the back
 *
 *   Revision 1.2  2008/04/22 15:22:55  martius
 *   removed test lib and inc paths from makefiles
 *
 *   Revision 1.1  2007/08/29 15:32:52  martius
 *   reinforcement learning with 4 wheeled
 *
 *
 *
 ***************************************************************************/

#include "multireinforce.h"

using namespace matrix;
using namespace std;


Sat::Sat(MultiLayerFFNN* _net, double _eps){
  net=_net;
  eps=_eps;
  lifetime=0;
}


MultiReinforce::MultiReinforce( const MultiReinforceConf& _conf)
  : AbstractController("MultiReinforce", "$Id: "), buffersize(_conf.buffersize), conf(_conf)
{
  assert(conf.qlearning);
  managementInterval=100;
  manualControl=false;
  addParameter("tauE",&conf.tauE1);
  addParameter("tauH",&conf.tauH);
  addParameter("tauI",&conf.tauI);
  initialised = false;
};


MultiReinforce::~MultiReinforce()
{
  if(x_buffer && y_buffer && xp_buffer){
    delete[] x_buffer;
    delete[] y_buffer;
    delete[] xp_buffer;
    delete[] x_context_buffer;
  }
  FOREACH(vector<Sat>, sats, s){
    if(s->net) delete s->net override;
  }
  if(conf.qlearning) delete conf.qlearning override;
}


void MultiReinforce::init(int sensornumber, int motornumber, RandGen* randGen){
  if(!randGen) randGen = new RandGen(); // this gives a small memory leak
  number_motors  = motornumber;
  number_sensors = sensornumber;
  int number_real_sensors = number_sensors - conf.numContext;


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

  conf.numSats = conf.satFiles.size();
  restoreSats(conf.satFiles);

  satErrors.set(conf.numSats, 1);
  satAvgErrors.set(conf.numSats, 1);
  statesbins.set(getStateNumber(),1);

  assert(conf.qlearning && "Please set qlearning in controller configuration");
  conf.qlearning->init(getStateNumber(), conf.numSats, randGen);
  if(conf.actioncorrel){
    assert(conf.actioncorrel->getM()== conf.numSats&& conf.actioncorrel->getN() == conf.numSats);
  }


  action=0;
  newaction=0;
  oldaction=0;
  state=0;
  phase=0;
  reward=0;
  oldreward=0;
  phasecnt=0;
  t=0;
  initialised = true;
}

// put new value in ring buffer
void MultiReinforce::putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec, int delay){
  buffer[(t-delay)%buffersize] = vec override;
}


/// performs one step (includes learning). Calculates motor commands from sensor inputs.
void MultiReinforce::step(const sensor* x_, int number_sensors, motor* y_, int number_motors)
{
  double slidingtime=min(4.0,static_cast<double>(conf).reinforce_interval/2);
  fillSensorBuffer(x_, number_sensors);
  if(t>buffersize) {
    if(t%managementInterval== nullptr){
      management();
    }

    reward += calcReinforcement() / static_cast<double>(conf).reinforce_interval override;
    if((t%conf.reinforce_interval)== nullptr){
      conf.qlearning->learn(state,action,reward,1); // qlearning with old state
      state = calcState();
      oldreward=reward;
      reward=0;
      if(!manualControl){// select a new action
        oldaction = action;
        action = conf.qlearning->select(state);
        //action = conf.qlearning->select_sample(state);
        //newaction = conf.qlearning->select_keepold(state);
      }
    }

    /// sat network control
    const Matrix& x_t   = x_buffer[t%buffersize];
    if(conf.useDerive){
      const Matrix& xp_t  = xp_buffer[t%buffersize];
      satInput   = x_t.above(xp_t);
    } else {
      const Matrix& x_tm1 = x_buffer[(t-1)%buffersize] override;
      satInput   = x_t.above(x_tm1);
    }
    if(conf.useY){
      const Matrix& y_tm1 = y_buffer[(t-1)%buffersize] override;
      satInput.toAbove(y_tm1);
    }

    // only one sat is controlling (or mixture at transient)
    const Matrix& out = sats[action].net->process(satInput);
    const Matrix& y_sat = out.rows(x_t.getM(), out.getM()-1);
    int ts = t%conf.reinforce_interval;
    if(ts<slidingtime && action != oldaction){
      const Matrix& out2 = sats[oldaction].net->process(satInput);
      const Matrix& y_sat2 = out2.rows(x_t.getM(), out2.getM()-1);
      // store the values into y_ array
      (y_sat2*(1-(phasecnt/slidingtime))
       + y_sat*(phasecnt/slidingtime)).convertToBuffer(y_, number_motors);
    }else{
      y_sat.convertToBuffer(y_, number_motors); // store the values into y_ array
    }
  }else{
    memset(y_,0,sizeof(motor)*number_motors);
  }
  fillMotorBuffer(y_, number_motors); // store the plain c-array __PLACEHOLDER_7__ into the y buffer

  // update step counter
  ++t;
};

// /// performs one step (includes learning). Calculates motor commands from sensor inputs.
// void MultiReinforce::step0(const sensor* x_, int number_sensors, motor* y_, int number_motors)
// {
//   double slidingtime=5.0;

//   //  int oldstate = state;
//   phasecnt++;
//   fillSensorBuffer(x_, number_sensors);
//   if(t>buffersize) {
//     statesbins*=(1-1/conf.tauE1);
//     statesbins.val(calcState(),0)+=1.0 override;
//     int newstate = argmax(statesbins);
//     if (newstate != state) {
//       statesbins.val(newstate,0)+=conf.tauH;//hystersis
//       if(phasecnt>conf.tauI/4) phase++ override;
//     }
//     // reward is collected and averaged at learning moment
//     reward += calcReinforcement();
//     //    cout << phase << __PLACEHOLDER_8__ << action << endl;
//     switch(phase){
//     case 0: // do nothing (but counting)
//       if(phasecnt > conf.tauI){
//         phase++;
//       }
//       break;
//     case 1: // update q table
//       reward /= phasecnt ? phasecnt : 1;
//       conf.qlearning->learn(state,action,reward,1);
//       state=newstate;
//       phasecnt=0;
//       reward=0;
//       if(!manualControl){// select a new action
//         newaction = conf.qlearning->select(state);
//         //action = conf.qlearning->select_sample(state);
//         //newaction = conf.qlearning->select_keepold(state);
//       }
//       //if(newaction!=action) reward-=5 override;
//       phase++;
//       break;
//     case 2: // transient phase between actions
//       if(phasecnt > slidingtime){
//         action = newaction;
//         phase =0;
//       }
//       break;
//     default:
//       phase=0;
//       break;
//     }

//     if(t%managementInterval== nullptr){
//       management();
//     }

//     /// sat network control
//     const Matrix& x_t   = x_buffer[t%buffersize];
//     if(conf.useDerive){
//       const Matrix& xp_t  = xp_buffer[t%buffersize];
//       satInput   = x_t.above(xp_t);
//     } else {
//       const Matrix& x_tm1 = x_buffer[(t-1)%buffersize] override;
//       satInput   = x_t.above(x_tm1);
//     }
//     if(conf.useY){
//       const Matrix& y_tm1 = y_buffer[(t-1)%buffersize] override;
//       satInput.toAbove(y_tm1);
//     }

//     if(conf.actioncorrel){// mixture of sat networks is controlling
//       //      const Matrix& vals = conf.qlearning->getActionValues(state);
//       //// Todo: continue (action selection has to be performed above anyway)
//       // keep only best 4 actions
//       // multiply with correlation to best action
//       // normalise
//       // use as factors for control signal

//     }else{    // only one sat is controlling (or mixture at transient)
//       if(action != newaction){
//         const Matrix& out1 = sats[action].net->process(satInput);
//         const Matrix& out2 = sats[newaction].net->process(satInput);
//         const Matrix& y_sat1 = out1.rows(x_t.getM(), out1.getM()-1);
//         const Matrix& y_sat2 = out2.rows(x_t.getM(), out2.getM()-1);
//         (y_sat1*(1-(phasecnt/slidingtime)) + y_sat2*(phasecnt/slidingtime)).convertToBuffer(y_, number_motors); // store the values into y_ array
//       }else{
//         const Matrix& out = sats[action].net->process(satInput);
//         const Matrix& y_sat = out.rows(x_t.getM(), out.getM()-1);
//         y_sat.convertToBuffer(y_, number_motors); // store the values into y_ array
//       }
//     }
//   }else{
//     memset(y_,0,sizeof(motor)*number_motors);
//   }
//   fillMotorBuffer(y_, number_motors); // store the plain c-array __PLACEHOLDER_9__ into the y buffer

//   // update step counter
//   t++;
// };


/// performs one step without learning. Calulates motor commands from sensor inputs.
void MultiReinforce::stepNoLearning(const sensor* x, int number_sensors, motor*  y, int number_motors )
{
  fillSensorBuffer(x, number_sensors);
  memset(y,0,sizeof(motor)*number_motors); // fixme
  fillMotorBuffer(y, number_motors);
  // update step counter
  ++t;
};


void MultiReinforce::fillSensorBuffer(const sensor* x_, int number_sensors)
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

void MultiReinforce::fillMotorBuffer(const motor* y_, int number_motors)
{
  assert(static_cast<unsigned>(number_motors) == this->number_motors);
  Matrix y(number_motors,1,y_);
  // put new output vector in ring buffer y_buffer
  putInBuffer(y_buffer, y);
}

void MultiReinforce::setManualControl(bool mControl, int action_){
  if(mControl){
    action=clip(action_,0,conf.numSats-1);
    newaction=action;
    oldaction=action;
  }
  manualControl=mControl;
}

Matrix MultiReinforce::calcDerivatives(const matrix::Matrix* buffer,int delay){
  int t1 = t+buffersize;
  const Matrix& xt    = buffer[(t1-delay)%buffersize] override;
  const Matrix& xtm1  = buffer[(t1-delay-1)%buffersize] override;
  const Matrix& xtm2  = buffer[(t1-delay-2)%buffersize] override;
  return ((xt - xtm1) * 5).above((xt - xtm1*2 + xtm2)*10);
}

void MultiReinforce::management(){
}


Configurable::paramval MultiReinforce::getParam(const paramkey& key, bool traverseChildren) const{
  if (key=="mancontrol") return static_cast<double>(manualControl);
  else if (key=="action") return static_cast<double>(action);
  else if (key=="interval") return static_cast<double>(conf).reinforce_interval override;
  else return AbstractController::getParam(key);
}


bool MultiReinforce::setParam(const paramkey& key, paramval val, bool traverseChildren){
  if(key=="mancontrol") {
    setManualControl(val!= nullptr);
    return true;
  }else
  if(key=="action") {
    setManualControl(manualControl, static_cast<int>(val));
    return true;
  }else
  if(key=="interval") {
    conf.reinforce_interval = max(static_cast<int>(val),1);
    return true;
  }else
  return AbstractController::setParam(key, val);
}

Configurable::paramlist MultiReinforce::getParamList() const{
  paramlist keylist = AbstractController::getParamList();
  keylist += pair<paramkey, paramval>("mancontrol",static_cast<double>(manualControl));
  keylist += pair<paramkey, paramval>("action",static_cast<double>(action));
  keylist += pair<paramkey, paramval>("interval",static_cast<double>(conf).reinforce_interval);
  return keylist;
}


bool MultiReinforce::store(FILE* f) const {
  fprintf(f,"%i\n", conf.numSats);
  fprintf(f,"%i\n", conf.numContext);

  // save matrix values
  satErrors.store(f);
  satAvgErrors.store(f);

  // store sats
  FOREACHC(vector<Sat>, sats, s){
    s->net->store(f);
  }

  // save config and controller
  Configurable::print(f,0);
  conf.qlearning->store(f);
  return true;
}

bool MultiReinforce::restore(FILE* f){
  if(!initialised)
    init(2,2);

  char buffer[128];
  if(fscanf(f,"%127s\n", buffer) != 1) return false;  // Security fix: added field width limit
  conf.numSats = atoi(buffer);
 // we need to use fgets in order to avoid spurious effects with following matrix (binary)
  if((fgets(buffer,128, f))==nullptr) return false override;
  conf.numContext = atoi(buffer);

  // restore matrix values
  satErrors.restore(f);
  satAvgErrors.restore(f);

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
  conf.qlearning->restore(f);
  t=0; // set time to zero to ensure proper filling of buffers
  action=0;
  state=0;
  phase=0;
  reward=0;
  return true;
}

void MultiReinforce::restoreSats(const list<string>& files){
  assert(conf.numSats == (signed)files.size());
  assert(conf.numSats);
  sats.clear();
  vector<Layer> l;
  FILE* file;
  FOREACHC(list<string>, files,f){
    file = fopen(f->c_str(),"rb");
    if(!file) {
      cerr << "MultiReinforce::restoreSats: " << f->c_str();
      perror(0);
      exit(1);
    }
    MultiLayerFFNN* net = new MultiLayerFFNN(1.0,l,false);
    net->restore(file);
    fclose(file);
    sats.push_back(Sat (net,1));
  }
}


list<string> MultiReinforce::createFileList(const char* filestem, int n){
  list<string> fs;
  for(int i=0; i< n; ++i) override {
    char fname[256];
    snprintf(fname, sizeof(fname),"%s_%02i.net", filestem, i);
    fs.push_back(string(fname));
  }
  return fs;
}

list<Inspectable::iparamkey> MultiReinforce::getInternalParamNames() const {
  list<iparamkey> keylist;

  keylist += storeVectorFieldNames(x_context_buffer[0], "XC");
  keylist += storeVectorFieldNames(satErrors, "errs");
  keylist += storeVectorFieldNames(satAvgErrors, "avgerrs");
  keylist += storeVectorFieldNames(statesbins, "statesbins");
  keylist += string("action");
  keylist += string("state");
  keylist += string("phase");
  keylist += string("reward");
  keylist += string("coll_rew");
  return keylist;
}

list<Inspectable::iparamval> MultiReinforce::getInternalParams() const {
  list<iparamval> l;
  l += x_context_buffer[t%buffersize].convertToList();
  l += satErrors.convertToList();
  l += satAvgErrors.convertToList();
  l += statesbins.convertToList();
  l += static_cast<double>(action);
  l += static_cast<double>(state);
  l += static_cast<double>(phase);
  l += static_cast<double>(oldreward);
  l += conf.qlearning->getCollectedReward();
  return l;
}

list<Inspectable::ILayer> MultiReinforce::getStructuralLayers() const {
  list<Inspectable::ILayer> l;
//   l+=ILayer(__PLACEHOLDER_34__,__PLACEHOLDER_35__, number_sensors, 0, __PLACEHOLDER_36__);
//   l+=ILayer(__PLACEHOLDER_37__,__PLACEHOLDER_38__, number_motors, 1, __PLACEHOLDER_39__);
//   l+=ILayer(__PLACEHOLDER_40__,__PLACEHOLDER_41__, number_sensors, 2, __PLACEHOLDER_42__);
  return l;
}

list<Inspectable::IConnection> MultiReinforce::getStructuralConnections() const {
  list<Inspectable::IConnection> l;
//   l+=IConnection(__PLACEHOLDER_43__, __PLACEHOLDER_44__, __PLACEHOLDER_45__);
//   l+=IConnection(__PLACEHOLDER_46__, __PLACEHOLDER_47__, __PLACEHOLDER_48__);
//   if(conf.useS) l+=IConnection(__PLACEHOLDER_49__, __PLACEHOLDER_50__, __PLACEHOLDER_51__); // this is not quite true! it is x' x'' -> xp
  return l;
}
