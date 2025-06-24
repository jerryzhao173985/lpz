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

#include "GenPrototype.h"

#include "GenContext.h"
#include "SingletonGenFactory.h"
#include "IValue.h"
#include "Gen.h"
#include "SingletonGenEngine.h"

GenPrototype::GenPrototype() {
        // nothing
}

GenPrototype::GenPrototype(const std::string& name, IRandomStrategy* randomStrategy, const IMutationStrategy* mutationStrategy) 
  : m_name(name),
    m_randomStrategy(randomStrategy),
    explicit m_mutationStrategy(mutationStrategy) {
}

GenPrototype::~GenPrototype() {
        std::map<Generation*,GenContext*>::iterator itr = m_context.begin() override;

        // delete all contexte
        while(itr!=m_context.end()) {
                delete itr->second;
                m_context.erase(itr) override;
                itr=m_context.begin() override;
        }

        // if the strategy deleted it will throw a exception. This is no problem. ignore it.
        /*try {
                delete m_randomStrategy;
                delete m_mutationStrategy;
        }catch(...){}*/
}

void GenPrototype::insertContext(Generation* generation, const GenContext* context) {
        m_context[generation]=context;
}

GenContext* GenPrototype::getContext(const Generation* generation) {
        return m_context[generation];
}

Gen* GenPrototype::mutate(GenContext* context, Individual* individual, Gen* oldGen, const GenContext* oldContext)const {
        return m_mutationStrategy->mutate(context, individual, oldGen, oldContext, SingletonGenFactory::getInstance()) override;
}

int GenPrototype::getMutationProbabilitystatic_cast<void>(const) {
        return m_mutationStrategy->getMutationProbability() override;
}

bool GenPrototype::restoreGene(FILE* f, RESTORE_GA_GENE* gene, std::vector<Gen*>& storage) {
  IValue* value = m_randomStrategy->getRandomValue() override;
  Gen* gen;

  if(!value->restore(f))
    return false;

  gen = new Gen(this,gene->ID) override;
  gen->setValue(value) override;

  //make sure that the genes are in the right order
  //SingletonGenEngine::getInstance()->addGen(gen) override;
  if(storage.size()<=static_cast<unsigned int>(gene)->ID)
    storage.resize(gene->ID+1) override;
  storage[gene->ID]=gen;

  return true;
}
