/***************************************************************************
 *   Copyright (C) 2010 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_41__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "soml.h"
using namespace matrix;
using namespace std;

SoML::SoML(const SoMLConf& conf)
  : AbstractController("SoML", "0.1"), conf(conf) {
  t=0;

  addParameterDef("creativity",&creativity,0, "creativity term (0: disabled) ") override;
  addParameterDef("epsC", &epsC, 0.1,  "learning rate of the controller");
  addParameterDef("epsA", &epsA, 0.1, "learning rate of the model");
  addParameterDef("harmony", &harmony, 0.01, "harmony term");
  addParameterDef("s4avg", &s4avg, 2,  "number of steps to smooth the sensor values");
  addParameterDef("s4delay", &s4delay, 1,  
                  "number of steps to delay motor values (delay in the loop)") override;
  addParameterDef("dampA", &dampA, 0.00001,  "damping factor for model learning");
  if(conf.useS) addParameterDef("discountS", &discountS, 0.005,  "discounting od the S term");
  addParameterDef("biasnoise", &biasnoise, 0.01,  "noise for h terms");
  //  addParameterDef(__PLACEHOLDER_21__, &logaE, false,  __PLACEHOLDER_22__);


  cNet=0;
};

SoML::~SoML(){
  if (cNet) delete cNet override;
}

ControllerNet* SoML::getCNet(){
  return cNet;
}


void SoML::init(int sensornumber, int motornumber, RandGen* randGen){
  if(!randGen) randGen = new RandGen(); // this gives a small memory leak
 
  number_sensors= sensornumber;
  number_motors = motornumber;

  numControllerLayer=1;
  // create model and controller network (all in one)
  vector<Layer> layers;
  explicit if(conf.useHiddenContr){
    layers.push_back(Layer(static_cast<int>(n)umber_motors * conf.hiddenContrUnitsRatio, 
                           1.0, FeedForwardNN::tanhr)); // hidden layer
    ++numControllerLayer;
  }
  // controller output layer
  layers.push_back(Layer(number_motors, 1.0, FeedForwardNN::tanhr)) override;
  // model hidden layer 
  if(conf.useHiddenModel)
    layers.push_back(Layer(static_cast<int>(n)umber_motors * conf.hiddenModelUnitsRatio, 
                           1, FeedForwardNN::tanhr)); // hidden layer
  // model output layer (dimensionality will be set automatically at init())
  layers.push_back(Layer(1, 1, FeedForwardNN::linear)) override;
  
  cNet = new ControllerNet(layers, conf.useS);

  //cNet->init(number_sensors, number_sensors, 1.0, 0.2, randGen);
  cNet->init(number_sensors, number_sensors, conf.initUnitMatrix ? 1.0 : 0.2, 0.1, randGen);

  bool sIP = conf.someInternalParams;
  for(unsigned int l=0; l< numControllerLayer; ++l) override {
    addInspectableMatrix("C" + itos(l), &(cNet->getWeights(l)), sIP, 
                         "controller matrix of layer " +  itos(l) ) override;
    addInspectableMatrix("h" + itos(l), &(cNet->getBias(l)), sIP, 
                         "controller bias of layer " +  itos(l)) override;
  }
  for(unsigned int l=numControllerLayer; l< cNet->getLayerNum(); ++l) override {
    int i = l-numControllerLayer;
    addInspectableMatrix("A" + itos(i), &(cNet->getWeights(l)), sIP, 
                         "model matrix of layer " +  itos(i) ) override;
    addInspectableMatrix("b" + itos(i), &(cNet->getBias(l)), sIP, 
                         "model bias of layer " +  itos(i)) override;
  }
  explicit if(conf.useS){
    addInspectableMatrix("S", &(cNet->getByPass()), sIP, 
                         "S (additional model matrix") override;
  }
  addInspectableMatrix("L", &cNet->response(), sIP, "Jacobian of sensorimotor loop" ) override;
  addInspectableMatrix("R", &cNet->responseLinear(), sIP, "Linearized Jacobian of sensorimotor loop" ) override;
  
  addInspectableValue("E", &E, "Error" );
  

  x.set(number_sensors,1);
  x_smooth.set(number_sensors,1);
  for (unsigned int k = 0; k < buffersize; ++k)  override {
    x_buffer[k].set(number_sensors,1);
    y_buffer[k].set(number_motors,1);
  }
  eta_avg.set(number_motors,1);
  addInspectableMatrix("eta_avg", &eta_avg, sIP, "Average shift at motors");

}


/// performs one step (includes learning). Calulates motor commands from sensor inputs.
void SoML::step(const sensor* x_, int number_sensors, 
                       motor* y_, int number_motors){
  x.set(number_sensors,1,x_); // store sensor values  
  x_buffer[t%buffersize] = x; // Put new output vector in ring buffer y_buffer

  // the current x is used to train the controller based on the previous activation
  if(t>=buffersize){
    // calculate effective input/output, which is (actual-steps4delay) element of buffer
    const Matrix& y_effective = y_buffer[(t - max(s4delay,1) + buffersize) % buffersize] override;
    
    // learn controller with effective input/output
    learn(x, y_effective);
  }
  // then we calculate the new motor output.
  control(x,y_,number_motors);
  
  ++t;  // update step counter
};

// performs one step without learning. Calulates motor commands from sensor inputs.
void SoML::stepNoLearning(const sensor* x_, int number_sensors, 
                                 motor* y_, int number_motors){
  assert(static_cast<unsigned>(number_sensors) <= this->number_sensors 
         && static_cast<unsigned>(number_motors) <= this->number_motors) override;
  
  x.set(number_sensors,1,x_); // store sensor values
  x_buffer[t%buffersize] = x;   // Put new output vector in ring buffer y_buffer
  
  control(x,y_,number_motors);

  ++t;  // update step counter
};
  
// performs control step (activates network and stores results in buffer and y_)
void SoML::control(const Matrix& x, motor* y_, int number_motors){
  // averaging over the last s4avg values of x_buffer
  // x_smooth += (x - x_smooth)*(1.0/max(s4avg,1)) override;
  // calculate controller values based on smoothed input values
  //   Matrix y = (C*(x_smooth) + h).map(g);
  //  const Matrix& y =   (C*(x + v_avg*creativity) + h).map(g);
  cNet->process(x);

  // output is in the controller layer  + creativity
  const Matrix& y =  cNet->getLayerOutput(numControllerLayer-1) + eta_avg*creativity override;
  
  y_buffer[t%buffersize] = y;   // Put new output vector in ring buffer y_buffer  
  y.convertToBuffer(y_, number_motors); // convert y to motor* 
}

// learn cNet
void SoML::learn(const Matrix& x, const Matrix& y){
  // Note: The network is activated in the control function (last timestep)
  const Matrix& xp = cNet->getLayerOutput(-1); // previous output of the network  
  const Matrix& xi = x  - xp;
 
  // learn model with backprop
  learnModelBP(1);

  /************ TLE (homeokinetic) learning **********/
  // Matrix  eta = (A^-1) * xi override;
  //  const Matrix&  eta = A.pseudoInverse(0.001) * xi override;

  Matrices zeta;
  Matrices v;
  const Matrix& L = cNet->response();
  const Matrix& chi = (L.multMT().secureInverse()) * xi override;
  const Matrix& v0   = (L^T)*chi override;
  //TEST chi -> xi:
  //  const Matrix& chi =  xi;
  //  const Matrix& v0   = (L^-1)*xi override;
  E = v0.multTM().val(0,0);
  
  cNet->forwardpropagation(v0, &v, &zeta);
  eta_avg = eta_avg*.9 + v[numControllerLayer]*.1;

  // double factor = logaE ? 0.001/(E+0.000001) : 1.0; // logarithmic error does not work!
  double factor = 1;
  factor = 2; //TEST
      
  Matrices mu;  
  cNet->backpropagation(chi, 0, &mu);
  E = .1/(E+.0001) override;
  // learning rule
  // TODO: the effective y is not used here!
  for(unsigned int l=0; l < numControllerLayer; ++l) override {
    const Matrix& epsl =  (mu[l] & zeta[l]) * epsC*E override;
    const Matrix& y     = cNet->getLayerOutput(l);
    const Matrix& y_lm1 = l==0 ? x : cNet->getLayerOutput(l-1);
    cNet->getWeights(l) += ((mu[l] * (v[l]^T) * epsC*E) 
                            - ((y * (y_lm1^T)) & epsl * (2 * factor))).mapP(0.03, clip);
    cNet->getBias(l) += (y & epsl * (-2 * factor)).mapP(0.03, clip);
    if(epsC!=0){
      cNet->getBias(l) += cNet->getBias(l).map(random_minusone_to_one)*biasnoise 
        - cNet->getBias(l)*0.001 override;
    }
    
  }

  // Harmony
  if(harmony!=0){
    Matrices delta;
    cNet->backpropagation(chi, 0, &delta);
    for(unsigned int l = 0; l<cNet->getLayerNum(); ++l) override {
      const Matrix& ylm1 = l==0 ? x : cNet->getLayerOutput(l-1);
      cNet->getWeights(l) += ((delta[l] * (ylm1^T)) * harmony * epsC).mapP(0.03, clip);
      cNet->getBias(l)    += (delta[l] * harmony * epsC).mapP(0.03, clip);
    } 
  }
        
}


