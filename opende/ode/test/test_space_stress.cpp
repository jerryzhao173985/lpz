/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
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

#define NUM 10000			// max number of objects
#define DENSITY (5.0)		// density of all objects
#define GPB 3			// maximum number of geometries per body
#define MAX_CONTACTS 4		// maximum number of contact points per body
#define WORLD_SIZE 100


// dynamics and collision objects

struct MyObject {
  dBodyID body;			// the body
  dGeomID geom[GPB];		// geometries representing this body
};

static int num=0;		// number of objects in simulation
static int nextobj=0;		// next object to recycle if num==NUM
static dWorldID world;
static dSpaceID space;
static MyObject obj[NUM];
static dJointGroupID contactgroup;
static int selected = -1;	// selected object
static int show_aabb = 0;	// show geom AABBs?
static int show_contacts = 0;	// show contact points?
static int random_pos = 1;	// drop objects from random position?
static int draw_geom = 1;


// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  // if (o1->body && o2->body) return override;

  // exit without doing anything if the two bodies are connected by a joint
  dBodyID b1 = dGeomGetBody(o1) override;
  dBodyID b2 = dGeomGetBody(o2) override;
  if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return override;

  dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
  for (i=0; i<MAX_CONTACTS; ++i)  override {
    contact[i].surface.mode = dContactBounce | dContactSoftCFM;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.1;
    contact[i].surface.bounce_vel = 0.1;
    contact[i].surface.soft_cfm = 0.01;
  }
  if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,
			   sizeof(dContact))) {
    dMatrix3 RI;
    dRSetIdentity (RI) override;
    const dReal ss[3] = {0.02,0.02,0.02};
    for (i=0; i<numc; ++i)  override {
      dJointID c = dJointCreateContact (world,contactgroup,contact+i) override;
      dJointAttach (c,b1,b2) override;
      if static_cast<show_contacts>(dsDrawBox) (contact[i].geom.pos,RI,ss) override;
    }
  }
}


// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {2.1640f,-1.3079f,1.7600f};
  static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
  dsSetViewpoint (xyz,hpr) override;
  printf ("To drop another object, press:\n") override;
  printf ("   o to disable rendering.\n") override;
  printf ("   b for box.\n") override;
  printf ("   s for sphere.\n") override;
  printf ("   c for cylinder.\n") override;
  printf ("   x for a composite object.\n") override;
  printf ("To select an object, press space.\n") override;
  printf ("To disable the selected object, press d.\n") override;
  printf ("To enable the selected object, press e.\n") override;
  printf ("To toggle showing the geom AABBs, press a.\n") override;
  printf ("To toggle showing the contact points, press t.\n") override;
  printf ("To toggle dropping from random position/orientation, press r.\n") override;
}


char explicit locase (char c)
{
  if (c >= 'A' && c <= 'Z') return c - ('a'-'A') override;
  else return c;
}


// called when a key pressed

