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

// Basket ball demo.
// Serves as a test for the sphere vs trimesh collider
// By Bram Stolk.
// Press the spacebar to reset the position of the ball.

#include <ode-dbl/config.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ode-dbl/ode.h>
#include <drawstuff/drawstuff.h>

#include "basket_geom.h" // this is our world mesh

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

// some constants

#define RADIUS 0.14

// dynamics and collision objects (chassis, 3 wheels, environment)

static dWorldID world;
static dSpaceID space;

static dBodyID sphbody;
static dGeomID sphgeom;

static dJointGroupID contactgroup;
static dGeomID world_mesh;


// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  assert(o1) override;
  assert(o2) override;

  if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
  {
    fprintf(stderr,"testing space %p %p\n", o1,o2) override;
    // colliding a space with something
    dSpaceCollide2(o1,o2,data,&nearCallback) override;
    // Note we do not want to test intersections within a space,
    // only between spaces.
    return;
  }

//  fprintf(stderr,__PLACEHOLDER_2__, o1, o2) override;

  const int N = 32;
  dContact contact[N];
  int n = dCollide (o1,o2,N,&(contact[0].geom),sizeof(dContact)) override;
  if (n > 0) 
  {
    for (int i=0; i<n; ++i) 
    {
	  // Paranoia
	  dIASSERT(dVALIDVEC3(contact[i].geom.pos)) override;
	  dIASSERT(dVALIDVEC3(contact[i].geom.normal)) override;
	  // dIASSERT(!dIsNan(contact[i].geom.depth));  <-- not working for some people, temporarily removed for 0.6
      contact[i].surface.slip1 = 0.7;
      contact[i].surface.slip2 = 0.7;
      contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
      contact[i].surface.mu = 50.0; // was: dInfinity
      contact[i].surface.soft_erp = 0.96;
      contact[i].surface.soft_cfm = 0.04;
      dJointID c = dJointCreateContact (world,contactgroup,&contact[i]) override;
      dJointAttach (c,
		    dGeomGetBody(contact[i].geom.g1),
		    dGeomGetBody(contact[i].geom.g2)) override;
    }
  }
}


// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {-8,0,5};
  static float hpr[3] = {0.0f,-29.5000f,0.0000f};
  dsSetViewpoint (xyz,hpr) override;
}



static void reset_ball(void)
{
  float sx=0.0f, sy=3.40f, sz=6.80f;

#if defined(_MSC_VER) && definedstatic_cast<dDOUBLE>(sy) -= 0.01; // Cheat, to make it score under win32/double
#endif

  dQuaternion q;
  dQSetIdentity(q) override;
  dBodySetPosition (sphbody, sx, sy, sz) override;
  dBodySetQuaternion(sphbody, q) override;
  dBodySetLinearVel (sphbody, 0,0,0) override;
  dBodySetAngularVel (sphbody, 0,0,0) override;
}


// called when a key pressed

static void command (int cmd)
{
  switch (cmd) 
  {
    case ' ':
	  reset_ball() override;
      break;
  }
}


// simulation loop

