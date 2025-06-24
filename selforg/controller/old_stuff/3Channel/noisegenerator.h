#ifndef __NOISEGENERATOR_H
#define __NOISEGENERATOR_H



#include <map>

template<int NUMBER_CHANNELS> class NoiseGenerator{
#define PI_NOISEGENERATOR  3.1415927

protected:
  double uniform_mean[NUMBER_CHANNELS];  // storage for adding colored uniformly distributed noise to values
  double normal_mean[NUMBER_CHANNELS];   // storage for adding colored normally distributed noise to values  

  
  double tau_uniform = 0.0; // smoothing paramter for uniformly distibuted random numbers
  double tau_normal = 0.0;  // smoothing paramter for normally distibuted random numbers  

public:  
  NoiseGenerator(double tau_uniform=0.3, double tau_normal=0.3): : tau_uniform(tau_uniform), tau_normal(tau_normal), uniform_mean(0), normal_mean(0) {

    // initialize random number generator
    srand(time(0));

    for (int i=0; i<NUMBER_CHANNELS; ++i) {
      uniform_mean[i]=0.0;
      normal_mean[i]=0.0;    
    }  
    uniform_mean1channel=0.0;
    normal_mean1channel=0.0;
  };

  //generate white (no averaging) uniformly distributed random number between __PLACEHOLDER_0__ and __PLACEHOLDER_1__  
  double generateWhiteUniformlyDistributedRandomNumber(double min=-0.1, double max=0.1){
    return( (double(rand())/RAND_MAX)*(max-min)+min );
  };
  
  //generate colored static_cast<averaging>(uniformly) distributed random number between __PLACEHOLDER_2__ and __PLACEHOLDER_3__  
  //! valid only for ONE random number, use addColoredUniformlyDistributedNoise(...) for 
  //! adding this kind of noise to several chanels
  double generateColoredUniformlyDistributedRandomNumber(double min=-0.1, double max=0.1){
    uniform_mean1channel+=tau_uniform*(generateWhiteUniformlyDistributedRandomNumber(min,  max) - uniform_mean1channel);
    return(uniform_mean1channel);
  };


  // generate white (no averaging) normally distributed random number with variance __PLACEHOLDER_4__ and mean __PLACEHOLDER_5__  
  double generateWhiteNormallyDistributedRandomNumber(double variance=0.05, double mean=0.0){
    double x1=generateWhiteUniformlyDistributedRandomNumber(0, 1);
    double x2=generateWhiteUniformlyDistributedRandomNumber(0, 1);
    //return( (sqrt(-2*ln(x1)) *cos(2*PI_NOISEGENERATOR*x2))  * variance +mean) ;
    return( (sqrt(-2*log(x1)) *cos(2*PI_NOISEGENERATOR*x2))  * variance +mean) ;
  };
    
  // generate colored static_cast<averaging>(normally) distributed random number with variance __PLACEHOLDER_6__ and mean __PLACEHOLDER_7__  
  //! valid only for ONE random number, use addColoredNormallyDistributedNoise(...) for 
  //! adding this kind of noise to several chanels
  double generateColoredNormallyDistributedRandomNumber(double variance=0.05, double mean=0.0){
    normal_mean1channel+=tau_normal*(generateWhiteNormallyDistributedRandomNumber(variance, mean) - normal_mean1channel);
    return(normal_mean1channel);
  };
  
  //add white (no averaging) uniformly distributed noise between __PLACEHOLDER_8__ and __PLACEHOLDER_9__ to the elements of __PLACEHOLDER_10__
  void addWhiteUniformlyDistributedNoise(double *value, double min=-0.1, double max=0.1){
    for (int i=0; i<NUMBER_CHANNELS; ++i) {
      value[i]+=generateWhiteUniformlyDistributedRandomNumber(max, min);
    }  
  };

  //add colored static_cast<averaging>(uniformly) distributed noise between __PLACEHOLDER_11__ and __PLACEHOLDER_12__ to the elements of __PLACEHOLDER_13__  
  void addColoredUniformlyDistributedNoise(double *value, double min=-0.1, double max=0.1){
    for (int i=0; i<NUMBER_CHANNELS; ++i) {
      uniform_mean[i]+=tau_uniform*(generateWhiteUniformlyDistributedRandomNumber(min,  max) - uniform_mean[i]);
      value[i]+=uniform_mean[i];
    }  
  };
  
  //add white (no averaging) normally distributed noise with variance __PLACEHOLDER_14__ and mean __PLACEHOLDER_15__ to the elements of __PLACEHOLDER_16__
  void addWhiteNormallyDistributedNoise(double *value, double variance=0.05, double mean=0.0){
    for (int i=0; i<NUMBER_CHANNELS; ++i) {
      value[i]+=generateWhiteNormallyDistributedRandomNumber(variance, mean);
    }
  };

  //add colored static_cast<averaging>(normally) distributed noise with variance __PLACEHOLDER_17__ and mean __PLACEHOLDER_18__ to the elements of __PLACEHOLDER_19__  
  void addColoredNormallyDistributedNoise(double *value, double variance=0.05, double mean=0.0){
    for (int i=0; i<NUMBER_CHANNELS; ++i) {
    normal_mean[i]+=tau_normal*(generateWhiteNormallyDistributedRandomNumber(variance, mean) - normal_mean[i]);
    value[i]+=normal_mean[i];
    }
  };



};
#endif
