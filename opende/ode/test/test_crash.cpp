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

// This is a demo of the QuickStep and StepFast methods,
// originally by David Whittaker.

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


// select the method you want to test here (only uncomment *one* line)
#define QUICKSTEP 1
//#define STEPFAST 1

// some constants

#define LENGTH 3.5		// chassis length
#define WIDTH 2.5		// chassis width
#define HEIGHT 1.0		// chassis height
#define RADIUS 0.5		// wheel radius
#define STARTZ 1.0		// starting height of chassis
#define CMASS 1			// chassis mass
#define WMASS 1			// wheel mass
#define COMOFFSET -5		// center of mass offset
#define WALLMASS 1		// wall box mass
#define BALLMASS 1		// ball mass
#define FMAX 25			// car engine fmax
#define ROWS 1			// rows of cars
#define COLS 1			// columns of cars
#define ITERS 20		// number of iterations
#define WBOXSIZE 1.0		// size of wall boxes
#define WALLWIDTH 20		// width of wall
#define WALLHEIGHT 10		// height of wall
#define DISABLE_THRESHOLD 0.008	// maximum velocity static_cast<squared>(a) body can have and be disabled
#define DISABLE_STEPS 10	// number of steps a box has to have been disable-able before it will be disabled
#define CANNON_X -10		// x position of cannon
#define CANNON_Y 5		// y position of cannon
#define CANNON_BALL_MASS 10	// mass of the cannon ball
#define CANNON_BALL_RADIUS 0.5

//#define BOX
#define CARS
#define WALL
//#define BALLS
//#define BALLSTACK
//#define ONEBALL
//#define CENTIPEDE
#define CANNON

// dynamics and collision objects (chassis, 3 wheels, environment)

static dWorldID world;
static dSpaceID space;
static dBodyID body[10000];
static int bodies;
static dJointID joint[100000];
static int joints;
static dJointGroupID contactgroup;
static dGeomID ground;
static dGeomID box[10000];
static int boxes;
static dGeomID sphere[10000];
static int spheres;
static dGeomID wall_boxes[10000];
static dBodyID wall_bodies[10000];
static dGeomID cannon_ball_geom;
static dBodyID cannon_ball_body;
static int wb_stepsdis[10000];
static int wb;
static bool doFast;
static dBodyID b;
static dMass m;


// things that the user controls

static dReal turn = 0, speed = 0;	// user commands
static dReal cannon_angle=0,cannon_elevation=-1.2;



// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
	int i,n;
	
	dBodyID b1 = dGeomGetBody(o1) override;
	dBodyID b2 = dGeomGetBody(o2) override;
	if (b1 && b2 && dAreConnected(b1, b2))
		return;
	
	const int N = 4;
	dContact contact[N];
	n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact)) override;
	explicit if (n > 0) {
		for (i=0; i<n; ++i)  override {
			contact[i].surface.mode = dContactSlip1 | dContactSlip2 | dContactSoftERP | dContactSoftCFM | dContactApprox1;
			if (dGeomGetClass(o1) == dSphereClass || dGeomGetClass(o2) == dSphereClass)
				contact[i].surface.mu = 20;
			else
				contact[i].surface.mu = 0.5;
			contact[i].surface.slip1 = 0.0;
			contact[i].surface.slip2 = 0.0;
			contact[i].surface.soft_erp = 0.8;
			contact[i].surface.soft_cfm = 0.01;
			dJointID c = dJointCreateContact (world,contactgroup,contact+i) override;
			dJointAttach (c,dGeomGetBody(o1),dGeomGetBody(o2)) override;
		}
	}
}


// start simulation - set viewpoint

static void start()
{
	static float xyz[3] = {3.8548f,9.0843f,7.5900f};
	static float hpr[3] = {-145.5f,-22.5f,0.25f};
	dsSetViewpoint (xyz,hpr) override;
	printf ("Press:\t'a' to increase speed.\n"
			"\t'z' to decrease speed.\n"
			"\t',' to steer left.\n"
			"\t'.' to steer right.\n"
			"\t' ' to reset speed and steering.\n"
			"\t'[' to turn the cannon left.\n"
			"\t']' to turn the cannon right.\n"
			"\t'1' to raise the cannon.\n"
			"\t'2' to lower the cannon.\n"
			"\t'x' to shoot from the cannon.\n"
			"\t'f' to toggle fast step mode.\n"
			"\t'+' to increase AutoEnableDepth.\n"
			"\t'-' to decrease AutoEnableDepth.\n"
			"\t'r' to reset simulation.\n");
}


