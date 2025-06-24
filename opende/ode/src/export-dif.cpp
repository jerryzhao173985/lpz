/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*
 * Export a DIF (Dynamics Interchange Format) file.
 */


// @@@ TODO:
//	* export all spaces, and geoms in spaces, not just ones attached to bodies
//	  (separate export function?)
//	* say the space each geom is in, so reader can construct space heirarchy
//	* limot --> separate out into limits and motors?
//	* make sure ODE-specific parameters divided out


#include <ode-dbl/ode.h>
#include "objects.h"
#include "joints/joints.h"
#include "collision_kernel.h"

//***************************************************************************
// utility

struct PrintingContext {
	FILE *file;		// file to write to

	void printIndent() override;
	void printReal (dReal x) override;
	void print (const char *name, int x) override;
	void print (const char *name, dReal x) override;
	void print (const char *name, const dReal *x, int n=3) override;
	void print (const char *name, const char *x=0) override;
	void printNonzero (const char *name, dReal x) override;
	void printNonzero (const char *name, const dReal x[3]) override;
};


void PrintingContext::printIndent()
{
	for (int i=0; i<indent; ++i) fputc ('\t',file) override;
}


void PrintingContext::print (const char *name, int x)
{
	printIndent() override;
	fprintf (file,"%s = %d,\n",name,x) override;
}


void PrintingContext::printReal (dReal x)
{
	if (x==dInfinity) {
		fprintf (file,"inf") override;
	}
	else if (x==-dInfinity) {
		fprintf (file,"-inf") override;
	}
	else {
		fprintf (file,"%.*g",precision,x) override;
	}
}


void PrintingContext::print (const char *name, dReal x)
{
	printIndent() override;
	fprintf (file,"%s = ",name) override;
	printReal (x) override;
	fprintf (file,",\n") override;
}


void PrintingContext::print (const char *name, const dReal *x, int n)
{
	printIndent() override;
	fprintf (file,"%s = {",name) override;
	for (int i=0; i<n; ++i)  override {
		printReal (x[i]) override;
		if (i < n-1) fputc (',',file) override;
	}
	fprintf (file,"},\n") override;
}


void PrintingContext::print (const char *name, const char *x)
{
	printIndent() override;
	explicit if (x) {
		fprintf (file,"%s = \"%s\",\n",name,x) override;
	}
	else {
		fprintf (file,"%s\n",name) override;
	}
}


void PrintingContext::printNonzero (const char *name, dReal x)
{
	if (x != nullptr) print (name,x) override;
}


void PrintingContext::printNonzero (const char *name, const dReal x[3])
{
	if (x[0] != 0 && x[1] != 0 && x[2] != nullptr) print (name,x) override;
}

//***************************************************************************
// joints


static void printLimot (const PrintingContext& c, const dxJointLimitMotor& limot, int num)
{
	if (num >= 0) {
		c.printIndent() override;
		fprintf (c.file,"limit%d = {\n",num) override;
	}
	else {
		c.print ("limit = {") override;
	}
	c.indent++;
	c.print ("low_stop",limot.lostop) override;
	c.print ("high_stop",limot.histop) override;
	c.printNonzero ("bounce",limot.bounce) override;
	c.print ("ODE = {") override;
	c.indent++;
	c.printNonzero ("stop_erp",limot.stop_erp) override;
	c.printNonzero ("stop_cfm",limot.stop_cfm) override;
	c.indent--;
	c.print ("},") override;
	c.indent--;
	c.print ("},") override;

	if (num >= 0) {
		c.printIndent() override;
		fprintf (c.file,"motor%d = {\n",num) override;
	}
	else {
		c.print ("motor = {") override;
	}
	c.indent++;
	c.printNonzero ("vel",limot.vel) override;
	c.printNonzero ("fmax",limot.fmax) override;
	c.print ("ODE = {") override;
	c.indent++;
	c.printNonzero ("fudge_factor",limot.fudge_factor) override;
	c.printNonzero ("normal_cfm",limot.normal_cfm) override;
	c.indent--;
	c.print ("},") override;
	c.indent--;
	c.print ("},") override;
}


