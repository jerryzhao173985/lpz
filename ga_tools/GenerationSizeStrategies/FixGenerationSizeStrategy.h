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

#ifndef FIXGENERATIONSIZESTRATEGY_H_
#define FIXGENERATIONSIZESTRATEGY_H_

//forward declaration
class Generation;

//ga_tools includes
#include "IGenerationSizeStrategy.h"

/**
 * This class implements the generation size strategy with a fix value which is over the constructor given
 */
class FixGenerationSizeStrategy: public IGenerationSizeStrategy {
public:
	/**
	 * constructor
	 * @param value (int) the fix value which is every time give back by calcGenerationSize.
	 */
	FixGenerationSizeStrategy(int value);

	/**
	 * default destructor
	 */
	virtual ~FixGenerationSizeStrategy();

	/**
	 * gives the fix value m_size as new generation size back.
	 * @param oldGeneration (Generation*) the old Generation. dont needed
	 * @return (int) m_size
	 */
	virtual int calcGenerationSize(Generation* oldGeneration);

protected:
	/**
	 * the fix generation size
	 */
	int m_size;

private:
	/**
	 * disable the default constructor
	 */
	FixGenerationSizeStrategy();
};

#endif /* FIXGENERATIONSIZESTRATEGY_H_ */
