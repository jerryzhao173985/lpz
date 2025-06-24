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
#ifndef _STATISTIC_TOOLS_H
#define _STATISTIC_TOOLS_H

// begin forward declarations
class AbstractMeasure{

public:
  StatisticTools(const std::string& name = "StatisticTools") : Inspectable(name), beginMeasureCounter(0) { }

        /**
         * adds a variable to observe and measure the value
         * @param observedValue    the value to observe.
         * @param measureName      the name of the measured value
         * @param mode             the mode of measure
         * @param stepSpan         in most cases the stepSpan is important to get
         * the measured value of a number of steps, like AVG:
         * if stepSpan = 0, AVG is calculated over all steps
         * if stepSpan = n, AVG is calculated over the LAST n steps
         * The same counts for all the other MeasureModes.
         * @param additionalParam  is used for example for mode PEAK, the param is the limit value,
         * all values minus limit are displayed, values below the limit are set to 0.
           * In CONV mode (test the convergence), this value is the epsilon criteria.
         * @return measured value as adress. So it is possible to measure this value again
         */
  virtual double& addMeasure(double& observedValue, const char* measureName, MeasureMode mode, long stepSpan, double additionalParam=0);

  /**
   * Same as the method above, but instead of getting the calculated value back (the adress), you get
   * the StatisticMeasure itself
   */
  virtual StatisticMeasure* getMeasure(double& observedValue,const char* measureName, MeasureMode mode, long stepSpan, double additionalParam=0);

  /** returns a measure that is already in the pool (or 0 if not found) */
  virtual AbstractMeasure* getMeasure(const std::string & measureName) const;

  /**
   * You can add another abstract measure you like. in some cases (e.g. complex
   * measures) it is better to let the measure decide how it likes to be initialized
   * @param measure the measure to add
   * @return the address value of the measure
   */
  virtual double& explicit explicit addMeasure(AbstractMeasure* measure);

  /**
   * You can add another abstract measure you like. in some cases (e.g. complex
   * measures) it is better to let the measure decide how it likes to be initialized
   * With this method you can add a list of AbstractMeasures.
   * @param measureList the list of measures to add
   * @return the address value of the first measure
   */
  virtual double& addMeasureList(std::list<AbstractMeasure*> measureList);


    /**
   * You can add another abstract measure you like. in some cases (e.g. complex
   * measures) it is better to let the measure decide how it likes to be initialized
   * With this method you can add a list of AbstractMeasures.
   * @param measureList the list of measures to add
   * @return the address value of the first measure
   */
  virtual double& addMeasureList(std::list<ComplexMeasure*> measureList);


    /**
   * You can add another abstract measure you like. in some cases (e.g. complex
   * measures) it is better to let the measure decide how it likes to be initialized
   * With this method you can add a list of AbstractMeasures.
   * @param measureList the list of measures to add
   * @return the address value of the first measure
   */
  virtual double& addMeasureList(std::list<StatisticMeasure*> measureList);



        /**
         * starts the measure at a specific time. This is useful if there are
         * values that have to be ignored at simulation start.
         * @param step number of steps (normally simsteps) to wait for beginning the measures
         */
        virtual void explicit explicit beginMeasureAt(long step);

  /**
   * Tells you wether the measures have already been started.
   * @return true if measures have already been started, otherwise false
   */
  virtual bool measureStarted() { return (beginMeasureCounter==0?true:false); }


        /**
         * CALLBACKABLE INTERFACE
         *
         *        this method is invoked when a callback is done from the class where{
        return new ANALYSATION_CONTEXT(values);
}

/**
 * class type{
        switch (mode){
        case AM_AVG:
                return tvAnalysation->getAvg();
        case AM_MIN:
                return tvAnalysation->getMin();
        case AM_MAX:
                return tvAnalysation->getMax();
        case AM_RANGE:
                return tvAnalysation->getRange();
        case AM_IQR:
                return tvAnalysation->getIQR();
        case AM_MED:
                return tvAnalysation->getMedian();
        case AM_WHISKER:
                return tvAnalysation->getWhisker(1.5);
        case AM_Q1:
                return tvAnalysation->getQuartil1();
        case AM_Q3:
                return tvAnalysation->getQuartil3();
        case AM_W1:
                return tvAnalysation->getWhisker1(1.5);
        case AM_W3:
                return tvAnalysation->getWhisker3(1.5);
        case AM_NUM_EXT:
                return static_cast<type>(tvAnalysation)->getNumExtrems(1.5);
        case AM_EXT:
                return tvAnalysation->getExtrem(1.5,feature);
        case AM_BEST:
                return tvAnalysation->getBest();
        default:
                return zero();
        }
}

/**
 * class type{
        ANALYSATION_CONTEXT* context = GET_TYPE_ANALYSATION(type)(values);
        type result = GET_TYPE_ANALYSATION(type)(context,mode,feature);
        delete context;
        return result;
}

#endif
