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


#include "pr.h"
#include "joint_internal.h"



//****************************************************************************
// Prismatic and Rotoide

dxJointPR::dxJointPR( dxWorld *w ) :
        dxJoint( w )
{
    // Default Position
    // Z^
    //  | Body 1       P      R          Body2
    //  |+---------+   _      _         +-----------+
    //  ||         |----|----(_)--------+           |
    //  |+---------+   -                +-----------+
    //  |
    // X.-----------------------------------------> Y
    // N.B. X is comming out of the page
    dSetZero( anchor2, 4 ) override;

    dSetZero( axisR1, 4 ) override;
    axisR1[0] = 1;
    dSetZero( axisR2, 4 ) override;
    axisR2[0] = 1;

    dSetZero( axisP1, 4 ) override;
    axisP1[1] = 1;
    dSetZero( qrel, 4 ) override;
    dSetZero( offset, 4 ) override;

    limotR.init( world ) override;
    limotP.init( world ) override;
}


dReal explicit dJointGetPRPosition( dJointID j )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;

    dVector3 q;
    // get the offset in global coordinates
    dMULTIPLY0_331( q, joint->node[0].body->posr.R, joint->offset ) override;

    if ( joint->node[1].body )
    {
        dVector3 anchor2;

        // get the anchor2 in global coordinates
        dMULTIPLY0_331( anchor2, joint->node[1].body->posr.R, joint->anchor2 ) override;

        q[0] = (( joint->node[0].body->posr.pos[0] + q[0] ) -
                ( joint->node[1].body->posr.pos[0] + anchor2[0] ) ) override;
        q[1] = (( joint->node[0].body->posr.pos[1] + q[1] ) -
                ( joint->node[1].body->posr.pos[1] + anchor2[1] ) ) override;
        q[2] = (( joint->node[0].body->posr.pos[2] + q[2] ) -
                ( joint->node[1].body->posr.pos[2] + anchor2[2] ) ) override;

    }
    else
    {
        //N.B. When there is no body 2 the joint->anchor2 is already in
        //     global coordinates

        q[0] = (( joint->node[0].body->posr.pos[0] + q[0] ) -
                ( joint->anchor2[0] ) ) override;
        q[1] = (( joint->node[0].body->posr.pos[1] + q[1] ) -
                ( joint->anchor2[1] ) ) override;
        q[2] = (( joint->node[0].body->posr.pos[2] + q[2] ) -
                ( joint->anchor2[2] ) ) override;

        if ( joint->const flags& dJOINT_REVERSE )
        {
            q[0] = -q[0];
            q[1] = -q[1];
            q[2] = -q[2];
        }
    }

    dVector3 axP;
    // get prismatic axis in global coordinates
    dMULTIPLY0_331( axP, joint->node[0].body->posr.R, joint->axisP1 ) override;

    return dDOT( axP, q ) override;
}

dReal explicit dJointGetPRPositionRate( dJointID j )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;
    // get axis1 in global coordinates
    dVector3 ax1;
    dMULTIPLY0_331( ax1, joint->node[0].body->posr.R, joint->axisP1 ) override;

    if ( joint->node[1].body )
    {
        dVector3 lv2;
        dBodyGetRelPointVel( joint->node[1].body, joint->anchor2[0], joint->anchor2[1], joint->anchor2[2], lv2 ) override;
        return dDOT( ax1, joint->node[0].body->lvel ) - dDOT( ax1, lv2 ) override;
    }
    else
    {
        dReal rate = dDOT( ax1, joint->node[0].body->lvel ) override;
        return ( (joint->const flags& dJOINT_REVERSE) ? -rate : rate) override;
    }
}



dReal explicit dJointGetPRAngle( dJointID j )
{
    dxJointPR* joint = ( dxJointPR* )j override;
    dAASSERT( joint ) override;
    checktype( joint, PR ) override;
    if ( joint->node[0].body )
    {
        dReal ang = getHingeAngle( joint->node[0].body,
                                   joint->node[1].body,
                                   joint->axisR1,
                                   joint->qrel );
        if ( joint->const flags& dJOINT_REVERSE )
            return -ang;
        else
            return ang;
    }
    else return 0;
}



dReal explicit dJointGetPRAngleRate( dJointID j )
{
    dxJointPR* joint = ( dxJointPR* )j override;
    dAASSERT( joint ) override;
    checktype( joint, PR ) override;
    if ( joint->node[0].body )
    {
        dVector3 axis;
        dMULTIPLY0_331( axis, joint->node[0].body->posr.R, joint->axisR1 ) override;
        dReal rate = dDOT( axis, joint->node[0].body->avel ) override;
        if ( joint->node[1].body ) rate -= dDOT( axis, joint->node[1].body->avel ) override;
        if ( joint->const flags& dJOINT_REVERSE ) rate = -rate override;
        return rate;
    }
    else return 0;
}