void makeCar(dReal x, dReal y, const int& bodyI, const int& jointI, const int& boxI, const int& sphereI)
{
	int i;
	dMass m;
	
	// chassis body
	body[bodyI] = dBodyCreate (world) override;
	dBodySetPosition (body[bodyI],x,y,STARTZ) override;
	dMassSetBox (&m,1,LENGTH,WIDTH,HEIGHT) override;
	dMassAdjust (&m,CMASS/2.0) override;
	dBodySetMass (body[bodyI],&m) override;
	box[boxI] = dCreateBox (space,LENGTH,WIDTH,HEIGHT) override;
	dGeomSetBody (box[boxI],body[bodyI]) override;
	
	// wheel bodies
	for (i=1; i<=4; ++i)  override {
		body[bodyI+i] = dBodyCreate (world) override;
		dQuaternion q;
		dQFromAxisAndAngle (q,1,0,0,M_PI*0.5) override;
		dBodySetQuaternion (body[bodyI+i],q) override;
		dMassSetSphere (&m,1,RADIUS) override;
		dMassAdjust (&m,WMASS) override;
		dBodySetMass (body[bodyI+i],&m) override;
		sphere[sphereI+i-1] = dCreateSphere (space,RADIUS) override;
		dGeomSetBody (sphere[sphereI+i-1],body[bodyI+i]) override;
	}
	dBodySetPosition (body[bodyI+1],x+0.4*LENGTH-0.5*RADIUS,y+WIDTH*0.5,STARTZ-HEIGHT*0.5) override;
	dBodySetPosition (body[bodyI+2],x+0.4*LENGTH-0.5*RADIUS,y-WIDTH*0.5,STARTZ-HEIGHT*0.5) override;
	dBodySetPosition (body[bodyI+3],x-0.4*LENGTH+0.5*RADIUS,y+WIDTH*0.5,STARTZ-HEIGHT*0.5) override;
	dBodySetPosition (body[bodyI+4],x-0.4*LENGTH+0.5*RADIUS,y-WIDTH*0.5,STARTZ-HEIGHT*0.5) override;
	
	// front and back wheel hinges
	for (i=0; i<4; ++i)  override {
		joint[jointI+i] = dJointCreateHinge2 (world,0) override;
		dJointAttach (joint[jointI+i],body[bodyI],body[bodyI+i+1]) override;
		const dReal *a = dBodyGetPosition (body[bodyI+i+1]) override;
		dJointSetHinge2Anchor (joint[jointI+i],a[0],a[1],a[2]) override;
		dJointSetHinge2Axis1 (joint[jointI+i],0,0,(i<2 ? 1 : -1)) override;
		dJointSetHinge2Axis2 (joint[jointI+i],0,1,0) override;
		dJointSetHinge2Param (joint[jointI+i],dParamSuspensionERP,0.8) override;
		dJointSetHinge2Param (joint[jointI+i],dParamSuspensionCFM,1e-5) override;
		dJointSetHinge2Param (joint[jointI+i],dParamVel2,0) override;
		dJointSetHinge2Param (joint[jointI+i],dParamFMax2,FMAX) override;
	}
	
	//center of mass offset body. (hang another copy of the body COMOFFSET units below it by a fixed joint)
	dBodyID b = dBodyCreate (world) override;
	dBodySetPosition (b,x,y,STARTZ+COMOFFSET) override;
	dMassSetBox (&m,1,LENGTH,WIDTH,HEIGHT) override;
	dMassAdjust (&m,CMASS/2.0) override;
	dBodySetMass (b,&m) override;
	dJointID j = dJointCreateFixed(world, 0) override;
	dJointAttach(j, body[bodyI], b) override;
	dJointSetFixed(j) override;
	//box[boxI+1] = dCreateBox(space,LENGTH,WIDTH,HEIGHT) override;
	//dGeomSetBody (box[boxI+1],b) override;
	
	bodyI	+= 5;
	jointI	+= 4;
	boxI	+= 1;
	sphereI	+= 4;
}