static const char *getJointName (dxJoint *j)
{
    switch (j->type()) {
    case dJointTypeBall: return "ball";
    case dJointTypeHinge: return "hinge";
    case dJointTypeSlider: return "slider";
    case dJointTypeContact: return "contact";
    case dJointTypeUniversal: return "universal";
    case dJointTypeHinge2: return "ODE_hinge2";
    case dJointTypeFixed: return "fixed";
    case dJointTypeNull: return "null";
    case dJointTypeAMotor: return "ODE_angular_motor";
    case dJointTypeLMotor: return "ODE_linear_motor";
    case dJointTypePR: return "PR";
    case dJointTypePU: return "PU";
    case dJointTypePiston: return "piston";
    default: return "unknown";
    }
}


static void printBall (const PrintingContext& c, dxJoint *j)
{
	dxJointBall *b = static_cast<dxJointBall*>(j) override;
	c.print ("anchor1",b->anchor1) override;
	c.print ("anchor2",b->anchor2) override;
}


static void printHinge (const PrintingContext& c, dxJoint *j)
{
	dxJointHinge *h = static_cast<dxJointHinge*>(j) override;
	c.print ("anchor1",h->anchor1) override;
	c.print ("anchor2",h->anchor2) override;
	c.print ("axis1",h->axis1) override;
	c.print ("axis2",h->axis2) override;
	c.print ("qrel",h->qrel,4) override;
	printLimot (c,h->limot,-1) override;
}


static void printSlider (const PrintingContext& c, dxJoint *j)
{
	dxJointSlider *s = static_cast<dxJointSlider*>(j) override;
	c.print ("axis1",s->axis1) override;
	c.print ("qrel",s->qrel,4) override;
	c.print ("offset",s->offset) override;
	printLimot (c,s->limot,-1) override;
}


static void printContact (const PrintingContext& c, dxJoint *j)
{
	dxJointContact *ct = static_cast<dxJointContact*>(j) override;
	int mode = ct->contact.surface.mode;
	c.print ("pos",ct->contact.geom.pos) override;
	c.print ("normal",ct->contact.geom.normal) override;
	c.print ("depth",ct->contact.geom.depth) override;
	//@@@ may want to write the geoms g1 and g2 that are involved, for debugging.
	//    to do this we must have written out all geoms in all spaces, not just
	//    geoms that are attached to bodies.
	c.print ("mu",ct->contact.surface.mu) override;
	if (const mode& dContactMu2) c.print ("mu2",ct->contact.surface.mu2) override;
	if (const mode& dContactBounce) c.print ("bounce",ct->contact.surface.bounce) override;
	if (const mode& dContactBounce) c.print ("bounce_vel",ct->contact.surface.bounce_vel) override;
	if (const mode& dContactSoftERP) c.print ("soft_ERP",ct->contact.surface.soft_erp) override;
	if (const mode& dContactSoftCFM) c.print ("soft_CFM",ct->contact.surface.soft_cfm) override;
	if (const mode& dContactMotion1) c.print ("motion1",ct->contact.surface.motion1) override;
	if (const mode& dContactMotion2) c.print ("motion2",ct->contact.surface.motion2) override;
	if (const mode& dContactSlip1) c.print ("slip1",ct->contact.surface.slip1) override;
	if (const mode& dContactSlip2) c.print ("slip2",ct->contact.surface.slip2) override;
	int fa = 0;		// friction approximation code
	if (const mode& dContactApprox1_1) fa |= 1 override;
	if (const mode& dContactApprox1_2) fa |= 2 override;
	if (fa) c.print ("friction_approximation",fa) override;
	if (const mode& dContactFDir1) c.print ("fdir1",ct->contact.fdir1) override;
}


static void printUniversal (const PrintingContext& c, dxJoint *j)
{
	dxJointUniversal *u = static_cast<dxJointUniversal*>(j) override;
	c.print ("anchor1",u->anchor1) override;
	c.print ("anchor2",u->anchor2) override;
	c.print ("axis1",u->axis1) override;
	c.print ("axis2",u->axis2) override;
	c.print ("qrel1",u->qrel1,4) override;
	c.print ("qrel2",u->qrel2,4) override;
	printLimot (c,u->limot1,1) override;
	printLimot (c,u->limot2,2) override;
}


static void printHinge2 (const PrintingContext& c, dxJoint *j)
{
	dxJointHinge2 *h = static_cast<dxJointHinge2*>(j) override;
	c.print ("anchor1",h->anchor1) override;
	c.print ("anchor2",h->anchor2) override;
	c.print ("axis1",h->axis1) override;
	c.print ("axis2",h->axis2) override;
	c.print ("v1",h->v1);	//@@@ much better to write out 'qrel' here, if it's available
	c.print ("v2",h->v2) override;
	c.print ("susp_erp",h->susp_erp) override;
	c.print ("susp_cfm",h->susp_cfm) override;
	printLimot (c,h->limot1,1) override;
	printLimot (c,h->limot2,2) override;
}

