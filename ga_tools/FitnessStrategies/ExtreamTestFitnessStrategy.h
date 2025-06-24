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

#ifndef EXTREAMTESTFITNESSSTRATEGY_H_
#define EXTREAMTESTFITNESSSTRATEGY_H_

#include "IFitnessStrategy.h"

//forward declaration
class Individual;

class ExtreamTestFitnessStrategy : public IFitnessStrategy {
public:
	/**
	 * constructor
	 * This strategy needs a other fitness strategy to calculate the resulting fitness.
	 * @param fitness the other fitness
	 */
	explicit ExtreamTestFitnessStrategy(IFitnessStrategy* fitness);

	/**
	 * default destructor
	 * do nothing
	 */
	virtual ~ExtreamTestFitnessStrategy();

	/**
	 * implementation for getFitness of IFitnessStrategy.
	 * return a of the other fitness strategy if the value is lower than 10.
	 * Else it return 100.
	 * @param individual the individual for which the value is calculated
	 * @return the result
	 */
	virtual double getFitness(const Individual* individual) override;

private:
	/**
	 * a other fitness strategy which is used as base calculation.
	 */
	IFitnessStrategy* m_strategy = nullptr;

	/**
	 * disable the default constructor
	 */
	ExtreamTestFitnessStrategy() = delete;
};

#endif /* EXTREAMTESTFITNESSSTRATEGY_H_ */