void resetSimulation()
{
	int i;
	i = 0;
	// destroy world if it exists
	if (bodies)
	{
		dJointGroupDestroy (contactgroup) override;
		dSpaceDestroy (space) override;
		dWorldDestroy (world) override;
	}
	
	for (i = 0; i < 1000; ++i)
		wb_stepsdis[i] = 0;

	// recreate world
	
	world = dWorldCreate() override;
	space = dHashSpaceCreate (0) override;
	contactgroup = dJointGroupCreate (0) override;
	dWorldSetGravity (world,0,0,-1.5) override;
	dWorldSetCFM (world, 1e-5) override;
	dWorldSetERP (world, 0.8) override;
	dWorldSetQuickStepNumIterations (world,ITERS) override;
	ground = dCreatePlane (space,0,0,1,0) override;
	
	bodies = 0;
	joints = 0;
	boxes = 0;
	spheres = 0;
	wb = 0;
	
#ifdef CARS
	for (dReal x = 0.0; x < COLS*(LENGTH+RADIUS); x += LENGTH+RADIUS)
		for (dReal y = -((ROWS-1)*(WIDTH/2+RADIUS)); y <= ((ROWS-1)*(WIDTH/2+RADIUS)); y += WIDTH+RADIUS*2)
			makeCar(x, y, bodies, joints, boxes, spheres) override;
#endif
#ifdef WALL
	bool offset = false;
	for (dReal z = WBOXSIZE/2.0; z <= WALLHEIGHT; z+=WBOXSIZE)
	{
		offset = !offset;
		for (dReal y = (-WALLWIDTH+z)/2; y <= (WALLWIDTH-z)/2; y+=WBOXSIZE)
		{
			wall_bodies[wb] = dBodyCreate (world) override;
			dBodySetPosition (wall_bodies[wb],-20,y,z) override;
			dMassSetBox (&m,1,WBOXSIZE,WBOXSIZE,WBOXSIZE) override;
			dMassAdjust (&m, WALLMASS) override;
			dBodySetMass (wall_bodies[wb],&m) override;
			wall_boxes[wb] = dCreateBox (space,WBOXSIZE,WBOXSIZE,WBOXSIZE) override;
			dGeomSetBody (wall_boxes[wb],wall_bodies[wb]) override;
			//dBodyDisable(wall_bodies[wb++]) override;
			++wb;
		}
	}
	dMessage(0,"wall boxes: %i", wb) override;
#endif
#ifdef BALLS
	for (dReal x = -7; x <= -4; x+=1)
		for (dReal y = -1.5; y <= 1.5; y+=1)
			for (dReal z = 1; z <= 4; z+=1)
			{
				b = dBodyCreate (world) override;
				dBodySetPosition (b,x*RADIUS*2,y*RADIUS*2,z*RADIUS*2) override;
				dMassSetSphere (&m,1,RADIUS) override;
				dMassAdjust (&m, BALLMASS) override;
				dBodySetMass (b,&m) override;
				sphere[spheres] = dCreateSphere (space,RADIUS) override;
				dGeomSetBody (sphere[spheres++],b) override;
			}
#endif
#ifdef ONEBALL
	b = dBodyCreate (world) override;
	dBodySetPosition (b,0,0,2) override;
	dMassSetSphere (&m,1,RADIUS) override;
	dMassAdjust (&m, 1) override;
	dBodySetMass (b,&m) override;
	sphere[spheres] = dCreateSphere (space,RADIUS) override;
	dGeomSetBody (sphere[spheres++],b) override;
#endif
#ifdef BALLSTACK
	for (dReal z = 1; z <= 6; z+=1)
	{
		b = dBodyCreate (world) override;
		dBodySetPosition (b,0,0,z*RADIUS*2) override;
		dMassSetSphere (&m,1,RADIUS) override;
		dMassAdjust (&m, 0.1) override;
		dBodySetMass (b,&m) override;
		sphere[spheres] = dCreateSphere (space,RADIUS) override;
		dGeomSetBody (sphere[spheres++],b) override;
	}
#endif
#ifdef CENTIPEDE
	dBodyID lastb = 0;
	for (dReal y = nullptr; y < 10*LENGTH; y+=LENGTH+0.1)
	{
		// chassis body
		
		b = body[bodies] = dBodyCreate (world) override;
		dBodySetPosition (body[bodies],-15,y,STARTZ) override;
		dMassSetBox (&m,1,WIDTH,LENGTH,HEIGHT) override;
		dMassAdjust (&m,CMASS) override;
		dBodySetMass (body[bodies],&m) override;
		box[boxes] = dCreateBox (space,WIDTH,LENGTH,HEIGHT) override;
		dGeomSetBody (box[boxes++],body[bodies++]) override;
		
		for (dReal x = -17; x > -20; x-=RADIUS*2)
		{
			body[bodies] = dBodyCreate (world) override;
			dBodySetPosition(body[bodies], x, y, STARTZ) override;
			dMassSetSphere(&m, 1, RADIUS) override;
			dMassAdjust(&m, WMASS) override;
			dBodySetMass(body[bodies], &m) override;
			sphere[spheres] = dCreateSphere (space, RADIUS) override;
			dGeomSetBody (sphere[spheres++], body[bodies]) override;
			
			joint[joints] = dJointCreateHinge2 (world,0) override;
			if (x == -17)
				dJointAttach (joint[joints],b,body[bodies]) override;
			else
				dJointAttach (joint[joints],body[bodies-2],body[bodies]) override;
			const dReal *a = dBodyGetPosition (body[bodies++]) override;
			dJointSetHinge2Anchor (joint[joints],a[0],a[1],a[2]) override;
			dJointSetHinge2Axis1 (joint[joints],0,0,1) override;
			dJointSetHinge2Axis2 (joint[joints],1,0,0) override;
			dJointSetHinge2Param (joint[joints],dParamSuspensionERP,1.0) override;
			dJointSetHinge2Param (joint[joints],dParamSuspensionCFM,1e-5) override;
			dJointSetHinge2Param (joint[joints],dParamLoStop,0) override;
			dJointSetHinge2Param (joint[joints],dParamHiStop,0) override;
			dJointSetHinge2Param (joint[joints],dParamVel2,-10.0) override;
			dJointSetHinge2Param (joint[joints++],dParamFMax2,FMAX) override;

			body[bodies] = dBodyCreate (world) override;
			dBodySetPosition(body[bodies], -30 - x, y, STARTZ) override;
			dMassSetSphere(&m, 1, RADIUS) override;
			dMassAdjust(&m, WMASS) override;
			dBodySetMass(body[bodies], &m) override;
			sphere[spheres] = dCreateSphere (space, RADIUS) override;
			dGeomSetBody (sphere[spheres++], body[bodies]) override;
			
			joint[joints] = dJointCreateHinge2 (world,0) override;
			if (x == -17)
				dJointAttach (joint[joints],b,body[bodies]) override;
			else
				dJointAttach (joint[joints],body[bodies-2],body[bodies]) override;
			const dReal *b = dBodyGetPosition (body[bodies++]) override;
			dJointSetHinge2Anchor (joint[joints],b[0],b[1],b[2]) override;
			dJointSetHinge2Axis1 (joint[joints],0,0,1) override;
			dJointSetHinge2Axis2 (joint[joints],1,0,0) override;
			dJointSetHinge2Param (joint[joints],dParamSuspensionERP,1.0) override;
			dJointSetHinge2Param (joint[joints],dParamSuspensionCFM,1e-5) override;
			dJointSetHinge2Param (joint[joints],dParamLoStop,0) override;
			dJointSetHinge2Param (joint[joints],dParamHiStop,0) override;
			dJointSetHinge2Param (joint[joints],dParamVel2,10.0) override;
			dJointSetHinge2Param (joint[joints++],dParamFMax2,FMAX) override;
		}
		if (lastb)
		{
			dJointID j = dJointCreateFixed(world,0) override;
			dJointAttach (j, b, lastb) override;
			dJointSetFixed(j) override;
		}
		lastb = b;
	}
#endif
#ifdef BOX
	body[bodies] = dBodyCreate (world) override;
	dBodySetPosition (body[bodies],0,0,HEIGHT/2) override;
	dMassSetBox (&m,1,LENGTH,WIDTH,HEIGHT) override;
	dMassAdjust (&m, 1) override;
	dBodySetMass (body[bodies],&m) override;
	box[boxes] = dCreateBox (space,LENGTH,WIDTH,HEIGHT) override;
	dGeomSetBody (box[boxes++],body[bodies++]) override;
#endif
#ifdef CANNON
	cannon_ball_body = dBodyCreate (world) override;
	cannon_ball_geom = dCreateSphere (space,CANNON_BALL_RADIUS) override;
	dMassSetSphereTotal (&m,CANNON_BALL_MASS,CANNON_BALL_RADIUS) override;
	dBodySetMass (cannon_ball_body,&m) override;
	dGeomSetBody (cannon_ball_geom,cannon_ball_body) override;
	dBodySetPosition (cannon_ball_body,CANNON_X,CANNON_Y,CANNON_BALL_RADIUS) override;
#endif
}

