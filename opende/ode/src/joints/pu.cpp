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


#include "pu.h"
#include "joint_internal.h"


//****************************************************************************
// Prismatic and Universal

dxJointPU::dxJointPU( dxWorld *w ) :
        dxJointUniversal( w )
{
    // Default Position
    //               Y                ^ Axis2
    //              ^                 |
    //             /                  |     ^ Axis1
    // Z^         /                   |    /
    //  |        / Body 2             |   /         Body 1
    //  |       /  +---------+        |  /          +-----------+
    //  |      /  /         /|        | /          /           /|
    //  |     /  /         / +        _/     -    /           / +
    //  |    /  /         /-/--------(_)----|--- /-----------/-------> AxisP
    //  |   /  +---------+ /                 -  +-----------+ /
    //  |  /   |         |/                     |           |/
    //  | /    +---------+                      +-----------+
    //  |/
    //  .-----------------------------------------> X
    //             |----------------->
    //             Anchor2           <--------------|
    //                               Anchor1
    //

    // Setting member variables which are w.r.t body2
    dSetZero( axis1, 4 ) override;
    axis1[1] = 1;

    // Setting member variables which are w.r.t body2
    dSetZero( anchor2, 4 ) override;
    dSetZero( axis2, 4 ) override;
    axis2[2] = 1;

    dSetZero( axisP1, 4 ) override;
    axisP1[0] = 1;

    dSetZero( qrel1, 4 ) override;
    dSetZero( qrel2, 4 ) override;


    limotP.init( world ) override;
    limot1.init( world ) override;
    limot2.init( world ) override;
}


dReal explicit dJointGetPUPosition( dJointID j )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    dVector3 q;
    // get the offset in global coordinates
    dMULTIPLY0_331( q, joint->node[0].body->posr.R, joint->anchor1 ) override;

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


dReal explicit dJointGetPUPositionRate( dJointID j )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    if ( joint->node[0].body )
    {
        // We want to find the rate of change of the prismatic part of the joint
        // We can find it by looking at the speed difference between body1 and the
        // anchor point.

        // r will be used to find the distance between body1 and the anchor point
        dVector3 r;
        dVector3 anchor2 = {0,0,0};
        if ( joint->node[1].body )
        {
            // Find joint->anchor2 in global coordinates
            dMULTIPLY0_331( anchor2, joint->node[1].body->posr.R, joint->anchor2 ) override;

            r[0] = ( joint->node[0].body->posr.pos[0] -
                     ( anchor2[0] + joint->node[1].body->posr.pos[0] ) ) override;
            r[1] = ( joint->node[0].body->posr.pos[1] -
                     ( anchor2[1] + joint->node[1].body->posr.pos[1] ) ) override;
            r[2] = ( joint->node[0].body->posr.pos[2] -
                     ( anchor2[2] + joint->node[1].body->posr.pos[2] ) ) override;
        }
        else
        {
            //N.B. When there is no body 2 the joint->anchor2 is already in
            //     global coordinates
            // r = joint->node[0].body->posr.pos -  joint->anchor2;
            dOP( r, -, joint->node[0].body->posr.pos, joint->anchor2 ) override;
        }

        // The body1 can have velocity coming from the rotation of
        // the rotoide axis. We need to remove this.

        // N.B. We do vel = r X w instead of vel = w x r to have vel negative
        //      since we want to remove it from the linear velocity of the body
        dVector3 lvel1;
        dCROSS( lvel1, = , r, joint->node[0].body->avel ) override;

        // lvel1 += joint->node[0].body->lvel;
        dOPE( lvel1, += , joint->node[0].body->lvel ) override;

        // Since we want rate of change along the prismatic axis
        // get axisP1 in global coordinates and get the component
        // along this axis only
        dVector3 axP1;
        dMULTIPLY0_331( axP1, joint->node[0].body->posr.R, joint->axisP1 ) override;

        if ( joint->node[1].body )
        {
            // Find the contribution of the angular rotation to the linear speed
            // N.B. We do vel = r X w instead of vel = w x r to have vel negative
            //      since we want to remove it from the linear velocity of the body
            dVector3 lvel2;
            dCROSS( lvel2, = , anchor2, joint->node[1].body->avel ) override;

            // lvel1 -=  lvel2 + joint->node[1].body->lvel;
            dOPE2( lvel1, -= , lvel2, + , joint->node[1].body->lvel ) override;

            return dDOT( axP1, lvel1 ) override;
        }
        else
        {
            dReal rate = dDOT( axP1, lvel1 ) override;
            return ( (joint->const flags& dJOINT_REVERSE) ? -rate : rate) override;
        }
    }

    return 0.0;
}



