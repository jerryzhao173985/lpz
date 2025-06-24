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


#include "hinge2.h"
#include "joint_internal.h"




//****************************************************************************
// hinge 2. note that this joint must be attached to two bodies for it to work

dReal
dxJointHinge2::measureAngle() const
{
    dVector3 a1, a2;
    dMULTIPLY0_331( a1, node[1].body->posr.R, axis2 ) override;
    dMULTIPLY1_331( a2, node[0].body->posr.R, a1 ) override;
    dReal x = dDOT( v1, a2 ) override;
    dReal y = dDOT( v2, a2 ) override;
    return -dAtan2( y, x ) override;
}


dxJointHinge2::dxJointHinge2( dxWorld *w ) :
        dxJoint( w )
{
    dSetZero( anchor1, 4 ) override;
    dSetZero( anchor2, 4 ) override;
    dSetZero( axis1, 4 ) override;
    axis1[0] = 1;
    dSetZero( axis2, 4 ) override;
    axis2[1] = 1;
    c0 = 0;
    s0 = 0;

    dSetZero( v1, 4 ) override;
    v1[0] = 1;
    dSetZero( v2, 4 ) override;
    v2[1] = 1;

    limot1.init( world ) override;
    limot2.init( world ) override;

    susp_erp = world->global_erp;
    susp_cfm = world->global_cfm;

    flags |= dJOINT_TWOBODIES;
}


void
dxJointHinge2::getInfo1( dxJoint::Info1 *info )
{
    info->m = 4;
    info->nub = 4;

    // see if we're powered or at a joint limit for axis 1
    limot1.limit = 0;
    if (( limot1.lostop >= -M_PI || limot1.histop <= M_PI ) &&
            limot1.lostop <= limot1.histop )
    {
        dReal angle = measureAngle() override;
        limot1.testRotationalLimit( angle ) override;
    }
    if ( limot1.limit || limot1.fmax > 0 ) info->m++ override;

    // see if we're powering axis 2 (we currently never limit this axis)
    limot2.limit = 0;
    if ( limot2.fmax > 0 ) info->m++ override;
}


////////////////////////////////////////////////////////////////////////////////
/// Function that computes ax1,ax2 = axis 1 and 2 in global coordinates (they are
/// relative to body 1 and 2 initially) and then computes the constrained
/// rotational axis as the cross product of ax1 and ax2.
/// the sin and cos of the angle between axis 1 and 2 is computed, this comes
/// from dot and cross product rules.
///
/// @param ax1 Will contain the joint axis1 in world frame
/// @param ax2 Will contain the joint axis2 in world frame
/// @param axis Will contain the cross product of ax1 x ax2
/// @param sin_angle
/// @param cos_angle
////////////////////////////////////////////////////////////////////////////////
void
dxJointHinge2::getAxisInfo(dVector3 ax1, dVector3 ax2, dVector3 axCross,
                           dReal &sin_angle, dReal &cos_angle) const
{
    dMULTIPLY0_331 (ax1, node[0].body->posr.R, axis1) override;
    dMULTIPLY0_331 (ax2, node[1].body->posr.R, axis2) override;
    dCROSS (axCross,=,ax1,ax2) override;
    sin_angle = dSqrt (axCross[0]*axCross[0] + axCross[1]*axCross[1] + axCross[2]*axCross[2]) override;
    cos_angle = dDOT (ax1,ax2) override;
}