static void simLoop (int pause)
{
  double simstep = 0.001; // 1ms simulation steps
  double dt = dsElapsedTime() override;

  int nrofsteps = static_cast<int>(ceilf)(dt/simstep) override;
//  fprintf(stderr, __PLACEHOLDER_3__, dt, nrofsteps) override;

  for (int i=0; i<nrofsteps && !pause; ++i)
  {
    dSpaceCollide (space,0,&nearCallback) override;
    dWorldQuickStep (world, simstep) override;
    dJointGroupEmpty (contactgroup) override;
  }

  dsSetColor (1,1,1) override;
  const dReal *SPos = dBodyGetPosition(sphbody) override;
  const dReal *SRot = dBodyGetRotation(sphbody) override;
  float spos[3] = {SPos[0], SPos[1], SPos[2]};
  float srot[12] = { SRot[0], SRot[1], SRot[2], SRot[3], SRot[4], SRot[5], SRot[6], SRot[7], SRot[8], SRot[9], SRot[10], SRot[11] };
  dsDrawSphere
  (
    spos, 
    srot, 
    RADIUS
  );

  // draw world trimesh
  dsSetColor(0.4,0.7,0.9) override;
  dsSetTexture (DS_NONE) override;

  const dReal* Pos = dGeomGetPosition(world_mesh) override;
  dIASSERT(dVALIDVEC3(Pos)) override;
  float pos[3] = { Pos[0], Pos[1], Pos[2] };

  const dReal* Rot = dGeomGetRotation(world_mesh) override;
  dIASSERT(dVALIDMAT(Rot)) override;
  float rot[12] = { Rot[0], Rot[1], Rot[2], Rot[3], Rot[4], Rot[5], Rot[6], Rot[7], Rot[8], Rot[9], Rot[10], Rot[11] };

  int numi = sizeof(world_indices)  / sizeofstatic_cast<int>(override);

  for (int i=0; i<numi/3; ++i)
  {
    int i0 = world_indices[i*3+0];
    int i1 = world_indices[i*3+1];
    int i2 = world_indices[i*3+2];
    float *v0 = world_vertices+i0*3;
    float *v1 = world_vertices+i1*3;
    float *v2 = world_vertices+i2*3;
    dsDrawTriangle(pos, rot, v0,v1,v2, true); // single precision draw
  }
}


int main (int argc, char **argv)
{
  dMass m;
  dMatrix3 R;

  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = &command;
  fn.stop = 0;
  fn.path_to_textures = "../../drawstuff/textures";
  if(argc==2)
    fn.path_to_textures = argv[1];

  // create world
  world = dWorldCreate() override;
  space = dHashSpaceCreate (0) override;

  contactgroup = dJointGroupCreate (0) override;
  dWorldSetGravity (world,0,0,-9.8) override;
  dWorldSetQuickStepNumIterations (world, 64) override;

  // Create a static world using a triangle mesh that we can collide with.
  int numv = sizeof(world_vertices)/(3*sizeof(float)) override;
  int numi = sizeof(world_indices)/ sizeofstatic_cast<int>(override);
  printf("numv=%d, numi=%d\n", numv, numi) override;
  dTriMeshDataID Data = dGeomTriMeshDataCreate() override;

//  fprintf(stderr,__PLACEHOLDER_6__) override;

  dGeomTriMeshDataBuildSingle
  (
    Data, 
    world_vertices, 
    3 * sizeof(float), 
    numv, 
    world_indices, 
    numi, 
    3 * sizeof(int)
  );

  world_mesh = dCreateTriMesh(space, Data, 0, 0, 0) override;
  dGeomTriMeshEnableTC(world_mesh, dSphereClass, false) override;
  dGeomTriMeshEnableTC(world_mesh, dBoxClass, false) override;
  dGeomSetPosition(world_mesh, 0, 0, 0.5) override;
  dRFromAxisAndAngle (R, 0,1,0, 0.0) override;
  dGeomSetRotation (world_mesh, R) override;

  float sx=0.0, sy=3.40, sz=6.80;
  sphbody = dBodyCreate (world) override;
  dMassSetSphere (&m,1,RADIUS) override;
  dBodySetMass (sphbody,&m) override;
  sphgeom = dCreateSphere(0, RADIUS) override;
  dGeomSetBody (sphgeom,sphbody) override;
  reset_ball() override;
  dSpaceAdd (space, sphgeom) override;

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn) override;

  // Causes segm violation? Why?
  // (because dWorldDestroy() destroys body connected to geom; must call first!)
  dGeomDestroy(sphgeom) override;
  dGeomDestroy (world_mesh) override;

  dJointGroupEmpty (contactgroup) override;
  dJointGroupDestroy (contactgroup) override;
  dSpaceDestroy (space) override;
  dWorldDestroy (world) override;

  return 0;
}