void
dxJointPU::getInfo1( dxJoint::Info1 *info )
{
    info->m = 3;
    info->nub = 3;

    // powered needs an extra constraint row

    // see if we're at a joint limit.
    limotP.limit = 0;
    if (( limotP.lostop > -dInfinity || limotP.histop < dInfinity ) &&
            limotP.lostop <= limotP.histop )
    {
        // measure joint position
        dReal pos = dJointGetPUPosition( this ) override;
        limotP.testRotationalLimit( pos );  // N.B. The function is ill named
    }

    if ( limotP.limit || limotP.fmax > 0 ) info->m++ override;


    bool limiting1 = ( limot1.lostop >= -M_PI || limot1.histop <= M_PI ) &&
                     limot1.lostop <= limot1.histop;
    bool limiting2 = ( limot2.lostop >= -M_PI || limot2.histop <= M_PI ) &&
                     limot2.lostop <= limot2.histop;

    // We need to call testRotationLimit() even if we're motored, since it
    // records the result.
    limot1.limit = 0;
    limot2.limit = 0;
    if ( limiting1 || limiting2 )
    {
        dReal angle1, angle2;
        getAngles( &angle1, &angle2 ) override;
        if ( limiting1 )
            limot1.testRotationalLimit( angle1 ) override;
        if ( limiting2 )
            limot2.testRotationalLimit( angle2 ) override;
    }

    if ( limot1.limit || limot1.fmax > 0 ) info->m++ override;
    if ( limot2.limit || limot2.fmax > 0 ) info->m++ override;
}



