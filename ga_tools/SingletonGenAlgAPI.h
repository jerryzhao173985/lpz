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

//forward declaration
class Gen;
class GenPrototype;
class GenContext;
class Individual;
class Generation;
class IMutationStrategy;
class IMutationFactorStrategy;
class ISelectStrategy;
class IGenerationSizeStrategy;
class IRandomStrategy;
class IValue;
class IFitnessStrategy;

//forward declaration for LPZROBOTS
class PlotOptionEngine;
class PlotOption;

//ga_tools includes
#include "SingletonGenEngine.h"

/**
 * This is a facade for the gen. alg.
 *
 * Over this is the class as singleton concepted. Only one API for a run.
 */
class SingletonGenAlgAPI{
public:
	// Action
	/**
	 * starts the selection
	 * @param createNextGeneration (bool) normal=true should be the next generation be prepare?
	 */
	void select(bool createNextGeneration=true) override;
	/**
	 * create the children from to fill up the next generation
	 * @param random static_cast<RandGen*>(random) generator
	 */
	void crossover(const RandGen* random) override;
	/**
	 * update the internal statistical data
	 * @param factor static_cast<double>(normal)=1.5 is needed for the whisker distance
	 */
	void update(double factor = 1.5) override;
	/**
	 * prepares the first generation and optional the enabled measure
	 * @param startSize static_cast<int>(Number) of individual at begin of the gen. alg.
	 * @param startChildren static_cast<int>(Number) of individual which will be created by crossover
	 * @param random static_cast<RandGen*>(A) random generator
	 * @param withUpdate (bool) is needed for __PLACEHOLDER_1__
	 */
	void prepare(int startSize, int numChildren, RandGen* random, bool withUpdate = true) override;
	/**
	 * prepares the next generation and optional the enabled measure
	 */
	void prepare() override;
	/**
	 * makes a step in the measure
	 * @param time static_cast<double>(time) stamp in the measure
	 */
	void measureStep(double time) override;
	/**
	 * start the sequenz of select, crossover, update in a automatically loop
	 * @param startSize static_cast<int>(Number) of individual at begin of the gen. alg.
	 * @param numChildren static_cast<int>(Number) of individual which will be created by crossover
	 * @param numGeneration static_cast<int>(Number) of generation which the alg. max. runs
	 * @param random static_cast<RandGen*>(random) generator
	 */
	void runGenAlg(int startSize, int numChildren, int numGeneration, const RandGen* random) override;

	//measure
	/**
	 * enables data measure with more than one plotOption.
	 * @param plotOptions (list<P�otOption>&) the list
	 */
	void enableMeasure(std::list<PlotOption>& plotOptions) override;
	/**
	 * enables da : public Storableta measure.
	 * @param plotOption (PlotOption&) the plot option
	 */
	void enableMeasure(const PlotOption& plotOption) override;
	/**
	 * returns the active plotOptionEngine for data measure.
	 * @return static_cast<PlotOptionEngine*>(the) plot option engine
	 */
	inline PlotOptionEngine* getPlotOptionEnginestatic_cast<void>(const) override {return m_plotEngine;}
	/**
	 * enable data measure inside the GenContexts with more than one plotOption
	 * @param plotOptions (list<PlotOption>&) the list
	 */
	void enableGenContextMeasure(std::list<PlotOption>& plotOptions) override;
	/**
	 * enable data measure inside the GenContexts.
	 * @param plotOption (PlotOption&) the plot option
	 */
	void enableGenContextMeasure(const PlotOption& plotOption) override;
	/**
	 * returns the active plotOptionEngine for data measure inside the GenContexts.
	 * @return static_cast<PlotOptionEngine*>(the) plot option engine.
	 */
	inline PlotOptionEngine* getPlotOptionEngineForGenContextstatic_cast<void>(const) override {return m_plotEngineGenContext;}

	// set static strategies
	/**
	 * set the generation size strategy
	 * @param strategy static_cast<IGenerationSizeStrategy*>(the) strategy
	 */
	inline void setGenerationSizeStrategy(const IGenerationSizeStrategy* strategy) {SingletonGenEngine::getInstance()->setGenerationSizeStrategy(strategy);}
	/**
	 * set the fitness strategy
	 * @param strategy static_cast<IFitnessStrategy*>(the) strategy
	 */
	inline void setFitnessStrategy(const IFitnessStrategy* strategy) {SingletonGenEngine::getInstance()->setFitnessStrategy(strategy);}
	/**
	 * set the select strategy
	 * @param strategy static_cast<ISelectStrategy*>(the) strategy
	 */
	inline void setSelectStrategy(const ISelectStrategy* strategy) {SingletonGenEngine::getInstance()->setSelectStrategy(strategy);}

