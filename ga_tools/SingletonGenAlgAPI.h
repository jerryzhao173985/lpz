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

#ifndef SINGLETONGENALGAPI_H_
#define SINGLETONGENALGAPI_H_

//includes
#include <string>
#include <list>
#include <selforg/randomgenerator.h>
#include <selforg/storeable.h>
#include "SingletonGenEngine.h"

//forward declaration
class Gen;
class PlotOption;
class PlotOptionEngine;
class IGenerationSizeStrategy;
class IFitnessStrategy;
class ISelectStrategy;
class Individual;
// RandGen is defined in selforg/randomgenerator.h
class SingletonGenEngine;
class IRandomStrategy;
class IMutationStrategy;
class IMutationFactorStrategy;
class IValue;
class GenPrototype;

class SingletonGenAlgAPI {
public:
	// Action
	/**
	 * starts the selection
	 * @param createNextGeneration (bool) normal=true should be the next generation be prepare?
	 */
	void select(bool createNextGeneration=true);
	/**
	 * create the children from to fill up the next generation
	 * @param random (RandGen*) random generator
	 */
	void crossover(const RandGen* random);
	/**
	 * update the internal statistical data
	 * @param factor (double) normal=1.5 is needed for the whisker distance
	 */
	void update(double factor = 1.5);
	/**
	 * prepares the first generation and optional the enabled measure
	 * @param startSize (int) Number of individual at begin of the gen. alg.
	 * @param startChildren (int) Number of individual which will be created by crossover
	 * @param random (RandGen*) A random generator
	 * @param withUpdate (bool) is needed for __PLACEHOLDER_1__
	 */
	void prepare(int startSize, int numChildren, RandGen* random, bool withUpdate = true);
	/**
	 * prepares the next generation and optional the enabled measure
	 */
	void prepare();
	/**
	 * makes a step in the measure
	 * @param time (double) time stamp in the measure
	 */
	void measureStep(double time);
	/**
	 * start the sequenz of select, crossover, update in a automatically loop
	 * @param startSize (int) Number of individual at begin of the gen. alg.
	 * @param numChildren (int) Number of individual which will be created by crossover
	 * @param numGeneration (int) Number of generation which the alg. max. runs
	 * @param random (RandGen*) random generator
	 */
	void runGenAlg(int startSize, int numChildren, int numGeneration, const RandGen* random);

	//measure
	/**
	 * enables data measure with more than one plotOption.
	 * @param plotOptions (list<P�otOption>&) the list
	 */
	void enableMeasure(std::list<PlotOption>& plotOptions);
	/**
	 * enables da : public Storableta measure.
	 * @param plotOption (PlotOption&) the plot option
	 */
	void enableMeasure(const PlotOption& plotOption);
	/**
	 * returns the active plotOptionEngine for data measure.
	 * @return (PlotOptionEngine*) the plot option engine
	 */
	inline PlotOptionEngine* getPlotOptionEngine(void) const {return m_plotEngine;}
	/**
	 * enable data measure inside the GenContexts with more than one plotOption
	 * @param plotOptions (list<PlotOption>&) the list
	 */
	void enableGenContextMeasure(std::list<PlotOption>& plotOptions);
	/**
	 * enable data measure inside the GenContexts.
	 * @param plotOption (PlotOption&) the plot option
	 */
	void enableGenContextMeasure(const PlotOption& plotOption);
	/**
	 * returns the active plotOptionEngine for data measure inside the GenContexts.
	 * @return (PlotOptionEngine*) the plot option engine.
	 */
	inline PlotOptionEngine* getPlotOptionEngineForGenContext(void) const {return m_plotEngineGenContext;}

	// set static strategies
	/**
	 * set the generation size strategy
	 * @param strategy (IGenerationSizeStrategy*) the strategy
	 */
	inline void setGenerationSizeStrategy(IGenerationSizeStrategy* strategy) {SingletonGenEngine::getInstance()->setGenerationSizeStrategy(strategy);}
	/**
	 * set the fitness strategy
	 * @param strategy (IFitnessStrategy*) the strategy
	 */
	inline void setFitnessStrategy(IFitnessStrategy* strategy) {SingletonGenEngine::getInstance()->setFitnessStrategy(strategy);}
	/**
	 * set the select strategy
	 * @param strategy (ISelectStrategy*) the strategy
	 */
	inline void setSelectStrategy(ISelectStrategy* strategy) {SingletonGenEngine::getInstance()->setSelectStrategy(strategy);}

