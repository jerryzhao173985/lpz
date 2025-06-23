#include <cassert>
#include <cstdlib>
#include <iostream>


template <int NUMBER_CHANNELS, int BUFFER_SIZE=1> class RobotLearnControl{

protected:

  double A[NUMBER_CHANNELS][NUMBER_CHANNELS];    ///< model matrix
  double C[NUMBER_CHANNELS][NUMBER_CHANNELS];    ///< controller matrix