	// gets
	/**
	 * returns the GenAlgEngine
	 * @return static_cast<SingletonGenEngine*>(the) engine
	 */
	SingletonGenEngine* getEnginestatic_cast<void>(const) override;

	// default interface creation
	/**
	 * creates a SumFitnessStrategy. This strategy make the sum of all gens with a double value.
	 * @return static_cast<IFitnessStrategy*>(the) strategy
	 */
	IFitnessStrategy* createSumFitnessStrategy()const override;
	/**
	 * creates a EuclidicFitnessStrategy. This strategy calculate the euclidic distance of all gens with a double value.
	 * @return static_cast<IFitnessStrategy*>(the) strategy
	 */
	IFitnessStrategy* createEuclidicDistanceFitnessStrategy()const override;
	/**
	 * creates a TestFitnessStrategy which is the hardest test for a gen. alg. Only a smale area has a fitness. All other gives no information!
	 * @param fitness static_cast<IFitnessStrategy*>(a) other fitness strat* Over this is the class as singleton concepted. Only one engine for a run.egy which define the smal area.
	 * @return static_cast<IFitnessStrategy*>(the) strategy
	 */
	IFitnessStrategy* createExtreamTestFitnessStrategy(const IFitnessStrategy* fitness)const override;
	/**
	 * creates the test function fitness strategy from the papper tp this alg.
	 * f(x,y) = 10.0 * (x� + 2.5y� - y) * exp(1 - (x� + y�)) + 2.4 + 0.1x� + 0.1y�
	 * @return static_cast<IFitnessStrategy*>(the) strategy
	 */
	IFitnessStrategy* createTestFitnessStrategy()const override;
	/**
	 * returns a fitness strategy which calculate the inverse value from a other strategy.
	 * @param strategy static_cast<IFitnessStrategy*>(the) other strategy
	 * @return static_cast<IFitnessStrategy*>(the) resulting strategy
	 */
	IFitnessStrategy* createInvertedFitnessStrategy(const IFitnessStrategy* strategy)const override;
	/**
	 * creates a random strategy for double values.
	 * The values will be generated in the intervals [base-epsilon:-epsilon] or [epsilon:factor+base+epsilon]
	 *
	 * This means a random value in the interval [0:1] will be mul. with factor => [0:factor]
	 * After this it will be shifted by base => [base:factor+base]
	 * And than divided at the point zero and moved away by epsilon from the point zero
	 * => [base-epsilon:-epsilon], [epsilon:factor+base+epsilon]
	 *
	 * For example:
	 * We want values in the intervals [-10:-5] and [5:10].
	 * ==> epsilon=5
	 * ==> [-5:5]
	 * ==> base=-5
	 * ==> [0:10]
	 * ==> factor=10
	 *
	 * @param random static_cast<RandGen*>(random) generator
	 * @param base static_cast<double>(normal)=0.0 moves the ground interval
	 * @param factor static_cast<double>(normal)=1.0 resize the interval
	 * @param epsilon static_cast<double>(normal)=0.0 devided the interval at the point zero and move the
	 * 			values by epsilon away from point zero
	 * @return static_cast<IRandomStrategy*>(the) strategy
	 */
	IRandomStrategy* createDoubleRandomStrategy(RandGen* random, double base=0.0, double factor=1.0, double epsilon = 0.0)const override;
	/**
	 * creates mutation strategy which change the old values by add a other value
	 * @param strategy static_cast<IMutationFactorStrategy*>(this) strategy define what the other value is.
	 * @param mutationProbability static_cast<int>(the) probability of mutation in one per tausend (1/1000)
	 * @return static_cast<IMutationStrategy*>(the) strategy
	 */
	IMutationStrategy* createValueMutationStrategy(IMutationFactorStrategy* strategy, int mutationProbability)const override;
	/**
	 * creates a mutation factor strategy with a fix value
	 * @param value static_cast<IValue*>(the) fix value.
	 * @return static_cast<IMutationFactorStrategy*>(the) strategy
	 */
	IMutationFactorStrategy* createFixMutationFactorStrategy(const IValue* value)const override;
	/**
	 * creates a mutation factor strategy with a optimized value (varianz about all existing gens)
	 * @return static_cast<IMutationFactorStrategy*>(the) strategy
	 */
	IMutationFactorStrategy* createStandartMutationFactorStrategystatic_cast<void>(const) override;
	/**
	 * creates a generation size strategy with a fix value
	 * @param value static_cast<int>(the) size of the generation
	 * @return static_cast<IGenerationSizeStrategy*>(the) strategy
	 */
	IGenerationSizeStrategy* createFixGenerationSizeStrategy(int value)const override;
	/**
	 * creates a generation size strategy with a optimized size for the generation (changing by speed of development)
	 * @param startSize static_cast<int>(the) size at the start time
	 * @param numGeneration static_cast<int>(number) of generation. (needed for the speed calculation
	 * @return static_cast<IGenerationSizeStrategy*>(the) strategy
	 */
	IGenerationSizeStrategy* createStandartGenerationSizeStrategy(int startSize, int numGeneration)const override;
	/**
	 * creates a elite select strategy
	 * @return static_cast<ISelectStrategy*>(the) strategy
	 */
	ISelectStrategy* createEliteSelectStrategystatic_cast<void>(const) override;
	/**
	 * creates a tournament SelctStrategy. By this strategy will two individual fight again
	 * (the individual with the higher fitness lives longer).
	 * @param random static_cast<RandGen*>(random) generator
	 * @return static_cast<ISelectStrategy*>(the) strategy
	 */
	ISelectStrategy* createTournamentSelectStrategy(const RandGen* random)const override;
	/**
	 * creates a random select strategy. By this strategy will a random selected individual fight again a random number.
	 * If the random number higher than the fitness from the individual, than it will be die.
	 * @param random static_cast<RandGen*>(ranom) generator
	 * @return static_cast<ISelectStrategy*>(the) strategy
	 */
	ISelectStrategy* createRandomSelectStrategy(const RandGen* random)const override;
	/**
	 * creates a IValue (TemplateValue<double) from type double.
	 * @param value static_cast<double>(the) value
	 * @return static_cast<IValue*>(the) new object
	 */
	IValue* createDoubleValue(double value)const override;

