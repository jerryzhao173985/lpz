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
#include <algorithm>
#include <selforg/statistictools.h>
#include "TemplateValue.h"
#include "Gen.h"
#include "GenPrototype.h"
#include "SingletonGenEngine.h"
#include "Generation.h"
#include "Individual.h"

GenContext::GenContext() :
  m_prototype(nullptr),
  m_min(0.0),
  m_w1(0.0),
  m_q1(0.0),
  m_med(0.0),
  m_avg(0.0),
  m_q3(0.0),
  m_w3(0.0),
  m_max(0.0) {
  // nothing
}

GenContext::GenContext(const GenPrototype* prototype) :
  m_prototype(const_cast<GenPrototype*>(prototype)),
  m_min(0.0),
  m_w1(0.0),
  m_q1(0.0),
  m_med(0.0),
  m_avg(0.0),
  m_q3(0.0),
  m_w3(0.0),
  m_max(0.0) {

  std::string name = prototype->getName();

  // Since GenContext doesn't inherit from Inspectable, we can't use addInspectableValue
  // We would need to inherit from Inspectable to use these methods
  // For now, commenting out these calls
  // addInspectableValue(name + "MIN", &m_min);
  // addInspectableValue(name + "W1", &m_w1);
  // addInspectableValue(name + "Q1", &m_q1);
  // addInspectableValue(name + "MED", &m_med);
  // addInspectableValue(name + "AVG", &m_avg);
  // addInspectableValue(name + "Q3", &m_q3);
  // addInspectableValue(name + "W3", &m_w3);
  // addInspectableValue(name + "MAX", &m_max);
}

GenContext::~GenContext() {
  m_storage.clear();
}

void GenContext::update(double factor) {
  std::vector<double> list;
  TemplateValue<double>* tValue;

  for (std::vector<Gen*>::const_iterator iter = m_storage.begin(); iter != m_storage.end(); ++iter) {
    tValue = dynamic_cast<TemplateValue<double>*> ((*iter)->getValue());
    if (tValue != nullptr)
      list.push_back(tValue->getValue());
  }
  // Calculate statistics directly instead of using DOUBLE_ANALYSATION_CONTEXT
  if (!list.empty()) {
    std::sort(list.begin(), list.end());
    
    // Min and max
    m_min = list.front();
    m_max = list.back();
    
    // Average
    double sum = 0.0;
    for (double val : list) {
      sum += val;
    }
    m_avg = sum / list.size();
    
    // Median
    size_t n = list.size();
    if (n % 2 == 0) {
      m_med = (list[n/2 - 1] + list[n/2]) / 2.0;
    } else {
      m_med = list[n/2];
    }
    
    // Quartiles
    if (n >= 4) {
      size_t q1_idx = n / 4;
      size_t q3_idx = 3 * n / 4;
      m_q1 = list[q1_idx];
      m_q3 = list[q3_idx];
      
      // Whiskers (1.5 * IQR)
      double iqr = m_q3 - m_q1;
      m_w1 = m_q1 - factor * iqr;
      m_w3 = m_q3 + factor * iqr;
      
      // Clamp whiskers to actual data range
      if (m_w1 < m_min) m_w1 = m_min;
      if (m_w3 > m_max) m_w3 = m_max;
    } else {
      m_q1 = m_min;
      m_q3 = m_max;
      m_w1 = m_min;
      m_w3 = m_max;
    }
  }
}

bool GenContext::restore() {
  int numGeneration = SingletonGenEngine::getInstance()->getActualGenerationNumber();
  int x, y, z, v;
  const std::vector<GenPrototype*>& prototypeSet = SingletonGenEngine::getInstance()->getSetOfGenPrototyps();
  int numPrototypes = prototypeSet.size();
  Generation* generation;
  GenPrototype* prototype;
  GenContext* context;
  Gen* gen;
  Individual* individual;

  generation = SingletonGenEngine::getInstance()->getGeneration(0);
  int numIndividuals = generation->getCurrentSize();
  for (y = 0; y < numPrototypes; ++y) {
    prototype = prototypeSet[y];
    context = new GenContext(prototype);
    for (z = 0; z < numIndividuals; ++z) {
      individual = generation->getIndividual(z);
      for (v = 0; v < numPrototypes; ++v) {
        gen = individual->getGen(v);
        if (gen->getPrototype() == prototype)
          break;
      }
      context->addGen(gen);
    }
    context->update();
    prototype->insertContext(generation, context);
  }

  for (x = 0; x < numGeneration; ++x) {
    generation = SingletonGenEngine::getInstance()->getGeneration(x + 1);
    numIndividuals = generation->getCurrentSize();
    for (y = 0; y < numPrototypes; ++y) {
      prototype = prototypeSet[y];
      context = new GenContext(prototype);
      for (z = 0; z < numIndividuals; ++z) {
        individual = generation->getIndividual(z);
        for (v = 0; v < numPrototypes; ++v) {
          gen = individual->getGen(v);
          if (gen->getPrototype() == prototype)
            break;
        }
        context->addGen(gen);
      }
      context->update();
      prototype->insertContext(generation, context);
    }
  }

  return true;
}
