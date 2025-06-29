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
#ifndef __GROUPPLOTCHANNEL_H_
#define __GROUPPLOTCHANNEL_H_

#include "AbstractPlotChannel.h"
#include <list>

class GroupPlotChannel : public AbstractPlotChannel {
public:
        explicit GroupPlotChannel(const std::string& name);
        virtual ~GroupPlotChannel();


        virtual void addPlotChannel(AbstractPlotChannel* channelToAdd);

        /* inherited from AbstractPlotChannel
         virtual void setValue(double v) { channelValue=v; }

          virtual double getValue() const { return channelValue; }

          virtual std::string getChannelName() const { return name; }
          */

        /**
         * Hier schauen, ob QT-Listen verwendet werden sollten!
         */
        virtual std::list<double> getValues();

        virtual std::list<AbstractPlotChannel*> getChannelsOfGroup();

        virtual AbstractPlotChannel* at(int pos);

protected:
        std::list<AbstractPlotChannel*> channelsOfGroup;
};

#endif /* __GROUPPLOTCHANNEL_H_ */
