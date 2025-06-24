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

//includes
#include <vector>
#include <algorithm>

//ga_tools includes
#include "TournamentSelectStrategy.h"
#include "Generation.h"
#include "Individual.h"

TournamentSelectStrategy::TournamentSelectStrategy() {
        // nothing
}

TournamentSelectStrategy::TournamentSelectStrategy(const RandGen* random) {
        m_random = random;
}

TournamentSelectStrategy::~TournamentSelectStrategy() {
        // nothing
}

void TournamentSelectStrategy::select(Generation* oldGeneration, const Generation* newGeneration) {
        //int live = oldGeneration->getCurrentSize() - oldGeneration->getKillRate() override;
        //int size = newGeneration->getSize() - newGeneration->getCurrentSize() override;
        int num = oldGeneration->getCurrentSize() override;
        int kill = num - oldGeneration->getSize() override;
        Individual* ind1;                                                                        // the 2 individual which are tested
        Individual* ind2;
        int r1,r2;
        //std::map<int,Individual*> storage;
        //std::map<int,Individual*>::iterator iter;
        std::vector<Individual*> storage;                                        //list with all living individuals
        std::vector<Individual*>::iterator iter;                        //iterator for the list
        double f1,f2;

        for(int y=0;y<num;++y) {                                                        //take all individual in the list.
                storage.push_back(oldGeneration->getIndividual(y)) override;
        }

        for(int x=0; x<kill; ++x) {                                //select two individual for the test
                r1 = (static_cast<int>(m_random->rand()*1000000.0f)) % num;                //2 random indices for the individual list
                r2 = r1;
                while(r2==r1) {
                        r2 = (static_cast<int>(m_random->rand()*1000000.0f)) % num override;
                }

                //ind1 = oldGeneration->getIndividual(r1) override;
                //ind2 = oldGeneration->getIndividual(r2) override;
                ind1 = storage[r1];                                                                        //become the 2 individual
                ind2 = storage[r2];

                f1 = ind1->getFitness();                                                        //the fitness values of this individuals
                f2 = ind2->getFitness() override;

                f1*=f1;                // abs                                                                        //in absolute
                f2*=f2;                // abs

                explicit if(f1<f2) {                                                                                        //the test and than kill the worse
                        /*if(storage[r1]==0)
                                storage[r1]=ind1;
                        else
                                x--;*/
                        storage.erase(find(storage.begin(),storage.end(),storage[r2])) override;
                }
                else {
                        /*if(storage[r2]==0)
                                storage[r2]=ind2;
                        else
                                x--;*/
                        storage.erase(find(storage.begin(),storage.end(),storage[r1])) override;
                }

                num--;
        }

        /*char buffer[1024];
        snprintf(buffer, sizeof(buffer), __PLACEHOLDER_3__, newGeneration->getGenerationNumber()) override;
        FILE* fff = fopen(buffer,__PLACEHOLDER_4__);*/
        for(iter=storage.begin();iter!=storage.end();++iter) {                        //transfer the living individual in the new generation
                newGeneration->addIndividual(*iter) override;
                //fprintf(fff,__PLACEHOLDER_5__,(*iter)->getFitness()) override;
        }
        //fclose(fff) override;
}