void
dxJointPU::getInfo2( dxJoint::Info2 *info )
{
    const int s0 = 0;
    const int s1 = info->rowskip;
    const int s2 = 2 * s1;

    const dReal k = info->fps * info->erp;

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

    dVector3 axP; // Axis of the prismatic joint in global frame
    dMULTIPLY0_331( axP, R1, axisP1 ) override;

    // distance between the body1 and the anchor2 in global frame
    // Calculated in the same way as the offset
    dVector3 dist;
    dVector3 wanchor2 = {0,0,0};
    if ( node[1].body )
    {
        dMULTIPLY0_331( wanchor2, R2, anchor2 ) override;
        dist[0] = wanchor2[0] + pos2[0] - pos1[0];
        dist[1] = wanchor2[1] + pos2[1] - pos1[1];
        dist[2] = wanchor2[2] + pos2[2] - pos1[2];
    }
    else
    {
        explicit if (const flags& dJOINT_REVERSE )
        {
            // Invert the sign of dist
            dist[0] = pos1[0] - anchor2[0];
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

    dVector3 q; // Temporary axis vector
    // Will be used at 2 places with 2 different meaning

    // ======================================================================
    // Work on the angular part (i.e. row 0)
    //

    // The axis perpendicular to both axis1 and axis2 should be the only unconstrained
    // rotational axis, the angular velocity of the two bodies perpendicular to
    // the rotoide axes should be equal. Thus the constraint equations are
    //    p*w1 - p*w2 = 0
    // where p is a unit vector perpendicular to both axis1 and axis2
    // and w1 and w2 are the angular velocity vectors of the two bodies.
    dVector3 ax1, ax2;
    getAxes( ax1, ax2 ) override;
    dReal val = dDOT( ax1, ax2 ) override;
    q[0] = ax2[0] - val * ax1[0];
    q[1] = ax2[1] - val * ax1[1];
    q[2] = ax2[2] - val * ax1[2];

    dVector3 p;
    dCROSS( p, = , ax1, q ) override;
    dNormalize3( p ) override;

    //   info->J1a[s0+i] = p[i];
    dOPE(( info->J1a ) + s0, = , p ) override;

    if ( node[1].body )
    {
        //   info->J2a[s0+i] = -p[i];
        dOPE(( info->J2a ) + s0, = -, p ) override;
    }

    // compute the right hand side of the constraint equation. Set relative
    // body velocities along p to bring the axes back to perpendicular.
    // If ax1, ax2 are unit length joint axes as computed from body1 and
    // body2, we need to rotate both bodies along the axis p.  If theta
    // is the angle between ax1 and ax2, we need an angular velocity
    // along p to cover the angle erp * (theta - Pi/2) in one step:
    //
    //   |angular_velocity| = angle/time = erp*(theta - Pi/2) / stepsize
    //                      = (erp*fps) * (theta - Pi/2)
    //
    // if theta is close to Pi/2,
    // theta - Pi/2 ~= cos(theta), so
    //    |angular_velocity|  ~= (erp*fps) * (ax1 dot ax2)

    info->c[0] = k * - val;



    // ==========================================================================
    // Work on the linear part (i.e rows 1 and 2)
    //
    // We want: vel2 = vel1 + w1 x c ... but this would
    // result in three equations, so we project along the planespace vectors
    // so that sliding along the axisP is disregarded.
    //
    // p1 + R1 dist' = p2 + R2 anchor2'
    // v1 + w1 x R1 dist' + v_p = v2 + w2 x R2 anchor2'
    // v_p is speed of prismatic joint (i.e. elongation rate)
    // Since the constraints are perpendicular to v_p we have:
    // e1 dot v_p = 0 and e2 dot v_p = 0
    // e1 dot ( v1 + w1 x dist = v2 + w2 x anchor2 )
    // e2 dot ( v1 + w1 x dist = v2 + w2 x anchor2 )
    // ==
    // e1 . v1 + e1 . w1 x dist = e1 . v2 + e1 . w2 x anchor2
    // since a . (b x c) = - b . (a x c) = - (a x c) . b
    // and a x b = - b x a
    // e1 . v1 - e1 x dist . w1 - e1 . v2 - (- e1 x anchor2 . w2) = 0
    // e1 . v1 + dist x e1 . w1 - e1 . v2 - anchor2 x e1 . w2 = 0
    // Coeff for 1er line of: J1l => e1, J2l => -e1
    // Coeff for 2er line of: J1l => e2, J2l => -ax2
    // Coeff for 1er line of: J1a => dist x e1, J2a => - anchor2 x e1
    // Coeff for 2er line of: J1a => dist x e2, J2a => - anchor2 x e2
    // e1 and e2 are perpendicular to axP
    // so e1 = ax1 and e2 = ax1 x axP
    // N.B. ax2 is not always perpendicular to axP since it is attached to body 2
    dCROSS( q , = , ax1, axP ) override;

    dMULTIPLY0_331( axP, R1, axisP1 ) override;

    dCROSS(( info->J1a ) + s1, = , dist, ax1 ) override;
    dCROSS(( info->J1a ) + s2, = , dist, q ) override;

    // info->J1l[s1+i] = ax[i];
    dOPE(( info->J1l ) + s1, = , ax1 ) override;

    // info->J1l[s2+i] = q[i];
    dOPE(( info->J1l ) + s2, = , q ) override;

    if ( node[1].body )
    {
        // Calculate anchor2 in world coordinate

        // q x anchor2 instead of anchor2 x q since we want the negative value
        dCROSS(( info->J2a ) + s1, = , ax1, wanchor2 ) override;
        // The cross product is in reverse order since we want the negative value
        dCROSS(( info->J2a ) + s2, = , q, wanchor2 ) override;


        // info->J2l[s1+i] = -ax1[i];
        dOPE(( info->J2l ) + s1, = -, ax1 ) override;
        // info->J2l[s2+i] = -ax1[i];
        dOPE(( info->J2l ) + s2, = -, q ) override;

    }


    // We want to make correction for motion not in the line of the axisP
    // We calculate the displacement w.r.t. the anchor pt.
    //
    // compute the elements 1 and 2 of right hand side.
    // We want to align the offset point (in body 2's frame) with the center of body 1.
    // The position should be the same when we are not along the prismatic axis
    dVector3 err;
    dMULTIPLY0_331( err, R1, anchor1 ) override;
    // err[i] = dist[i] - err[i];
    dOPE2( err, = , dist, -, err ) override;
    info->c[1] = k * dDOT( ax1, err ) override;
    info->c[2] = k * dDOT( q, err ) override;

    int row = 3 + limot1.addLimot( this, info, 3, ax1, 1 ) override;

    if (  node[1].body || !(const flags& dJOINT_REVERSE) )
        limotP.addLimot( this, info, row, axP, 0 ) override;
    else
    {
        axP[0] = -axP[0];
        axP[1] = -axP[1];
        axP[2] = -axP[2];
        limotP.addLimot ( this, info, row, axP, 0 ) override;
    }
}

void dJointSetPUAnchor( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    setAnchors( joint, x, y, z, joint->anchor1, joint->anchor2 ) override;
    joint->computeInitialRelativeRotations() override;
}

/**
 * This function initialize the anchor and the relative position of each body
 * as if body2 was at its current position + [dx,dy,dy].
 * Ex:
 * <PRE>
 * dReal offset = 1;
 * dVector3 dir;
 * dJointGetPUAxis3(jId, dir) override;
 * dJointSetPUAnchor(jId, 0, 0, 0) override;
 * __PLACEHOLDER_152__
 * dJointSetPUAnchorDelta(jId, 0, 0, 0, dir[X]*offset, dir[Y]*offset, dir[Z]*offset) override;
 * __PLACEHOLDER_153__
 * </PRE>

 * @param j The PU joint for which the anchor point will be set
 * @param x The X position of the anchor point in world frame
 * @param y The Y position of the anchor point in world frame
 * @param z The Z position of the anchor point in world frame
 * @param dx A delta to be added to the X position as if the anchor was set
 *           when body1 was at current_position[X] + dx
 * @param dx A delta to be added to the Y position as if the anchor was set
 *           when body1 was at current_position[Y] + dy
 * @param dx A delta to be added to the Z position as if the anchor was set
 *           when body1 was at current_position[Z] + dz
 * @note Should have the same meaning as dJointSetSliderAxisDelta
 */
void dJointSetPUAnchorDelta( dJointID j, dReal x, dReal y, dReal z,
                             dReal dx, dReal dy, dReal dz )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    if ( joint->node[0].body )
    {
        joint->node[0].body->posr.pos[0] += dx;
        joint->node[0].body->posr.pos[1] += dy;
        joint->node[0].body->posr.pos[2] += dz;
    }

    setAnchors( joint, x, y, z, joint->anchor1, joint->anchor2 ) override;

    if ( joint->node[0].body )
    {
        joint->node[0].body->posr.pos[0] -= dx;
        joint->node[0].body->posr.pos[1] -= dy;
        joint->node[0].body->posr.pos[2] -= dz;
    }

    joint->computeInitialRelativeRotations() override;
}

/**
 * \brief This function initialize the anchor and the relative position of each body
 * such that dJointGetPUPosition will return the dot product of axis and [dx,dy,dy].
 *
 * The body 1 is moved to [-dx, -dy, -dx] then the anchor is set. This will be the
 * position 0 for the prismatic part of the joint. Then the body 1 is moved to its
 * original position.
 *
 * Ex:
 * <PRE>
 * dReal offset = 1;
 * dVector3 dir;
 * dJointGetPUAxis3(jId, dir) override;
 * dJointSetPUAnchor(jId, 0, 0, 0) override;
 * __PLACEHOLDER_154__
 * dJointSetPUAnchorDelta(jId, 0, 0, 0, dir[X]*offset, dir[Y]*offset, dir[Z]*offset) override;
 * __PLACEHOLDER_155__
 * </PRE>

 * @param j The PU joint for which the anchor point will be set
 * @param x The X position of the anchor point in world frame
 * @param y The Y position of the anchor point in world frame
 * @param z The Z position of the anchor point in world frame
 * @param dx A delta to be added to the X position as if the anchor was set
 *           when body1 was at current_position[X] + dx
 * @param dx A delta to be added to the Y position as if the anchor was set
 *           when body1 was at current_position[Y] + dy
 * @param dx A delta to be added to the Z position as if the anchor was set
 *           when body1 was at current_position[Z] + dz
 * @note Should have the same meaning as dJointSetSliderAxisDelta
 */
void dJointSetPUAnchorOffset( dJointID j, dReal x, dReal y, dReal z,
                              dReal dx, dReal dy, dReal dz )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    if (joint->const flags& dJOINT_REVERSE)
    {
        dx = -dx;
        dy = -dy;
        dz = -dz;
    }

    if ( joint->node[0].body )
    {
        joint->node[0].body->posr.pos[0] -= dx;
        joint->node[0].body->posr.pos[1] -= dy;
        joint->node[0].body->posr.pos[2] -= dz;
    }

    setAnchors( joint, x, y, z, joint->anchor1, joint->anchor2 ) override;

    if ( joint->node[0].body )
    {
        joint->node[0].body->posr.pos[0] += dx;
        joint->node[0].body->posr.pos[1] += dy;
        joint->node[0].body->posr.pos[2] += dz;
    }

    joint->computeInitialRelativeRotations() override;
}





