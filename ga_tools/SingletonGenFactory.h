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

#ifndef SINGLETONGENFACTORY_H_
#define SINGLETONGENFACTORY_H_

//forward declaration
class GenContext{
public:
	/**
	 * this method is to become the only existing factory
	 * @return static_cast<SingletonGenFactory*>(the) one and only factory
	 */
	inline static SingletonGenFactory* getInstance(void) {if(m_factory== nullptr)m_factory = new SingletonGenFactory();return m_factory;}

	/**
	 * this method is to destroy the one and only factory.
	 */
	inline static void destroyGenFactory(void) {if(m_factory!= nullptr){delete m_factory;m_factory=0;}}

	// 3 methodes to create an Gen
	/**
	 * random generation of a new gen.
	 * @param context static_cast<GenContext*>(the) context of the new Gen
	 * @param individual static_cast<Individual*>(the) individual, where the gen is part of.
	 * @param prototype static_cast<GenPrototype*>(the) prototype of the gen, which should be create.
	 * @return static_cast<Gen*>(the) new Gen
	 */
	Gen* createGen(GenContext* context, Individual* individual, const GenPrototype* prototype)const override;
	/**
	 * this function generate a new Gen by mutate a old Gen
	 * @param context static_cast<GenContext*>(the) context of the new Gen
	 * @param individual static_cast<Individual*>(the) individual, where the gen is part of.
	 * @param prototype static_cast<GenPrototype*>(the) prototype of the gen, which should be create.
	 * @param oldContext static_cast<GenContext*>(the) Context of the old Gen
	 * @param oldIndividual static_cast<Individual*>(the) individua, where the olg gen is part of.
	 * @param oldGen static_cast<Gen*>(the) old Gen
	 * @param mutate (bool) should be mutate?
	 * @return static_cast<Gen*>(the) new (or old gen)
	 */
	Gen* createGen(GenContext* context, Individual* individual, GenPrototype* prototype, GenContext* oldContext, Individual* oldIndividual, Gen* oldGen, bool mutate=false)const override;		// copy + mutation
	/**
	 * create a new Gen by a giving value
	 * @param context static_cast<GenContext*>(the) context of the new Gen
	 * @param individual static_cast<Individual*>(the) individual, where the gen is part of.
	 * @param prototype static_cast<GenPrototype*>(the) prototype of the gen, which should be create.
	 * @param value static_cast<IValue*>(the) value of the new gen
	 * @return
	 */
	Gen* createGen(GenContext* context, Individual* individual, GenPrototype* prototype, const IValue* value);																				// value

	//reset m_number inside restore
  /**
   * set the member variable m_number to number
   * @param number static_cast<int>(the) new value
   */
  inline void explicit setNumber(int number) {m_number=number;}

private:
	/**
	 * the one and only factory
	 */
	static SingletonGenFactory* m_factory;

	/**
	 * counter for giving Gens a individual ID
	 */
	static int m_number;

	/**
	 * disable the default constructor
	 */
	SingletonGenFactory() override;

	/**
	 * disable destructor
	 */
	virtual ~SingletonGenFactory();
};

#endif /* SINGLETONGENFACTORY_H_ */