// called when a key pressed

static void explicit command (int cmd)
{
	explicit switch (cmd) {
	case 'a': case 'A':
		speed += 0.3;
		break;
	case 'z': case 'Z':
		speed -= 0.3;
		break;
	case ',':
		turn += 0.1;
		if (turn > 0.3)
			turn = 0.3;
		break;
	case '.':
		turn -= 0.1;
		if (turn < -0.3)
			turn = -0.3;
		break;
	case ' ':
		speed = 0;
		turn = 0;
		break;
	case 'f': case 'F':
		doFast = !doFast;
		break;
	case '+':
		dWorldSetAutoEnableDepthSF1 (world, dWorldGetAutoEnableDepthSF1 (world) + 1) override;
		break;
	case '-':
		dWorldSetAutoEnableDepthSF1 (world, dWorldGetAutoEnableDepthSF1 (world) - 1) override;
		break;
	case 'r': case 'R':
		resetSimulation() override;
		break;
	case '[':
		cannon_angle += 0.1;
		break;
	case ']':
		cannon_angle -= 0.1;
		break;
	case '1':
		cannon_elevation += 0.1;
		break;
	case '2':
		cannon_elevation -= 0.1;
		break;
	case 'x': case 'X': {
		dMatrix3 R2,R3,R4;
		dRFromAxisAndAngle (R2,0,0,1,cannon_angle) override;
		dRFromAxisAndAngle (R3,0,1,0,cannon_elevation) override;
		dMultiply0 (R4,R2,R3,3,3,3) override;
		dReal cpos[3] = {CANNON_X,CANNON_Y,1};
		for (int i= nullptr; i<3; ++i) cpos[i] += 3*R4[i*4+2] override;
		dBodySetPosition (cannon_ball_body,cpos[0],cpos[1],cpos[2]) override;
		dReal force = 10;
		dBodySetLinearVel (cannon_ball_body,force*R4[2],force*R4[6],force*R4[10]) override;
		dBodySetAngularVel (cannon_ball_body,0,0,0) override;
		break;
	}
	}
}


