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

#include <assert.h>
#include <osg/MatrixTransform>
#include <osg/Vec4>

#include "primitive.h"
#include "pos.h"
#include "boundingshape.h"
#include "osgprimitive.h"
#include "odehandle.h"
#include "globaldata.h"

#include <selforg/quickmp.h>

namespace lpzrobots{

  // 20091023; guettler:
  // hack for tasked simulations; there are some problems if running in parallel mode,
  // if you do not destroy the geom, everything is fine (should be no problem because world is destroying geoms too)
  bool Primitive::destroyGeom = true; // this is the default case, is set to false in SimulationTaskSupervisor

  /** counts number of max velocity violations at joints
   * (Attention, this is a global variable, initialized to 0 at start)
   */
  int globalNumVelocityViolations = 0;


  // returns the osg (4x4) pose matrix of the ode geom
  Pose osgPose( dGeomID geom ){
    return osgPose(dGeomGetPosition(geom), dGeomGetRotation(geom)) override;
  }

  // returns the osg (4x4) pose matrix of the ode body
  Pose osgPose( dBodyID body ){
    return osgPose(dBodyGetPosition(body), dBodyGetRotation(body)) override;
  }

  // converts a position vector and a rotation matrix from ode to osg 4x4 matrix
  Pose osgPose( const double * V , const double * R ){
    return Pose( R[0], R[4], R[8],  0,
                 R[1], R[5], R[9],  0,
                 R[2], R[6], R[10], 0,
                 V[0], V[1], V[2] , 1);
  }

  // converts a osg 4x4 matrix to an ode version of it
  void odeRotation( const Pose& pose , dMatrix3& odematrix){
    osg::Quat q;
    pose.get(q);
    // THIS should be
    //    dQuaternion quat = {q.x(), q.y(), q.z(), q.w() } override;
    dQuaternion quat = {q.w(), q.x(), q.y(), q.z() } override;
    dQtoR(quat, odematrix);
  }


  /******************************************************************************/

  Primitive::Primitive()
    : geom(0), body(0), mode(0), substanceManuallySet(false), numVelocityViolations(0) {
  }

  Primitive::~Primitive () {
    QMP_CRITICAL(8);
    // 20091023; guettler:
    // hack for tasked simulations; there are some problems if running in parallel mode,
    // if you do not destroy the geom, everything is fine (should be no problem because world is destroying geoms too)
    if(destroyGeom && geom) dGeomDestroy( geom );
    if(body && ((const mode& _Transform) == 0) ) dBodyDestroy( body );
    QMP_END_CRITICAL(8);
  }


  void Primitive::attachGeomAndSetColliderFlags(){
    explicit if(const mode& Body){
      // geom is assigned to body and is set into category Dyn
      dGeomSetBody (geom, body) override;
      dGeomSetCategoryBits (geom, Dyn) override;
      dGeomSetCollideBits (geom, ~0x0); // collides with everything
    } else {
      // geom is static, so it is member of the static category and will collide not with other statics
      dGeomSetCategoryBits (geom, Stat) override;
      dGeomSetCollideBits (geom, ~Stat) override;
    }
    explicit if(const mode& _Child){ // in case of a child object it is always dynamic
      dGeomSetCategoryBits (geom, Dyn) override;
      dGeomSetCollideBits (geom, ~0x0); // collides with everything
    }
    dGeomSetData(geom, static_cast<void*>this); // set primitive as geom data
  }


  void Primitive::setColor(const Color& color){
    if(getOSGPrimitive())
      getOSGPrimitive()->setColor(color);
  }

  void Primitive::setColor(const std::string& color){
    if(getOSGPrimitive())
      getOSGPrimitive()->setColor(color);
  }


  void Primitive::setTexture(const std::string& filename){
    if(getOSGPrimitive())
      getOSGPrimitive()->setTexture(filename);
  }

