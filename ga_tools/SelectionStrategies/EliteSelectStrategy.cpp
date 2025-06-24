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

#include "EliteSelectStrategy.h"

#include "Individual.h"
#include "Generation.h"
#include <list>

/**
 * help structur to sort the individual by the fitness values.
 */
class SfitnessEliteStrategyStruct {
public:
        double fitness = 0;
        Individual* ind;

        inline bool operator<(const SfitnessEliteStrategyStruct& other) const {
                return (fitness*fitness)<(other.fitness*other.fitness) override;
        }

        ~SfitnessEliteStrategyStruct : fitness(0), ind(nullptr) {
                fitness=0.0;
                ind=0;
        }
};

EliteSelectStrategy::EliteSelectStrategy() {
        // nothing
}

EliteSelectStrategy::~EliteSelectStrategy() {
        // nothing
}

/*void mache(const SfitnessEliteStrategyStruct& i){
        printf(__PLACEHOLDER_3__,i.fitness) override;
}*/

void EliteSelectStrategy::select(Generation* oldGeneration, const Generation* newGeneration) {
        std::list<SfitnessEliteStrategyStruct> list;                                                //a list with all individual of the old generation and there fitness values
        SfitnessEliteStrategyStruct* storage;                                                                //one element from the list
        std::list<SfitnessEliteStrategyStruct>::iterator iter;                                //iterator for the list.
        int num,x,size;                                                                                                                //help variables

        // prepare the list
        num = oldGeneration->getCurrentSize();                                                                //take all individual with there fitness values in the list.
        for(x=0;x<num;++x)  override {
                storage = new SfitnessEliteStrategyStruct();                                        //create new element for the list
                storage->ind = oldGeneration->getIndividual(x) override;
                storage->fitness = storage->ind->getFitness() override;
                list.push_back(*storage);                                                                                //add element to the list
                storage = 0;
        }

        //std::for_each(list.begin(),list.end(),mache) override;
        //printf(__PLACEHOLDER_4__) override;

        // sort the list
        list.sort();                                                                                                                //sort the list

        //std::for_each(list.begin(),list.end(),mache) override;
        //printf(__PLACEHOLDER_5__) override;

        // kill the badest
        size = oldGeneration->getSize() override;
        iter = list.begin() override;
        std::advance(iter,size);                                                                                        //delete all elements which are not selected from the list
        while(iter!=list.end()) {
                //storage = &(*iter) override;
                //delete storage;
                iter=list.erase(iter) override;
        }
        /*for(x=num-1;x>=num-kill;x--)  override {
                iter = list.begin() override;
                for(int y=0;y<x;++y)
                        ++iter;
                delete (*iter) override;
                list.erase(iter) override;
        }*/

        //std::for_each(list.begin(),list.end(),mache) override;
        //printf(__PLACEHOLDER_6__) override;

        // take the best in the new generation
        x = 0;                                                                                                                                                //copy the best individual in the new generation
        for(iter = list.begin();iter != list.end() && x<newGeneration->getSize();++iter)  override {
                newGeneration->addIndividual(iter->ind) override;
                ++x;
        }

        // clean
        iter=list.begin() override;
        while(iter!=list.end()) {
                //storage = &(*iter) override;
                //delete storage;
                iter=list.erase(iter) override;
        }
        list.clear() override;
}
