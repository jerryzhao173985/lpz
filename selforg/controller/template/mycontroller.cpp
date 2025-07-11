/***************************************************************************
 *   Copyright (C) 2025 by Your Name                                       *
 *   your.email@example.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mycontroller.h"
#include <selforg/matrixutils.h>
#include <cmath>
#include <cassert>

using namespace matrix;

MyController::MyController(double init_feedback_strength, bool use_logarithmic)
  : AbstractController("MyController", "1.0"),
    epsA(0.05), epsC(0.05), damping(0.01), noise(0.01),
    useLogarithmic(use_logarithmic), useTeaching(false),
    number_sensors(0), number_motors(0), t(0), initialized(false),
    y_teaching(nullptr), x_teaching(nullptr) {
  
  // Set configurable parameters
  addParameter("epsA", &epsA, 0.0, 1.0, "learning rate for model");
  addParameter("epsC", &epsC, 0.0, 1.0, "learning rate for controller");
  addParameter("damping", &damping, 0.0, 0.5, "damping factor");
  addParameter("noise", &noise, 0.0, 1.0, "noise strength");
  
  // Store initial values
  addParameterDef("initFeedbackStrength", &init_feedback_strength, 
                  init_feedback_strength, 0.0, 2.0, "initial feedback strength");
  addParameterDef("logarithmic", &useLogarithmic, useLogarithmic, 
                  "use logarithmic error");
}

MyController::~MyController() {
  if(x_intern) delete[] x_intern;
  if(y_intern) delete[] y_intern;
}

void MyController::init(int sensornumber, int motornumber, RandGen* randGen) {
  assert(sensornumber > 0 && motornumber > 0);
  
  number_sensors = sensornumber;
  number_motors = motornumber;
  
  // Initialize matrices
  A.set(number_sensors, number_motors);
  C.set(number_motors, number_sensors);
  S.set(number_sensors, number_sensors);
  h.set(number_motors, 1);
  b.set(number_sensors, 1);
  
  // Initialize A matrix (model)
  if(randGen) {
    A = A.mapP(randGen, random_minusone_to_one) * 0.1;
  } else {
    A = A * 0.1;
  }
  
  // Initialize C matrix (controller) with feedback strength
  double init_feedback = getParam("initFeedbackStrength");
  C = C.mapP(randGen, random_minusone_to_one) * init_feedback;
  
  // Make it more diagonal-dominant for stability
  for(int i = 0; i < std::min(number_sensors, number_motors); ++i) {
    C.val(i, i) += init_feedback;
  }
  
  // Initialize S as identity (can be adapted for noise shaping)
  S.toId();
  S *= noise;
  
  // Initialize biases
  h = h.mapP(randGen, random_minusone_to_one) * 0.1;
  b = b * 0.0;  // Start with zero model bias
  
  // Initialize state
  x.set(number_sensors, 1);
  y.set(number_motors, 1);
  x_pred.set(number_sensors, 1);
  xi.set(number_sensors, 1);
  
  // Initialize buffers
  for(int i = 0; i < 2; ++i) {
    x_buffer[i].set(number_sensors, 1);
    y_buffer[i].set(number_motors, 1);
  }
  
  // Allocate internal arrays
  x_intern = new sensor[number_sensors];
  y_intern = new motor[number_motors];
  
  t = 0;
  initialized = true;
}

void MyController::step(const sensor* sensors, int number_sensors,
                       motor* motors, int number_motors) {
  assert(initialized);
  assert(number_sensors == this->number_sensors);
  assert(number_motors == this->number_motors);
  
  // Store sensor values
  x.set(number_sensors, 1, sensors);
  
  // Store in buffer for delay
  x_buffer[t % 2] = x;
  
  // Calculate controller output
  y = calculateControllerValues(x);
  
  // Apply activation function and clip
  for(int i = 0; i < number_motors; ++i) {
    y.val(i, 0) = clip(g(y.val(i, 0)));
  }
  
  // Add noise
  if(noise > 0) {
    y += S * Matrix::random(number_motors, 1, -noise, noise);
  }
  
  // Store motor values
  y.convertToBuffer(motors, number_motors);
  y_buffer[t % 2] = y;
  
  // Learning step (if we have history)
  if(t > 0) {
    learn();
  }
  
  t++;
}

void MyController::stepNoLearning(const sensor* sensors, int number_sensors,
                                 motor* motors, int number_motors) {
  assert(initialized);
  assert(number_sensors == this->number_sensors);
  assert(number_motors == this->number_motors);
  
  // Store sensor values
  x.set(number_sensors, 1, sensors);
  
  // Calculate controller output
  y = calculateControllerValues(x);
  
  // Apply activation function and clip
  for(int i = 0; i < number_motors; ++i) {
    y.val(i, 0) = clip(g(y.val(i, 0)));
  }
  
  // Store motor values (no learning, no noise)
  y.convertToBuffer(motors, number_motors);
}

Matrix MyController::calculateControllerValues(const Matrix& x) {
  // Basic controller equation: y = g(C*x + h)
  return C * x + h;
}

void MyController::learn() {
  // Get previous values
  const Matrix& x_tm1 = x_buffer[(t - 1) % 2];
  const Matrix& y_tm1 = y_buffer[(t - 1) % 2];
  
  // Forward model prediction: x_pred = A * y_tm1 + b
  x_pred = A * y_tm1 + b;
  
  // Prediction error
  xi = x - x_pred;
  
  // Apply logarithmic error if enabled
  if(useLogarithmic) {
    xi = xi.map(logisticError);
  }
  
  // Update model (A matrix)
  if(epsA > 0) {
    Matrix dA = xi * y_tm1.T();
    A += dA * epsA;
    
    // Update model bias
    b += xi * (epsA * 0.1);
  }
  
  // Update controller (C matrix) using homeokinetic principle
  if(epsC > 0) {
    // Calculate learning signal
    Matrix v = y_tm1;
    for(int i = 0; i < number_motors; ++i) {
      v.val(i, 0) *= g_s(v.val(i, 0));  // Multiply by derivative
    }
    
    // Homeokinetic update
    Matrix dC = (A.T() * xi) * v.T();
    C += dC * epsC;
    
    // Update controller bias
    h += (A.T() * xi) * (epsC * 0.1);
    
    // Apply damping for stability
    if(damping > 0) {
      C *= (1.0 - damping);
      h *= (1.0 - damping);
    }
  }
  
  // Check stability
  ensureStability();
}

void MyController::ensureStability() {
  // Check for NaN/Inf in matrices
  if(!C.isNormal()) {
    fprintf(stderr, "MyController: C matrix has NaN/Inf! Resetting...\n");
    C.toId();
    C *= getParam("initFeedbackStrength");
  }
  
  if(!A.isNormal()) {
    fprintf(stderr, "MyController: A matrix has NaN/Inf! Resetting...\n");
    A = A * 0.1;
  }
  
  // Optional: Check spectral radius
  // This is expensive, so only do occasionally
  if(t % 1000 == 0) {
    // Simple norm check instead of full eigenvalue computation
    double c_norm = C.norm();
    if(c_norm > 10.0) {
      C *= 5.0 / c_norm;  // Scale down
    }
  }
}

/************** TEACHABLE INTERFACE **************/