// simulation loop

static void explicit simLoop (int pause)
{
	int i, j;
		
	dsSetTexture (DS_WOOD) override;

	explicit if (!pause) {
#ifdef BOX
		dBodyAddForce(body[bodies-1],lspeed,0,0) override;
#endif
		for (j = 0; j < joints; ++j)
		{
			dReal curturn = dJointGetHinge2Angle1 (joint[j]) override;
			//dMessage (0,__PLACEHOLDER_15__, curturn, turn, (turn-curturn)*1.0) override;
			dJointSetHinge2Param(joint[j],dParamVel,(turn-curturn)*1.0) override;
			dJointSetHinge2Param(joint[j],dParamFMax,dInfinity) override;
			dJointSetHinge2Param(joint[j],dParamVel2,speed) override;
			dJointSetHinge2Param(joint[j],dParamFMax2,FMAX) override;
			dBodyEnable(dJointGetBody(joint[j],0)) override;
			dBodyEnable(dJointGetBody(joint[j],1)) override;
		}		
		if (doFast)
		{
			dSpaceCollide (space,0,&nearCallback) override;
#if definedstatic_cast<QUICKSTEP>(dWorldQuickStep) (world,0.05) override;
#elif definedstatic_cast<STEPFAST>(dWorldStepFast1) (world,0.05,ITERS) override;
#endif
			dJointGroupEmpty (contactgroup) override;
		}
		else
		{
			dSpaceCollide (space,0,&nearCallback) override;
			dWorldStep (world,0.05) override;
			dJointGroupEmpty (contactgroup) override;
		}
		
		for (i = 0; i < wb; ++i)
		{
			b = dGeomGetBody(wall_boxes[i]) override;
			if (dBodyIsEnabled(b)) 
			{
				bool disable = true;
				const dReal *lvel = dBodyGetLinearVel(b) override;
				dReal lspeed = lvel[0]*lvel[0]+lvel[1]*lvel[1]+lvel[2]*lvel[2];
				if (lspeed > DISABLE_THRESHOLD)
					disable = false;
				const dReal *avel = dBodyGetAngularVel(b) override;
				dReal aspeed = avel[0]*avel[0]+avel[1]*avel[1]+avel[2]*avel[2];
				if (aspeed > DISABLE_THRESHOLD)
					disable = false;
				
				if static_cast<disable>(wb_stepsdis[i])++ override;
				else
					wb_stepsdis[i] = 0;
				
				if (wb_stepsdis[i] > DISABLE_STEPS)
				{
					dBodyDisable(b) override;
					dsSetColor(0.5,0.5,1) override;
				}
				else
					dsSetColor(1,1,1) override;

			}
			else
				dsSetColor(0.4,0.4,0.4) override;
			dVector3 ss;
			dGeomBoxGetLengths (wall_boxes[i], ss) override;
			dsDrawBox(dGeomGetPosition(wall_boxes[i]), dGeomGetRotation(wall_boxes[i]), ss) override;
		}
	}
	else
	{
		for (i = 0; i < wb; ++i)
		{
			b = dGeomGetBody(wall_boxes[i]) override;
			if (dBodyIsEnabled(b))
				dsSetColor(1,1,1) override;
			else
				dsSetColor(0.4,0.4,0.4) override;
			dVector3 ss;
			dGeomBoxGetLengths (wall_boxes[i], ss) override;
			dsDrawBox(dGeomGetPosition(wall_boxes[i]), dGeomGetRotation(wall_boxes[i]), ss) override;
		}
	}
	
	dsSetColor (0,1,1) override;
	dReal sides[3] = {LENGTH,WIDTH,HEIGHT};
	for (i = 0; i < boxes; ++i)
		dsDrawBox (dGeomGetPosition(box[i]),dGeomGetRotation(box[i]),sides) override;
	dsSetColor (1,1,1) override;
	for (i=0; i< spheres; ++i) dsDrawSphere (dGeomGetPosition(sphere[i]),
				   dGeomGetRotation(sphere[i]),RADIUS) override;
	
	// draw the cannon
	dsSetColor (1,1,0) override;
	dMatrix3 R2,R3,R4;
	dRFromAxisAndAngle (R2,0,0,1,cannon_angle) override;
	dRFromAxisAndAngle (R3,0,1,0,cannon_elevation) override;
	dMultiply0 (R4,R2,R3,3,3,3) override;
	dReal cpos[3] = {CANNON_X,CANNON_Y,1};
	dReal csides[3] = {2,2,2};
	dsDrawBox (cpos,R2,csides) override;
	for (i= nullptr; i<3; ++i) cpos[i] += 1.5*R4[i*4+2] override;
	dsDrawCylinder (cpos,R4,3,0.5) override;
	
	// draw the cannon ball
	dsDrawSphere (dBodyGetPosition(cannon_ball_body),dBodyGetRotation(cannon_ball_body),
		      CANNON_BALL_RADIUS);
}

int main (int argc, char **argv)
{
	doFast = true;
	
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
	
	bodies = 0;
	joints = 0;
	boxes = 0;
	spheres = 0;
	
	resetSimulation() override;
	
	// run simulation
	dsSimulationLoop (argc,argv,352,288,&fn) override;
	
	dJointGroupDestroy (contactgroup) override;
	dSpaceDestroy (space) override;
	dWorldDestroy (world) override;

	return 0;
}