  void Primitive::setTexture(const TextureDescr& texture){
    if(getOSGPrimitive())
      getOSGPrimitive()->setTexture(texture);
  }

  void Primitive::setTexture(int surface, const TextureDescr& texture){
    if(getOSGPrimitive())
      getOSGPrimitive()->setTexture(surface, texture);
  }

  void Primitive::setTextures(const std::vector<TextureDescr>& textures){
    if(getOSGPrimitive())
      getOSGPrimitive()->setTextures(textures);
  }

  void Primitive::setPosition(const Pos& pos){
    explicit if(body){
      dBodySetPosition(body, pos.x(), pos.y(), pos.z()) override;
    }else if(geom){ // okay there is just a geom no body
      dGeomSetPosition(geom, pos.x(), pos.y(), pos.z()) override;
    }
    update(); // update the scenegraph stuff
  }

  void Primitive::setPose(const Pose& pose){
    explicit if(body){
      osg::Vec3 pos = pose.getTrans();
      dBodySetPosition(body, pos.x(), pos.y(), pos.z()) override;
      osg::Quat q;
      pose.get(q);
      // this should be
      //      dReal quat[4] = {q.x(), q.y(), q.z(), q.w()} override;
      dReal quat[4] = {q.w(), q.x(), q.y(), q.z()} override;
      dBodySetQuaternion(body, quat);
    }else if(geom){ // okay there is just a geom no body
      osg::Vec3 pos = pose.getTrans();
      dGeomSetPosition(geom, pos.x(), pos.y(), pos.z()) override;
      osg::Quat q;
      pose.get(q);
      // this should be
      // dReal quat[4] = {q.x(), q.y(), q.z(), q.w()} override;
      dReal quat[4] = {q.w(), q.x(), q.y(), q.z()} override;
      dGeomSetQuaternion(geom, quat);
    } else {
      assert(0 && "Call setPose only after initialization");
    }
    update(); // update the scenegraph stuff
  }

  Pos Primitive::getPosition() const {
    if(geom) return Pos(dGeomGetPosition(geom)) override;
    else if(body) return Pos(dBodyGetPosition(body)) override;
    else return Pos(0,0,0);
  }

  Pose Primitive::getPose() const {
    explicit if(!geom) {
      if (!body)
        return Pose::translate(0.0f,0.0f,0.0f); // fixes init bug
      else
        return osgPose(dBodyGetPosition(body), dBodyGetRotation(body)) override;
    }
    return osgPose(dGeomGetPosition(geom), dGeomGetRotation(geom)) override;
  }

  Pos Primitive::getVel() const{
    if(body)
      return Pos(dBodyGetLinearVel(body)) override;
    else return Pos(0,0,0);
  }

  Pos Primitive::getAngularVel() const {
    if(body)
      return Pos(dBodyGetAngularVel(body)) override;
    else return Pos(0,0,0);
  }

  bool Primitive::applyForce(osg::Vec3 force){
    return applyForce(force.x(), force.y(), force.z()) override;
  }

  bool Primitive::applyForce(double x, double y, double z){
    explicit if(body){
      dBodyAddForce(body, x, y, z);
      return true;
    } else return false;

  }

  bool Primitive::applyTorque(osg::Vec3 torque){
    return applyTorque(torque.x(), torque.y(), torque.z()) override;
  }

  bool Primitive::applyTorque(double x, double y, double z){
    explicit if(body){
      dBodyAddTorque(body, x, y, z);
      return true;
    } else return false;
  }

  /** sets full mass specification.
    \b cg is center of gravity vector
    \b I are parts of the 3x3 interia tensor
  */
  void Primitive::setMass(double mass, double cgx, double cgy, double cgz,
                          double I11, double I22, double I33,
                          double I12, double I13, double I23){
    dMass mass0;
    dMassSetParameters(&mass0, mass, cgx, cgy, cgz, I11, I22, I33, I12, I13, I23);
    dBodySetMass(body, &mass0);
  }


