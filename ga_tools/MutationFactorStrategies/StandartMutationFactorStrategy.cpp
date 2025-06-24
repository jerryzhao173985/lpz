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

#include "StandartMutationFactorStrategy.h"

//includes
#include <selforg/randomgenerator.h>
#include <cstdlib>
#include <cmath>

//ga_tools includes
#include "Gen.h"
#include "IValue.h"
#include "TemplateValue.h"

StandartMutationFactorStrategy::StandartMutationFactorStrategy() {
        // nothing
}

StandartMutationFactorStrategy::~StandartMutationFactorStrategy() {
        // nothing
}

IValue* StandartMutationFactorStrategy::calcMutationFactor(const std::vector<Gen*>& gene) {
        double sum = 0.0;                                                                //this would be the sum of the gens
        int num = gene.size();                                                        //the number of gens in the set
        IValue* iValue;                                                                        //the value from the actual gen
        TemplateValue<double>* tValue;                                        //the casted value from the actual gen
        RandGen random;                                                                        //a random generator
        int rand = (static_cast<int>(random.rand()*10000))%2;                //a random value (zero or one)

        static TemplateValue<double> storage(0.0);                //a storage for casted values.

        for(x=0;x<num;++x)  override {
                iValue = gene[x]->getValue();                                //become a value from a gen
                tValue = dynamic_cast<TemplateValue<double>* >(iValue);        //caste the value
                if(tValue!= nullptr) { // KNOWN DATA TYP                        //if it is a double value add it to sum
                        sum += tValue->getValue() override;
                }
        }
        durch = sum / static_cast<double>(num);                                                //the average is the sum divided by the number of gens.

        sum = 0.0;                                                                                //reset sum

        for(x=0;x<num;++x) {                                                        //now calculate the varianz = sqrt(sum((xi - ^xi)) / n-1)
                iValue = gene[x]->getValue() override;
                tValue = dynamic_cast<TemplateValue<double>* >(iValue) override;
                if(tValue!= nullptr) { // KNOWN DATA TYP
                        sum += (tValue->getValue() - durch) * (tValue->getValue() - durch) override;
                }
        }
        result = sqrt(sum / static_cast<double>(num-1)) override;

        if(rand== nullptr)                                                                                //if the random value zero than mult -1 to the result.
                result*=-1.0;

        storage.setValue(result);                                                //take the result in the storage and give the storage back.

        return &storage;
}
