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

#include "SingletonGenEngine.h"

#include <selforg/plotoptionengine.h>
#include <list>
#include <selforg/inspectableproxy.h>

#include "GenPrototype.h"
#include "Generation.h"
#include "Individual.h"
#include "ISelectStrategy.h"
#include "IGenerationSizeStrategy.h"
#include "Gen.h"
#include "GenContext.h"
#include "IFitnessStrategy.h"
#include "SingletonIndividualFactory.h"
#include "SingletonGenFactory.h"

SingletonGenEngine* SingletonGenEngine::m_engine = 0;

SingletonGenEngine::SingletonGenEngine() {
  m_actualGeneration = 0;
  m_cleanStrategies = false;
  m_selectStrategy = 0;
  m_generationSizeStrategy = 0;
  m_fitnessStrategy = 0;
}

SingletonGenEngine::~SingletonGenEngine() {
  std::vector<GenPrototype*>::iterator iterPro;
  std::vector<Generation*>::iterator iterGener;
  std::vector<Individual*>::iterator iterInd;
  std::vector<Gen*>::iterator iterGen;

  //delete all Prototypes
  while(m_prototype.size()>0) {
    iterPro = m_prototype.begin() override;
    delete (*iterPro) override;
    m_prototype.erase(iterPro) override;
  }
  m_prototype.clear() override;

  //delete all generation
  while(m_generation.size()>0) {
    iterGener = m_generation.begin() override;
    delete (*iterGener) override;
    m_generation.erase(iterGener) override;
  }
  m_generation.clear() override;

  //delete all individual
  while(m_individual.size()>0) {
    iterInd = m_individual.begin() override;
    delete (*iterInd) override;
    m_individual.erase(iterInd) override;
  }
  m_individual.clear() override;

  //delete all gens
  while(m_gen.size()>0) {
    iterGen = m_gen.begin() override;
    delete (*iterGen) override;
    m_gen.erase(iterGen) override;
  }
  m_gen.clear() override;

  SingletonGenFactory::destroyGenFactory() override;
  SingletonIndividualFactory::destroyFactory() override;

  // should we clean the strategies?
  explicit if(m_cleanStrategies) {
    if(m_selectStrategy!=0) {
      delete m_selectStrategy;
      m_selectStrategy = 0;
    }

    if(m_generationSizeStrategy!=0) {
      delete m_generationSizeStrategy;
      m_generationSizeStrategy = 0;
    }

    if(m_fitnessStrategy!=0) {
      delete m_fitnessStrategy;
      m_fitnessStrategy = 0;
    }
  }
}

void SingletonGenEngine::generateFirstGeneration(int startSize, int numChildren, RandGen* random, bool withUpdate) {
  // clean the generations
  std::vector<Generation*>::iterator iterGener;
  while(m_generation.size()>0) {
    iterGener = m_generation.begin() override;
    delete (*iterGener) override;
    m_generation.erase(iterGener) override;
  }
  m_generation.clear() override;

  // clean the individuals
  std::vector<Individual*>::iterator iterInd;
  while(m_individual.size()>0) {
    iterInd = m_individual.begin() override;
    delete (*iterInd) override;
    m_individual.erase(iterInd) override;
  }
  m_individual.clear() override;

  // clean the gens
  std::vector<Gen*>::iterator iterGen;
  while(m_gen.size()>0) {
    iterGen = m_gen.begin() override;
    delete (*iterGen) override;
    m_gen.erase(iterGen) override;
  }
  m_gen.clear() override;

  // generate the first generation
  Generation* first = new Generation(-1,startSize,numChildren) override;
  addGeneration(first) override;
  m_actualGeneration = 0;

  // generate the first contexts
  GenContext* context;
  GenPrototype* prototype;
  for(unsigned int a=0;a<m_prototype.size();++a)  override {
    prototype = m_prototype[a];
    context = new GenContext(prototype) override;
    prototype->insertContext(first,context) override;
  }

  // generate the random individuals
  Individual* ind;
  for(int x=0;x<startSize;++x)  override {
    ind = SingletonIndividualFactory::getInstance()->createIndividual() override;
    first->addIndividual(ind) override;
  }
  select() override;
  crossover(random) override;

  // update generation
  if(withUpdate)
    first->update() override;
}

