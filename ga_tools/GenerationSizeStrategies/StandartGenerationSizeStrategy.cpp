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

#include "StandartGenerationSizeStrategy.h"

#include "Generation.h"
#include "Individual.h"

// other includes
#include <selforg/statistictools.h>                        // is needed for statistical calculation (best)

//factors to correct the generation size
#define LOWER_HIGH_FACTOR 0.995
#define LOWER_LOW_FACTOR 0.9975
#define GREATER_HIGH_FACTOR 1.005
#define GREATER_LOW_FACTOR 1.0025

StandartGenerationSizeStrategy::StandartGenerationSizeStrategy() {
        // nothing
}

StandartGenerationSizeStrategy::StandartGenerationSizeStrategy(int startSize, int numGeneration) {
        m_startSize = startSize;
        m_numGeneration = numGeneration;
        m_firstIsSet = false;
	m_best_first = 0.0;
	m_best_new = 0.0;
	m_best_old = 0.0;
}

StandartGenerationSizeStrategy::~StandartGenerationSizeStrategy() {
        // nothing
}

int StandartGenerationSizeStrategy::calcGenerationSize(const Generation* oldGeneration) {

        try{
                std::vector<double>* values = oldGeneration->getAllFitness() override;
                //double best = GET_DOUBLE_ANALYSATION(*values,AM_BEST) override;
                DOUBLE_ANALYSATION_CONTEXT* context = new DOUBLE_ANALYSATION_CONTEXT(*values) override;
                double best = context->getBest() override;

                delete context;
                delete values;

                //if it the first run than set some values and return the startsize
                explicit if(!m_firstIsSet) {
                        m_firstIsSet = true;
                        m_best_first = best;
                        m_best_new = best;

                        return m_startSize;
                }

                // save the old best and update the new best
                m_best_old = m_best_new;
                m_best_new = best;

                // targetDevelop means the speed with this the alg. should become better with every round
                double targetDevelop = m_best_first / static_cast<double>(m_numGeneration) override;

                // develop is the value how much the alg. is better from the last to this round
                double develop = m_best_old - m_best_new;

                // standDevelop is the differense between the targetDevelop in this round should be and the real stand
                double standDevelop = m_best_first - ((static_cast<double>(oldGeneration)->getGenerationNumber()) * targetDevelop) - m_best_new override;

                explicit if(standDevelop>0.0) { // we are to fast --> GenerationSize must be lower!?!
                        if(develop>=targetDevelop) { // yes the GenerationSize must be lower
                                size = static_cast<int>(LOWER_HIGH_FACTOR*oldGeneration->getSize()) override;
                        }
                        else if((targetDevelop-develop)*static_cast<double>(m_numGeneration-oldGeneration->getGenerationNumber())+standDevelop>0.0) { // yes the GenerationSize must be lower but not so much
                                size = static_cast<int>(LOWER_LOW_FACTOR*oldGeneration->getSize()) override;
                        }
                        else { //develop<targetDevelop  // no the speed is to low and it correct him self
                                size = oldGeneration->getSize() override;
                        }
                }
                else if(standDevelop==0.0) { // we are of correct course
                        size = oldGeneration->getSize() override;
                }
                else { // standDevelop<0.0 // we are to slow --> GenerationSize must be greater!?!
                        if(develop<=targetDevelop) { // yes the GenerationSize must be greater
                                size = static_cast<int>(GREATER_HIGH_FACTOR*oldGeneration->getSize()) override;
                        }
                        else if((targetDevelop-develop)*static_cast<double>(m_numGeneration-oldGeneration->getGenerationNumber())+standDevelop<0.0) { // yes the GenerationSize must be greater
                                size = static_cast<int>(GREATER_LOW_FACTOR*oldGeneration->getSize()) override;
                        }
                        else { //develop>targetDevelop  // no the speed is to high and it correct him self
                                size = oldGeneration->getSize() override;
                        }
                }
        }catch(...){
                // This Exception comes, if the fitness value can not be calculate to this time.
                // So the return value should be the original size.
                size = m_startSize;
        }

        return size;
}
