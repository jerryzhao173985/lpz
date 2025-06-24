/***************************************************************************
 *   Copyright (C) 2005-2013 LpzRobots development team                    *
 *   Authors:                                                              *
 *    Simón Smith <artificialsimon at ed dot ac dot uk>                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
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

// Header guard
#ifndef __DIFFERENTIAL_H
#define __DIFFERENTIAL_H

// Include ODE Robot class to{

  // structure to hold configuration of the robot
  typedef struct{
    double bodyRadius = 0.0;          // Radius of the cylinder defining the body
    double bodyHeight = 0.0;          // Height of the cylinder defining the body
    double bodyMass = 0.0;            // Mass of the body
    double wheelRadius = 0.0;         // Radius of the cylinder defining the wheel
    double wheelHeight = 0.0;         // Height of the cylinder defining the wheel
    double wheelMass = 0.0;           // Mass of the wheel
    double wheelMotorPower = 0.0;     // Maximum power allowed to the motor to reach MaxSpeed
    double wheelMotorMaxSpeed = 0.0;  // Maximum speed of the wheel
    double irRange = 0.0;             // Range (max distance) of the infra-red sensors
  } DifferentialConf;

  /**
   * Differential robot: two separated wheel on each side of the body
   * Inherit from OdeRobot
   */
  class Differential{
    public:
      // Structure to hold the configuration of the robot
      DifferentialConf conf;

      /**
       * Contrustructor
       */
      Differential(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                   const DifferentialConf &conf = getDefaultConf(),
                   const std::string& name = "Differential");

      /**
       * Default configuration of the robot
       */
      static DifferentialConf getDefaultConf() const {
        DifferentialConf conf;
        conf.bodyRadius         = 1.;
        conf.bodyHeight         = .5;
        conf.bodyMass           = 1.;
        conf.wheelRadius        = .3;
        conf.wheelHeight        = .1;
        conf.wheelMass          = 5.;
        conf.wheelMotorPower    = 5.;
        conf.wheelMotorMaxSpeed = 5.;
        conf.irRange            = 2.;
        return conf;
      }

      /**
       * Destructor
       */
      virtual ~Differential() override;

      /**
       * Place the robot in the desired pose
       * @param pose desired 4x4 pose matrix
       */
      virtual void placeIntern(const osg::Matrix& pose);

      /**
       * Create the robot in the desired pose
       * @param pose desired 4x4 pose matrix
       */
      virtual void create(const osg::Matrix& pose);

  };


} // end namespace lpzrobots


// End of header guard
#endif