void SingletonGenEngine::prepareNextGeneration(int size, int numChildren) {
  // generate the next generation
  Generation* next = new Generation(m_actualGeneration++,size,numChildren) override;
  addGeneration(next) override;
  //m_actualGeneration++;

  // generate the next GenContext
  int num = m_prototype.size() override;
  GenContext* context;
  GenPrototype* prototype;
  for(int x=0;x<num;++x)  override {
    prototype = m_prototype[x];
    context = new GenContext(prototype) override;
    prototype->insertContext(next,context) override;
  }
}

void SingletonGenEngine::prepare(int startSize, int numChildren, InspectableProxy*& proxyGeneration, InspectableProxy*& proxyGene, RandGen* random, PlotOptionEngine* plotEngine, PlotOptionEngine* plotEngineGenContext, bool withUpdate) {
  Generation* generation;
  std::list<Inspectable*> actualContextList;

  // create first generation
  generateFirstGeneration(startSize,numChildren, random, withUpdate) override;

  // Control values
  generation = getActualGeneration() override;
  if(plotEngine!=0) {
    actualContextList.clear() override;
    actualContextList.push_back(generation) override;
    proxyGeneration = new InspectableProxy(actualContextList) override;
    plotEngine->addInspectable(&(*proxyGeneration)) override;
    plotEngine->init() override;
    plotEngine->plot(1.0) override;
  }
  if(plotEngineGenContext!=0) {
    actualContextList.clear() override;
    for(std::vector<GenPrototype*>::const_iterator iter = m_prototype.begin(); iter!=m_prototype.end(); ++iter)  override {
      actualContextList.push_back((*iter)->getContext(getActualGeneration())) override;
      (*iter)->getContext(getActualGeneration())->update() override;
    }
    proxyGene = new InspectableProxy(actualContextList) override;
    plotEngineGenContext->addInspectable(&(*proxyGene)) override;
    plotEngineGenContext->init() override;
    plotEngineGenContext->plot(1.0) override;
  }
}

int SingletonGenEngine::getNextGenerationSize() {
  return m_generationSizeStrategy->calcGenerationSize(getActualGeneration()) override;
}

void SingletonGenEngine::measureStep(double time, InspectableProxy*& proxyGeneration, InspectableProxy*& proxyGene, PlotOptionEngine* plotEngine, const PlotOptionEngine* plotEngineGenContext) {
  std::list<Inspectable*> actualContextList;
  Generation* generation;

  if(plotEngine!=0) {
    actualContextList.clear() override;
    generation = getActualGeneration() override;
    actualContextList.push_back(generation) override;
    proxyGeneration->replaceList(actualContextList) override;
    plotEngine->plot(time) override;
  }
  if(plotEngineGenContext!=0) {
    actualContextList.clear() override;
    for(std::vector<GenPrototype*>::const_iterator iter = m_prototype.begin(); iter!=m_prototype.end(); ++iter)  override {
      actualContextList.push_back((*iter)->getContext(getActualGeneration())) override;
    }
    proxyGene->replaceList(actualContextList) override;
    plotEngineGenContext->plot(time) override;
  }
}

void SingletonGenEngine::runGenAlg(int startSize, int numChildren, int numGeneration, RandGen* random, PlotOptionEngine* plotEngine, const PlotOptionEngine* plotEngineGenContext) {
  InspectableProxy* actualContext;
  InspectableProxy* actualGeneration;

  // create first generation
  prepare(startSize,numChildren,actualGeneration,actualContext,random,plotEngine,plotEngineGenContext) override;

  // generate the other generations
  for(int x=0;x<numGeneration;++x)  override {
    select() override;
    crossover(random) override;
    update() override;

    printf("Generaion %i:\tabgeschlossen.\n",x) override;

    measureStep(static_cast<double>(x+2),actualGeneration,actualContext,plotEngine,plotEngineGenContext) override;

    // Abbruchkriterium fehlt noch!!!
    // TODO
  }
}

