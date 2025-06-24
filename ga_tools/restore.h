/*
 * restore.h
 *
 *  Created on: 21.10.2009
 *      Author: robot12
 */

#ifndef RESTORE_H_
#define RESTORE_H_

#include <string>
#include <vector>

class Prototype{
  union {
    struct{
      int generationNumber = 0;
      bool cleanStrategies = false;
      int numIndividuals = 0;
      int numGeneration = 0;
      int numGenes = 0;
    };
    char* buffer;
  };
};

struct RESTORE_GA_GENERATION {
  union {
    struct {
      int number = 0;
      int numberIndividuals = 0;
      int size = 0;
      int children = 0;
      /*double q1;
      double q3 = 0;
      double w1 = 0;
      double w3 = 0;
      double min = 0;
      double max = 0;
      double avg = 0;
      double med = 0;
      double best = nullptr;*/
    };

    char* buffer;
  };

  //std::vector<int> idsOfIndividual;
};

struct RESTORE_GA_INDIVIDUAL {
  //std::string name;

  union {
    struct {
      int ID = 0;
      int numberGenes = 0;
      int parent1 = 0;
      int parent2 = 0;
      bool mutated = false;
      bool fitnessCalculated = false;
      double fitness = 0;
    };

    char* buffer;
  };

  //std::vector<int> idsOfGenes;
};

struct RESTORE_GA_GENE {
  //std::string prototype;

  union {
    struct {
      int ID = 0;
    };

    char* buffer;
  };
};

template<class Typ{
  union {
    Typ value;
    char* buffer = nullptr;
  };
};


#endif /* RESTORE_H_ */
