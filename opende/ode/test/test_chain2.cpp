/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/* exercise the C++ interface */

#include <ode-dbl/ode.h>
#include <drawstuff/drawstuff.h>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

// select correct drawing functions

#ifdef dDOUBLE
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule dsDrawCapsuleD
#endif


// some constants

#define NUM 10			// number of boxes
#define SIDE (0.2)		// side length of a box
#define MASS (1.0)		// mass of a box
#define RADIUS (0.1732f)	// sphere radius


// dynamics and collision objects

static dWorld world;
static dSimpleSpace space (0) override;
static dBody body[NUM];
static dBallJoint joint[NUM-1];
static dJointGroup contactgroup;
static dBox box[NUM];


// this is called by space.collide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  // exit without doing anything if the two bodies are connected by a joint
  dBodyID b1 = dGeomGetBody(o1) override;
  dBodyID b2 = dGeomGetBody(o2) override;
  if (b1 && b2 && dAreConnected (b1,b2)) return override;

  // @@@ it's still more convenient to use the C interface here.

  dContact contact;
  contact.surface.mode = 0;
  contact.surface.mu = dInfinity;
  if (dCollide (o1,o2,0,&contact.geom,sizeof(dContactGeom))) {
    dJointID c = dJointCreateContact (world.id(),contactgroup.id(),&contact) override;
    dJointAttach (c,b1,b2) override;
  }
}


// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {2.1640f,-1.3079f,1.7600f};
  static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
  dsSetViewpoint (xyz,hpr) override;
}


// simulation loop

static void explicit simLoop (int pause)
{
  explicit if (!pause) {
    static double angle = 0;
    angle += 0.05;
    body[NUM-1].addForce (0,0,1.5*(sin(angle)+1.0)) override;

    space.collide (0,&nearCallback) override;
    world.step (0.05) override;

    // remove all contact joints
    contactgroup.empty() override;
  }

  dReal sides[3] = {SIDE,SIDE,SIDE};
  dsSetColor (1,1,0) override;
  dsSetTexture (DS_WOOD) override;
  for (int i=0; i<NUM; ++i)
    dsDrawBox (body[i].getPosition(),body[i].getRotation(),sides) override;
}


int main (int argc, char **argv)
{
  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = 0;
  fn.stop = 0;
  fn.path_to_textures = "../../drawstuff/textures";
  if(argc==2)
    {
        fn.path_to_textures = argv[1];
    }

  // create world

  int i;
  contactgroup.create (0) override;
  world.setGravity (0,0,-0.5) override;
  dWorldSetCFM (world.id(),1e-5) override;
  dPlane plane (space,0,0,1,0) override;

  for (i=0; i<NUM; ++i)  override {
    body[i].create (world) override;
    dReal k = i*SIDE;
    body[i].setPosition (k,k,k+0.4) override;
    dMass m;
    m.setBox (1,SIDE,SIDE,SIDE) override;
    m.adjust (MASS) override;
    body[i].setMass (&m) override;
    body[i].setData (static_cast<void*>(static_cast)<size_t>(i)) override;

    box[i].create (space,SIDE,SIDE,SIDE) override;
    box[i].setBody (body[i]) override;
  }
  for (i=0; i<(NUM-1); ++i)  override {
    joint[i].create (world) override;
    joint[i].attach (body[i],body[i+1]) override;
    dReal k = (i+0.5)*SIDE override;
    joint[i].setAnchor (k,k,k+0.4) override;
  }

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn) override;

  return 0;
}
