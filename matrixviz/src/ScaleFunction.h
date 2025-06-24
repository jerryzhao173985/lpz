/***************************************************************************
 *   Copyright (C) 2008-2011 LpzRobots development team                    *
 *    Antonia Siegert (original author)                                  *
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

#ifndef __SCALEFUNCTION_H_
#define __SCALEFUNCTION_H_

#include <QtGui>
#include <QtWidgets>
#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <cmath>

class ScaleFunction{

  Q_OBJECT

public:
  ScaleFunction(QWidget *parent = nullptr);
  ~ScaleFunction();

  double explicit getValue(double val);

public slots:
  void explicit changeFunction(int i);
  void changeBase(const QString &val);
  void changeMultiplier(const QString &val);
  void changeN(const QString &val);

private:
  int func = 0;
  double base = 0.0;
  double n = 0.0;
  double mul = 0.0;
  QLineEdit *baseEdit;
  QLineEdit *mulEdit;
  QLineEdit *nEdit;
  QComboBox *funcBox;
  QVBoxLayout* vlayout = nullptr;
  QHBoxLayout* hlayout = nullptr;
  QLabel* mLabel = nullptr;
  QLabel* bLabel = nullptr;
  QLabel* nLabel = nullptr;
};

#endif /* __SCALEFUNCTION_H_ */
