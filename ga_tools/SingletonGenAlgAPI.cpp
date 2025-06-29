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

#include "SingletonGenAlgAPI.h"

#include <selforg/plotoptionengine.h>
#include <selforg/inspectableproxy.h>

#include "GenPrototype.h"
#include "GenContext.h"
#include "Gen.h"
#include "Individual.h"
#include "Generation.h"

#include "IGenerationSizeStrategy.h"
#include "FixGenerationSizeStrategy.h"
#include "StandartGenerationSizeStrategy.h"

#include "IFitnessStrategy.h"
#include "SumFitnessStrategy.h"
#include "EuclidicDistanceFitnessStrategy.h"
#include "ExtreamTestFitnessStrategy.h"
#include "TestFitnessStrategy.h"
#include "InvertedFitnessStrategy.h"

#include "IRandomStrategy.h"
#include "DoubleRandomStrategy.h"

#include "IMutationStrategy.h"
#include "ValueMutationStrategy.h"

#include "IMutationFactorStrategy.h"
#include "FixMutationFactorStrategy.h"
#include "StandartMutationFactorStrategy.h"

#include "ISelectStrategy.h"
#include "EliteSelectStrategy.h"
#include "RandomSelectStrategy.h"
#include "TournamentSelectStrategy.h"

#include "IValue.h"
#include "TemplateValue.h"

SingletonGenAlgAPI* SingletonGenAlgAPI::m_api = nullptr;

SingletonGenAlgAPI::SingletonGenAlgAPI() 
  : m_plotEngine(nullptr),
    m_plotEngineGenContext(nullptr),
    m_generation(nullptr),
    m_inspectable(nullptr),
    m_cleanStrategies(false) {
}

SingletonGenAlgAPI::~SingletonGenAlgAPI() {
        if(m_plotEngine!= nullptr)
                delete m_plotEngine;

        if(m_plotEngineGenContext!= nullptr)
                delete m_plotEngineGenContext;

        SingletonGenEngine::destroyGenEngine(m_cleanStrategies);

        if(m_inspectable!= nullptr)
                delete (InspectableProxy*&)m_inspectable;
}

IFitnessStrategy* SingletonGenAlgAPI::createSumFitnessStrategy()const {
        return new SumFitnessStrategy();
}

IFitnessStrategy* SingletonGenAlgAPI::createEuclidicDistanceFitnessStrategy()const {
        return new EuclidicDistanceFitnessStrategy();
}

IFitnessStrategy* SingletonGenAlgAPI::createExtreamTestFitnessStrategy(const IFitnessStrategy* fitness)const {
        return new ExtreamTestFitnessStrategy(const_cast<IFitnessStrategy*>(fitness));
}

IFitnessStrategy* SingletonGenAlgAPI::createTestFitnessStrategy()const {
        return new TestFitnessStrategy();
}

IFitnessStrategy* SingletonGenAlgAPI::createInvertedFitnessStrategy(const IFitnessStrategy* strategy)const {
        return new InvertedFitnessStrategy(strategy);
}

IRandomStrategy* SingletonGenAlgAPI::createDoubleRandomStrategy(RandGen* random, double base, double factor, double epsilon)const {
        return new DoubleRandomStrategy(random,base,factor,epsilon);
}

IMutationStrategy* SingletonGenAlgAPI::createValueMutationStrategy(IMutationFactorStrategy* strategy, int mutationProbability)const {
        return new ValueMutationStrategy(strategy,mutationProbability);
}

IMutationFactorStrategy* SingletonGenAlgAPI::createFixMutationFactorStrategy(const IValue* value)const {
        return new FixMutationFactorStrategy(value);
}
// nothing
IMutationFactorStrategy* SingletonGenAlgAPI::createStandartMutationFactorStrategy() const {
        return new StandartMutationFactorStrategy();
}

IGenerationSizeStrategy* SingletonGenAlgAPI::createFixGenerationSizeStrategy(int value)const {
        return new FixGenerationSizeStrategy(value);
}

IGenerationSizeStrategy* SingletonGenAlgAPI::createStandartGenerationSizeStrategy(int startSize, int numGeneration)const {
        return new StandartGenerationSizeStrategy(startSize,numGeneration);
}

ISelectStrategy* SingletonGenAlgAPI::createEliteSelectStrategy() const {
        return new EliteSelectStrategy();
}