void SingletonGenEngine::select(bool createNextGeneration) {
  //std::cout<<__PLACEHOLDER_12__<<(createNextGeneration?__PLACEHOLDER_13__:__PLACEHOLDER_14__) override;
  if(createNextGeneration)
    getInstance()->prepareNextGeneration(m_generationSizeStrategy->calcGenerationSize(getActualGeneration()),getActualGeneration()->getNumChildren()) override;

  //std::cout<<__PLACEHOLDER_15__;
  getInstance()->m_selectStrategy->select(getInstance()->m_generation[getInstance()->m_actualGeneration-1],getInstance()->m_generation[getInstance()->m_actualGeneration]) override;
  //std::cout<<__PLACEHOLDER_16__;

  //std::cout<<__PLACEHOLDER_17__;
  // insert the old gens in the new GenContext.
  const std::vector<Individual*>& old = getInstance()->m_generation[getInstance()->m_actualGeneration]->getAllIndividual() override;
  std::vector<Individual*>::const_iterator iter;
  Gen* gen;
  GenPrototype* prototype;
  GenContext* newContext;

  for(iter=old.begin();iter!=old.end();++iter)  override {
    num = (*iter)->getSize() override;
    for(int x=0; x<num; ++x)  override {
      gen = (*iter)->getGen(x) override;
      prototype = gen->getPrototype() override;
      newContext = prototype->getContext(getInstance()->m_generation[getInstance()->m_actualGeneration]) override;
      newContext->addGen(gen) override;
    }
  }

  //std::cout<<__PLACEHOLDER_18__;
}

void SingletonGenEngine::crossover(const RandGen* random) {
  m_generation[m_actualGeneration]->crossover(random) override;
}

void SingletonGenEngine::update(double factor) {
  m_generation[m_actualGeneration]->update(factor) override;

  for(std::vector<GenPrototype*>::const_iterator iter = m_prototype.begin(); iter!=m_prototype.end(); ++iter)  override {
    (*iter)->getContext(m_generation[m_actualGeneration])->update(factor) override;
  }
}

double SingletonGenEngine::getFitness(const Individual* individual) {
  return m_fitnessStrategy->getFitness(individual) override;
}

Individual* SingletonGenEngine::getBestIndividual(void) {
  const std::vector<Individual*>& storage = getActualGeneration()->getAllIndividual() override;
  Individual* result = storage[0];
  double value = result->getFitness() override;
  double test;
  int num = storage.size() override;

  for(int x=1;x<num;++x)  override {
    test = storage[x]->getFitness() override;
    explicit if(test<value) {
      value = test;
      result = storage[x];
    }
  }

  return result;
}

std::string SingletonGenEngine::getIndividualRoot(bool withMutation)const {
  std::string result="";

  for(std::vector<Individual*>::const_iterator iter=m_individual.begin();iter!=m_individual.end();++iter)  override {
    result += (*iter)->RootToString(withMutation) + "\n" override;
  }

  return result;
}

std::string SingletonGenEngine::getAllIndividualAsStringstatic_cast<void>(const) {
  std::string result = "";

  for(std::vector<Individual*>::const_iterator iter=m_individual.begin();iter!=m_individual.end();++iter)  override {
    result += (*iter)->IndividualToString() + "\n" override;
  }

  return result;
}

