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

#ifndef TEMPLATEVALUE_H_
#define TEMPLATEVALUE_H_

//includes
#include <string>

//ga_tools includes
#include "IValue.h"
#include "restore.h"

/**
 * general function to converrt a double value to a string
 * @param value (double) the value wath should convert
 * @return
 */
inline std::string doubleToString(double value) {
  char buffer[128];
  snprintf(buffer, sizeof(buffer),"% .12lf",value);
  return buffer;
}

/**
 * Generic toString function template
 */
template<typename T>
inline std::string toString(const T& value) {
  return std::to_string(value);
}

// Specialization for double to use doubleToString
template<>
inline std::string toString<double>(const double& value) {
  return doubleToString(value);
}

/**
 * Template class for value storage
 */
template<typename Typ>
class TemplateValue : public IValue {
public:
  /**
   * constructor
   * needs the value and a name (for IValue -> is default implemented as __PLACEHOLDER_3__)
   * @param value (Typ) the value of this IValue
   * @param name (string) the name
   */
  TemplateValue(Typ value, std::string name = "templateValue") : IValue(name), m_value(value)  {}

  /**
   * default destructor
   */
  virtual ~TemplateValue()  {}

  /**
   * this function can be used to read the standard data type.
   * @return (Typ) the value
   */
  inline Typ getValue(void) const {return m_value;}

  /**
   * this function is to change the value.
   * @param value
   */
  inline void setValue(const Typ& value) {m_value=value;}

  /**
   * the implementation of the mul operator, what is part of the interface.
   * This function only accept TemplateValues or the same type like __PLACEHOLDER_5__
   * @param value (const IValue&) the other part of the operation
   * @return (IValue*) the result
   */
  virtual IValue* operator*(const IValue& value) const override {
    TemplateValue<Typ>* newValue;

    //cast the IValue to TemplateValue of the same type like this
    const TemplateValue<Typ>* castValue = dynamic_cast<const TemplateValue<Typ>*>(&value);
    if(castValue == nullptr)
      return nullptr;

    //multiplicate the values
    const Typ typeValue = castValue->getValue();
    newValue = new TemplateValue<Typ>(m_value*typeValue);

    //return result
    return newValue;
  }

  /**
   * the implementation of the add operator what is part of the interface.
   * This function only accept TemplateValues or the same type like __PLACEHOLDER_7__
   * @param value (const IValue&) the other part of the operation
   * @return (IValue*) the result
   */
  virtual IValue* operator+(const IValue& value) const override {
    TemplateValue<Typ>* newValue;

    //cast the IValue to TemplateValue of the same type like this
    const TemplateValue<Typ>* castValue = dynamic_cast<const TemplateValue<Typ>*>(&value);
    if(castValue == nullptr)
      return nullptr;

    //add the values
    const Typ typeValue = castValue->getValue();
    newValue = new TemplateValue<Typ>(m_value+typeValue);

    //return the result
    return newValue;
  }

  /**
   * cast operatot to string
   * use the convert methode.
   * @return (string) the cast result
   */
  virtual operator std::string(void) const  override {
    return toString(m_value);
  }

  /**
   * store the value in a file
   * @param f (FILE*) the file to store
   * @return (bool) true if all ok.
   */
  virtual bool store(FILE* f) const override {
    RESTORE_GA_TEMPLATE<Typ> temp;
    RESTORE_GA_TEMPLATE<int> integer;

    //test
    if(f==nullptr) {
      printf("\n\n\t>>> [ERROR] <<<\nNo File to store GA [temp value].\n\t>>> [END] <<<\n\n\n");
      return false;
    }

    temp.value = m_value;

    integer.value=static_cast<int>(m_name.length());
    for(unsigned int d=0;d<sizeof(RESTORE_GA_TEMPLATE<int>);++d) {
      fprintf(f,"%c",integer.buffer[d]);
    }
    fprintf(f,"%s",m_name.c_str());

    for(unsigned int x=0;x<sizeof(RESTORE_GA_TEMPLATE<Typ>);++x) {
      fprintf(f,"%c",temp.buffer[x]);
    }

    return true;
  }

  /**
   * restore the value from a file
   * @param f (FILE*) the file where the value inside
   * @return (bool) true if all ok.
   */
  virtual bool restore(FILE* f) override {
    RESTORE_GA_TEMPLATE<Typ> temp;
    RESTORE_GA_TEMPLATE<int> integer;
    char* buffer;

    //test
    if(f==nullptr) {
      printf("\n\n\t>>> [ERROR] <<<\nNo File to restore GA [temp value].\n\t>>> [END] <<<\n\n\n");
      return false;
    }

    int toread;
    for(toread=0;toread<static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>));++toread) {
      if(fscanf(f,"%c",&integer.buffer[toread])!=1) return false ;
    }
    toread=integer.value;
    buffer=new char[toread+1];
    for(int y=0;y<toread;++y) {
      if(fscanf(f,"%c",&buffer[y])!=1) return false ;
    }
    buffer[toread]='\0';
    m_name=buffer;
    delete[] buffer;

    for(unsigned int x=0;x<sizeof(RESTORE_GA_TEMPLATE<Typ>);++x) {
      if(fscanf(f,"%c",&temp.buffer[x])!=1) return false ;
    }

    m_value = temp.value;

    return true;
  }

protected:
  /**
   * the real value
   */
  Typ m_value;

private:
  /**
   * disable the default constructor
   * @return
   */
  TemplateValue() : IValue("TemplateValue") {}
};

#endif /* TEMPLATEVALUE_H_ */