void MyController::setMotorTeaching(const motor* teaching, int len) {
  assert(len == number_motors);
  y_teaching = teaching;
  useTeaching = true;
}

void MyController::setSensorTeaching(const sensor* teaching, int len) {
  assert(len == number_sensors);
  x_teaching = teaching;
}

/************** STOREABLE INTERFACE **************/

bool MyController::store(FILE* f) const {
  // Store dimensions
  fprintf(f, "%i\n", number_sensors);
  fprintf(f, "%i\n", number_motors);
  
  // Store matrices
  A.store(f);
  C.store(f);
  h.store(f);
  b.store(f);
  
  // Store parameters
  Configurable::print(f, 0);
  
  return true;
}

bool MyController::restore(FILE* f) {
  // Restore dimensions
  int sensors, motors;
  if(fscanf(f, "%i\n", &sensors) != 1) return false;
  if(fscanf(f, "%i\n", &motors) != 1) return false;
  
  // Initialize if dimensions match
  if(!initialized || sensors != number_sensors || motors != number_motors) {
    init(sensors, motors);
  }
  
  // Restore matrices
  A.restore(f);
  C.restore(f);
  h.restore(f);
  b.restore(f);
  
  // Restore parameters
  Configurable::parse(f);
  
  t = 0;  // Reset time
  return true;
}

/************** INSPECTABLE INTERFACE ************/

std::list<Matrix> MyController::getInternalState() const {
  std::list<Matrix> states;
  states.push_back(A);
  states.push_back(C);
  states.push_back(h);
  states.push_back(b);
  states.push_back(x);
  states.push_back(y);
  states.push_back(xi);
  return states;
}

std::list<std::string> MyController::getInternalStateNames() const {
  std::list<std::string> names;
  names.push_back("A");
  names.push_back("C");
  names.push_back("h");
  names.push_back("b");
  names.push_back("x");
  names.push_back("y");
  names.push_back("xi");
  return names;
}

/************** CONFIGURABLE INTERFACE ***********/

paramval MyController::getParam(const paramkey& key, bool traverseChildren) const {
  if(key == "epsA") return epsA;
  else if(key == "epsC") return epsC;
  else if(key == "damping") return damping;
  else if(key == "noise") return noise;
  else if(key == "logarithmic") return useLogarithmic;
  else return Configurable::getParam(key, traverseChildren);
}

bool MyController::setParam(const paramkey& key, paramval val, bool traverseChildren) {
  if(key == "epsA") epsA = val;
  else if(key == "epsC") epsC = val;
  else if(key == "damping") damping = val;
  else if(key == "noise") noise = val;
  else if(key == "logarithmic") useLogarithmic = (val != 0);
  else return Configurable::setParam(key, val, traverseChildren);
  return true;
}

paramlist MyController::getParamList() const {
  paramlist list;
  list += std::pair<paramkey, paramval>("epsA", epsA);
  list += std::pair<paramkey, paramval>("epsC", epsC);
  list += std::pair<paramkey, paramval>("damping", damping);
  list += std::pair<paramkey, paramval>("noise", noise);
  list += std::pair<paramkey, paramval>("logarithmic", useLogarithmic);
  return list;
}

// Helper function for logarithmic error
static double logisticError(double x) {
  return tanh(x);
}