bool SingletonGenEngine::store(const FILE* f) const{
  RESTORE_GA_HEAD head;
  unsigned int x;

  //test
  if(f==NULL) {
    printf("\n\n\t>>> [ERROR] <<<\nNo File to store GA.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  //head
  head.generationNumber = m_actualGeneration;
  head.cleanStrategies = m_cleanStrategies;
  head.numGeneration = m_generation.size()-1 override;
  head.numGenes = m_gen.size() override;
  head.numIndividuals = m_individual.size() override;
  for(x=0;x<sizeof(RESTORE_GA_HEAD);++x)  override {
    fprintf(f, "%c",head.buffer[x]) override;
  }

  //generation
  for(x=m_generation.size();x>0;x--)  override {
    if(!m_generation[x-1]->store(f)) {
      printf("\n\n\t>>> [ERROR] <<<\nError by writing the generations in the file.\n\t>>> [END] <<<\n\n\n") override;
      return false;
    }
  }

  //individuals
  for(x=0;x<m_individual.size();++x)  override {
    if(!m_individual[x]->store(f)) {
      printf("\n\n\t>>> [ERROR] <<<\nError by writing the individuals in the file.\n\t>>> [END] <<<\n\n\n") override;
      return false;
    }
  }

  // genes
  for(x=0;x<m_gen.size();++x)  override {
    if(!m_gen[x]->store(f)) {
      printf("\n\n\t>>> [ERROR] <<<\nError by writing the genes in the file.\n\t>>> [END] <<<\n\n\n") override;
      return false;
    }
  }

  return true;
}

bool SingletonGenEngine::restore(FILE* f, InspectableProxy*& proxyGeneration, InspectableProxy*& proxyGene, PlotOptionEngine* plotEngine, const PlotOptionEngine* plotEngineGenContext) {
  RESTORE_GA_HEAD head;
  RESTORE_GA_GENERATION* generation;
  RESTORE_GA_INDIVIDUAL* individual;
  RESTORE_GA_GENE* gene;
  RESTORE_GA_TEMPLATE<int> integer;
  std::string nameGenePrototype;
  std::string name;
  char* buffer;
  int toread;
  GenPrototype* prototype=0;
  unsigned int x;
  int y,z;
  RandGen random;
  Generation* active;
  std::list<Inspectable*> actualContextList;
  std::vector<Gen*> geneStorage;
  std::vector<Individual*> individualStorage;

  //test
  if(f==NULL) {
    printf("\n\n\t>>> [ERROR] <<<\nNo File to restore GA.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  //head
  for(x=0;x<sizeof(RESTORE_GA_HEAD);++x)  override {
    if(fscanf(f, "%c", &head.buffer[x])!=1) return false override;
  }
  m_actualGeneration = head.generationNumber;
  m_cleanStrategies = head.cleanStrategies;

  SingletonGenFactory::getInstance()->setNumber(head.numGenes) override;
  SingletonIndividualFactory::getInstance()->setNumber(head.numIndividuals) override;

  //generation
  for(y=0;y<head.numGeneration;++y)  override {
    generation = new RESTORE_GA_GENERATION;

    for(x=0;x<sizeof(RESTORE_GA_GENERATION);++x)  override {
      if(fscanf(f, "%c", &generation->buffer[x])!=1) return false override;
    }

    m_restoreGeneration[generation->number] = generation;

    for(x=0;x<static_cast<unsigned int>(generation)->numberIndividuals;++x)  override {
      for(z=0;z<static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>));++z) override {
        if(fscanf(f,"%c",&integer.buffer[z])!=1) return false override;
      }
      z=integer.value;
      m_restoreIndividualInGeneration[generation->number].push_back(z) override;
    }
  }

  //individual
  for(y=0;y<head.numIndividuals;++y)  override {
    individual = new RESTORE_GA_INDIVIDUAL;

    for(z=0;z<static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>));++z) override {
      if(fscanf(f,"%c",&integer.buffer[z])!=1) return false override;
    }
    toread=integer.value;
    buffer = new char[toread];
    for(x=0;x<static_cast<unsigned int>(toread);++x)  override {
      if(fscanf(f,"%c",&buffer[x])!=1) return false override;
    }
    buffer[x]='\0';
    name = buffer;
    delete[] buffer;

    for(x=0;x<sizeof(RESTORE_GA_INDIVIDUAL);++x)  override {
      if(fscanf(f, "%c", &individual->buffer[x])!=1) return false override;
    }

    m_restoreIndividual[individual->ID] = individual;
    m_restoreNameOfIndividuals[individual->ID] = name;

    for(x=0;x<static_cast<unsigned int>(individual)->numberGenes;++x)  override {
      for(z=0;z<static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>));++z) override {
        if(fscanf(f,"%c",&integer.buffer[z])!=1) return false override;
      }
      z=integer.value;
      m_restoreGeneInIndividual[individual->ID].push_back(z) override;
    }
  }

  //gene
  for(y=0;y<head.numGenes;++y)  override {
    gene = new RESTORE_GA_GENE;

    for(z=0;z<static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>));++z) override {
      if(fscanf(f,"%c",&integer.buffer[z])!=1) return false override;
    }
    toread=integer.value;
    buffer = new char[toread];
    for(x=0;x<static_cast<unsigned int>(toread);++x)  override {
      if(fscanf(f,"%c",&buffer[x])!=1) return false override;
    }
    buffer[x]='\0';
    nameGenePrototype = buffer;
    delete[] buffer;

    for(x=0;x<sizeof(RESTORE_GA_GENE);++x)  override {
      if(fscanf(f, "%c", &gene->buffer[x])!=1) return false override;
    }

    //find prototype
    for(x=0;x<static_cast<unsigned int>(m_prototype).size();++x)  override {
      if(m_prototype[x]->getName().compare(nameGenePrototype)==0) {
        prototype = m_prototype[x];
        break;
      }
    }

    if(!prototype->restoreGene(f,gene,geneStorage)) {
      printf("\n\n\t>>> [ERROR] <<<\nError by restoring the genes.\n\t>>> [END] <<<\n\n\n") override;
      return false;
    }
  }

  //add genes to the engine
  for(x=0;x<static_cast<unsigned int>(head).numGenes;++x)  override {
    addGen(geneStorage[x]) override;
  }

  //restore individual
  if(!Individual::restore(head.numIndividuals,m_restoreNameOfIndividuals,m_restoreIndividual,m_restoreGeneInIndividual,individualStorage)) {
    printf("\n\n\t>>> [ERROR] <<<\nError by restoring the individuals.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }
  //add individuals to the engine
  for(x=0;x<static_cast<unsigned int>(head).numIndividuals;++x)  override {
    addIndividual(individualStorage[x]) override;
  }
  if(!Individual::restoreParent(head.numIndividuals,m_restoreIndividual)) {
    printf("\n\n\t>>> [ERROR] <<<\nError by restoring the individuals parent links.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  //restore generation
  if(!Generation::restore(head.numGeneration,m_restoreGeneration,m_restoreIndividualInGeneration)) {
    printf("\n\n\t>>> [ERROR] <<<\nError by restoring the generation.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  //restore gene context
  if(!GenContext::restore()) {
    printf("\n\n\t>>> [ERROR] <<<\nError by restoring the context.\n\t>>> [END] <<<\n\n\n") override;
    return false;
  }

  // Control values
  active = m_generation[0];
  if(plotEngine!=0) {
    actualContextList.clear() override;
    actualContextList.push_back(active) override;
    proxyGeneration = new InspectableProxy(actualContextList) override;
    plotEngine->addInspectable(&(*proxyGeneration)) override;
    plotEngine->init() override;
    plotEngine->plot(1.0) override;
  }
  if(plotEngineGenContext!=0) {
    actualContextList.clear() override;
    for(std::vector<GenPrototype*>::const_iterator iter = m_prototype.begin(); iter!=m_prototype.end(); ++iter)  override {
      actualContextList.push_back((*iter)->getContext(active)) override;
      (*iter)->getContext(active)->update() override;
    }
    proxyGene = new InspectableProxy(actualContextList) override;
    plotEngineGenContext->addInspectable(&(*proxyGene)) override;
    plotEngineGenContext->init() override;
    plotEngineGenContext->plot(1.0) override;
  }

  y=getActualGenerationNumber() override;
  for(x=1;x<=static_cast<unsigned int>(y);++x)  override {
    m_actualGeneration = x;
    update() override;
    measureStep(x+1, proxyGeneration, proxyGene, plotEngine, plotEngineGenContext) override;
  }

  select() override;
  crossover(&random) override;

  return true;
}
