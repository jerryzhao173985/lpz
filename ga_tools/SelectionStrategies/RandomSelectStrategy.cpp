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

#include "RandomSelectStrategy.h"

//includes
#include <selforg/statistictools.h>
#include <vector>
#include <algorithm>

//ga_tools includes
#include "Generation.h"
#include "Individual.h"
#include "SingletonGenAlgAPI.h"

RandomSelectStrategy::RandomSelectStrategy() {
        // nothing
}

RandomSelectStrategy::RandomSelectStrategy(const RandGen* random) {
        m_random = const_cast<RandGen*>(random);
}

RandomSelectStrategy::~RandomSelectStrategy() {
        // nothing
}

void RandomSelectStrategy::select(Generation* oldGeneration, Generation* newGeneration) {// TODO: Review scope of variable
        std::vector<Individual*> list;                // darf nicht const und & sein!!!                        //the list of the individual which are __PLACEHOLDER_4__
        double min;                                                                                                                                                //the minimum of the fitness values
        int num = oldGeneration->getCurrentSize();                                                                                //number of individual
        int kill = num - oldGeneration->getSize();                                                                                //the kill rate
        std::vector<Individual*>::iterator iter;                                                                                //iterator for the list
        int test=0;                                                                                                                                                //make sure that the function terminate...
        double range;
        int r1;

        for(int y=0;y<num;++y) {                                                                                                //insert the individual of the old generation in the living list
                list.push_back(oldGeneration->getIndividual(y));
        }

        // calc range and min of all fitness
        std::vector<double>* fitnessVector = oldGeneration->getAllFitness();
        if (!fitnessVector->empty()) {
            min = *std::min_element(fitnessVector->begin(), fitnessVector->end());
            double max = *std::max_element(fitnessVector->begin(), fitnessVector->end());
            range = max - min;
        } else {
            min = 0.0;
            range = 1.0;
        }
        delete fitnessVector;

        // kill some elements
        while(kill>0) {
                r1 = (static_cast<int>(m_random->rand()*1000000.0))%list.size();
                if(m_random->rand()*range+min<list[r1]->getFitness()) {                                //if the random value over the range
                        iter=list.begin();                                                                                                //of values better than the fitness
                        test=0;                                                                                                                        //value of the random selected
                        std::advance(iter,r1);                                                                                        //individual, so it will be die.
                        list.erase(iter);
                        --kill;
                }
                else
                        ++test;

                if(test==10000)                                                                                                                //if they are no kills after 10000 test, so stop and make an elite select.
                        break;
        }

        if(kill>0) {                                                                                                                        //elite select, if not enough individual are killed
                ISelectStrategy* elite = SingletonGenAlgAPI::getInstance()->createEliteSelectStrategy();
                Generation* newold = new Generation(oldGeneration->getGenerationNumber(),oldGeneration->getSize(),kill);
                // take the rest in the __PLACEHOLDER_5__ with the name newold
                for(int x=0;x<static_cast<int>(list.size()) && x<oldGeneration->getSize()+kill;++x) {
                        newold->addIndividual(list[x]);
                }
                elite->select(newold,newGeneration);

                // clean
                while(list.size()>0) {
                        list.erase(list.begin());
                }
                list.clear();

                delete elite;

                return;
        }

        // take the rest in the new generation
        for(int x=0;x<static_cast<int>(list.size()) && x<newGeneration->getSize();++x) {
                newGeneration->addIndividual(list[x]);
        }

        // clean
        while(list.size()>0) {
                list.erase(list.begin());
        }
        list.clear();
}
