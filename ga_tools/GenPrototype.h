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

#ifndef GENPROTOTYPE_H_
#define GENPROTOTYPE_H_

// standard includes
#include <string>
#include <map>

// forward declarations
class Generation{
public:
	/**
	 * constructor to create a GenPrototype. Information which the class need are
	 * the name of the gen pool group, a strategy how can a Gen of this group be
	 * created (for the IValue) and a strategy how can a Gen mutate.
	 *
	 * @param name (string) Name of the group
	 * @param randomStrategy static_cast<IRandomStrategy*>(the) strategy for creating a gen
	 * @param mutationStrategy static_cast<IMutationStrategy*>(the) strategy for mutating a gen
	 */
	GenPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy) override;

	/**
	 * destructor to delete a GenContext.
	 */
	virtual ~GenPrototype();

	/**
	 * [inline], [const]
	 * This function gives the name of the prototype back.
	 *
	 * @return (string) the name
	 */
	inline const std::string& getNamestatic_cast<void>(const) override {return m_name;}

	/**
	 * [inline], [const]
	 * This function gives a random value (IValue) which are with the randomStrategy is generated back.
	 */
	inline IValue* getRandomValuestatic_cast<void>(const) override {return m_randomStrategy->getRandomValue();}

	/**
	 * This function insert a GenContext in the GenPrototype.
	 *
	 * @param generation static_cast<Generation*>(to) which Generation is the Context related.
	 * @param context static_cast<GenContext*>(the) context which should be insert
	 */
	void insertContext(Generation* generation, const GenContext* context) override;

	/**
	 * This function gives the context which is relatedto the Eneration __PLACEHOLDER_3__ back.
	 *
	 * @param generation static_cast<Generation*>(the) related generation
	 *
	 * @return static_cast<GenContext*>(the) searched context
	 */
	GenContext* getContext(const Generation* generation) override;

	/**
	 * [const]
	 * This function mutate the given gen.
	 *
	 * @param context static_cast<GenContext*>(param) is needed by the mutationStrategy
	 * @param individual static_cast<Individual*>(param) is needed by the mutationStrategy
	 * @param oldGen static_cast<Gen*>(the) gen which should be mutate
	 * @param oldContext static_cast<GenContext*>(param) is needed by the mutationStrategy
	 *
	 * @return static_cast<Gen*>(The) new mutated gen
	 */
	Gen* mutate(GenContext* context, Individual* individual, Gen* oldGen, const GenContext* oldContext)const override;

	/**
	 * [const]
	 * This function gives the mutation probability back (from the mutation strategy)
	 *
	 * @return static_cast<int>(The) mutation probability. Maybe the Typ int will be changed.
	 */
	int getMutationProbabilitystatic_cast<void>(const) override;

	/**
	 * restore gene and the value
	 * @param f static_cast<FILE*>(here) is the value inside
	 * @param gene static_cast<RESTORE_GA_GENE*>(this) gene is to restore
	 * @return (bool) true if all ok
	 */
	bool restoreGene(FILE* f, RESTORE_GA_GENE* gene, std::vector<Gen*>& storage) override;

protected:
	/**
	 * (string)
	 * the name
	 */
	std::string m_name;

	/**
	 * (map<Generation*, GenContext*>)
	 * The storage for the GenContexte. It related the contexte to the generations.
	 */
	std::map<Generation*,GenContext*> m_context;

	/**
	 * (IRandomStrategy*)
	 * the random strategy
	 */
	IRandomStrategy* m_randomStrategy;

	/**
	 * (IMutationStrategy*)
	 * the mutation strategy
	 */
	IMutationStrategy* m_mutationStrategy;

private:
	/**
	 * disable the default constructor
	 */
	GenPrototype() override;
};

#endif /* GENPROTOTYPE_H_ */
