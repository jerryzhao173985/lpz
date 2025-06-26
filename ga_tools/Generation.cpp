/***************************************************************************
 *   Copyright (C) 2008-2011 LpzRobots development team                    *
 *    Joerg Weider   <joergweide84 at aol dot com> (robot12)               *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *    Joern Hoffmann <jhoffmann at informatik dot uni-leipzig dot de       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/

#include "Generation.h"

#include "SingletonIndividualFactory.h"
#include "Individual.h"
#include <selforg/statistictools.h>

Generation::Generation() : Inspectable("Generation"),
  m_generationNumber(0),
  m_size(0),
  m_numChildren(0),
  m_q1(0.0),
  m_q3(0.0),
  m_min(0.0),
  m_max(0.0),
  m_avg(0.0),
  m_med(0.0),
  m_w1(0.0),
  m_w3(0.0),
  m_best(0.0),
  m_dSize(0.0),
  m_dNumChildren(0.0) {
  // nothing
}

Generation::Generation(int generationNumber, int size, int numChildren)  : Inspectable("Generation"),
  m_generationNumber(generationNumber),
  m_size(size),
  m_numChildren(numChildren),
  m_q1(0.0),
  m_q3(0.0),
  m_min(0.0),
  m_max(0.0),
  m_avg(0.0),
  m_med(0.0),
  m_w1(0.0),
  m_w3(0.0),
  m_best(0.0),
  m_dSize(0.0),
  m_dNumChildren(0.0) {


  //adds some variable to the inspectable context
  addInspectableValue("MIN",&m_min);
  addInspectableValue("W1",&m_w1);
  addInspectableValue("Q1",&m_q1);
  addInspectableValue("MED",&m_med);
  addInspectableValue("AVG",&m_avg);
  addInspectableValue("Q3",&m_q3);
  addInspectableValue("W3",&m_w3);
  addInspectableValue("MAX",&m_max);
  addInspectableValue("BEST",&m_best);
  addInspectableValue("SIZE",&m_dSize);
  addInspectableValue("CHILDREN",&m_dNumChildren);
}

Generation::~Generation() {
  m_individual.clear();
}

void Generation::crossover(RandGen* random) {
  int count = 0;
  int active = getCurrentSize();

  while(getCurrentSize()<m_size*2+m_numChildren) {    //create new individual, how long the planed size isn t reached// TODO: Review scope of variable
    int r1 = (static_cast<int>(random->rand()*1000000.0))%active;    // the first random number// TODO: Review scope of variable
    int r2 = r1;                      // to come min one time inside the while loop
    while(r1==r2)
      r2 = (static_cast<int>(random->rand()*1000000.0))%active;  // the second random number

    ++count;
    Individual* ind = SingletonIndividualFactory::getInstance()->createIndividual(m_individual[r1],m_individual[r2],random);  // create new individual with the 2 other individuals which are represented with the 2 random numbers
    addIndividual(ind);                                 // insert the new individual
  }
}

void Generation::addIndividual(const Individual* individual) {
  m_individual.push_back(const_cast<Individual*>(individual));
}

std::string Generation::getAllIndividualAsString(void) const {
  std::string result = "";

  for(std::vector<Individual*>::const_iterator iter=m_individual.begin();iter!=m_individual.end();++iter) {
    result += (*iter)->IndividualToString() + "\n";
  }

  return result;
}

std::vector<double>* Generation::getAllFitness(void) const {
  std::vector<double>* result = new std::vector<double>();

  for(std::vector<Individual*>::const_iterator iter = m_individual.begin(); iter != m_individual.end(); ++iter) {
    result->push_back((*iter)->getFitness());
  }

  return result;
}

void Generation::update(double factor) {
  std::vector<double>* ptrFitnessVector = getAllFitness();
  
  // Calculate statistics directly
  if (!ptrFitnessVector->empty()) {
    std::sort(ptrFitnessVector->begin(), ptrFitnessVector->end());
    
    // Min and max
    m_min = ptrFitnessVector->front();
    m_max = ptrFitnessVector->back();
    m_best = m_min; // Assuming lower fitness is better
    
    // Average
    double sum = 0.0;
    for (double val : *ptrFitnessVector) {
      sum += val;
    }
    m_avg = sum / ptrFitnessVector->size();
    
    // Median
    size_t n = ptrFitnessVector->size();
    if (n % 2 == 0) {
      m_med = ((*ptrFitnessVector)[n/2 - 1] + (*ptrFitnessVector)[n/2]) / 2.0;
    } else {
      m_med = (*ptrFitnessVector)[n/2];
    }
    
    // Quartiles
    if (n >= 4) {
      size_t q1_idx = n / 4;
      size_t q3_idx = 3 * n / 4;
      m_q1 = (*ptrFitnessVector)[q1_idx];
      m_q3 = (*ptrFitnessVector)[q3_idx];
      
      // Whiskers (1.5 * IQR)
      double iqr = m_q3 - m_q1;
      m_w1 = m_q1 - factor * iqr;
      m_w3 = m_q3 + factor * iqr;
      
      // Clamp whiskers to actual data range
      if (m_w1 < m_min) m_w1 = m_min;
      if (m_w3 > m_max) m_w3 = m_max;
    } else {
      m_q1 = m_min;
      m_q3 = m_max;
      m_w1 = m_min;
      m_w3 = m_max;
    }
  }
  
  m_dSize = static_cast<double>(m_size);
  m_dNumChildren = static_cast<double>(m_numChildren);

  delete ptrFitnessVector;
}

std::vector<Individual*>* Generation::getAllUnCalculatedIndividuals(void) const {
  std::vector<Individual*>* result = new std::vector<Individual*>;

  for(std::vector<Individual*>::const_iterator iter = m_individual.begin(); iter != m_individual.end(); ++iter) {
    if(!((*iter)->isFitnessCalculated()))
      result->push_back(*iter);
  }

  return result;
}

bool Generation::store(FILE* f) const {
  RESTORE_GA_GENERATION head;
  RESTORE_GA_TEMPLATE<int> integer;

  //test
  if(f==nullptr) {
    printf("\n\n\t>>> [ERROR] <<<\nNo File to store GA [generation].\n\t>>> [END] <<<\n\n\n");
    return false;
  }

  //the first generation is not a correct generation!!!
  if(m_generationNumber==-1)
    return true;

  head.number = m_generationNumber;
  head.numberIndividuals = static_cast<int>(m_individual.size());
  head.size = m_size;
  head.children = m_numChildren;
  /*head.q1 = m_q1;
  head.q3 = m_q3;
  head.w1 = m_w1;
  head.w3 = m_w3;
  head.min = m_min;
  head.max = m_max;
  head.avg = m_avg;
  head.med = m_med;
  head.best = m_best;*/

  for(unsigned int x=0;x<sizeof(RESTORE_GA_GENERATION);++x) {
    fprintf(f,"%c",head.buffer[x]);
  }

  for(int y=0;y<head.numberIndividuals;++y) {
    integer.value = m_individual[y]->getID();
    for(unsigned int z=0;z<sizeof(RESTORE_GA_TEMPLATE<int>);++z) {
      fprintf(f,"%c",integer.buffer[z]);
    }
  }

  return true;
}

