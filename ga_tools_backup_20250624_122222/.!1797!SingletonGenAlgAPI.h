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
	void select(bool createNextGeneration=true);
	/**
	 * create the children from to fill up the next generation
	 * @param random (RandGen*) random generator
	 */
	void crossover(RandGen* random);
	/**
	 * update the internal statistical data
	 * @param factor (double) normal=1.5 is needed for the whisker distance
	 */
	void update(double factor = 1.5);
	/**
	 * prepares the first generation and optional the enabled measure
	 * @param startSize (int) Number of individual at begin of the gen. alg.
	 * @param startChildren (int) Number of individual which will be created by crossover
	 * @param random (RandGen*) A random generator
	 * @param withUpdate (bool) is needed for "generateFirstGeneration"
	 */
	void prepare(int startSize, int numChildren, RandGen* random, bool withUpdate = true);
	/**
	 * prepares the next generation and optional the enabled measure
	 */
	void prepare();
	/**
	 * makes a step in the measure
	 * @param time (double) time stamp in the measure
	 */
	void measureStep(double time);
	/**
	 * start the sequenz of select, crossover, update in a automatically loop
	 * @param startSize (int) Number of individual at begin of the gen. alg.
	 * @param numChildren (int) Number of individual which will be created by crossover
	 * @param numGeneration (int) Number of generation which the alg. max. runs
	 * @param random (RandGen*) random generator
	 */
	void runGenAlg(int startSize, int numChildren, int numGeneration, RandGen* random);

	//measure
	/**
	 * enables data measure with more than one plotOption.
