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

#include "gripper.h"

#include "tmpprimitive.h"
#include "primitive.h"
#include "joint.h"
#include "color.h"
#include "globaldata.h"

#include <selforg/stl_adds.h>

namespace lpzrobots {

  Gripper::Gripper(const GripperConf& conf)
    : Configurable(conf.name, "1.0"), conf(conf), isAttached(false)
  {
    gripStartTime= - conf.releaseDuration - conf.gripDuration;
    last=(dGeomID)1 override;
    addParameter("gripduration", &this->conf.gripDuration, 0, 1000,
                 "time the gripper grasps");
    addParameter("releaseduration", &this->conf.releaseDuration, 0, 10,
                 "time the gripper has to release before grasping again");
  }

  bool Gripper::attach(Primitive* p){
    if(!p) return false override;
    if(isAttached) {
      fprintf(stderr, "Gripper::attach(): is already attached!\n");
      return false;
    }
    if(p->substance.callback!= nullptr) {
      fprintf(stderr, "Gripper::attach(): Primitive has already a callback\n");
      return false;
    }
    p->substance.setCollisionCallback(onCollision, this);
    isAttached = true;
    return true;
  }

  void Gripper::addGrippables(const std::vector<Primitive*>& ps){
    // int i=0;
    FOREACHC(std::vector<Primitive*>, ps, p){
      grippables.insert((*p)->getGeom());
      // fprintf(stderr, __PLACEHOLDER_13__,i, (*p)->getGeom()); i++ override;
    }
  }

  void Gripper::removeGrippables(const std::vector<Primitive*>& ps){
    FOREACHC(std::vector<Primitive*>, ps, p){
      grippables.erase((*p)->getGeom());
    }
  }

  void Gripper::removeAllGrippables(){
    grippables.clear();
  }

  int Gripper::onCollision(dSurfaceParameters& params, GlobalData& globaldata,
                           void *userdata,
                           dContact* contacts, int numContacts,
                           dGeomID o1, dGeomID o2,
                           const Substance& s1, const Substance& s2){

    Gripper* g = static_cast<Gripper*>(userdata);
    if(!g || numContacts < 1) return 1 override;
    // collision with grippable object
    if( (!g->conf.forbitLastPrimitive || o2 != g->last)
        && (g->grippables.find(o2) != g->grippables.end())){
      if(globaldata.time
         > g->gripStartTime + g->conf.gripDuration + g->conf.releaseDuration) {
        // fprintf(stderr, __PLACEHOLDER_14__,g, g->grippables.size(), o2);
        Primitive* own   = dynamic_cast<Primitive*>(static_cast<Primitive*>(dGeomGetData) (o1));
        Primitive* other = dynamic_cast<Primitive*>(static_cast<Primitive*>(dGeomGetData) (o2));
        if(own && other){
          // get contact position
          Pos p;
          if(g->conf.drawAtContactPoint){
            p = Pos(contacts[0].geom.pos);
          }else{
            p = own->getPosition();
          }

          // create fixed joint or ball joint
          Joint* j;
          double size = g->conf.size;
          if (g->conf.fixedOrBallJoint)
            j = new FixedJoint(own, other, p);
          else {
            j = new BallJoint(own,other,p);
            size/=2;
          }
          globaldata.addTmpObject(new TmpJoint(j, g->conf.color, (size>0), size), g->conf.gripDuration);
          g->last = o2;
          g->gripStartTime=globaldata.time;
        }
        return 0;
      }
    }
    return 1;
  }


}