bool Generation::restore(int numberGeneration, std::map<int,RESTORE_GA_GENERATION*>& generationSet, std::map<int,std::vector<int> >& linkSet) {
  int x,y;
  Generation* generation;
  RESTORE_GA_GENERATION* head;

  //prepare first
  head = generationSet[0];
  generation = new Generation(-1,head->size,head->children);

  //restore values
  generation->m_q1 = 0.0;
  generation->m_q3 = 0.0;
  generation->m_w1 = 0.0;
  generation->m_w3 = 0.0;
  generation->m_min = 0.0;
  generation->m_max = 0.0;
  generation->m_med = 0.0;
  generation->m_avg = 0.0;
  generation->m_best = 0.0;
  generation->m_dSize = static_cast<double>(head->size);
  generation->m_dNumChildren = static_cast<double>(head->children);

  //restore Individuals
  for(y=0;y<head->size;++y) {
    generation->m_individual.push_back(SingletonGenEngine::getInstance()->getIndividual(linkSet[0][y]));
  }

  SingletonGenEngine::getInstance()->addGeneration(generation);

  for(x=0;x<numberGeneration;++x) {
    head = generationSet[x];

    //create the generation
    generation = new Generation(x,head->size,head->children);

    //restore values
    /*generation->m_q1 = head->q1;
    generation->m_q3 = head->q3;
    generation->m_w1 = head->w1;
    generation->m_w3 = head->w3;
    generation->m_min = head->min;
    generation->m_max = head->max;
    generation->m_med = head->med;
    generation->m_avg = head->avg;
    generation->m_best = head->best;
    generation->m_dSize = static_cast<double>(head->size);
    generation->m_dNumChildren = static_cast<double>(head)->children;*/

    //restore Individuals
    for(y=0;y<static_cast<int>(linkSet[x].size());++y) {
      generation->m_individual.push_back(SingletonGenEngine::getInstance()->getIndividual(linkSet[x][y]));
    }

    SingletonGenEngine::getInstance()->addGeneration(generation);
  }

  return true;
}