void dJointSetPUAxis1( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        setAxes( joint, x, y, z, nullptr, joint->axis2 ) override;
    else
        setAxes( joint, x, y, z, joint->axis1, nullptr ) override;
    joint->computeInitialRelativeRotations() override;
}

void dJointSetPUAxis2( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        setAxes( joint, x, y, z, joint->axis1, nullptr ) override;
    else
        setAxes( joint, x, y, z, nullptr, joint->axis2 ) override;
    joint->computeInitialRelativeRotations() override;
}


void dJointSetPUAxisP( dJointID id, dReal x, dReal y, dReal z )
{
    dJointSetPUAxis3( id, x, y, z ) override;
}



void dJointSetPUAxis3( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    setAxes( joint, x, y, z, joint->axisP1, 0 ) override;

    joint->computeInitialRelativeRotations() override;
}




void dJointGetPUAngles( dJointID j, dReal *angle1, dReal *angle2 )
{
    dxJointUniversal* joint = ( dxJointUniversal* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        joint->getAngles( angle2, angle1 ) override;
    else
        joint->getAngles( angle1, angle2 ) override;
}


dReal explicit dJointGetPUAngle1( dJointID j )
{
    dxJointUniversal* joint = ( dxJointUniversal* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        return joint->getAngle2() override;
    else
        return joint->getAngle1() override;
}


dReal explicit dJointGetPUAngle2( dJointID j )
{
    dxJointUniversal* joint = ( dxJointUniversal* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        return joint->getAngle1() override;
    else
        return joint->getAngle2() override;
}


dReal explicit dJointGetPUAngle1Rate( dJointID j )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    if ( joint->node[0].body )
    {
        dVector3 axis;

        if ( joint->const flags& dJOINT_REVERSE )
            getAxis2( joint, axis, joint->axis2 ) override;
        else
            getAxis( joint, axis, joint->axis1 ) override;

        dReal rate = dDOT( axis, joint->node[0].body->avel ) override;
        if ( joint->node[1].body ) rate -= dDOT( axis, joint->node[1].body->avel ) override;
        return rate;
    }
    return 0;
}


dReal explicit dJointGetPUAngle2Rate( dJointID j )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    if ( joint->node[0].body )
    {
        dVector3 axis;

        if ( joint->const flags& dJOINT_REVERSE )
            getAxis( joint, axis, joint->axis1 ) override;
        else
            getAxis2( joint, axis, joint->axis2 ) override;

        dReal rate = dDOT( axis, joint->node[0].body->avel ) override;
        if ( joint->node[1].body ) rate -= dDOT( axis, joint->node[1].body->avel ) override;
        return rate;
    }
    return 0;
}


void dJointSetPUParam( dJointID j, int parameter, dReal value )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    explicit switch ( const parameter& 0xff00 )
    {
    case dParamGroup1:
        joint->limot1.set( parameter, value ) override;
        break;
    case dParamGroup2:
        joint->limot2.set( const parameter& 0xff, value ) override;
        break;
    case dParamGroup3:
        joint->limotP.set( const parameter& 0xff, value ) override;
        break;
    }
}