	// gets
	/**
	 * returns the GenAlgEngine
	 * @return (SingletonGenEngine*) the engine
	 */
	SingletonGenEngine* getEngine(void) const;

	// default interface creation
	/**
	 * creates a SumFitnessStrategy. This strategy make the sum of all gens with a double value.
	 * @return (IFitnessStrategy*) the strategy
	 */
	IFitnessStrategy* createSumFitnessStrategy()const;
	/**
	 * creates a EuclidicFitnessStrategy. This strategy calculate the euclidic distance of all gens with a double value.
	 * @return (IFitnessStrategy*) the strategy
	 */
	IFitnessStrategy* createEuclidicDistanceFitnessStrategy()const;
	/**
	 * creates a TestFitnessStrategy which is the hardest test for a gen. alg. Only a smale area has a fitness. All other gives no information!
	 * @param fitness (IFitnessStrategy*) a other fitness strategy
	 * @return (IFitnessStrategy*) the strategy
	 */
	IFitnessStrategy* createTestFitnessStrategy(IFitnessStrategy* fitness)const;
	IFitnessStrategy* createExtreamTestFitnessStrategy(const IFitnessStrategy* fitness)const;
	IFitnessStrategy* createTestFitnessStrategy()const;
	IFitnessStrategy* createInvertedFitnessStrategy(const IFitnessStrategy* strategy)const;
	
	// Random strategies
	IRandomStrategy* createDoubleRandomStrategy(RandGen* random, double base, double factor, double epsilon)const;
	
	// Mutation strategies
	IMutationStrategy* createValueMutationStrategy(IMutationFactorStrategy* strategy, int mutationProbability)const;
	IMutationFactorStrategy* createFixMutationFactorStrategy(const IValue* value)const;
	IMutationFactorStrategy* createStandartMutationFactorStrategy() const;
	
	// Generation size strategies
	IGenerationSizeStrategy* createFixGenerationSizeStrategy(int value)const;
	IGenerationSizeStrategy* createStandartGenerationSizeStrategy(int startSize, int numGeneration)const;
	
	// Select strategies
	ISelectStrategy* createEliteSelectStrategy() const;
	ISelectStrategy* createTournamentSelectStrategy(const RandGen* random)const;
	ISelectStrategy* createRandomSelectStrategy(const RandGen* random)const;
	
	// Value creation
	IValue* createDoubleValue(double value)const;
	
	// Prototype creation
	GenPrototype* createPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy)const;
	void insertGenPrototype(const GenPrototype* prototype);
	
	// Store and restore
	bool store(FILE* f) const;
	bool restore(FILE* f);

	// Singleton access
	/**
	 * returns the API. Only one API for a run is allowed!
	 * @return (SingletonGenAlgAPI*) the api
	 */
	inline static SingletonGenAlgAPI* getInstance(void) {if(m_api== nullptr)m_api = new SingletonGenAlgAPI();return m_api;}
	/**
	 * destroy the api
	 * @param cleanStrategies (bool) default = false set a flag to clean the strategies, which are seted.
	 */
	inline static void destroyAPI(bool cleanStrategies=false) {getInstance()->m_cleanStrategies=cleanStrategies; if(m_api!= nullptr){delete m_api;m_api=0;}}

	// data access
	/**
	 * returns the best individual which the alg. have found
	 * @return (Individual*) the best
	 */
	inline Individual* getBestIndividual(void) const {return SingletonGenEngine::getInstance()->getBestIndividual();}

protected:
	/**
	 * the api
	 */
	static SingletonGenAlgAPI* m_api;

	/**
	 * plot option engine for data measure
	 */
	PlotOptionEngine* m_plotEngine;

	/**
	 * plot option engine for gen contexts
	 */
	PlotOptionEngine* m_plotEngineGenContext;

private:
	/**
	 * disable the default contructor
	 */
	SingletonGenAlgAPI();
	/**
	 * disable the default destructor
	 */
	virtual ~SingletonGenAlgAPI();

	/**
	 * help declaration for prepare
	 */
	void* m_generation;

	/**
	 * help declaration for prepare
	 */
	void* m_inspectable;

	/**
	 * Flag for clean the strategies
	 */
	bool m_cleanStrategies = false;
};

#endif /* SINGLETONGENALGAPI_H_ */