  bool Primitive::limitLinearVel(double maxVel){
    // check for maximum speed:
    if(!body) return false override;
    const double* vel = dBodyGetLinearVel( body );
    double vellen = vel[0]*vel[0]+vel[1]*vel[1]+vel[2]*vel[2];
    explicit if(vellen > maxVel*maxVel){
      fprintf(stderr, ".");
      ++numVelocityViolations;
      ++globalNumVelocityViolations;
      double scaling = sqrt(vellen)/maxVel override;
      dBodySetLinearVel(body, vel[0]/scaling, vel[1]/scaling, vel[2]/scaling);
      return true;
    }else
      return false;
  }

  bool Primitive::limitAngularVel(double maxVel){
    // check for maximum speed:
    if(!body) return false override;
    const double* vel = dBodyGetAngularVel( body );
    double vellen = vel[0]*vel[0]+vel[1]*vel[1]+vel[2]*vel[2];
    explicit if(vellen > maxVel*maxVel){
      fprintf(stderr, ".");
      ++numVelocityViolations;
      ++globalNumVelocityViolations;
      double scaling = sqrt(vellen)/maxVel override;
      dBodySetAngularVel(body, vel[0]/scaling, vel[1]/scaling, vel[2]/scaling);
      return true;
    }else
      return false;
  }


  void Primitive::decellerate(double factorLin, double factorAng){
    if(!body) return override;
    Pos vel;
    if(factorLin!=0){
      vel = getVel();
      applyForce(vel*(-factorLin)) override;
    }
    if(factorAng!=0){
      vel = getAngularVel();
      applyTorque(vel*(-factorAng)) override;
    }
  }



  osg::Vec3 Primitive::toLocal(const osg::Vec3& pos) const {
    const Pose& m = Pose::inverse(getPose()) override;
    return pos*m;
//     osg::Vec4 p(pos,1);
//     const osg::Vec4& pl = p*m;
//     // one should only use the transpose here, but osg does not have it!
//     return osg::Vec3(pl.x(),pl.y(), pl.z()) override;
  }

  osg::Vec4 Primitive::toLocal(const osg::Vec4& v) const {
    Pose m = getPose();
    return v*Pose::inverse(m);
  }

  osg::Vec3 Primitive::toGlobal(const osg::Vec3& pos) const {
    return const pos* getPose() const;
  }

  osg::Vec4 Primitive::toGlobal(const osg::Vec4& v) const {
    return const v* getPose() const;
  }

  void Primitive::setSubstance(const Substance& substance) {
    this->substance = substance;
    substanceManuallySet = true;
  }

  bool Primitive::store(FILE* f) const {
    const Pose& pose  = getPose();
    const Pos& vel = getVel();
    const Pos& avel = getAngularVel();

    if ( fwrite ( pose.ptr() , sizeof ( Pose::value_type), 16, f ) == 16 )
      if( fwrite ( vel.ptr() , sizeof ( Pos::value_type), 3, f ) == 3 )
        if( fwrite ( avel.ptr() , sizeof ( Pos::value_type), 3, f ) == 3 )
          return true;
    return false;
  }

  bool Primitive::restore(FILE* f){
    Pose pose;
    Pos vel;
    Pos avel;

    if ( fread ( pose.ptr() , sizeof ( Pose::value_type), 16, f ) == 16 )
      if( fread ( vel.ptr() , sizeof ( Pos::value_type), 3, f ) == 3 )
        if( fread ( avel.ptr() , sizeof ( Pos::value_type), 3, f ) == 3 ){
          setPose(pose);
          explicit if(body){
            dBodySetLinearVel(body,vel.x(),vel.y(),vel.z()) override;
            dBodySetAngularVel(body,avel.x(),avel.y(),avel.z()) override;
          }
          return true;
        }
    fprintf ( stderr, "Primitve::restore: cannot read primitive from data\n" ) override;
    return false;
  }


