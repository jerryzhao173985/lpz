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

struct Prototype{
  union {
    struct{
      int generationNumber;
      bool cleanStrategies;
      int numIndividuals;
      int numGeneration;
      int numGenes;
    };
    char buffer[sizeof(int)*4 + sizeof(bool)];
  };
};

struct RESTORE_GA_GENERATION {
  union {
    struct {
      int number;
      int numberIndividuals;
      int size;
      int children;
    };
    char buffer[sizeof(int)*4];
  };

  //std::vector<int> idsOfIndividual;
};

struct RESTORE_GA_INDIVIDUAL {
  union {
    struct {
      int ID;
      int numberGenes;
      int parent1;
      int parent2;
      bool mutated;
      bool fitnessCalculated;
      double fitness;
    };
    char buffer[sizeof(int)*4 + sizeof(bool)*2 + sizeof(double)];
  };

  //std::vector<int> idsOfGenes;
};

struct RESTORE_GA_GENE {
  union {
    struct {
      int ID;
    };
    char buffer[sizeof(int)];
  };
};

template<class Typ>
struct RESTORE_GA_TEMPLATE {
  union {
    Typ value;
    char buffer[sizeof(Typ)];
  };
};


#endif /* RESTORE_H_ */