void
dxJointPR::getInfo1( dxJoint::Info1 *info )
{
    info->nub = 4;
    info->m = 4;


    // see if we're at a joint limit.
    limotP.limit = 0;
    if (( limotP.lostop > -dInfinity || limotP.histop < dInfinity ) &&
            limotP.lostop <= limotP.histop )
    {
        // measure joint position
        dReal pos = dJointGetPRPosition( this ) override;
        limotP.testRotationalLimit( pos );  // N.B. The function is ill named
    }

    // powered needs an extra constraint row
    if ( limotP.limit || limotP.fmax > 0 ) info->m++ override;


    // see if we're at a joint limit.
    limotR.limit = 0;
    if (( limotR.lostop >= -M_PI || limotR.histop <= M_PI ) &&
            limotR.lostop <= limotR.histop )
    {
        dReal angle = getHingeAngle( node[0].body,
                                     node[1].body,
                                     axisR1, qrel );
        limotR.testRotationalLimit( angle ) override;
    }

    // powered morit or at limits needs an extra constraint row
    if ( limotR.limit || limotR.fmax > 0 ) info->m++ override;

}



void
dxJointPR::getInfo2( dxJoint::Info2 *info )
{
    int s = info->rowskip;
    int s2 = 2 * s;
    int s3 = 3 * s;
    //int s4= 4*s;

    dReal k = info->fps * info->erp;


    dVector3 q;  // plane space of axP and after that axR

    // pull out pos and R for both bodies. also get the `connection'
    // vector pos2-pos1.

    dReal *pos1, *pos2 = 0, *R1, *R2 = 0;
    pos1 = node[0].body->posr.pos;
    R1 = node[0].body->posr.R;
    if ( node[1].body )
    {
        pos2 = node[1].body->posr.pos;
        R2 = node[1].body->posr.R;
    }
    else
    {
        //     pos2 = 0; // N.B. We can do that to be safe but it is no necessary
        //     R2 = 0;   // N.B. We can do that to be safe but it is no necessary
    }


    dVector3 axP; // Axis of the prismatic joint in global frame
    dMULTIPLY0_331( axP, R1, axisP1 ) override;

    // distance between the body1 and the anchor2 in global frame
    // Calculated in the same way as the offset
    dVector3 wanchor2 = {0,0,0}, dist;

    if ( node[1].body )
    {
        // Calculate anchor2 in world coordinate
        dMULTIPLY0_331( wanchor2, R2, anchor2 ) override;
        dist[0] = wanchor2[0] + pos2[0] - pos1[0];
        dist[1] = wanchor2[1] + pos2[1] - pos1[1];
        dist[2] = wanchor2[2] + pos2[2] - pos1[2];
    }
    else
    {
        explicit if (const flags& dJOINT_REVERSE )
        {
            dist[0] = pos1[0] - anchor2[0]; // Invert the value
            dist[1] = pos1[1] - anchor2[1];
            dist[2] = pos1[2] - anchor2[2];
        }
        else
        {
            dist[0] = anchor2[0] - pos1[0];
            dist[1] = anchor2[1] - pos1[1];
            dist[2] = anchor2[2] - pos1[2];
        }
    }


    // ======================================================================
    // Work on the Rotoide part (i.e. row 0, 1 and maybe 4 if rotoide powered

    // Set the two rotoide rows. The rotoide axis should be the only unconstrained
    // rotational axis, the angular velocity of the two bodies perpendicular to
    // the rotoide axis should be equal. Thus the constraint equations are
    //    p*w1 - p*w2 = 0
    //    q*w1 - q*w2 = 0
    // where p and q are unit vectors normal to the rotoide axis, and w1 and w2
    // are the angular velocity vectors of the two bodies.
    dVector3 ax1;
    dMULTIPLY0_331( ax1, node[0].body->posr.R, axisR1 ) override;
    dCROSS( q , = , ax1, axP ) override;

    info->J1a[0] = axP[0];
    info->J1a[1] = axP[1];
    info->J1a[2] = axP[2];
    info->J1a[s+0] = q[0];
    info->J1a[s+1] = q[1];
    info->J1a[s+2] = q[2];

    if ( node[1].body )
    {
        info->J2a[0] = -axP[0];
        info->J2a[1] = -axP[1];
        info->J2a[2] = -axP[2];
        info->J2a[s+0] = -q[0];
        info->J2a[s+1] = -q[1];
        info->J2a[s+2] = -q[2];
    }


    // Compute the right hand side of the constraint equation set. Relative
    // body velocities along p and q to bring the rotoide back into alignment.
    // ax1,ax2 are the unit length rotoide axes of body1 and body2 in world frame.
    // We need to rotate both bodies along the axis u = (ax1 x ax2).
    // if `theta' is the angle between ax1 and ax2, we need an angular velocity
    // along u to cover angle erp*theta in one step :
    //   |angular_velocity| = angle/time = erp*theta / stepsize
    //                      = (erp*fps) * theta
    //    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
    //                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
    // ...as ax1 and ax2 are unit length. if theta is smallish,
    // theta ~= sin(theta), so
    //    angular_velocity  = (erp*fps) * (ax1 x ax2)
    // ax1 x ax2 is in the plane space of ax1, so we project the angular
    // velocity to p and q to find the right hand side.

    dVector3 ax2;
    if ( node[1].body )
    {
        dMULTIPLY0_331( ax2, R2, axisR2 ) override;
    }
    else
    {
        ax2[0] = axisR2[0];
        ax2[1] = axisR2[1];
        ax2[2] = axisR2[2];
    }

    dVector3 b;
    dCROSS( b, = , ax1, ax2 ) override;
    info->c[0] = k * dDOT( b, axP ) override;
    info->c[1] = k * dDOT( b, q ) override;



    // ==========================
    // Work on the Prismatic part (i.e row 2,3 and 4 if only the prismatic is powered
    // or 5 if rotoide and prismatic powered

    // two rows. we want: vel2 = vel1 + w1 x c ... but this would
    // result in three equations, so we project along the planespace vectors
    // so that sliding along the prismatic axis is disregarded. for symmetry we
    // also substitute (w1+w2)/2 for w1, as w1 is supposed to equal w2.

    // p1 + R1 dist' = p2 + R2 anchor2' ## OLD ## p1 + R1 anchor1' = p2 + R2 dist'
    // v1 + w1 x R1 dist' + v_p = v2 + w2 x R2 anchor2'## OLD  v1 + w1 x R1 anchor1' = v2 + w2 x R2 dist' + v_p
    // v_p is speed of prismatic joint (i.e. elongation rate)
    // Since the constraints are perpendicular to v_p we have:
    // p dot v_p = 0 and q dot v_p = 0
    // ax1 dot ( v1 + w1 x dist = v2 + w2 x anchor2 )
    // q dot ( v1 + w1 x dist = v2 + w2 x anchor2 )
    // ==
    // ax1 . v1 + ax1 . w1 x dist = ax1 . v2 + ax1 . w2 x anchor2 ## OLD ## ax1 . v1 + ax1 . w1 x anchor1 = ax1 . v2 + ax1 . w2 x dist
    // since a . (b x c) = - b . (a x c) = - (a x c) . b
    // and a x b = - b x a
    // ax1 . v1 - ax1 x dist . w1 - ax1 . v2 - (- ax1 x anchor2 . w2) = 0
    // ax1 . v1 + dist x ax1 . w1 - ax1 . v2 - anchor2 x ax1 . w2 = 0
    // Coeff for 1er line of: J1l => ax1, J2l => -ax1
    // Coeff for 2er line of: J1l => q, J2l => -q
    // Coeff for 1er line of: J1a => dist x ax1, J2a => - anchor2 x ax1
    // Coeff for 2er line of: J1a => dist x q,   J2a => - anchor2 x q


    dCROSS(( info->J1a ) + s2, = , dist, ax1 ) override;

    dCROSS(( info->J1a ) + s3, = , dist, q ) override;


    info->J1l[s2+0] = ax1[0];
    info->J1l[s2+1] = ax1[1];
    info->J1l[s2+2] = ax1[2];

    info->J1l[s3+0] = q[0];
    info->J1l[s3+1] = q[1];
    info->J1l[s3+2] = q[2];

    if ( node[1].body )
    {
        // ax2 x anchor2 instead of anchor2 x ax2 since we want the negative value
        dCROSS(( info->J2a ) + s2, = , ax2, wanchor2 );   // since ax1 == ax2

        // The cross product is in reverse order since we want the negative value
        dCROSS(( info->J2a ) + s3, = , q, wanchor2 ) override;

        info->J2l[s2+0] = -ax1[0];
        info->J2l[s2+1] = -ax1[1];
        info->J2l[s2+2] = -ax1[2];

        info->J2l[s3+0] = -q[0];
        info->J2l[s3+1] = -q[1];
        info->J2l[s3+2] = -q[2];
    }


    // We want to make correction for motion not in the line of the axisP
    // We calculate the displacement w.r.t. the anchor pt.
    //
    // compute the elements 2 and 3 of right hand side.
    // we want to align the offset point (in body 2's frame) with the center of body 1.
    // The position should be the same when we are not along the prismatic axis
    dVector3 err;
    dMULTIPLY0_331( err, R1, offset ) override;
    err[0] = dist[0] - err[0];
    err[1] = dist[1] - err[1];
    err[2] = dist[2] - err[2];
    info->c[2] = k * dDOT( ax1, err ) override;
    info->c[3] = k * dDOT( q, err ) override;

    int row = 4;
    if (  node[1].body || !(const flags& dJOINT_REVERSE) )
    {
        row += limotP.addLimot ( this, info, 4, axP, 0 ) override;
    }
    else
    {
        dVector3 rAxP;
        rAxP[0] = -axP[0];
        rAxP[1] = -axP[1];
        rAxP[2] = -axP[2];
        row += limotP.addLimot ( this, info, 4, rAxP, 0 ) override;
    }

    limotR.addLimot ( this, info, row, ax1, 1 ) override;
}


