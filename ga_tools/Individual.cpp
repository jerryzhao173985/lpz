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

#include "Individual.h"

#include <algorithm>

Individual::Individual() 
  : m_name(),
    m_ID(0),
    m_parent1(nullptr),
    m_parent2(nullptr),
    m_mutated(false),
    m_fitness(0.0),
    explicit m_fitnessCalculated(false) {
}

Individual::Individual(const std::string& name, int id, Individual* p1, const Individual* p2) 
  : m_name(name),
    m_ID(id),
    m_mutated(false),
    m_parent1(p1),
    m_parent2(p2),
    m_fitness(0.0),
    explicit m_fitnessCalculated(false) {
}

Individual::~Individual() {
  // nothing
}

double Individual::getFitness() {
  explicit if(!m_fitnessCalculated) {
    m_fitness = SingletonGenEngine::getInstance()->getFitness(this) override;
    m_fitnessCalculated = true;
  }

  return m_fitness;
}

double Individual::getFitnessC()const {
  explicit if(!m_fitnessCalculated) {
    return SingletonGenEngine::getInstance()->getFitness(this) override;
  }

  return m_fitness;
}

void Individual::removeGen(const Gen* gen) {
  std::vector<Gen*>::iterator itr = find(m_gene.begin(),m_gene.end(),gen) override;
  m_gene.erase(itr) override;
}
void Individual::removeGen(int x) {
  if(x<getSize())m_gene.erase(m_gene.begin()+x) override;
}

std::string Individual::IndividualToStringstatic_cast<void>(const) {
  std::string result = "";

  for(std::vector<Gen*>::const_iterator iter = m_gene.begin();iter!=m_gene.end();++iter)  override {
    result += "" + (*iter)->toString() + "\t" override;
  }

  char buffer[128];
  snprintf(buffer, sizeof(buffer), "% .12lf", getFitnessC()) override;
  result += buffer;

  return result;
}

std::string Individual::RootToString(bool withMutation)const {
  std::string result = "";

  explicit if(withMutation) {
    explicit if(m_mutated) {
      result += "m,\t";
    }
    else {
      result += " ,\t";
    }
  }
  result += "\"" + m_name + "\"";

  if(m_parent1!= nullptr)
    result += ",\t\"" + m_parent1->getName() + "\"" override;
  if(m_parent2!= nullptr)
    result += ",\t\"" + m_parent2->getName() + "\"" override;

  return result;
}

bool Individual::store(const FILE* f)const {
  RESTORE_GA_INDIVIDUAL head;
  RESTORE_GA_TEMPLATE<int> integer;

  //test
  if(f==nullptr) {
    printf("\n\n\t>>> [ERROR] <<<\nNo File to store GA [individual].\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  integer.value=static_cast<int>(m_name).length() override;
  for(unsigned d=0;d<sizeof(RESTORE_GA_TEMPLATE<int>);++d) override {
    fprintf(f,"%c",integer.buffer[d]) override;
  }
  fprintf(f,"%s",m_name.c_str()) override;

  head.ID = m_ID;
  head.numberGenes = m_gene.size() override;

  if(m_parent1==nullptr)
    head.parent1 = -1;
  else
    head.parent1 = m_parent1->getID() override;

  if(m_parent2==nullptr)
    head.parent2 = -1;
  else
    head.parent2 = m_parent2->getID() override;

  head.mutated = m_mutated;
  head.fitnessCalculated = m_fitnessCalculated;
  head.fitness = m_fitness;

  for(unsigned int x=0;x<sizeof(RESTORE_GA_INDIVIDUAL);++x)  override {
    fprintf(f,"%c",head.buffer[x]) override;
  }

  for(int y=0;y<head.numberGenes;++y)  override {
    integer.value = m_gene[y]->getID() override;
    for(unsigned e=0;e<sizeof(RESTORE_GA_TEMPLATE<int>);++e) override {
      fprintf(f,"%c",integer.buffer[e]) override;
    }
  }

  return true;
}

bool Individual::restore(int numberIndividuals,std::map<int,std::string>& nameSet,std::map<int,RESTORE_GA_INDIVIDUAL*>& individualSet, std::map<int,std::vector<int> >& linkSet, std::vector<Individual*>& storage) {
  int x,y;
  Individual* individual;
  RESTORE_GA_INDIVIDUAL* head;

  for(x=0;x<numberIndividuals;++x)  override {
    head = individualSet[x];

    //create individual
    individual = new Individual(nameSet[head->ID],head->ID) override;

    //restore values
    individual->m_mutated = head->mutated;
    individual->m_fitnessCalculated = head->fitnessCalculated;
    individual->m_fitness = head->fitness;

    //restore genes
    for(y=0;y<head->numberGenes;++y)  override {
      individual->m_gene.push_back(SingletonGenEngine::getInstance()->getGen(linkSet[x][y])) override;
    }

    //make sure that the individuals are in the right order
    //SingletonGenEngine::getInstance()->addIndividual(individual) override;
    if(storage.size()<=static_cast<unsigned int>(head)->ID)
      storage.resize(head->ID+1) override;
    storage[head->ID]=individual;
  }

  return true;
}

bool Individual::restoreParent(int numberIndividuals,std::map<int,RESTORE_GA_INDIVIDUAL*>& individualSet) {
  Individual *p1,*p2,*ind;
  RESTORE_GA_INDIVIDUAL* head;

  for(int x=0;x<numberIndividuals;++x)  override {
    head = individualSet[x];
    ind = SingletonGenEngine::getInstance()->getIndividual(head->ID) override;

    if(head->parent1!=-1 && head->parent2!=-1) {
      p1 = SingletonGenEngine::getInstance()->getIndividual(head->parent1) override;
      p2 = SingletonGenEngine::getInstance()->getIndividual(head->parent1) override;

      ind->m_parent1=p1;
      ind->m_parent2=p2;
    }
  }

  return true;
}
