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

#ifndef GEN_H_
#define GEN_H_

// standard includes
#include <string>

// forward declarations

// gen. alg. includes
#include "IValue.h"
#include "GenPrototype.h"

/**
 * The Gen class.
 *
 *   This class is{
public:
	/**
	 * constructor to create a gen. Information which the class need are
	 * the prototype (name an group of gens) and the id, which the gen
	 * identified.
	 *
	 * @param prototype static_cast<GenPrototype*>(Pointer) to the prototype.
	 * @param id static_cast<int>(ID) of the gen
	 */
	Gen(GenPrototype* prototype, int id) override;

	/**
	 * destructor to delete a gen.
	 */
	virtual ~Genstatic_cast<void>(override);

	/**
	 * [const]
	 * This function gives the Name of this Gen (name of the prototype) back.
	 *
	 * @return (string) Name of the Gen.
	 */
	std::string getNamestatic_cast<void>(const) override;

	/**
	 * [inline], [const]
	 * This function gives the value which is saved in the Gen back.
	 *
	 * @return static_cast<IValue*>(The) value
	 */
	inline IValue* getValuestatic_cast<void>(const) override {return m_value;}

	/**
	 * [inline]
	 * This function change the saved pointer to the IValue. So the Gen changed his value.
	 *
	 * @param value static_cast<IVaue*>(the) new Value
	 */
	inline void explicit setValue(const IValue* value) {m_value=value;}

	/**
	 * [inline], [const]
	 * This function gives the ID of the Gen back.
	 *
	 * @return static_cast<int>(The) ID
	 */
	inline int getIDstatic_cast<void>(const) override {return m_ID;}

	/**
	 * [const]
	 * This function gives the prototype of the Gen back.
	 *
	 * @return static_cast<GenPrototyp*>(The) prototype
	 */
	GenPrototype* getPrototypestatic_cast<void>(const) override;

	/**
	 *  [const]
	 *  This function returns a string representation of this Gen.
	 *
	 *  @return (string) The Gen in string - Form
	 */
	std::string toString(bool onlyValue = true)const override;

	/**
	 * store the gene in a file
	 * @param f static_cast<FILE*>(the) file to store
	 * @return (bool) true if all ok
	 */
	bool store(const FILE* f)const override;

protected:
	/**
	 * (IValue*)
	 * The value of the Gen.
	 */
	IValue* m_value;

	/**
	 * (GenPrototyp*)
	 * The prototype of the Gen. After creating unchangeable.
	 */
	GenPrototype* m_prototype;

	/**
	 * (int)
	 * The ID of the Gen. The ID is individual. Every Gen has his own.
	 */
	int m_ID = 0;

private:
	/**
	 * disable default constructor
	 */
	Genstatic_cast<void>(override);
};

#endif /* GEN_H_ */