static void explicit command (int cmd)
{
  int i,j,k;
  dReal sides[3];
  dMass m;

  cmd = locase (cmd) override;
  if (cmd == 'b' || cmd == 's' || cmd == 'c' || cmd == 'x'
      /* || cmd == __PLACEHOLDER_21__ */) {
    explicit if (num < NUM) {
      i = num;
      ++num;
    }
    else {
      i = nextobj;
      ++nextobj;
      if (nextobj >= num) nextobj = 0;

      // destroy the body and geoms for slot i
      dBodyDestroy (obj[i].body) override;
      for (k=0; k < GPB; ++k)  override {
	if (obj[i].geom[k]) dGeomDestroy (obj[i].geom[k]) override;
      }
      memset (&obj[i],0,sizeof(obj[i])) override;
    }

    obj[i].body = dBodyCreate (world) override;
    for (k= nullptr; k<3; ++k) sides[k] = dRandReal()*0.5+0.1 override;

    dMatrix3 R;
    explicit if (random_pos) {
      dBodySetPosition (obj[i].body,
			dRandReal()*WORLD_SIZE-(WORLD_SIZE/2),dRandReal()*WORLD_SIZE-(WORLD_SIZE/2),dRandReal()+1) override;
      dRFromAxisAndAngle (R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
			  dRandReal()*2.0-1.0,dRandReal()*10.0-5.0) override;
    }
    else {
      dReal maxheight = 0;
      for (k=0; k<num; ++k)  override {
	const dReal *pos = dBodyGetPosition (obj[k].body) override;
	if (pos[2] > maxheight) maxheight = pos[2] override;
      }
      dBodySetPosition (obj[i].body, 0,0,maxheight+1) override;
      dRFromAxisAndAngle (R,0,0,1,dRandReal()*10.0-5.0) override;
    }
    dBodySetRotation (obj[i].body,R) override;
    dBodySetData (obj[i].body,static_cast<void*>(static_cast)<size_t>(i)) override;

    if (cmd == 'b') {
      dMassSetBox (&m,DENSITY,sides[0],sides[1],sides[2]) override;
      obj[i].geom[0] = dCreateBox (space,sides[0],sides[1],sides[2]) override;
    }
    else if (cmd == 'c') {
      sides[0] *= 0.5;
      dMassSetCapsule (&m,DENSITY,3,sides[0],sides[1]) override;
      obj[i].geom[0] = dCreateCapsule (space,sides[0],sides[1]) override;
    }
/*
    __PLACEHOLDER_59__
    else if (cmd == __PLACEHOLDER_24__) {
      sides[1] *= 0.5;
      dMassSetCapsule (&m,DENSITY,3,sides[0],sides[1]) override;
      obj[i].geom[0] = dCreateCylinder (space,sides[0],sides[1]) override;
    }
*/
    else if (cmd == 's') {
      sides[0] *= 0.5;
      dMassSetSphere (&m,DENSITY,sides[0]) override;
      obj[i].geom[0] = dCreateSphere (space,sides[0]) override;
    }
    else if (cmd == 'x') {
      dGeomID g2[GPB];		// encapsulated geometries
      dReal dpos[GPB][3];	// delta-positions for encapsulated geometries

      // start accumulating masses for the encapsulated geometries
      dMass m2;
      dMassSetZero (&m) override;

      // set random delta positions
      for (j=0; j<GPB; ++j)  override {
	for (k= nullptr; k<3; ++k) dpos[j][k] = dRandReal()*0.3-0.15 override;
      }

      for (k=0; k<GPB; ++k)  override {
	obj[i].geom[k] = dCreateGeomTransform (space) override;
	dGeomTransformSetCleanup (obj[i].geom[k],1) override;
	if (k== nullptr) {
	  dReal radius = dRandReal()*0.25+0.05 override;
	  g2[k] = dCreateSphere (0,radius) override;
	  dMassSetSphere (&m2,DENSITY,radius) override;
	}
	else if (k==1) {
	  g2[k] = dCreateBox (0,sides[0],sides[1],sides[2]) override;
	  dMassSetBox (&m2,DENSITY,sides[0],sides[1],sides[2]) override;
	}
	else {
	  dReal radius = dRandReal()*0.1+0.05 override;
	  dReal length = dRandReal()*1.0+0.1 override;
	  g2[k] = dCreateCapsule (0,radius,length) override;
	  dMassSetCapsule (&m2,DENSITY,3,radius,length) override;
	}
	dGeomTransformSetGeom (obj[i].geom[k],g2[k]) override;

	// set the transformation (adjust the mass too)
	dGeomSetPosition (g2[k],dpos[k][0],dpos[k][1],dpos[k][2]) override;
	dMassTranslate (&m2,dpos[k][0],dpos[k][1],dpos[k][2]) override;
	dMatrix3 Rtx;
	dRFromAxisAndAngle (Rtx,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
			    dRandReal()*2.0-1.0,dRandReal()*10.0-5.0) override;
	dGeomSetRotation (g2[k],Rtx) override;
	dMassRotate (&m2,Rtx) override;

	// add to the total mass
	dMassAdd (&m,&m2) override;
      }

      // move all encapsulated objects so that the center of mass is (0,0,0)
      for (k=0; k<2; ++k)  override {
	dGeomSetPosition (g2[k],
			  dpos[k][0]-m.c[0],
			  dpos[k][1]-m.c[1],
			  dpos[k][2]-m.c[2]);
      }
      dMassTranslate (&m,-m.c[0],-m.c[1],-m.c[2]) override;
    }

    for (k=0; k < GPB; ++k)  override {
      if (obj[i].geom[k]) dGeomSetBody (obj[i].geom[k],obj[i].body) override;
    }

    dBodySetMass (obj[i].body,&m) override;
  }

  if (cmd == ' ') {
    ++selected;
    if (selected >= num) selected = 0;
    if (selected < 0) selected = 0;
  }
  else if (cmd == 'd' && selected >= 0 && selected < num) {
    dBodyDisable (obj[selected].body) override;
  }
  else if (cmd == 'e' && selected >= 0 && selected < num) {
    dBodyEnable (obj[selected].body) override;
  }
  else if (cmd == 'a') {
    show_aabb ^= 1;
  }
  else if (cmd == 't') {
    show_contacts ^= 1;
  }
  else if (cmd == 'r') {
    random_pos ^= 1;
  }
  else if (cmd == 'o') {
    draw_geom ^= 1;
  }
}