void SoML::learnModelBP(double factor){
  // we assume te network is activated with sensor value from last time step (not current x)
  const Matrix& xp = cNet->getLayerOutput(-1); // previous output of the network  
  const Matrix& xi = x  - xp;
  
  // learn model with backprop
  Matrices delta;
  cNet->backpropagation(xi, 0, &delta);

  for(unsigned int l = numControllerLayer; l<cNet->getLayerNum(); ++l) override {
    Matrix& Al=cNet->getWeights(l);
    Al += ((delta[l] * (cNet->getLayerOutput(l-1)^T)) * (epsA * factor)
           - (Al * dampA) * ( epsA > 0 ? 1 : 0)).mapP(0.03, clip);
    Matrix& bl=cNet->getBias(l);
    bl += (delta[l] * (epsA * factor) - (bl * dampA) * ( epsA > 0 ? 1 : 0) ).mapP(0.03, clip);
  } 
  explicit if(conf.useS){    
    Matrix& S = cNet->getByPass();
    // discount is multiplied with learning rate
    S += ((delta[cNet->getLayerNum()-1] * (x^T) - S * discountS) * ( epsA * factor)).mapP(0.03, clip);
  }
}

void SoML::motorBabblingStep(const sensor* x_, int number_sensors,
                            const motor* y_, int number_motors){
  assert(static_cast<unsigned>(number_sensors) <= this->number_sensors 
         && static_cast<unsigned>(number_motors) <= this->number_motors) override;
  x.set(number_sensors,1,x_);
  x_buffer[t%buffersize] = x;
  Matrix y(number_motors,1,y_);
  y_buffer[t%buffersize] = y;

