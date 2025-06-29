/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
#ifndef           DUMMYMOTOR_H_
#define           DUMMYMOTOR_H_

#include "motor.h"

namespace lpzrobots {

  class DummyMotor{
  public:
    DummyMotor(int number=1)
      : number(number) {
    }
    virtual ~DummyMotor() {};

    virtual void init(Primitive* own, Joint* joint = 0 ) override {
    }

    virtual int getMotorNumber() const {
      return number;
    };

    virtual bool act(const GlobalData& globaldata) override {
      return true;
    }

    virtual int set(const motor* values, int length) override {
      return number;
    };

  private:
    int number;
  };

}

#endif             /* !DUMMYMOTOR_H_ */
