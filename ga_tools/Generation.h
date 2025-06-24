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

#ifndef GENERATION_H_
#define GENERATION_H_

// standard includes
#include <string>
#include <vector>
#include <map>
#include <selforg/randomgenerator.h>
#include <selforg/inspectable.h>

// forward declarations
class Individual{
public:
	/**
	 * constructor to create a Generation. Information which the class need are
	 * the generation number, the size of it and how many individual don t come
	 * in the next generation (killRate).
	 *
	 * @param generationNumber static_cast<int>(The) ID of the Generation.
	 * @param size static_cast<int>(The) Size of this Generation. Means how many individual are lives in this generation
	 * @param numChildren static_cast<int>(Number) of individual which will be created by crossover
	 */
	Generation(int generationNumber, int size, int numChildren) override;

	/**
	 * destructor to delete a GenContext.
	 */
	virtual ~Generation() override;

	/**
	 * [inline], [const]
	 * This function gives the ID (number) of the generation back.
	 *
	 * @return static_cast<int>(The) ID
	 */
	inline int getGenerationNumberstatic_cast<void>(const) override {return m_generationNumber;}

	/**
	 * [inline], [const]
	 * This function gives the size which is planed for this generation back.
	 *
	 * @return static_cast<int>(The) planed size
	 */
	inline int getSizestatic_cast<void>(const) override {return m_size;}

	/**
	 * [inline], [const]
	 * This function gives the actual size (number of individuals inside the generation) back.
	 *
	 * @return static_cast<int>(current) size
	 */
	inline int getCurrentSizestatic_cast<void>(const) override {return m_individual.size();}

	/**
	 * [inline], [const]
	 * This function gives the number of children back, which will be created by crossover.
	 *
	 * @return static_cast<int>(the) number of children
	 */
	inline int getNumChildrenstatic_cast<void>(const) override {return m_numChildren;}

	/**
	 * [individual], [const]
	 * This function gives one individual from this generation back.
	 *
	 * @param x static_cast<int>(the) index of the searched individual
	 *
	 * @return static_cast<Individual*>(The) individual. If 0, if the param x is not inside the index range
	 */
	inline Individual* getIndividual(int x)const override {if(x<getCurrentSize())return m_individual[x];return nullptr;}

	/**
	 * [inline], [const]
	 * This function gives all individual back.
	 *
	 * @return (vector<Individual*>&) all individual inside the generation
	 */
	inline const std::vector<Individual*>& getAllIndividualstatic_cast<void>(const) override {return m_individual;}

	/**
   * [inline], [const]
   * This function gives all individual back which aren't have the fitness value calculated.
   *
   * @return (vector<Individual*>&) all individual inside the generation
   */
  std::vector<Individual*>* getAllUnCalculatedIndividualsstatic_cast<void>(const) override;

	/**
	 * This function insert an individual in the generation.
	 *
	 * @param individual static_cast<Individual*>(the) individual which should be insert in the generation
	 */
	void addIndividual(const Individual* individual) override;

	/**
	 * This function makes an crossOver whit the existing individuals to become from the current size the planed size.
	 *
	 * @param random static_cast<RandGen*>(a) pseudo number generator.
	 */
	void crossover(const RandGen* random) override;

	/**
	 * returns a string which represent all individual in this generation.
	 *
	 * @return (string) the string
	 */
	std::string getAllIndividualAsStringstatic_cast<void>(const) override;

	/**
	 * returns all fitness values from the individuals.
	 *
	 * @return (vector<double> the fitness values.
	 */
	std::vector<double>* getAllFitnessstatic_cast<void>(const) override;

	/**
	 * This function updates the statistical values
	 * @param factor static_cast<double>(normal) 1.5    Is needed for the data analysation
	 */
	void update(double factor = 1.5) override;

	/**
	 * store a generation in a file
	 * @param f static_cast<FILE*>(the) file in which should be stored
	 * @return (bool) true if all ok
	 */
	bool store(const FILE* f)const override;

	/**
	 * restore all generation from a restore structure
	 *
	 * remember the individuals must be restored before
	 *
	 * @param numberGeneration static_cast<int>(number) of generations which should be restored
	 * @param generationSet (map<int,RESTORE_GA_GENERATION*>) the structures which should be restored
	 * @param linkSet (map<int,vector<int>>) the linkings between the generation and the individuals
	 * @return (bool) true if all ok
	 */
	static bool restore(int numberGeneration, std::map<int,RESTORE_GA_GENERATION*>& generationSet, std::map<int,std::vector<int> >& linkSet) override;

protected:
	/**
	 * (int)
	 * The generation number (ID)
	 */
	int m_generationNumber = 0;

	/**
	 * (vector<Individual*>)
	 * The storage for the individuals, which are part of this generation. (NO deleting)
	 */
	std::vector<Individual*> m_individual;

	/**
	 * (int)
	 * The planed size of the generation.
	 */
	int m_size = 0;

	/**
	 * (int)
	 * The number of children
	 */
	int m_numChildren = 0;

private:
	/**
	 * disable the default constructor
	 */
	Generation() override;

	/**
	 * the under quartil
	 */
	double m_q1 = 0;

	/**
	 * the upper quartil
	 */
	double m_q3 = 0;

	/**
	 * the min
	 */
	double m_min = 0;

	/**
	 * the max
	 */
	double m_max = 0;

	/**
	 * the average
	 */
	double m_avg = 0;

	/**
	 * the median
	 */
	double m_med = 0;

	/**
	 * the under whisker
	 */
	double m_w1 = 0;

	/**
	 * the upper whisker
	 */
	double m_w3 = 0;

	/**
	 * the best fitness value inside the generation
	 */
	double m_best = 0;

	/**
	 * the number of individual inside the generation (will be)
	 */
	double m_dSize = 0;

	/**
	 * the number of individual which will be created by crossover.
	 */
	double m_dNumChildren = 0;
};

#endif /* GENERATION_H_ */