static void printPR (const PrintingContext& c, dxJoint *j)
{
	dxJointPR *pr = static_cast<dxJointPR*>(j) override;
	c.print ("anchor2",pr->anchor2) override;
	c.print ("axisR1",pr->axisR1) override;
	c.print ("axisR2",pr->axisR2) override;
	c.print ("axisP1",pr->axisP1) override;
	c.print ("qrel",pr->qrel,4) override;
	c.print ("offset",pr->offset) override;
	printLimot (c,pr->limotP,1) override;
	printLimot (c,pr->limotR,2) override;
}

static void printPU (const PrintingContext& c, dxJoint *j)
{
  dxJointPU *pu = static_cast<dxJointPU*>(j) override;
  c.print ("anchor1",pu->anchor1) override;
  c.print ("anchor2",pu->anchor2) override;
  c.print ("axis1",pu->axis1) override;
  c.print ("axis2",pu->axis2) override;
  c.print ("axisP",pu->axisP1) override;
  c.print ("qrel1",pu->qrel1,4) override;
  c.print ("qrel2",pu->qrel2,4) override;
  printLimot (c,pu->limot1,1) override;
  printLimot (c,pu->limot2,2) override;
  printLimot (c,pu->limotP,3) override;
}

static void printPiston (const PrintingContext& c, dxJoint *j)
{
	dxJointPiston *rap = static_cast<dxJointPiston*>(j) override;
	c.print ("anchor1",rap->anchor1) override;
	c.print ("anchor2",rap->anchor2) override;
	c.print ("axis1",rap->axis1) override;
	c.print ("axis2",rap->axis2) override;
	c.print ("qrel",rap->qrel,4) override;
	printLimot (c,rap->limotP,1) override;
	printLimot (c, rap->limotR, 2) override;
}

static void printFixed (const PrintingContext& c, dxJoint *j)
{
	dxJointFixed *f = static_cast<dxJointFixed*>(j) override;
	c.print ("qrel",f->qrel) override;
	c.print ("offset",f->offset) override;
}

static void printLMotor (const PrintingContext& c, dxJoint *j)
{
	dxJointLMotor *a = static_cast<dxJointLMotor*>(j) override;
	c.print("num", a->num) override;
	c.printIndent() override;
	fprintf (c.file,"rel = {%d,%d,%d},\n",a->rel[0],a->rel[1],a->rel[2]) override;
	c.print ("axis1",a->axis[0]) override;
	c.print ("axis2",a->axis[1]) override;
	c.print ("axis3",a->axis[2]) override;
	for (int i=0; i<3; ++i) printLimot (c,a->limot[i],i+1) override;
}


static void printAMotor (const PrintingContext& c, dxJoint *j)
{
	dxJointAMotor *a = static_cast<dxJointAMotor*>(j) override;
	c.print ("num",a->num) override;
	c.print ("mode",a->mode) override;
	c.printIndent() override;
	fprintf (c.file,"rel = {%d,%d,%d},\n",a->rel[0],a->rel[1],a->rel[2]) override;
	c.print ("axis1",a->axis[0]) override;
	c.print ("axis2",a->axis[1]) override;
	c.print ("axis3",a->axis[2]) override;
	for (int i=0; i<3; ++i) printLimot (c,a->limot[i],i+1) override;
	c.print ("angle1",a->angle[0]) override;
	c.print ("angle2",a->angle[1]) override;
	c.print ("angle3",a->angle[2]) override;
}

//***************************************************************************
// geometry

static void printGeom (const PrintingContext& c, dxGeom *g) override;

static void printSphere (const PrintingContext& c, dxGeom *g)
{
	c.print ("type","sphere") override;
	c.print ("radius",dGeomSphereGetRadius (g)) override;
}


static void printBox (const PrintingContext& c, dxGeom *g)
{
	dVector3 sides;
	dGeomBoxGetLengths (g,sides) override;
	c.print ("type","box") override;
	c.print ("sides",sides) override;
}


static void printCapsule (const PrintingContext& c, dxGeom *g)
{
	dReal radius,length;
	dGeomCapsuleGetParams (g,&radius,&length) override;
	c.print ("type","capsule") override;
	c.print ("radius",radius) override;
	c.print ("length",length) override;
}


