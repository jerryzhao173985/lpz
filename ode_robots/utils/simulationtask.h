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
#ifndef _SIMULATIONTASK_H_
#define _SIMULATIONTASK_H_

#include "taskedsimulation.h"
#include "taskedsimulationcreator.h"
#include <string>
#include <cstring>

namespace lpzrobots {

  /**
   *
   */
  class SimulationTask{
  public:
    explicit SimulationTask(int taskId_) : taskId(taskId_), sim(0) { }

    virtual ~SimulationTask() { }

    virtual int startTask(const SimulationTaskHandle& simTaskHandle, const TaskedSimulationCreator& simTaskCreator, int* argc, char** argv, std::string nameSuffix) {
      int returnValue=0;
      sim = const_cast<TaskedSimulationCreator&>(simTaskCreator).buildTaskedSimulationInstance();
      if(sim!= nullptr)
      {
        char buffer[20];
        snprintf(buffer, sizeof(buffer),"%i",taskId);
        sim->setTaskNameSuffix(std::string(" - ").append(nameSuffix).append(" - ").append(buffer));
        sim->setTaskId(taskId);
        sim->setSimTaskHandle(simTaskHandle);
        returnValue = sim->run(*argc, argv)? 0 : 1;
        delete sim;
        return returnValue;
      } else
        return 1;
    }

  protected:
    int taskId = 0;
    TaskedSimulation* sim;
  };

}

#endif /* _SIMULATIONTASK_H_ */