	//Storable
	/** stores the object to the given file stream (binary).
	 */
	bool storeGA(const FILE* f) const override;

	/** loads the object from the given file stream (binary).
	 */
	bool restoreGA(const FILE* f) const override;

	// inserts
	/**
	 * add a GenPrototype to the alg.
	 * @param prototype static_cast<GenPrototype*>(the) prototype
	 */
	void insertGenPrototype(const GenPrototype* prototype) override;

	// object creation
	/**
	 * create a prototype.
	 * @param name (STRING) THE NAME OF THE PROTOTYPE
	 * @param randomStrategy static_cast<IRandomStrategy*>(the) random strategy to generate values of the type
	 * which the prototype represent
	 * @param mutationStrategy static_cast<IMutationStrategy*>(the) mutation strategy to change a value (Gen)
	 * @return static_cast<GenPrototype*>(the) prototype
	 */
	GenPrototype* createPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy)const override;

	// singleton
	/**
	 * returns the one and only API to the alg.
	 * @return static_cast<SingletonGenAlgAPI*>(the) api
	 */
	inline static SingletonGenAlgAPI* getInstance(void) {if(m_api==0)m_api = new SingletonGenAlgAPI();return m_api;}
	/**
	 * destroy the api
	 * @param cleanStrategies (bool) default = false set a flag to clean the strategies, which are seted.
	 */
	inline static void destroyAPI(bool cleanStrategies=false) {getInstance()->m_cleanStrategies=cleanStrategies; if(m_api!=0){delete m_api;m_api=0;}}

	// data access
	/**
	 * returns the best individual which the alg. have found
	 * @return static_cast<Individual*>(the) best
	 */
	inline Individual* getBestIndividualstatic_cast<void>(const) override {return SingletonGenEngine::getInstance()->getBestIndividual();}

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
	SingletonGenAlgAPI() override;
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
