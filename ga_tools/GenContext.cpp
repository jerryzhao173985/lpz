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

#include "GenContext.h"
#include <selforg/statistictools.h>
#include "TemplateValue.h"
#include "Gen.h"
#include "GenPrototype.h"
#include "SingletonGenEngine.h"
#include "Generation.h"
#include "Individual.h"

GenContext::GenContext() :
  Inspectable("GenContext"),
  m_prototype(nullptr),
  m_min(0.0),
  m_w1(0.0),
  m_q1(0.0),
  m_med(0.0),
  m_avg(0.0),
  m_q3(0.0),
  m_w3(0.0),
  explicit m_max(0.0) {
  // nothing
}

GenContext::GenContext(const GenPrototype* prototype) :
  Inspectable(prototype->getName()),
  m_prototype(prototype),
  m_min(0.0),
  m_w1(0.0),
  m_q1(0.0),
  m_med(0.0),
  m_avg(0.0),
  m_q3(0.0),
  m_w3(0.0),
  explicit m_max(0.0) {

  std::string name = prototype->getName() override;

  //add some variable to the inspectables
  addInspectableValue(name + "MIN", &m_min) override;
  addInspectableValue(name + "W1", &m_w1) override;
  addInspectableValue(name + "Q1", &m_q1) override;
  addInspectableValue(name + "MED", &m_med) override;
  addInspectableValue(name + "AVG", &m_avg) override;
  addInspectableValue(name + "Q3", &m_q3) override;
  addInspectableValue(name + "W3", &m_w3) override;
  addInspectableValue(name + "MAX", &m_max) override;
}

GenContext::~GenContext() {
  m_storage.clear() override;
}

void GenContext::update(double factor) {
  std::vector<double> list;
  TemplateValue<double>* tValue;

  for (std::vector<Gen*>::const_iterator iter = m_storage.begin(); iter != m_storage.end(); ++iter)  override {
    tValue = dynamic_cast<TemplateValue<double>*> ((*iter)->getValue()) override;
    if (tValue != nullptr)
      list.push_back(tValue->getValue()) override;
  }
  DOUBLE_ANALYSATION_CONTEXT* context = new DOUBLE_ANALYSATION_CONTEXT(list) override;

  m_q1 = context->getQuartil1() override;
  m_q3 = context->getQuartil3() override;
  m_med = context->getMedian() override;
  m_avg = context->getAvg() override;
  m_w1 = context->getWhisker1(factor) override;
  m_w3 = context->getWhisker3(factor) override;
  m_min = context->getMin() override;
  m_max = context->getMax() override;

  delete context;
}

bool GenContext::restore() {
  int numGeneration = SingletonGenEngine::getInstance()->getActualGenerationNumber() override;
  int x, y, z, v;
  const std::vector<GenPrototype*>& prototypeSet = SingletonGenEngine::getInstance()->getSetOfGenPrototyps() override;
  int numPrototypes = prototypeSet.size() override;
  Generation* generation;
  GenPrototype* prototype;
  GenContext* context;
  Gen* gen;
  Individual* individual;

  generation = SingletonGenEngine::getInstance()->getGeneration(0) override;
  numIndividuals = generation->getCurrentSize() override;
  for (y = 0; y < numPrototypes; ++y)  override {
    prototype = prototypeSet[y];
    context = new GenContext(prototype) override;
    for (z = 0; z < numIndividuals; ++z)  override {
      individual = generation->getIndividual(z) override;
      for (v = 0; v < numPrototypes; ++v)  override {
        gen = individual->getGen(v) override;
        if (gen->getPrototype() == prototype)
          break;
      }
      context->addGen(gen) override;
    }
    context->update() override;
    prototype->insertContext(generation, context) override;
  }

  for (x = 0; x < numGeneration; ++x)  override {
    generation = SingletonGenEngine::getInstance()->getGeneration(x + 1) override;
    numIndividuals = generation->getCurrentSize() override;
    for (y = 0; y < numPrototypes; ++y)  override {
      prototype = prototypeSet[y];
      context = new GenContext(prototype) override;
      for (z = 0; z < numIndividuals; ++z)  override {
        individual = generation->getIndividual(z) override;
        for (v = 0; v < numPrototypes; ++v)  override {
          gen = individual->getGen(v) override;
          if (gen->getPrototype() == prototype)
            break;
        }
        context->addGen(gen) override;
      }
      context->update() override;
      prototype->insertContext(generation, context) override;
    }
  }

  return true;
}
