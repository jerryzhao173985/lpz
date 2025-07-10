#ifndef _ODE_ODE_H_
#define _ODE_ODE_H_

/* 
 * Basic ODE header for bundled builds
 * This includes the core ODE functionality needed by LPZRobots
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Basic definitions */
#ifndef dDOUBLE
#define dDOUBLE
#endif

/* Basic types */
#ifdef dDOUBLE
typedef double dReal;
#else
typedef float dReal;
#endif

typedef dReal dVector3[4];
typedef dReal dVector4[4];
typedef dReal dMatrix3[4*3];
typedef dReal dMatrix4[4*4];
typedef dReal dMatrix6[8*6];
typedef dReal dQuaternion[4];

/* Basic constants */
#define dInfinity 1e20

/* Forward declarations for key structures */
typedef struct dxWorld *dWorldID;
typedef struct dxSpace *dSpaceID;
typedef struct dxBody *dBodyID;
typedef struct dxGeom *dGeomID;
typedef struct dxJoint *dJointID;
typedef struct dxJointGroup *dJointGroupID;

/* Contact structure */
typedef struct dContact {
    struct {
        dReal mu;        /* Coulomb friction coefficient */
        dReal mu2;       /* optional Coulomb friction coefficient for second friction direction */
        dReal bounce;    /* bounces parameter */
        dReal bounce_vel; /* minimum velocity for bounce */
        dReal soft_erp;  /* soft error reduction parameter */
        dReal soft_cfm;  /* soft constraint force mixing parameter */
        dReal motion1, motion2; /* surface velocity for two directions */
        dReal slip1, slip2;     /* slip velocity for two directions */
    } surface;
    dVector3 fdir1;      /* first friction direction vector */
    dVector3 pos;        /* contact position */
    dVector3 normal;     /* normal vector */
    dReal depth;         /* penetration depth */
    dGeomID g1, g2;      /* the colliding geoms */
    int side1, side2;    /* (to be documented) */
} dContact;

/* Basic world functions */
dWorldID dWorldCreate(void);
void dWorldDestroy(dWorldID world);
void dWorldSetGravity(dWorldID, dReal x, dReal y, dReal z);
void dWorldGetGravity(dWorldID, dVector3 gravity);
void dWorldStep(dWorldID, dReal stepsize);
void dWorldQuickStep(dWorldID, dReal stepsize);

/* Basic body functions */
dBodyID dBodyCreate(dWorldID);
void dBodyDestroy(dBodyID);
void dBodySetPosition(dBodyID, dReal x, dReal y, dReal z);
void dBodySetRotation(dBodyID, const dMatrix3 R);
void dBodySetLinearVel(dBodyID, dReal x, dReal y, dReal z);
void dBodySetAngularVel(dBodyID, dReal x, dReal y, dReal z);
void dBodyGetPosition(dBodyID, dVector3 result);
void dBodyGetRotation(dBodyID, dMatrix3 result);
void dBodyGetLinearVel(dBodyID, dVector3 result);
void dBodyGetAngularVel(dBodyID, dVector3 result);
void dBodySetMass(dBodyID, const struct dMass *mass);
void dBodyGetMass(dBodyID, struct dMass *mass);
void dBodyAddForce(dBodyID, dReal fx, dReal fy, dReal fz);
void dBodyAddTorque(dBodyID, dReal fx, dReal fy, dReal fz);

/* Basic collision functions */
dSpaceID dSimpleSpaceCreate(dSpaceID space);
dSpaceID dHashSpaceCreate(dSpaceID space);
void dSpaceDestroy(dSpaceID);
void dSpaceAdd(dSpaceID, dGeomID);
void dSpaceRemove(dSpaceID, dGeomID);
int dCollide(dGeomID o1, dGeomID o2, int flags, dContact *contact, int skip);
void dSpaceCollide(dSpaceID space, void *data, void (*callback)(void *data, dGeomID o1, dGeomID o2));

/* Basic geometry functions */
dGeomID dCreateSphere(dSpaceID space, dReal radius);
dGeomID dCreateBox(dSpaceID space, dReal lx, dReal ly, dReal lz);
dGeomID dCreateCylinder(dSpaceID space, dReal radius, dReal length);
dGeomID dCreateCapsule(dSpaceID space, dReal radius, dReal length);
dGeomID dCreatePlane(dSpaceID space, dReal a, dReal b, dReal c, dReal d);
void dGeomDestroy(dGeomID geom);
void dGeomSetBody(dGeomID geom, dBodyID body);
dBodyID dGeomGetBody(dGeomID geom);
void dGeomSetPosition(dGeomID geom, dReal x, dReal y, dReal z);
void dGeomSetRotation(dGeomID geom, const dMatrix3 R);
void dGeomGetPosition(dGeomID geom, dVector3 result);
void dGeomGetRotation(dGeomID geom, dMatrix3 result);

/* Basic joint functions */
dJointGroupID dJointGroupCreate(int max_size);
void dJointGroupDestroy(dJointGroupID);
void dJointGroupEmpty(dJointGroupID);
dJointID dJointCreateContact(dWorldID, dJointGroupID, const dContact *);
dJointID dJointCreateHinge(dWorldID, dJointGroupID);
dJointID dJointCreateSlider(dWorldID, dJointGroupID);
dJointID dJointCreateBall(dWorldID, dJointGroupID);
void dJointDestroy(dJointID);
void dJointAttach(dJointID, dBodyID body1, dBodyID body2);

/* Mass structure and functions */
typedef struct dMass {
    dReal mass;
    dVector3 c;
    dMatrix3 I;
} dMass;

void dMassSetZero(dMass *);
void dMassSetSphere(dMass *, dReal density, dReal radius);
void dMassSetBox(dMass *, dReal density, dReal lx, dReal ly, dReal lz);
void dMassSetCylinder(dMass *, dReal density, int direction, dReal radius, dReal length);

/* Basic math functions */
void dRSetIdentity(dMatrix3 R);
void dRFromAxisAndAngle(dMatrix3 R, dReal ax, dReal ay, dReal az, dReal angle);
void dRFromEulerAngles(dMatrix3 R, dReal phi, dReal theta, dReal psi);

/* Initialization and cleanup */
void dInitODE(void);
void dCloseODE(void);

#ifdef __cplusplus
}
#endif

#endif /* _ODE_ODE_H_ */