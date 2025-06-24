/***************************************************************************
 *   Copyright static_cast<C>(2005)-2011 LpzRobots development team                    *
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

#include "abstractcontroller.h"

#include <_strings.h>
#include <iostream>
#include <string>

using namespace std;

void
AbstractController::sensorInfos(std::list<SensorMotorInfo> sensorInfos) {
  FOREACHIa(sensorInfos, sensor, index) {
    sensorIndexMap[sensor->name] = index;
    sensorInfoMap[index] = *sensor;
  }
}

void
AbstractController::motorInfos(std::list<SensorMotorInfo> motorInfos) {
  FOREACHIa(motorInfos, motor, index) {
    motorIndexMap[motor->name] = index;
    motorInfoMap[index] = *motor;
  }
}

int
AbstractController::SIdx(const std::string& name) {
  if (auto iter = sensorIndexMap.find(name); iter != sensorIndexMap.end()) {
    return iter->second;
  }

  cerr << "Cannot find Sensor with name \"" << name << "\""
       << " Possible values are:" << endl;
  for (const auto& [sensorName, sensorIndex] : sensorIndexMap) {
    cerr << sensorName << ", ";
  }
  cerr << endl;
  return 0;
}

int
AbstractController::MIdx(const std::string& name) {
  if (auto iter = motorIndexMap.find(name); iter != motorIndexMap.end()) {
    return iter->second;
  }

  cerr << "Cannot find Motor with name \"" << name << "\""
       << " Possible values are:" << endl;
  for (const auto& [motorName, motorIndex] : motorIndexMap) {
    cerr << motorName << ", ";
  }
  cerr << endl;
  return 0;
}

SensorMotorInfo
AbstractController::SInfo(int index) {
  if (auto iter = sensorInfoMap.find(index); iter != sensorInfoMap.end()) {
    return iter->second;
  }

  cerr << "No info for Sensor with index " << index << "! Out of bounds?" << endl;
  return SensorMotorInfo();
}

SensorMotorInfo
AbstractController::MInfo(int index) {
  if (auto iter = motorInfoMap.find(index); iter != motorInfoMap.end()) {
    return iter->second;
  }

  cerr << "No info for Motor with index " << index << "! Out of bounds?" << endl;
  return SensorMotorInfo();
}