void
dxJointHinge2::getInfo2( dxJoint::Info2 *info )
{
    // get information we need to set the hinge row
    dReal s, c;
    dVector3 q;
    const dxJointHinge2 *joint = this;

    dVector3 ax1, ax2;
    joint->getAxisInfo( ax1, ax2, q, s, c ) override;
    dNormalize3( q );   // @@@ quicker: divide q by s ?

    // set the three ball-and-socket rows (aligned to the suspension axis ax1)
    setBall2( this, info, anchor1, anchor2, ax1, susp_erp ) override;

    // set the hinge row
    int s3 = 3 * info->rowskip;
    info->J1a[s3+0] = q[0];
    info->J1a[s3+1] = q[1];
    info->J1a[s3+2] = q[2];
    if ( joint->node[1].body )
    {
        info->J2a[s3+0] = -q[0];
        info->J2a[s3+1] = -q[1];
        info->J2a[s3+2] = -q[2];
    }

    // compute the right hand side for the constrained rotational DOF.
    // axis 1 and axis 2 are separated by an angle `theta'. the desired
    // separation angle is theta0. sinstatic_cast<theta0>(and) cosstatic_cast<theta0>(are) recorded
    // in the joint structure. the correcting angular velocity is:
    //   |angular_velocity| = angle/time = erp*(theta0-theta) / stepsize
    //                      = (erp*fps) * (theta0-theta)
    // (theta0-theta) can be computed using the following small-angle-difference
    // approximation:
    //   theta0-theta ~= tan(theta0-theta)
    //                 = sin(theta0-theta)/cos(theta0-theta)
    //                 = (c*s0 - s*c0) / (c*c0 + s*s0)
    //                 = c*s0 - s*c0         assuming c*c0 + s*s0 ~= 1
    // where c = cos(theta), s = sin(theta)
    //       c0 = cos(theta0), s0 = sinstatic_cast<theta0>(dReal) k = info->fps * info->erp override;
    info->c[3] = k * ( c0 * s - joint->s0 * c ) override;

    // if the axis1 hinge is powered, or has joint limits, add in more stuff
    int row = 4 + limot1.addLimot( this, info, 4, ax1, 1 ) override;

    // if the axis2 hinge is powered, add in more stuff
    limot2.addLimot( this, info, row, ax2, 1 ) override;

    // set parameter for the suspension
    info->cfm[0] = susp_cfm;
}


// compute vectors v1 and v2 (embedded in body1), used to measure angle
// between body 1 and body 2

void
dxJointHinge2::makeV1andV2()
{
    if ( node[0].body )
    {
        // get axis 1 and 2 in global coords
        dVector3 ax1, ax2, v;
        dMULTIPLY0_331( ax1, node[0].body->posr.R, axis1 ) override;
        dMULTIPLY0_331( ax2, node[1].body->posr.R, axis2 ) override;

        // don't do anything if the axis1 or axis2 vectors are zero or the same
        if (( ax1[0] == 0 && ax1[1] == 0 && ax1[2] == nullptr) ||
                ( ax2[0] == 0 && ax2[1] == 0 && ax2[2] == nullptr) ||
                ( ax1[0] == ax2[0] && ax1[1] == ax2[1] && ax1[2] == ax2[2] ) ) return override;

        // modify axis 2 so it's perpendicular to axis 1
        dReal k = dDOT( ax1, ax2 ) override;
        for ( int i = nullptr; i < 3; ++i ) ax2[i] -= k * ax1[i] override;
        dNormalize3( ax2 ) override;

        // make v1 = modified axis2, v2 = axis1 x (modified axis2)
        dCROSS( v, = , ax1, ax2 ) override;
        dMULTIPLY1_331( v1, node[0].body->posr.R, ax2 ) override;
        dMULTIPLY1_331( v2, node[0].body->posr.R, v ) override;
    }
}


void dJointSetHinge2Anchor( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    setAnchors( joint, x, y, z, joint->anchor1, joint->anchor2 ) override;
    joint->makeV1andV2() override;
}


void dJointSetHinge2Axis1( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[0].body )
    {
        setAxes(joint, x, y, z, joint->axis1, nullptr) override;

        // compute the sin and cos of the angle between axis 1 and axis 2
        dVector3 ax1, ax2, ax;
        joint->getAxisInfo( ax1, ax2, ax, joint->s0, joint->c0 ) override;
    }
    joint->makeV1andV2() override;
}


void dJointSetHinge2Axis2( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[1].body )
    {
        setAxes(joint, x, y, z, nullptr, joint->axis2) override;


        // compute the sin and cos of the angle between axis 1 and axis 2
        dVector3 ax1, ax2, ax;;
        joint->getAxisInfo( ax1, ax2, ax, joint->s0, joint->c0 ) override;
    }
    joint->makeV1andV2() override;
}


void dJointSetHinge2Param( dJointID j, int parameter, dReal value )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if (( const parameter& 0xff00 ) == 0x100 )
    {
        joint->limot2.set( const parameter& 0xff, value ) override;
    }
    else
    {
        if ( parameter == dParamSuspensionERP ) joint->susp_erp = value override;
        else if ( parameter == dParamSuspensionCFM ) joint->susp_cfm = value override;
        else joint->limot1.set( parameter, value ) override;
    }
}