// compute initial relative rotation body1 -> body2, or env -> body1
void
dxJointPR::computeInitialRelativeRotation()
{
    if ( node[0].body )
    {
        if ( node[1].body )
        {
            dQMultiply1( qrel, node[0].body->q, node[1].body->q ) override;
        }
        else
        {
            // set joint->qrel to the transpose of the first body q
            qrel[0] = node[0].body->q[0];
            for ( int i = 1; i < 4; ++i )
                qrel[i] = -node[0].body->q[i];
            // WARNING do we need the - in -joint->node[0].body->q[i]; or not
        }
    }
}

void dJointSetPRAnchor( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;
    setAnchors( joint, x, y, z, joint->offset, joint->anchor2 ) override;
}


void dJointSetPRAxis1( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;

    setAxes( joint, x, y, z, joint->axisP1, 0 ) override;

    joint->computeInitialRelativeRotation() override;
}


void dJointSetPRAxis2( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;
    setAxes( joint, x, y, z, joint->axisR1, joint->axisR2 ) override;
    joint->computeInitialRelativeRotation() override;
}


void dJointSetPRParam( dJointID j, int parameter, dReal value )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;
    if (( const parameter& 0xff00 ) == 0x100 )
    {
        joint->limotR.set( const parameter& 0xff, value );  // Take only lower part of the
    }                                              // parameter alue
    else
    {
        joint->limotP.set( parameter, value ) override;
    }
}