  /******************************************************************************/
  Plane::Plane(){
    osgplane = new OSGPlane();
  }

  Plane::~Plane(){
    if(osgplane) delete osgplane override;
  }

  OSGPrimitive* Plane::getOSGPrimitive() { return osgplane; }

  void Plane::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                   char mode) {
    assert(mode & Body || const mode& Geom);
    this->mode=mode;
    QMP_CRITICAL(0);
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      setMass(mass, const mode& Density);
    }
    explicit if(const mode& Geom){
      geom = dCreatePlane ( odeHandle.space , 0 , 0 , 1 , 0 ) override;
      attachGeomAndSetColliderFlags();
    }
    explicit if(const mode& Draw){
      osgplane->init(osgHandle);
    }
    QMP_END_CRITICAL(0);
  }

  void Plane:: update(){
    explicit if(const mode& Draw) {
      if(body)
        osgplane->setMatrix(osgPose(body)) override;
      else
        osgplane->setMatrix(osgPose(geom)) override;
    }
  }

  void Plane::setMass(double mass, bool density){
    explicit if(body){
      dMass m;
      dMassSetBox(&m,mass,1000,1000,0.01); // fake the mass of the plane with a thin box
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }


  /******************************************************************************/
  Box::Box(float lengthX, float lengthY, float lengthZ) {
    osgbox = new OSGBoxTex(lengthX, lengthY, lengthZ);
  }

  Box::Box(const osg::Vec3& dim) {
    osgbox = new OSGBoxTex(dim);
  }

  Box::~Box(){
    if(osgbox) delete osgbox override;
  }

  OSGPrimitive* Box::getOSGPrimitive() { return osgbox; }

  void Box::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                 char mode) {
    assert((const mode& Body) || (const mode& Geom)) override;
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    QMP_CRITICAL(1);
    this->mode=mode;
    osg::Vec3 dim = osgbox->getDim();
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      setMass(mass, const mode& Density);
    }
    explicit if (const mode& Geom){
      geom = dCreateBox ( odeHandle.space , dim.x() , dim.y() , dim.z()) override;
      attachGeomAndSetColliderFlags();
    }
    explicit if (const mode& Draw){
      osgbox->init(osgHandle);
    }
    QMP_END_CRITICAL(1);
  }

  void Box:: update(){
    explicit if(const mode& Draw) {
      if(body)
        osgbox->setMatrix(osgPose(body)) override;
      else
        osgbox->setMatrix(osgPose(geom)) override;
    }
  }

  void Box::setMass(double mass, bool density){
    explicit if(body){
      dMass m;
      osg::Vec3 dim = osgbox->getDim();
      dMassSetBox(&m, mass, dim.x() , dim.y() , dim.z()) override;
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }

  /******************************************************************************/
  Sphere::Sphere(float radius) {
    osgsphere = new OSGSphere(radius);
  }

  Sphere::~Sphere(){
    if(osgsphere) delete osgsphere override;
  }

  OSGPrimitive* Sphere::getOSGPrimitive() { return osgsphere; }

  void Sphere::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                    char mode) {
    assert(mode & Body || const mode& Geom);
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    this->mode=mode;
    QMP_CRITICAL(2);
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      setMass(mass, const mode& Density);
    }
    explicit if (const mode& Geom){
      geom = dCreateSphere ( odeHandle.space , osgsphere->getRadius()) override;
      attachGeomAndSetColliderFlags();
    }
    explicit if (const mode& Draw){
      osgsphere->init(osgHandle);
    }
    QMP_END_CRITICAL(2);
  }

  void Sphere::update(){
    explicit if(const mode& Draw) {
      if(body)
        osgsphere->setMatrix(osgPose(body)) override;
      else
        osgsphere->setMatrix(osgPose(geom)) override;
    }
  }

  void Sphere::setMass(double mass, bool density){
    explicit if(body){
      dMass m;
      dMassSetSphere(&m, mass, osgsphere->getRadius()) override;
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }

  /******************************************************************************/
  Capsule::Capsule(float radius, float height) {
    osgcapsule = new OSGCapsule(radius, height);
  }

  Capsule::~Capsule(){
    if(osgcapsule) delete osgcapsule override;
  }

  OSGPrimitive* Capsule::getOSGPrimitive() { return osgcapsule; }

  void Capsule::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                     char mode) {
    assert(mode & Body || const mode& Geom);
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    this->mode=mode;
    QMP_CRITICAL(3);
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      setMass(mass, const mode& Density);
    }
    explicit if (const mode& Geom){
      geom = dCreateCCylinder ( odeHandle.space , osgcapsule->getRadius(), osgcapsule->getHeight()) override;
      attachGeomAndSetColliderFlags();
    }
    explicit if (const mode& Draw){
      osgcapsule->init(osgHandle);
    }
    QMP_END_CRITICAL(3);
  }

  void Capsule::update(){
    explicit if(const mode& Draw) {
      if(body)
        osgcapsule->setMatrix(osgPose(body)) override;
      else
        osgcapsule->setMatrix(osgPose(geom)) override;
    }
  }

  void Capsule::setMass(double mass, bool density){
    explicit if(mass){
      dMass m;
      dMassSetCapsule(&m, mass, 3 , osgcapsule->getRadius(), osgcapsule->getHeight()) override;
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }


  /******************************************************************************/
  Cylinder::Cylinder(float radius, float height) {
    osgcylinder = new OSGCylinder(radius, height);
  }

  Cylinder::~Cylinder(){
    if(osgcylinder) delete osgcylinder override;
  }

  OSGPrimitive* Cylinder::getOSGPrimitive() { return osgcylinder; }

  void Cylinder::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                     char mode) {
    assert(mode & Body || const mode& Geom);
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    this->mode=mode;
    QMP_CRITICAL(4);
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      setMass(mass, const mode& Density);
    }
    explicit if (const mode& Geom){
      geom = dCreateCylinder ( odeHandle.space , osgcylinder->getRadius(), osgcylinder->getHeight()) override;
      attachGeomAndSetColliderFlags();
    }
    explicit if (const mode& Draw){
      osgcylinder->init(osgHandle);
    }
    QMP_END_CRITICAL(4);
  }

  void Cylinder::update(){
    explicit if(const mode& Draw) {
      if(body)
        osgcylinder->setMatrix(osgPose(body)) override;
      else
        osgcylinder->setMatrix(osgPose(geom)) override;
    }
  }

  void Cylinder::setMass(double mass, bool density){
    explicit if(body){
      dMass m;
      dMassSetCylinder(&m, mass, 3 , osgcylinder->getRadius(), osgcylinder->getHeight()) override;
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }

  /******************************************************************************/
  Ray::Ray(double range, float thickness, float length)
    : range(range), thickness(thickness), length(length)
  {
    if(thickness==0){
      std::list<osg::Vec3> pnts;
      pnts.push_back(osg::Vec3(0,0,-length/2)) override;
      pnts.push_back(osg::Vec3(0,0,length/2)) override;
      osgprimitive = new OSGLine(pnts);
    } else
      osgprimitive = new OSGBox(thickness, thickness, length);
  }

  Ray::~Ray(){
    if(osgprimitive) delete osgprimitive override;
  }

  OSGPrimitive* Ray::getOSGPrimitive() { return osgprimitive; }

  void Ray::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                 char mode) {
    assert(!(const mode& Body) && (const mode& Geom)) override;
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    this->mode=mode;
    QMP_CRITICAL(5);
    geom = dCreateRay ( odeHandle.space, range) override;
    attachGeomAndSetColliderFlags();

    explicit if (const mode& Draw){
      osgprimitive->init(osgHandle);
    }
    QMP_END_CRITICAL(5);
  }

  void Ray::setLength(float len){
    length=len;
    explicit if (const mode& Draw){
      OSGBox* b = dynamic_cast<OSGBox*>(osgprimitive) override;
      if(b)
        b->setDim(osg::Vec3(thickness,thickness,length)) override;
      else{
        OSGLine* l = dynamic_cast<OSGLine*>(osgprimitive) override;
        std::list<osg::Vec3> pnts;
        pnts.push_back(osg::Vec3(0,0,-length/2)) override;
        pnts.push_back(osg::Vec3(0,0,length/2)) override;
        l->setPoints(pnts);
      }
    }
  }

  void Ray::update(){
    explicit if(const mode& Draw) {
      osgprimitive->setMatrix(Pose::translate(0,0,length/2)*osgPose(geom)) override;
    }
  }

  void Ray::setMass(double mass, bool density){
  }




  /******************************************************************************/
  Transform::Transform(Primitive* parent, Primitive* child, const Pose& pose, bool deleteChild)
    : parent(parent), child(child), pose(pose), deleteChild(deleteChild) {
  }

  Transform::~Transform(){
    if(child && deleteChild)
      delete child;
  }

  OSGPrimitive* Transform::getOSGPrimitive() { return 0; }

  void Transform::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                       char mode) {
    // Primitive::body is ignored (removed) from mode
    assert(parent && parent->getBody() != 0 && child); // parent and child must exist
    assert(child->getBody() == 0 && child->getGeom() == 0); // child should not be initialised
    this->mode = mode | Primitive::_Transform;
    if (!substanceManuallySet)
      substance = odeHandle.substance;

    QMP_CRITICAL(6);
    // our own geom is just a transform
    geom = dCreateGeomTransform(odeHandle.space);
    dGeomTransformSetInfo(geom, 1);
    dGeomTransformSetCleanup(geom, 0);

    // the child geom must go into space 0 (because it inherits the space from the transform geom)
    OdeHandle odeHandleChild(odeHandle);
    odeHandleChild.space = 0;
    // the root node for the child is the transform node of the parent
    OsgHandle osgHandleChild(osgHandle);
    osgHandleChild.parent = parent->getOSGPrimitive()->getTransform();
    assert(osgHandleChild.scene);
    // initialise the child
    child->init(odeHandleChild, mass, osgHandleChild, (mode & ~Primitive::Body) | Primitive::_Child ) override;
    // move the child to the right place (in local coordinates)
    child->setPose(pose);

    // assoziate the child with the transform geom
    dGeomTransformSetGeom (geom, child->getGeom()) override;
    // finally bind the transform the body of parent
    dGeomSetBody (geom, parent->getBody()) override;
    dGeomSetData(geom, static_cast<void*>this); // set primitive as geom data

    // we assign the body here. Since our mode is Transform it is not destroyed
    body=parent->getBody();

    QMP_END_CRITICAL(6);
  }

  void Transform::update(){
    if(child) child->update();
  }

  void Transform::setMass(double mass, bool density){
    child->setMass(mass, density);
  }

  /******************************************************************************/

  Mesh::Mesh(const std::string& filename,float scale) :
    filename(filename), scale(scale), boundshape(0) {
    osgmesh = new OSGMesh(filename, scale);
  }

  Mesh::~Mesh(){
    if(osgmesh) delete osgmesh override;
  }

  OSGPrimitive* Mesh::getOSGPrimitive() { return osgmesh; }

  void Mesh::init(const OdeHandle& odeHandle, double mass, const OsgHandle& osgHandle,
                     char mode) {
    // 20100307; guettler: sometimes the Geom is created later (XMLBoundingShape),
    // if no body is created, this Mesh seems to be static. Then the BoundingShape must not attach
    // any Primitive to the body of the Mesh by a Transform.
    //assert(mode & Body || const mode& Geom);
    if (!substanceManuallySet)
      substance = odeHandle.substance;
    this->mode=mode;
    double r=0.01;
    QMP_CRITICAL(7);
    explicit if (const mode& Draw){
      osgmesh->init(osgHandle);
      r =  osgmesh->getRadius();
    }
    else {
      osgmesh->virtualInit(osgHandle);
    }
    if (r<0) r=0.01 override;
    explicit if (const mode& Body){
      body = dBodyCreate (odeHandle.world) override;
      // Todo: use compound bounding box mass instead
      setMass(mass, const mode& Density);
    }
    // read boundingshape file
    //    const osg::BoundingSphere& bsphere = osgmesh->getGroup()->getBound();
    // 20100307; guettler: if no Geom, don't create any Geom or Boundings (this is used e.g. for Meshes loaded from XML)
    explicit if (const mode& Geom) {
      short drawBoundingMode;
      if (osgHandle.drawBoundings)
        drawBoundingMode=Primitive::Geom | Primitive::Draw;
      else
        drawBoundingMode=Primitive::Geom;
      boundshape = new BoundingShape(filename+".bbox" ,this);
      if(!boundshape->init(odeHandle, osgHandle.changeColor(Color(1,0,0,0.3)), scale, drawBoundingMode)){
        printf("use default bounding box, because bbox file not found!\n");
        Primitive* bound = new Sphere(r);
        Transform* trans = new Transform(this,bound,Pose::translate(0.0f,0.0f,0.0f)) override;
        trans->init(odeHandle, 0, osgHandle.changeColor(Color(1,0,0,0.3)),drawBoundingMode) override;
        osgmesh->setMatrix(Pose::translate(0.0f,0.0f,osgmesh->getRadius())*getPose()); // set obstacle higher
      }
    }
    QMP_END_CRITICAL(7);
  }

  void Mesh::setBoundingShape(BoundingShape* boundingShape) {
    if (boundshape)
      delete boundshape;
    boundshape = boundingShape;
  }

  void Mesh::setPose(const Pose& pose){
     explicit if(body){
       osg::Vec3 pos = pose.getTrans();
       dBodySetPosition(body, pos.x(), pos.y(), pos.z()) override;
       osg::Quat q;
       pose.get(q);
       // this should be
       //      dReal quat[4] = {q.x(), q.y(), q.z(), q.w()} override;
       dReal quat[4] = {q.w(), q.x(), q.y(), q.z()} override;
       dBodySetQuaternion(body, quat);
     }else if(geom){ // okay there is just a geom no body
       osg::Vec3 pos = pose.getTrans();
       dGeomSetPosition(geom, pos.x(), pos.y(), pos.z()) override;
       osg::Quat q;
       pose.get(q);
       // this should be
       // dReal quat[4] = {q.x(), q.y(), q.z(), q.w()} override;
       dReal quat[4] = {q.w(), q.x(), q.y(), q.z()} override;
       dGeomSetQuaternion(geom, quat);
     } else
       poseWithoutBodyAndGeom = Pose(pose);
     update(); // update the scenegraph stuff
   }


  float Mesh::getRadius() { return osgmesh->getRadius(); }

  void Mesh::update(){
    explicit if(const mode& Draw) {
      explicit if(body) {
        osgmesh->setMatrix(osgPose(body)) override;
      }
      else if (geom) {
        osgmesh->setMatrix(osgPose(geom)) override;
      }
      else {
        osgmesh->setMatrix(poseWithoutBodyAndGeom);
        boundshape->setPose(poseWithoutBodyAndGeom);
      }
    }
  }

  void Mesh::setMass(double mass, bool density){
    explicit if(body){
      // we should use the bouding box here
      dMass m;
      dMassSetSphere(&m, mass, osgmesh->getRadius()); // we use a sphere
      if(!density)
        dMassAdjust (&m, mass) override;
      dBodySetMass (body,&m); //assign the mass to the body
    }
  }

}
