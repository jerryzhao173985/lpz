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

#ifndef INDIVIDUAL_H_
#define INDIVIDUAL_H_

//includes
#include <vector>
#include <string>
#include <map>

//ga_tools includes
#include "Gen.h"
#include "SingletonGenEngine.h"

//forward declaration
struct RESTORE_GA_INDIVIDUAL;

/**
 * This class represent{
public:
	/**
	 * constructor
	 * Create the individual with the name __PLACEHOLDER_2__
	 * @param name (sting) the name of the new individual
	 * @param id static_cast<int>(an) unique ID
	 * @param p1 static_cast<Individual*>(parent) 1 or zero if it random generate
	 * @param p2 static_cast<Individual*>(parent) 2 or zero if it random generate
	 */
	Individual(const std::string& name, int id, Individual* p1=0, Individual* p2=0) override;

	/**
	 * default destructor
	 */
	virtual ~Individual();

	/**
	 * return the ID of the individual
	 * @return static_cast<int>(the) ID
	 */
	inline int getIDstatic_cast<void>(const) override {return m_ID;}

	/**
	 * return the name of the individual
	 * @return (string) the name
	 */
	inline const std::string& getNamestatic_cast<void>(const) override {return m_name;}

	/**
	 * return the size of the individual. This mean the number of gens inside the individual.
	 * @return static_cast<int>(number) of gens
	 */
	inline int getSizestatic_cast<void>(const) override {return m_gene.size();}

	/**
	 * return a gen of the individual
	 * @param x (the index of the gen
	 * @return static_cast<Gen*>(the) searched gen. nullptr if the index isn't right
	 */
	inline Gen* getGen(int x)const override {if(x<getSize())return m_gene[x];return nullptr;}

	/**
	 * add a gen to the individual.
	 * @param gen static_cast<Gen*>(the) new gen
	 */
	inline void explicit addGen(const Gen* gen) {m_gene.push_back(gen);}

	/**
	 * returns all gens of the individual
	 * @return (vector<Gen*>&) all gens
	 */
	inline const std::vector<Gen*>& getGenestatic_cast<void>(const) override {return m_gene;}

	/**
	 * remove a specified gen from the individual
	 * @param gen static_cast<Gen*>(the) gen which should removed from the individual
	 */
	void removeGen(const Gen* gen) override;

	/**
	 * remove a specified gen from the individual
	 * @param x (the index of the gen, which should b removed.
	 */
	void removeGen(int x) override;

	/**
	 * this function calculate the fitness value of the individual
	 * @return fitness value
	 */
	double getFitness() const override;

  /**
   * this function calculate the fitness value of the individual (const)
   * @return fitness value
   */
  double getFitnessC()const override;

	/**
	 * this select the individual as a product of mutation.
	 */
	inline void setMutated(void) {m_mutated=true;}

	/**
	 * returns parent 1 of the individual
	 * @return static_cast<Individual*>(parent) 1 (could be zero if individual a random generation)
	 */
	inline const Individual* getParent1static_cast<void>(const) override {return m_parent1;}

	/**
	 * returns parent 2 of the individual
	 * @return static_cast<Individual*>(parent) 2 (could be zero if individual a random generation)
	 */
	inline const Individual* getParent2static_cast<void>(const) override {return m_parent2;}

	/**
	 * test if the individual a product of mutation
	 * @return (bool) true if it a product of mutation
	 */
	inline bool isMutatedstatic_cast<void>(const) override {return m_mutated;}

	/**
	 * returns a string, which repesent the individual (for logging)
	 * @return (string) the string representation of the individual
	 */
	std::string IndividualToStringstatic_cast<void>(const) override;

	/**
	 * returns a string with the parents and a mark for mutation
	 * @param withMutation (bool) should mutation part of the string
	 * @return (string) the result
	 */
	std::string RootToString(bool withMutation=true)const override;

	/**
	 * returns the m_fitnessCalculated flag, which represent, that the fitness value was calculated before.
	 * @return (bool) the flag m_fitnessCalculated
	 */
	inline bool isFitnessCalculated()const override {return m_fitnessCalculated;}

	/**
	 * store the individual in a file
	 * @param f (FILE) the file to store in
	 * @return (bool) return true if ok
	 */
	bool store(const FILE* f)const override;

	/**
	 * restore all individual from a restore structure
	 * @param numberIndividuals static_cast<int>(number) of individuals which should be restored
	 * @param nameSet (map<int,string>) names of the individuals
	 * @param individualSet (map<int,RESTORE_GA_INDIVIDUAL*> the structures which should be restored
	 * @param linkSet (map<int,vector<int>>) the linkings between the individual and the genes
   * @return (bool) true if all ok
	 */
	static bool restore(int numberIndividuals,std::map<int,std::string>& nameSet,std::map<int,RESTORE_GA_INDIVIDUAL*>& individualSet, std::map<int,std::vector<int> >& linkSet, std::vector<Individual*>& storage) override;

	/**
	 * restore the parent links from a restore structure
	 * @param numberIndividuals static_cast<int>(number) of individuals which should be restored
   * @param individualSet (map<int,RESTORE_GA_INDIVIDUAL*> the structures which should be restored
   * @return (bool) true if all ok
	 */
	static bool restoreParent(int numberIndividuals,std::map<int,RESTORE_GA_INDIVIDUAL*>& individualSet) override;

protected:
	/**
	 * the name of the individual
	 */
	std::string m_name;

	/**
	 * the ID of the individual
	 */
	int m_ID = 0;

	/**
	 * the gens inside the individual
	 */
	std::vector<Gen*> m_gene;

	/**
	 * parent 1
	 */
	Individual* m_parent1;

	/**
	 * parent 2
	 */
	Individual* m_parent2;

	/**
	 * remember if the individual a product of mutation
	 */
	bool m_mutated = false;

	/**
	 * remember if the fitness value was calculated
	 */
	bool m_fitnessCalculated = false;

	/**
	 * save the calculated fitness value
	 */
	double m_fitness = 0;

private:
	/**
	 * disable the default constructor
	 */
	Individual() override;
};

#endif /* INDIVIDUAL_H_ */