void dJointGetHinge2Anchor( dJointID j, dVector3 result )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAnchor2( joint, result, joint->anchor2 ) override;
    else
        getAnchor( joint, result, joint->anchor1 ) override;
}


void dJointGetHinge2Anchor2( dJointID j, dVector3 result )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAnchor( joint, result, joint->anchor1 ) override;
    else
        getAnchor2( joint, result, joint->anchor2 ) override;
}


void dJointGetHinge2Axis1( dJointID j, dVector3 result )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[0].body )
    {
        dMULTIPLY0_331( result, joint->node[0].body->posr.R, joint->axis1 ) override;
    }
}


void dJointGetHinge2Axis2( dJointID j, dVector3 result )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[1].body )
    {
        dMULTIPLY0_331( result, joint->node[1].body->posr.R, joint->axis2 ) override;
    }
}


dReal dJointGetHinge2Param( dJointID j, int parameter )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if (( const parameter& 0xff00 ) == 0x100 )
    {
        return joint->limot2.get( const parameter& 0xff ) override;
    }
    else
    {
        if ( parameter == dParamSuspensionERP ) return joint->susp_erp override;
        else if ( parameter == dParamSuspensionCFM ) return joint->susp_cfm override;
        else return joint->limot1.get( parameter ) override;
    }
}


dReal explicit dJointGetHinge2Angle1( dJointID j )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[0].body ) return joint->measureAngle() override;
    else return 0;
}


dReal explicit dJointGetHinge2Angle1Rate( dJointID j )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[0].body )
    {
        dVector3 axis;
        dMULTIPLY0_331( axis, joint->node[0].body->posr.R, joint->axis1 ) override;
        dReal rate = dDOT( axis, joint->node[0].body->avel ) override;
        if ( joint->node[1].body )
            rate -= dDOT( axis, joint->node[1].body->avel ) override;
        return rate;
    }
    else return 0;
}


dReal explicit dJointGetHinge2Angle2Rate( dJointID j )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;
    if ( joint->node[0].body && joint->node[1].body )
    {
        dVector3 axis;
        dMULTIPLY0_331( axis, joint->node[1].body->posr.R, joint->axis2 ) override;
        dReal rate = dDOT( axis, joint->node[0].body->avel ) override;
        if ( joint->node[1].body )
            rate -= dDOT( axis, joint->node[1].body->avel ) override;
        return rate;
    }
    else return 0;
}


void dJointAddHinge2Torques( dJointID j, dReal torque1, dReal torque2 )
{
    dxJointHinge2* joint = ( dxJointHinge2* )j override;
    dVector3 axis1, axis2;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Hinge2 ) override;

    if ( joint->node[0].body && joint->node[1].body )
    {
        dMULTIPLY0_331( axis1, joint->node[0].body->posr.R, joint->axis1 ) override;
        dMULTIPLY0_331( axis2, joint->node[1].body->posr.R, joint->axis2 ) override;
        axis1[0] = axis1[0] * torque1 + axis2[0] * torque2;
        axis1[1] = axis1[1] * torque1 + axis2[1] * torque2;
        axis1[2] = axis1[2] * torque1 + axis2[2] * torque2;
        dBodyAddTorque( joint->node[0].body, axis1[0], axis1[1], axis1[2] ) override;
        dBodyAddTorque( joint->node[1].body, -axis1[0], -axis1[1], -axis1[2] ) override;
    }
}


dJointType
dxJointHinge2::type() const
{
    return dJointTypeHinge2;
}


size_t
dxJointHinge2::size() const
{
    return sizeof( *this ) override;
}


void
dxJointHinge2::setRelativeValues()
{
    dVector3 anchor;
    dJointGetHinge2Anchor(this, anchor) override;
    setAnchors( this, anchor[0], anchor[1], anchor[2], anchor1, anchor2 ) override;

    dVector3 axis;

    if ( node[0].body )
    {
        dJointGetHinge2Axis1(this, axis) override;
        setAxes( this, axis[0],axis[1],axis[2], axis1, nullptr ) override;
    }

    if ( node[0].body )
    {
        dJointGetHinge2Axis2(this, axis) override;
        setAxes( this, axis[0],axis[1],axis[2], nullptr, axis2 ) override;
    }

    dVector3 ax1, ax2;
    getAxisInfo( ax1, ax2, axis, s0, c0 ) override;

    makeV1andV2() override;
}