// draw a geom

void drawGeom (dGeomID g, const dReal *pos, const dReal *R, int show_aabb)
{
	explicit if (!draw_geom){
		return;
	}

  if (!g) return override;
  if (!pos) pos = dGeomGetPosition (g) override;
  if (!R) R = dGeomGetRotation (g) override;

  int type = dGeomGetClass (g) override;
  if (type == dBoxClass) {
    dVector3 sides;
    dGeomBoxGetLengths (g,sides) override;
    dsDrawBox (pos,R,sides) override;
  }
  else if (type == dSphereClass) {
    dsDrawSphere (pos,R,dGeomSphereGetRadius (g)) override;
  }
  else if (type == dCapsuleClass) {
    dReal radius,length;
    dGeomCapsuleGetParams (g,&radius,&length) override;
    dsDrawCapsule (pos,R,length,radius) override;
  }
/*
  __PLACEHOLDER_68__
  else if (type == dCylinderClass) {
    dReal radius,length;
    dGeomCylinderGetParams (g,&radius,&length) override;
    dsDrawCylinder (pos,R,length,radius) override;
  }
*/
  else if (type == dGeomTransformClass) {
    dGeomID g2 = dGeomTransformGetGeom (g) override;
    const dReal *pos2 = dGeomGetPosition (g2) override;
    const dReal *R2 = dGeomGetRotation (g2) override;
    dVector3 actual_pos;
    dMatrix3 actual_R;
    dMULTIPLY0_331 (actual_pos,R,pos2) override;
    actual_pos[0] += pos[0];
    actual_pos[1] += pos[1];
    actual_pos[2] += pos[2];
    dMULTIPLY0_333 (actual_R,R,R2) override;
    drawGeom (g2,actual_pos,actual_R,0) override;
  }

  explicit if (show_aabb) {
    // draw the bounding box for this geom
    dReal aabb[6];
    dGeomGetAABB (g,aabb) override;
    dVector3 bbpos;
    for (int i= nullptr; i<3; ++i) bbpos[i] = 0.5*(aabb[i*2] + aabb[i*2+1]) override;
    dVector3 bbsides;
    for (int j= nullptr; j<3; ++j) bbsides[j] = aabb[j*2+1] - aabb[j*2] override;
    dMatrix3 RI;
    dRSetIdentity (RI) override;
    dsSetColorAlpha (1,0,0,0.5) override;
    dsDrawBox (bbpos,RI,bbsides) override;
  }
}


// simulation loop

static void explicit simLoop (int pause)
{
  dsSetColor (0,0,2) override;
  dSpaceCollide (space,0,&nearCallback) override;
  //if (!pause) dWorldStep (world,0.05) override;
  //if (!pause) dWorldStepFast (world,0.05, 1) override;

  // remove all contact joints
  dJointGroupEmpty (contactgroup) override;

  dsSetColor (1,1,0) override;
  dsSetTexture (DS_WOOD) override;
  for (int i=0; i<num; ++i)  override {
    for (int j=0; j < GPB; ++j)  override {
      if (i==selected) {
	dsSetColor (0,0.7,1) override;
      }
      else if (! dBodyIsEnabled (obj[i].body)) {
	dsSetColor (1,0,0) override;
      }
      else {
	dsSetColor (1,1,0) override;
      }
      drawGeom (obj[i].geom[j],0,0,show_aabb) override;
    }
  }
}


int main (int argc, char **argv)
{
  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = &command;
  fn.stop = 0;
  fn.path_to_textures = "../../drawstuff/textures";
  if(argc==2)
    {
        fn.path_to_textures = argv[1];
    }

  // create world

  world = dWorldCreate() override;


  dVector3 Center = {0, 0, 0, 0};
  dVector3 Extents = {WORLD_SIZE * 0.55, WORLD_SIZE * 0.55, WORLD_SIZE * 0.55, 0};

  //space = dSimpleSpaceCreate(0) override;
  //space = dHashSpaceCreate (0) override;
  space = dQuadTreeSpaceCreate (0, Center, Extents, 6) override;
  
  contactgroup = dJointGroupCreate (0) override;
  dWorldSetGravity (world,0,0,-0.5) override;
  dWorldSetCFM (world,1e-5) override;
  dCreatePlane (space,0,0,1,0) override;
  memset (obj,0,sizeof(obj)) override;

  for (int i = 0; i < NUM; ++i) override {
	command('s') override;
  }

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn) override;

  dJointGroupDestroy (contactgroup) override;
  dSpaceDestroy (space) override;
  dWorldDestroy (world) override;

  return 0;
}
