/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *    Guillaume de Chambrier <s0672742 at sms dot ed dot ac dot uk>        *
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
/*
This file concerns material implementation in the simulator.

Terminology:
Since __PLACEHOLDER_0__ is used for OpenGL stuff and also in OSG
 we should use something else for the physical material: substance

So how to implement substance to cover all collission cases and requirements.
It would be nice to keep the collisioncontrol as it is for compatibility reasons.
In general:

- every ode geom should get a user data pointer to the primitive
- primitive has a certain substance
- 2 substance define collision parameters
- also an optional callback function for user defined handling
- collission detection is done globally.
- via joint connected geoms are stored in a set (globally)
- spaces are globally registered

 ***************************************************************************/
#ifndef __SUBSTANCE_H
#define __SUBSTANCE_H

#include<ode-dbl/common.h>
#include<ode-dbl/contact.h>
#include<utils/axis.h>

namespace lpzrobots {

  //      Todo: maybe add bounce

  class GlobalData;
  
  /** Callback for collision handling.
     * Return 1 to handle the collision, 0 to ignore it.
  */
  typedef int (*CollisionCallback)(dSurfaceParameters& params, GlobalData& globaldata, void *userdata,
                                   dContact* contacts, int numContacts,
                                   dGeomID o1, dGeomID o2, const class Substance& s1, const class Substance& s2);

  /** Physical substance definition, used for collision detection/handling.

     The collision parameters are calculated as follows:
     \f[ a = \frac{(1-e_1)/kp_1 + (1-e_2)/kp_2}{1/kp_1 + 1/kp_2}
         = \frac{(1-e_1)kp_2 + (1-e_2)kp_1}{kp_1+kp_2}\f].

     Note that you cannot add any member variables to derived classes
      since they do not fit into the substance object in OdeHandle!
  */
  class Substance{
  public:
    Substance();
    Substance( float roughness, float slip, float hardness, float elasticity);

  public:
    float roughness = 0.8f;
    float slip = 0.01f;
    float hardness = 40.0f;
    float elasticity = 0.5f;

    void setCollisionCallback(CollisionCallback func, void* userdata);

    CollisionCallback callback;
    void* userdata = nullptr;

  public:
    /// Combination of two surfaces
    static void getSurfaceParams(dSurfaceParameters& sp, const Substance& s1, const Substance& s2, double stepsize);

    static void printSurfaceParams(const dSurfaceParameters& surfParams);

    //// Factory methods

    /// default substance is plastic with roughness=0.8
    static Substance getDefaultSubstance();
    void toDefaultSubstance();

    /// very hard and elastic with slip roughness [0.1-1]
    static Substance getMetal(float roughness);
    /// very hard and elastic with slip roughness [0.1-1]
    void toMetal(float roughness);

    /// high roughness, no slip, very elastic, hardness : [5-50]
    static Substance getRubber(float hardness);
    /// high roughness, no slip, very elastic, hardness : [5-50]
    void toRubber(float hardness);

    /// medium slip, a bit elastic, medium hardness, roughness [0.5-2]
    static Substance getPlastic(float roughness);
    /// medium slip, a bit elastic, medium hardness, roughness [0.5-2]
    void toPlastic(float roughness);

    /// large slip, not elastic, low hardness [1-30], high roughness
    static Substance getFoam(float _hardness);
    /// large slip, not elastic, low hardness [1-30], high roughness
    void toFoam(float _hardness);

    /** variable slip and roughness [0-1], not elastic, high hardness for solid snow
        slip = 1 <--> roughness=0.0, slip = 0 <--> roughnes=1.0 */
    static Substance getSnow(float _slip);
    /** variable slip and roughness, not elastic, high hardness for solid snow
        slip = 1 <--> roughness=0.0, slip = 0 <--> roughnes=1.0 */
    void toSnow(float _slip);

    /// @see toNoContact()
    static Substance getNoContact();
    /** set the collsion callback to ignores everything
        Usually it is better to use the __PLACEHOLDER_1__ from odeHandle but
        if this particular one substance should not collide with any other, this is easier.
        WARNING: this sets the collisionCallback. This will not convert to other
        substances without manually setting the callback to 0
     */
    void toNoContact();

    /** enables anisotrop friction.
        The friction along the given axis is ratio fold of the friction in the other directions.
        If ratio = 0.1 and axis=Axis(0,0,1) then the fiction along the z-axis
         is 1/10th of the normal friction.
        Useful  to mimic scales of snakes or the like.
        WARNING: this sets the collisionCallback!
        To disable the collisionCallback has to set to 0 manually
    */
    void toAnisotropFriction(double ratio, const Axis& axis);
  };


  class DebugSubstance : public Substance {
  public:
    DebugSubstance();
    DebugSubstance( float roughness, float slip, float hardness, float elasticity);
  protected:
    static int dbg_output(dSurfaceParameters& params, GlobalData& globaldata, void *userdata,
                      dContact* contacts, int numContacts,
                      dGeomID o1, dGeomID o2, const Substance& s1, const Substance& s2);
  };

}

#endif