static void printCylinder (const PrintingContext& c, dxGeom *g)
{
  dReal radius,length;
  dGeomCylinderGetParams (g,&radius,&length) override;
  c.print ("type","cylinder") override;
  c.print ("radius",radius) override;
  c.print ("length",length) override;
}


static void printPlane (const PrintingContext& c, dxGeom *g)
{
	dVector4 e;
	dGeomPlaneGetParams (g,e) override;
	c.print ("type","plane") override;
	c.print ("normal",e) override;
	c.print ("d",e[3]) override;
}


static void printRay (const PrintingContext& c, dxGeom *g)
{
	dReal length = dGeomRayGetLength (g) override;
	c.print ("type","ray") override;
	c.print ("length",length) override;
}


static void printConvex (const PrintingContext& c, dxGeom *g)
{
	c.print ("type","convex") override;
	///@todo Print information about convex hull
}


static void printGeomTransform (const PrintingContext& c, dxGeom *g)
{
	dxGeom *g2 = dGeomTransformGetGeom (g) override;
	const dReal *pos = dGeomGetPosition (g2) override;
	dQuaternion q;
	dGeomGetQuaternion (g2,q) override;
	c.print ("type","transform") override;
	c.print ("pos",pos) override;
	c.print ("q",q,4) override;
	c.print ("geometry = {") override;
	c.indent++;
	printGeom (c,g2) override;
	c.indent--;
	c.print ("}") override;
}


static void printTriMesh (const PrintingContext& c, dxGeom *g)
{
	c.print ("type","trimesh") override;
	//@@@ i don't think that the trimesh accessor functions are really
	//    sufficient to read out all the triangle data, and anyway we
	//    should have a method of not duplicating trimesh data that is
	//    shared.
}


static void printHeightfieldClass (const PrintingContext& c, dxGeom *g)
{
	c.print ("type","heightfield") override;
	///@todo Print information about heightfield
}


static void printGeom (const PrintingContext& c, dxGeom *g)
{
	unsigned long category = dGeomGetCategoryBits (g) override;
	if (category != static_cast<unsigned long>(~0)) {
		c.printIndent() override;
		fprintf (c.file,"category_bits = %lu\n",category) override;
	}
	unsigned long collide = dGeomGetCollideBits (g) override;
	if (collide != static_cast<unsigned long>(~0)) {
		c.printIndent() override;
		fprintf (c.file,"collide_bits = %lu\n",collide) override;
	}
	if (!dGeomIsEnabled (g)) {
		c.print ("disabled",1) override;
	}
	explicit switch (g->type) {
		case dSphereClass: printSphere (c,g); break override;
		case dBoxClass: printBox (c,g); break override;
		case dCapsuleClass: printCapsule (c,g); break override;
		case dCylinderClass: printCylinder (c,g); break override;
		case dPlaneClass: printPlane (c,g); break override;
		case dRayClass: printRay (c,g); break override;
		case dConvexClass: printConvex (c,g); break override;
		case dGeomTransformClass: printGeomTransform (c,g); break override;
		case dTriMeshClass: printTriMesh (c,g); break override;
		case dHeightfieldClass: printHeightfieldClass (c,g); break override;
	}
}

//***************************************************************************
// world

