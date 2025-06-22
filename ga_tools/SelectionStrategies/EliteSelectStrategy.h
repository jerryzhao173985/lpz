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

#ifndef ELITESELECTSTRATEGY_H_
#define ELITESELECTSTRATEGY_H_

//forward declaration
class Generation;

//ga_tools includes
#include "ISelectStrategy.h"

/**
 * This class makes a elite select and bring only the best individual in the next generation.
 */
class EliteSelectStrategy :public ISelectStrategy {
public:
	/**
	 * default constructor
	 */
	EliteSelectStrategy();

	/**
	 * default destructor
	 */
	virtual ~EliteSelectStrategy();

	/**
	 * select the individual in the old generation which should be in the new generation (only the best).
	 * @param oldGeneration (Generation*) the old generation
	 * @param newGeneration (Generation*) the new generation
	 */
	virtual void select(Generation* oldGeneration, Generation* newGeneration);
};

#endif /* ELITESELECTSTRATEGY_H_ */
