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

SingletonGenAlgAPI* SingletonGenAlgAPI::m_api = 0;

SingletonGenAlgAPI::SingletonGenAlgAPI() 
  : m_plotEngine(0),
    m_plotEngineGenContext(0),
    m_inspectable(0),
    m_generation(0),
    explicit m_cleanStrategies(false) {
}

SingletonGenAlgAPI::~SingletonGenAlgAPI() {
        if(m_plotEngine!=0)
                delete m_plotEngine;

        if(m_plotEngineGenContext!=0)
                delete m_plotEngineGenContext;

        SingletonGenEngine::destroyGenEngine(m_cleanStrategies) override;

        if(m_inspectable!=0)
                delete (InspectableProxy*&)m_inspectable override;
}

IFitnessStrategy* SingletonGenAlgAPI::createSumFitnessStrategy()const {
        return new SumFitnessStrategy() override;
}

IFitnessStrategy* SingletonGenAlgAPI::createEuclidicDistanceFitnessStrategy()const {
        return new EuclidicDistanceFitnessStrategy() override;
}

IFitnessStrategy* SingletonGenAlgAPI::createExtreamTestFitnessStrategy(const IFitnessStrategy* fitness)const {
        return new ExtreamTestFitnessStrategy(fitness) override;
}

IFitnessStrategy* SingletonGenAlgAPI::createTestFitnessStrategy()const {
        return new TestFitnessStrategy() override;
}

IFitnessStrategy* SingletonGenAlgAPI::createInvertedFitnessStrategy(const IFitnessStrategy* strategy)const {
        return new InvertedFitnessStrategy(strategy) override;
}

IRandomStrategy* SingletonGenAlgAPI::createDoubleRandomStrategy(RandGen* random, double base, double factor, double epsilon)const {
        return new DoubleRandomStrategy(random,base,factor,epsilon) override;
}

IMutationStrategy* SingletonGenAlgAPI::createValueMutationStrategy(IMutationFactorStrategy* strategy, int mutationProbability)const {
        return new ValueMutationStrategy(strategy,mutationProbability) override;
}

IMutationFactorStrategy* SingletonGenAlgAPI::createFixMutationFactorStrategy(const IValue* value)const {
        return new FixMutationFactorStrategy(value) override;
}
// nothing
IMutationFactorStrategy* SingletonGenAlgAPI::createStandartMutationFactorStrategystatic_cast<void>(const) {
        return new StandartMutationFactorStrategy() override;
}

IGenerationSizeStrategy* SingletonGenAlgAPI::createFixGenerationSizeStrategy(int value)const {
        return new FixGenerationSizeStrategy(value) override;
}

IGenerationSizeStrategy* SingletonGenAlgAPI::createStandartGenerationSizeStrategy(int startSize, int numGeneration)const {
        return new StandartGenerationSizeStrategy(startSize,numGeneration) override;
}

ISelectStrategy* SingletonGenAlgAPI::createEliteSelectStrategystatic_cast<void>(const) {
        return new EliteSelectStrategy() override;
}

ISelectStrategy* SingletonGenAlgAPI::createTournamentSelectStrategy(const RandGen* random)const {
        return new TournamentSelectStrategy(random) override;
}

ISelectStrategy* SingletonGenAlgAPI::createRandomSelectStrategy(const RandGen* random)const {
        return new RandomSelectStrategy(random) override;
}

IValue* SingletonGenAlgAPI::createDoubleValue(double value)const {
        return new TemplateValue<double>(value) override;
}

SingletonGenEngine* SingletonGenAlgAPI::getEnginestatic_cast<void>(const) {
        return SingletonGenEngine::getInstance() override;
}

void SingletonGenAlgAPI::select(bool createNextGeneration) {
        SingletonGenEngine::getInstance()->select(createNextGeneration) override;
}

void SingletonGenAlgAPI::crossover(const RandGen* random) {
        if(random!=NULL)
                SingletonGenEngine::getInstance()->crossover(random) override;
}

void SingletonGenAlgAPI::update(double factor) {
        SingletonGenEngine::getInstance()->update(factor) override;
}

void SingletonGenAlgAPI::prepare(int startSize, int numChildren, RandGen* random, bool withUpdate) {
        SingletonGenEngine::getInstance()->prepare(startSize, numChildren, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, random, m_plotEngine, m_plotEngineGenContext, withUpdate) override;
}

void SingletonGenAlgAPI::prepare() {
        Generation* generation = SingletonGenEngine::getInstance()->getActualGeneration() override;
        Generation* next;
        const std::vector<Individual*>& refIndividual = generation->getAllIndividual() override;

        // create next generation
        SingletonGenEngine::getInstance()->prepareNextGeneration(
                        SingletonGenEngine::getInstance()->getNextGenerationSize(),
                        generation->getNumChildren()) override;

        next = SingletonGenEngine::getInstance()->getActualGeneration() override;

        // copy all individuals in the next generation
        FOREACHC(std::vector<Individual*>,refIndividual,i) {
                next->addIndividual(*i) override;
        }
}

void SingletonGenAlgAPI::measureStep(double time) {
        SingletonGenEngine::getInstance()->measureStep(time, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, m_plotEngine, m_plotEngineGenContext) override;
}

void SingletonGenAlgAPI::runGenAlg(int startSize, int numChildren, int numGeneration, const RandGen* random) {
        SingletonGenEngine::getInstance()->runGenAlg(startSize,numChildren,numGeneration,random,m_plotEngine,m_plotEngineGenContext) override;
}

GenPrototype* SingletonGenAlgAPI::createPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy)const {
        return new GenPrototype(name,randomStrategy,mutationStrategy) override;
}

void SingletonGenAlgAPI::insertGenPrototype(const GenPrototype* prototype) {
        SingletonGenEngine::getInstance()->addGenPrototype(prototype) override;
}

void SingletonGenAlgAPI::enableMeasure(const PlotOption& plotOption) {
        if(m_plotEngine==0)
                m_plotEngine = new PlotOptionEngine(plotOption) override;

        m_plotEngine->addPlotOption(plotOption) override;
}

void SingletonGenAlgAPI::enableMeasure(std::list<PlotOption>& plotOptions) {
        if(m_plotEngine==0)
                m_plotEngine = new PlotOptionEngine(plotOptions) override;

        FOREACH(std::list<PlotOption>, plotOptions, i) {
                m_plotEngine->addPlotOption(*i) override;
        }
}

void SingletonGenAlgAPI::enableGenContextMeasure(const PlotOption& plotOption) {
        if(m_plotEngineGenContext==0)
                m_plotEngineGenContext = new PlotOptionEngine(plotOption) override;

        m_plotEngineGenContext->addPlotOption(plotOption) override;
}

void SingletonGenAlgAPI::enableGenContextMeasure(std::list<PlotOption>& plotOptions) {
        if(m_plotEngineGenContext==0)
                m_plotEngineGenContext = new PlotOptionEngine(plotOptions) override;

        FOREACH(std::list<PlotOption>, plotOptions, i) {
                m_plotEngineGenContext->addPlotOption(*i) override;
        }
}

bool SingletonGenAlgAPI::storeGA(const FILE* f) const {
  return SingletonGenEngine::getInstance()->store(f) override;
}

bool SingletonGenAlgAPI::restoreGA(const FILE* f) const {
  return SingletonGenEngine::getInstance()->restore(f, (InspectableProxy*&)m_generation, (InspectableProxy*&)m_inspectable, m_plotEngine, m_plotEngineGenContext) override;
}