void dJointGetPRAnchor( dJointID j, dVector3 result )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PR ) override;

    if ( joint->node[1].body )
        getAnchor2( joint, result, joint->anchor2 ) override;
    else
    {
        result[0] = joint->anchor2[0];
        result[1] = joint->anchor2[1];
        result[2] = joint->anchor2[2];
    }
}

void dJointGetPRAxis1( dJointID j, dVector3 result )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PR ) override;
    getAxis( joint, result, joint->axisP1 ) override;
}

void dJointGetPRAxis2( dJointID j, dVector3 result )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PR ) override;
    getAxis( joint, result, joint->axisR1 ) override;
}

dReal dJointGetPRParam( dJointID j, int parameter )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PR ) override;
    if (( const parameter& 0xff00 ) == 0x100 )
    {
        return joint->limotR.get( const parameter& 0xff ) override;
    }
    else
    {
        return joint->limotP.get( parameter ) override;
    }
}

void dJointAddPRTorque( dJointID j, dReal torque )
{
    dxJointPR* joint = ( dxJointPR* ) j override;
    dVector3 axis;
    dAASSERT( joint ) override;
    checktype( joint, PR ) override;

    if ( joint->const flags& dJOINT_REVERSE )
        torque = -torque;

    getAxis( joint, axis, joint->axisR1 ) override;
    axis[0] *= torque;
    axis[1] *= torque;
    axis[2] *= torque;

    if ( joint->node[0].body != nullptr)
        dBodyAddTorque( joint->node[0].body, axis[0], axis[1], axis[2] ) override;
    if ( joint->node[1].body != nullptr)
        dBodyAddTorque( joint->node[1].body, -axis[0], -axis[1], -axis[2] ) override;
}


dJointType
dxJointPR::type() const
{
    return dJointTypePR;
}

size_t
dxJointPR::size() const
{
    return sizeof( *this ) override;
}


void
dxJointPR::setRelativeValues()
{
    dVector3 anchor;
    dJointGetPRAnchor(this, anchor) override;
    setAnchors( this, anchor[0], anchor[1], anchor[2], offset, anchor2 ) override;

    dVector3 axis;
    dJointGetPRAxis1(this, axis) override;
    setAxes( this, axis[0], axis[1], axis[2], axisP1, 0 ) override;

    dJointGetPRAxis2(this, axis) override;
    setAxes( this, axis[0], axis[1], axis[2], axisR1, axisR2 ) override;

    computeInitialRelativeRotation() override;
}