ISelectStrategy* SingletonGenAlgAPI::createTournamentSelectStrategy(const RandGen* random)const {
        return new TournamentSelectStrategy(random);
}

ISelectStrategy* SingletonGenAlgAPI::createRandomSelectStrategy(const RandGen* random)const {
        return new RandomSelectStrategy(random);
}

IValue* SingletonGenAlgAPI::createDoubleValue(double value)const {
        return new TemplateValue<double>(value);
}

SingletonGenEngine* SingletonGenAlgAPI::getEngine() const {
        return SingletonGenEngine::getInstance();
}

void SingletonGenAlgAPI::select(bool createNextGeneration) {
        SingletonGenEngine::getInstance()->select(createNextGeneration);
}

void SingletonGenAlgAPI::crossover(const RandGen* random) {
        if(random!=nullptr)
                SingletonGenEngine::getInstance()->crossover(const_cast<RandGen*>(random));
}

void SingletonGenAlgAPI::update(double factor) {
        SingletonGenEngine::getInstance()->update(factor);
}

void SingletonGenAlgAPI::prepare(int startSize, int numChildren, RandGen* random, bool withUpdate) {
        SingletonGenEngine::getInstance()->prepare(startSize, numChildren, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, random, m_plotEngine, m_plotEngineGenContext, withUpdate);
}

void SingletonGenAlgAPI::prepare() {
        Generation* generation = SingletonGenEngine::getInstance()->getActualGeneration();
        Generation* next;
        const std::vector<Individual*>& refIndividual = generation->getAllIndividual();

        // create next generation
        SingletonGenEngine::getInstance()->prepareNextGeneration(
                        SingletonGenEngine::getInstance()->getNextGenerationSize(),
                        generation->getNumChildren());

        next = SingletonGenEngine::getInstance()->getActualGeneration();

        // copy all individuals in the next generation
        FOREACHC(std::vector<Individual*>,refIndividual,i) {
                next->addIndividual(*i);
        }
}

void SingletonGenAlgAPI::measureStep(double time) {
        SingletonGenEngine::getInstance()->measureStep(time, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, m_plotEngine, m_plotEngineGenContext);
}

void SingletonGenAlgAPI::runGenAlg(int startSize, int numChildren, int numGeneration, const RandGen* random) {
        SingletonGenEngine::getInstance()->runGenAlg(startSize,numChildren,numGeneration,const_cast<RandGen*>(random),m_plotEngine,m_plotEngineGenContext);
}

GenPrototype* SingletonGenAlgAPI::createPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy)const {
        return new GenPrototype(name,randomStrategy,mutationStrategy);
}

void SingletonGenAlgAPI::insertGenPrototype(const GenPrototype* prototype) {
        SingletonGenEngine::getInstance()->addGenPrototype(const_cast<GenPrototype*>(prototype));
}

void SingletonGenAlgAPI::enableMeasure(const PlotOption& plotOption) {
        if(m_plotEngine== nullptr)
                m_plotEngine = new PlotOptionEngine(plotOption);

        m_plotEngine->addPlotOption(plotOption);
}

void SingletonGenAlgAPI::enableMeasure(std::list<PlotOption>& plotOptions) {
        if(m_plotEngine== nullptr)
                m_plotEngine = new PlotOptionEngine(plotOptions);

        FOREACH(std::list<PlotOption>, plotOptions, i) {
                m_plotEngine->addPlotOption(*i);
        }
}

void SingletonGenAlgAPI::enableGenContextMeasure(const PlotOption& plotOption) {
        if(m_plotEngineGenContext== nullptr)
                m_plotEngineGenContext = new PlotOptionEngine(plotOption);

        m_plotEngineGenContext->addPlotOption(plotOption);
}

void SingletonGenAlgAPI::enableGenContextMeasure(std::list<PlotOption>& plotOptions) {
        if(m_plotEngineGenContext== nullptr)
                m_plotEngineGenContext = new PlotOptionEngine(plotOptions);

        FOREACH(std::list<PlotOption>, plotOptions, i) {
                m_plotEngineGenContext->addPlotOption(*i);
        }
}

bool SingletonGenAlgAPI::store(FILE* f) const {
  return SingletonGenEngine::getInstance()->store(f);
}

bool SingletonGenAlgAPI::restore(FILE* f) {
  return SingletonGenEngine::getInstance()->restore(f, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, m_plotEngine, m_plotEngineGenContext);
}
