/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    guettler@informatik.uni-leipzig.de                                   *
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
 ***************************************************************************
 *                                                                         *
 *  DESCRIPTION                                                            *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 *  $Log$
 *  Revision 1.1  2009-04-17 14:17:33  guettler
 *  New PlotChannels and Filters for matrices
 *										   *
 *                                                                         *
 **************************************************************************/
/*
 * GroupPlotChannel.h
 *
 *  Created on: 17.04.2009
 *      Author: guettler
 */

#ifndef GROUPPLOTCHANNEL_H_
#define GROUPPLOTCHANNEL_H_

/*
 *
 */
#include "AbstractPlotChannel.h"

class GroupPlotChannel: public AbstractPlotChannel {
public:
	GroupPlotChannel();
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

protected:
	std::list<AbstractPlotChannel*> channelsOfGroup;
};

#endif /* GROUPPLOTCHANNEL_H_ */