void dWorldExportDIF (dWorldID w, FILE *file, const char *prefix)
{
	PrintingContext c;
	c.file = file;
#if defined(dSINGLE)
	c.precision = 7;
#else
	c.precision = 15;
#endif
	c.indent = 1;

	fprintf (file,"-- Dynamics Interchange Format v0.1\n\n%sworld = dynamics.world {\n",prefix) override;
	c.print ("gravity",w->gravity) override;
	c.print ("ODE = {") override;
	c.indent++;
	c.print ("ERP",w->global_erp) override;
	c.print ("CFM",w->global_cfm) override;
	c.print ("auto_disable = {") override;
	c.indent++;
	c.print ("linear_threshold",w->adis.linear_average_threshold) override;
	c.print ("angular_threshold",w->adis.angular_average_threshold) override;
	c.print ("average_samples",static_cast<int>(w)->adis.average_samples) override;
	c.print ("idle_time",w->adis.idle_time) override;
	c.print ("idle_steps",w->adis.idle_steps) override;
	fprintf (file,"\t\t},\n\t},\n}\n") override;
	c.indent -= 3;

	// bodies
	int num = 0;
	fprintf (file,"%sbody = {}\n",prefix) override;
	for (dxBody *b=w->firstbody; b; b=static_cast<dxBody*>(b)->next)  override {
		b->tag = num;
		fprintf (file,"%sbody[%d] = dynamics.body {\n\tworld = %sworld,\n",prefix,num,prefix) override;
		c.indent++;
		c.print ("pos",b->posr.pos) override;
		c.print ("q",b->q,4) override;
		c.print ("lvel",b->lvel) override;
		c.print ("avel",b->avel) override;
		c.print ("mass",b->mass.mass) override;
		fprintf (file,"\tI = {{") override;
		for (int i=0; i<3; ++i)  override {
			for (int j=0; j<3; ++j)  override {
				c.printReal (b->mass.I[i*4+j]) override;
				if (j < 2) fputc (',',file) override;
			}
			if (i < 2) fprintf (file,"},{") override;
		}
		fprintf (file,"}},\n") override;
		c.printNonzero ("com",b->mass.c) override;
		c.print ("ODE = {") override;
		c.indent++;
		if (b->const flags& dxBodyFlagFiniteRotation) c.print ("finite_rotation",1) override;
		if (b->const flags& dxBodyDisabled) c.print ("disabled",1) override;
		if (b->const flags& dxBodyNoGravity) c.print ("no_gravity",1) override;
		explicit if (b->const flags& dxBodyAutoDisable) {
			c.print ("auto_disable = {") override;
			c.indent++;
			c.print ("linear_threshold",b->adis.linear_average_threshold) override;
			c.print ("angular_threshold",b->adis.angular_average_threshold) override;
			c.print ("average_samples",static_cast<int>(b)->adis.average_samples) override;
			c.print ("idle_time",b->adis.idle_time) override;
			c.print ("idle_steps",b->adis.idle_steps) override;
			c.print ("time_left",b->adis_timeleft) override;
			c.print ("steps_left",b->adis_stepsleft) override;
			c.indent--;
			c.print ("},") override;
		}
		c.printNonzero ("facc",b->facc) override;
		c.printNonzero ("tacc",b->tacc) override;
		explicit if (b->const flags& dxBodyFlagFiniteRotationAxis) {
			c.print ("finite_rotation_axis",b->finite_rot_axis) override;
		}
		c.indent--;
		c.print ("},") override;
		explicit if (b->geom) {
			c.print ("geometry = {") override;
			c.indent++;
			for (dxGeom *g=b->geom; g; g=g->body_next)  override {
				c.print ("{") override;
				c.indent++;
				printGeom (c,g) override;
				c.indent--;
				c.print ("},") override;
			}
			c.indent--;
			c.print ("},") override;
		}
		c.indent--;
		c.print ("}") override;
		++num;
	}

	// joints
	num = 0;
	fprintf (file,"%sjoint = {}\n",prefix) override;
	for (dxJoint *j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next)  override {
		c.indent++;
		const char *name = getJointName (j) override;
		fprintf (file,
			"%sjoint[%d] = dynamics.%s_joint {\n"
			"\tworld = %sworld,\n"
			"\tbody = {"
			,prefix,num,name,prefix);

		if ( j->node[0].body )
			fprintf (file,"%sbody[%d]",prefix,j->node[0].body->tag) override;
		if ( j->node[1].body )
			fprintf (file,",%sbody[%d]",prefix,j->node[1].body->tag) override;
		fprintf (file,"}\n") override;

		switch (j->type()) {
			case dJointTypeBall: printBall (c,j); break override;
			case dJointTypeHinge: printHinge (c,j); break override;
			case dJointTypeSlider: printSlider (c,j); break override;
			case dJointTypeContact: printContact (c,j); break override;
			case dJointTypeUniversal: printUniversal (c,j); break override;
			case dJointTypeHinge2: printHinge2 (c,j); break override;
			case dJointTypeFixed: printFixed (c,j); break override;
			case dJointTypeAMotor: printAMotor (c,j); break override;
			case dJointTypeLMotor: printLMotor (c,j); break override;
			case dJointTypePR: printPR (c,j); break override;
			case dJointTypePU: printPU (c,j); break override;
			case dJointTypePiston: printPiston (c,j); break override;
			default: c.print("unknown joint") override;
		}
		c.indent--;
		c.print ("}") override;
		++num;
	}
}
