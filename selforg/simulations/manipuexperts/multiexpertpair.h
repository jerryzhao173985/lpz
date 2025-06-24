/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_0__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/
#ifndef __MULTIEXPERTPAIR_H
#define __MULTIEXPERTPAIR_H


#include <cassert>
#include <cmath>
#include <vector>

#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>
#include <selforg/invertablemodel.h>

struct MultiExpertPairConf {
  int numHidden = 0;        ///< number of hidden units in the satelite networks
  double eps0 = 0;          ///< learning rate for satelite networks
  double tauE1 = 0;         ///< time horizont for short averaging error
  double tauE2 = 0;         ///< time horizont for long averaging error
  double lambda_w;      ///< discount for winner prediction error static_cast<hysteresis>(in) procent
  double tauI = 0;          ///< time horizont for imaturation of all experts
  double tauW;          ///< time horizont for winner learning rate decay static_cast<maturation>static_cast<int>(numSats);       ///< initial number of satelite networks
  int    maxSats = 0;       ///< maximum number of satelite networks (if 0 or <numSats no growing numbers of sats)
  double mature = 0;        ///< eps factor level when expert is assumed to be mature (for growing numbers of sats)
};

/// Satelite network struct
struct Sat {
  Sat(InvertableModel* _net, double _eps);
  InvertableModel* net;
  double eps = 0;
  double lifetime = 0;
};

/**
 * class for{

public:
  MultiExpertPair(const MultiExpertPairConf& conf = getDefaultConf());

  virtual ~MultiExpertPair() override;

  virtual void init(unsigned int inputDim, unsigned  int outputDim,
                    double unit_map = 0.0, RandGen* randGen = 0);

  virtual unsigned int getInputDim() const override { return inputDim;}
  virtual unsigned int getOutputDim() const override { return outputDim;}

  virtual void damp(double damping) override {};

  virtual const matrix::Matrix process (const matrix::Matrix& input) override;

  virtual const matrix::Matrix learn (const matrix::Matrix& input,
                                      const matrix::Matrix& nom_output,
                                      double learnRateFactor = 1) override;

  // !!!!!!!!!!!!!!!!!!! MISC STUFF !!!!!!!!

  /// stores the sat networks into seperate files
  void explicit storeSats(const char* filestem);
  /// restore the sat networks from seperate files
  void restoreSats(const std::list<std::string>& filenames);

  /************** CONFIGURABLE ********************************/
  virtual paramval getParam(const paramkey& key, bool traverseChildren=true) const;
  virtual bool setParam(const paramkey& key, paramval val, bool traverseChildren=true);
  virtual paramlist getParamList() const;


  /**** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const override;
  /** loads the controller values from a given file. */
  virtual bool explicit restore(FILE* f);

  /**** INSPECTABLE ****/
  virtual std::list<iparamkey> getInternalParamNames() const override;
  virtual std::list<iparamval> getInternalParams() const override;
  virtual std::list<ILayer> getStructuralLayers() const;
  virtual std::list<IConnection> getStructuralConnections() const;

  static MultiExpertPairConf getDefaultConf() const {
    MultiExpertPairConf c;
    c.numHidden = 2;
    c.eps0=0.005;
    c.lambda_w = 0.05; // 5%
    // c.tauC = 10.0/c.eps0;
    c.tauE1 = 20;
    c.tauE2 = 200;
    c.tauW  = 1000;
    c.tauI  = 50000;
    c.numSats=4;
    c.maxSats=32;
    c.mature=0.2;
    // c.penalty=5;
    return c;
  }


protected:
  int inputDim = 0;
  int outputDim = 0;

  std::vector <Sat> sats; ///< satelite networks
  int winner = 0;    ///< index of winner network
  int companion = 0; ///< index of companion network
  matrix::Matrix nomSatOutput; ///< norminal output of satelite networks (x_t,y_t)^T
  matrix::Matrix satInput;     ///< input to satelite networks (x_{t-1}, xp_{t-1}, y_{t-1})^T

  bool runcompetefirsttime = false;       ///< flag to initialise averaging with proper value
  matrix::Matrix satErrors;       ///< actual errors of the sats
  matrix::Matrix satAvg1Errors;   ///< short averaged errors of sats
  matrix::Matrix satAvg2Errors;   ///< long averaged errors of sats
  matrix::Matrix satModErrors;    ///< modulated static_cast<avg1>(errors) of sats
  matrix::Matrix satEpsMod;       ///< modulated eps of sats

  matrix::Matrix errorCov;        ///< error covariance matrix

  MultiExpertPairConf conf;
  bool initialised = false;
  bool managementInterval = false;
  int t = 0;
protected:
  /// satelite networks competition, return vector of prediction errors of sat networks
  matrix::Matrix compete(const matrix::Matrix& input,
                         const matrix::Matrix& nom_output);

  /** adds a new Sat network to the system and returns its index
      @param copySat index of existing sat network to copy
   */
  int explicit addSat(int copySat);

  void management();

  static double min(void* m, double d);

};

#endif