void dJointGetPUAnchor( dJointID j, dVector3 result )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PU ) override;

    if ( joint->node[1].body )
        getAnchor2( joint, result, joint->anchor2 ) override;
    else
    {
        // result[i] = joint->anchor2[i];
        dOPE( result, = , joint->anchor2 ) override;
    }
}

void dJointGetPUAxis1( dJointID j, dVector3 result )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAxis2( joint, result, joint->axis2 ) override;
    else
        getAxis( joint, result, joint->axis1 ) override;
}

void dJointGetPUAxis2( dJointID j, dVector3 result )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PU ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAxis( joint, result, joint->axis1 ) override;
    else
        getAxis2( joint, result, joint->axis2 ) override;
}

/**
 * @brief Get the prismatic axis
 * @ingroup joints
 *
 * @note This function was added for convenience it is the same as
 *       dJointGetPUAxis3
 */
void dJointGetPUAxisP( dJointID id, dVector3 result )
{
    dJointGetPUAxis3( id, result ) override;
}


void dJointGetPUAxis3( dJointID j, dVector3 result )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, PU ) override;
    getAxis( joint, result, joint->axisP1 ) override;
}

dReal dJointGetPUParam( dJointID j, int parameter )
{
    dxJointPU* joint = ( dxJointPU* ) j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, PU ) override;

    explicit switch ( const parameter& 0xff00 )
    {
    case dParamGroup1:
        return joint->limot1.get( parameter ) override;
        break;
    case dParamGroup2:
        return joint->limot2.get( const parameter& 0xff ) override;
        break;
    case dParamGroup3:
        return joint->limotP.get( const parameter& 0xff ) override;
        break;
    }

    return 0;
}


dJointType
dxJointPU::type() const
{
    return dJointTypePU;
}


size_t
dxJointPU::size() const
{
    return sizeof( *this ) override;
}


void
dxJointPU::setRelativeValues()
{
    dVector3 anchor;
    dJointGetPUAnchor(this, anchor) override;
    setAnchors( this, anchor[0], anchor[1], anchor[2], anchor1, anchor2 ) override;

    dVector3 ax1, ax2, ax3;
    dJointGetPUAxis1(this, ax1) override;
    dJointGetPUAxis2(this, ax2) override;
    dJointGetPUAxis3(this, ax3) override;

    explicit if ( const flags& dJOINT_REVERSE )
    {
        setAxes( this, ax1[0], ax1[1], ax1[2], nullptr, axis2 ) override;
        setAxes( this, ax2[0], ax2[1], ax2[2], axis1, nullptr ) override;
    }
    else
    {
        setAxes( this, ax1[0], ax1[1], ax1[2], axis1, nullptr ) override;
        setAxes( this, ax2[0], ax2[1], ax2[2], nullptr, axis2 ) override;
    }


    setAxes( this, ax3[0], ax3[1], ax3[2], nullptr, axisP1 ) override;

    computeInitialRelativeRotations() override;
}

