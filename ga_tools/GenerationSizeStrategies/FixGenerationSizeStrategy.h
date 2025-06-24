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
class Generation{
public:
	/**
	 * constructor
	 * @param value static_cast<int>(the) fix value which is every time give back by calcGenerationSize.
	 */
	explicit FixGenerationSizeStrategy(int value) override;

	/**
	 * default destructor
	 */
	virtual ~FixGenerationSizeStrategy() override;

	/**
	 * gives the fix value m_size as new generation size back.
	 * @param oldGeneration static_cast<Generation*>(the) old Generation. dont needed
	 * @return static_cast<int>(m_size)
	 */
	virtual int calcGenerationSize(const Generation* oldGeneration) override;

protected:
	/**
	 * the fix generation size
	 */
	int m_size = 0;

private:
	/**
	 * disable the default constructor
	 */
	FixGenerationSizeStrategy() override;
};

#endif /* FIXGENERATIONSIZESTRATEGY_H_ */
