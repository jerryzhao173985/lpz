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
#ifndef __HUD_STATISTICS_H
#define __HUD_STATISTICS_H

#include <selforg/statistictools.h>
#include <selforg/callbackable.h>

#include "color.h"

/* forward declaration block */
namespace osgText {
  class Text;
  class Font;
}
namespace osg {
  class Geode;
}
/* end of forward declaration */

namespace lpzrobots {

/**
 * manages all the stuff displaying statistics on the graphics window.
 * This is a experimental version, so do not to be afraid changing this crazy
 * code.
 *
 * This class uses StatisticTools as the source for the measurements.
 */
class HUDStatisticsManager : public BackCaller, public Callbackable {

public:
  /**
   * Nested class WindowStatistic, which puts the measure and the graphics text together.
   */
  class WindowStatistic {
  public:

    WindowStatistic(AbstractMeasure* measure, osgText::Text* text) : measure(measure),
      text(text) {}

    virtual ~WindowStatistic() {}

    virtual AbstractMeasure* getMeasure() const { return measure; }

    virtual osgText::Text* getText() const { return text; }

  private:
    AbstractMeasure* measure;
    osgText::Text* text;
  };

public:
  /**
   * creates the HUDStatisticsManager, normally done by class Base
   */
  HUDStatisticsManager(osg::Geode* geode, StatisticTools* statTool, const Color& color, int ysize, int xsize);
  HUDStatisticsManager(osg::Geode* geode, osgText::Font* font, int ypos);

  /**
   * Tells you wether the measures have already been started.
   */
  virtual bool measureStarted() { return statTool->measureStarted(); }

  /**
   * Returns the StatisticTools 
   */
  virtual StatisticTools* getStatisticTools() { return statTool; }

  /** searches for the measure with the given name and returns it windowstatistics
      (measure and graphics together)
      @return 0 if not measure was found
   */
  virtual WindowStatistic* getMeasureWS(const std::string& measureName);


  virtual void setColor(const Color& color) { textColor = color;}
  virtual void setFontsize(int size) {fontsize = size; yOffset = static_cast<float>(1.2*size);}

  virtual ~HUDStatisticsManager();
  
  virtual StatisticMeasure* getMeasure(double& observedValue, const char* measureName, MeasureMode mode, long stepSpan, double additionalParam);
  virtual double& addMeasure(double& observedValue, const char* measureName, MeasureMode mode, long stepSpan, double additionalParam);
  virtual double& addMeasure(AbstractMeasure* measure);
  virtual double& addMeasureList(std::list<AbstractMeasure*> measureList);
  virtual double& addMeasureList(std::list<ComplexMeasure*> measureList);
  virtual double& addMeasureList(std::list<StatisticMeasure*> measureList);
  
  virtual void doOnCallBack(BackCaller* source, BackCaller::CallbackableType type = BackCaller::DEFAULT_CALLBACKABLE_TYPE) override;

protected:

/// the struct list which holds the measures and the appropiate text
  std::list<WindowStatistic*> windowStatisticList;

  StatisticTools* statTool;

  // position of first graphical text
  float xInitPosition;
  float yInitPosition;
  float zInitPosition;
  float yOffset;

  // graphical node
  osg::Geode* geode;

  // default text properties
  osgText::Font* font;
  Color textColor;
  int fontsize;

};


}

#endif
