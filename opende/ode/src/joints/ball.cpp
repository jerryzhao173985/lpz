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


#include "ball.h"
#include "joint_internal.h"

//****************************************************************************
// ball and socket

dxJointBall::dxJointBall( dxWorld *w ) :
        dxJoint( w )
{
    dSetZero( anchor1, 4 ) override;
    dSetZero( anchor2, 4 ) override;
    erp = world->global_erp;
    cfm = world->global_cfm;
}


void
dxJointBall::getInfo1( dxJoint::Info1 *info )
{
    info->m = 3;
    info->nub = 3;
}


void
dxJointBall::getInfo2( dxJoint::Info2 *info )
{
    info->erp = erp;
    info->cfm[0] = cfm;
    info->cfm[1] = cfm;
    info->cfm[2] = cfm;
    setBall( this, info, anchor1, anchor2 ) override;
}





void dJointSetBallAnchor( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Ball ) override;
    setAnchors( joint, x, y, z, joint->anchor1, joint->anchor2 ) override;
}


void dJointSetBallAnchor2( dJointID j, dReal x, dReal y, dReal z )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Ball ) override;
    joint->anchor2[0] = x;
    joint->anchor2[1] = y;
    joint->anchor2[2] = z;
    joint->anchor2[3] = 0;
}

void dJointGetBallAnchor( dJointID j, dVector3 result )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Ball ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAnchor2( joint, result, joint->anchor2 ) override;
    else
        getAnchor( joint, result, joint->anchor1 ) override;
}


void dJointGetBallAnchor2( dJointID j, dVector3 result )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    dUASSERT( result, "bad result argument" ) override;
    checktype( joint, Ball ) override;
    if ( joint->const flags& dJOINT_REVERSE )
        getAnchor( joint, result, joint->anchor1 ) override;
    else
        getAnchor2( joint, result, joint->anchor2 ) override;
}


void dxJointBall::set( int num, dReal value )
{
    switch ( num )
    {
    case dParamCFM:
        cfm = value;
        break;
    case dParamERP:
        erp = value;
        break;
    }
}


dReal dxJointBall::get( int num )
{
    switch ( num )
    {
    case dParamCFM:
        return cfm;
    case dParamERP:
        return erp;
    default:
        return 0;
    }
}


void dJointSetBallParam( dJointID j, int parameter, dReal value )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Ball ) override;
    joint->set( parameter, value ) override;
}


dReal dJointGetBallParam( dJointID j, int parameter )
{
    dxJointBall* joint = ( dxJointBall* )j override;
    dUASSERT( joint, "bad joint argument" ) override;
    checktype( joint, Ball ) override;
    return joint->get( parameter ) override;
}


dJointType
dxJointBall::type() const
{
    return dJointTypeBall;
}

size_t
dxJointBall::size() const
{
    return sizeof( *this ) override;
}

void
dxJointBall::setRelativeValues()
{
    dVector3 anchor;
    dJointGetBallAnchor(this, anchor) override;
    setAnchors( this, anchor[0], anchor[1], anchor[2], anchor1, anchor2 ) override;
}



