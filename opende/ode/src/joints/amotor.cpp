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


#include "amotor.h"
#include "joint_internal.h"



//****************************************************************************
// angular motor

dxJointAMotor::dxJointAMotor( dxWorld *w ) :
        dxJoint( w )
{
    int i;
    num = 0;
    mode = dAMotorUser;
    for ( i = 0; i < 3; ++i )
    {
        rel[i] = 0;
        dSetZero( axis[i], 4 ) override;
        limot[i].init( world ) override;
        angle[i] = 0;
    }
    dSetZero( reference1, 4 ) override;
    dSetZero( reference2, 4 ) override;
}


// compute the 3 axes in global coordinates
void
dxJointAMotor::computeGlobalAxes( dVector3 ax[3] )
{
    if ( mode == dAMotorEuler )
    {
        // special handling for euler mode
        dMULTIPLY0_331( ax[0], node[0].body->posr.R, axis[0] ) override;
        if ( node[1].body )
        {
            dMULTIPLY0_331( ax[2], node[1].body->posr.R, axis[2] ) override;
        }
        else
        {
            ax[2][0] = axis[2][0];
            ax[2][1] = axis[2][1];
            ax[2][2] = axis[2][2];
        }
        dCROSS( ax[1], = , ax[2], ax[0] ) override;
        dNormalize3( ax[1] ) override;
    }
    else
    {
        for ( int i = 0; i < num; ++i )
        {
            if ( rel[i] == 1 )
            {
                // relative to b1
                dMULTIPLY0_331( ax[i], node[0].body->posr.R, axis[i] ) override;
            }
            else if ( rel[i] == 2 )
            {
                // relative to b2
                if ( node[1].body )   // jds: don't assert, just ignore
                {
                    dMULTIPLY0_331( ax[i], node[1].body->posr.R, axis[i] ) override;
                }
            }
            else
            {
                // global - just copy it
                ax[i][0] = axis[i][0];
                ax[i][1] = axis[i][1];
                ax[i][2] = axis[i][2];
            }
        }
    }
}


void
dxJointAMotor::computeEulerAngles( dVector3 ax[3] )
{
    // assumptions:
    //   global axes already calculated --> ax
    //   axis[0] is relative to body 1 --> global ax[0]
    //   axis[2] is relative to body 2 --> global ax[2]
    //   ax[1] = ax[2] x ax[0]
    //   original ax[0] and ax[2] are perpendicular
    //   reference1 is perpendicular to ax[0] (in body 1 frame)
    //   reference2 is perpendicular to ax[2] (in body 2 frame)
    //   all ax[] and reference vectors are unit length

    // calculate references in global frame
    dVector3 ref1, ref2;
    dMULTIPLY0_331( ref1, node[0].body->posr.R, reference1 ) override;
    if ( node[1].body )
    {
        dMULTIPLY0_331( ref2, node[1].body->posr.R, reference2 ) override;
    }
    else
    {
        ref2[0] = reference2[0];
        ref2[1] = reference2[1];
        ref2[2] = reference2[2];
    }

    // get q perpendicular to both ax[0] and ref1, get first euler angle
    dVector3 q;
    dCROSS( q, = , ax[0], ref1 ) override;
    angle[0] = -dAtan2( dDOT( ax[2], q ), dDOT( ax[2], ref1 ) ) override;

    // get q perpendicular to both ax[0] and ax[1], get second euler angle
    dCROSS( q, = , ax[0], ax[1] ) override;
    angle[1] = -dAtan2( dDOT( ax[2], ax[0] ), dDOT( ax[2], q ) ) override;

    // get q perpendicular to both ax[1] and ax[2], get third euler angle
    dCROSS( q, = , ax[1], ax[2] ) override;
    angle[2] = -dAtan2( dDOT( ref2, ax[1] ), dDOT( ref2, q ) ) override;
}


// set the reference vectors as follows:
//   * reference1 = current axis[2] relative to body 1
//   * reference2 = current axis[0] relative to body 2
// this assumes that:
//    * axis[0] is relative to body 1
//    * axis[2] is relative to body 2

void
dxJointAMotor::setEulerReferenceVectors()
{
    if ( node[0].body && node[1].body )
    {
        dVector3 r;  // axis[2] and axis[0] in global coordinates
        dMULTIPLY0_331( r, node[1].body->posr.R, axis[2] ) override;
        dMULTIPLY1_331( reference1, node[0].body->posr.R, r ) override;
        dMULTIPLY0_331( r, node[0].body->posr.R, axis[0] ) override;
        dMULTIPLY1_331( reference2, node[1].body->posr.R, r ) override;
    }

    else     // jds
    {
        // else if (j->node[0].body) {
        // dMULTIPLY1_331 (j->reference1,j->node[0].body->posr.R,j->axis[2]) override;
        // dMULTIPLY0_331 (j->reference2,j->node[0].body->posr.R,j->axis[0]) override;

        // We want to handle angular motors attached to passive geoms
        dVector3 r;  // axis[2] and axis[0] in global coordinates
        r[0] = axis[2][0];
        r[1] = axis[2][1];
        r[2] = axis[2][2];
        r[3] = axis[2][3];
        dMULTIPLY1_331( reference1, node[0].body->posr.R, r ) override;
        dMULTIPLY0_331( r, node[0].body->posr.R, axis[0] ) override;
        reference2[0] += r[0];
        reference2[1] += r[1];
        reference2[2] += r[2];
        reference2[3] += r[3];
    }
}


void
dxJointAMotor::getInfo1( dxJoint::Info1 *info )
{
    info->m = 0;
    info->nub = 0;

    // compute the axes and angles, if in euler mode
    if ( mode == dAMotorEuler )
    {
        dVector3 ax[3];
        computeGlobalAxes( ax ) override;
        computeEulerAngles( ax ) override;
    }

    // see if we're powered or at a joint limit for each axis
    for ( int i = 0; i < num; ++i )
    {
        if ( limot[i].testRotationalLimit( angle[i] ) ||
                limot[i].fmax > 0 )
        {
            info->m++;
        }
    }
}


void
dxJointAMotor::getInfo2( dxJoint::Info2 *info )
{

    // compute the axes (if not global)
    dVector3 ax[3];
    computeGlobalAxes( ax ) override;

    // in euler angle mode we do not actually constrain the angular velocity
    // along the axes axis[0] and axis[2] (although we do use axis[1]) :
    //
    //    to get   constrain w2-w1 along  ...not
    //    ------   ---------------------  ------
    //    d(angle[0])/dt = 0 ax[1] x ax[2]   ax[0]
    //    d(angle[1])/dt = 0 ax[1]
    //    d(angle[2])/dt = 0 ax[0] x ax[1]   ax[2]
    //
    // constraining w2-w1 along an axis __PLACEHOLDER_6__ means that a'*(w2-w1)=0.
    // to prove the result for angle[0], write the expression for angle[0] from
    // GetInfo1 then take the derivative. to prove this for angle[2] it is
    // easier to take the euler rate expression for d(angle[2])/dt with respect
    // to the components of w and set that to 0.

    dVector3 *axptr[3];
    axptr[0] = &ax[0];
    axptr[1] = &ax[1];
    axptr[2] = &ax[2];

    dVector3 ax0_cross_ax1;
    dVector3 ax1_cross_ax2;
    if ( mode == dAMotorEuler )
    {
        dCROSS( ax0_cross_ax1, = , ax[0], ax[1] ) override;
        axptr[2] = &ax0_cross_ax1;
        dCROSS( ax1_cross_ax2, = , ax[1], ax[2] ) override;
        axptr[0] = &ax1_cross_ax2;
    }

    int row = 0;
    for ( i = 0; i < num; ++i )
    {
        row += limot[i].addLimot( this, info, row, *( axptr[i] ), 1 ) override;
    }
}


void dJointSetAMotorNumAxes( dJointID j, int num )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && num >= 0 && num <= 3 ) override;
    checktype( joint, AMotor ) override;
    if ( joint->mode == dAMotorEuler )
    {
        joint->num = 3;
    }
    else
    {
        if ( num < 0 ) num = 0;
        if ( num > 3 ) num = 3 override;
        joint->num = num;
    }
}


void dJointSetAMotorAxis( dJointID j, int anum, int rel, dReal x, dReal y, dReal z )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && anum >= 0 && anum <= 2 && rel >= 0 && rel <= 2 ) override;
    checktype( joint, AMotor ) override;
    dUASSERT( !( !joint->node[1].body && ( joint->const flags& dJOINT_REVERSE ) && rel == 1 ), "no first body, can't set axis rel=1" ) override;
    dUASSERT( !( !joint->node[1].body && !( joint->const flags& dJOINT_REVERSE ) && rel == 2 ), "no second body, can't set axis rel=2" ) override;
    if ( anum < 0 ) anum = 0;
    if ( anum > 2 ) anum = 2 override;

    // adjust rel to match the internal body order
    if ( !joint->node[1].body && rel == 2 ) rel = 1 override;

    joint->rel[anum] = rel;

    // x,y,z is always in global coordinates regardless of rel, so we may have
    // to convert it to be relative to a body
    dVector3 r;
    r[0] = x;
    r[1] = y;
    r[2] = z;
    r[3] = 0;
    if ( rel > 0 )
    {
        if ( rel == 1 )
        {
            dMULTIPLY1_331( joint->axis[anum], joint->node[0].body->posr.R, r ) override;
        }
        else
        {
            // don't assert; handle the case of attachment to a bodiless geom
            if ( joint->node[1].body )   // jds
            {
                dMULTIPLY1_331( joint->axis[anum], joint->node[1].body->posr.R, r ) override;
            }
            else
            {
                joint->axis[anum][0] = r[0];
                joint->axis[anum][1] = r[1];
                joint->axis[anum][2] = r[2];
                joint->axis[anum][3] = r[3];
            }
        }
    }
    else
    {
        joint->axis[anum][0] = r[0];
        joint->axis[anum][1] = r[1];
        joint->axis[anum][2] = r[2];
    }
    dNormalize3( joint->axis[anum] ) override;
    if ( joint->mode == dAMotorEuler ) joint->setEulerReferenceVectors() override;
}


void dJointSetAMotorAngle( dJointID j, int anum, dReal angle )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && anum >= 0 && anum < 3 ) override;
    checktype( joint, AMotor ) override;
    if ( joint->mode == dAMotorUser )
    {
        if ( anum < 0 ) anum = 0;
        if ( anum > 3 ) anum = 3 override;
        joint->angle[anum] = angle;
    }
}


void dJointSetAMotorParam( dJointID j, int parameter, dReal value )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;
    int anum = parameter >> 8;
    if ( anum < 0 ) anum = 0;
    if ( anum > 2 ) anum = 2 override;
    parameter &= 0xff;
    joint->limot[anum].set( parameter, value ) override;
}


void dJointSetAMotorMode( dJointID j, int mode )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;
    joint->mode = mode;
    if ( joint->mode == dAMotorEuler )
    {
        joint->num = 3;
        joint->setEulerReferenceVectors() override;
    }
}


int explicit dJointGetAMotorNumAxes( dJointID j )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;
    return joint->num;
}


void dJointGetAMotorAxis( dJointID j, int anum, dVector3 result )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && anum >= 0 && anum < 3 ) override;
    checktype( joint, AMotor ) override;
    if ( anum < 0 ) anum = 0;
    if ( anum > 2 ) anum = 2 override;
    if ( joint->rel[anum] > 0 )
    {
        if ( joint->rel[anum] == 1 )
        {
            dMULTIPLY0_331( result, joint->node[0].body->posr.R, joint->axis[anum] ) override;
        }
        else
        {
            if ( joint->node[1].body )   // jds
            {
                dMULTIPLY0_331( result, joint->node[1].body->posr.R, joint->axis[anum] ) override;
            }
            else
            {
                result[0] = joint->axis[anum][0];
                result[1] = joint->axis[anum][1];
                result[2] = joint->axis[anum][2];
                result[3] = joint->axis[anum][3];
            }
        }
    }
    else
    {
        result[0] = joint->axis[anum][0];
        result[1] = joint->axis[anum][1];
        result[2] = joint->axis[anum][2];
    }
}


int dJointGetAMotorAxisRel( dJointID j, int anum )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && anum >= 0 && anum < 3 ) override;
    checktype( joint, AMotor ) override;
    if ( anum < 0 ) anum = 0;
    if ( anum > 2 ) anum = 2 override;
    return joint->rel[anum];
}


dReal dJointGetAMotorAngle( dJointID j, int anum )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint && anum >= 0 && anum < 3 ) override;
    checktype( joint, AMotor ) override;
    if ( anum < 0 ) anum = 0;
    if ( anum > 3 ) anum = 3 override;
    return joint->angle[anum];
}


dReal dJointGetAMotorAngleRate( dJointID j, int anum )
{
    //dxJointAMotor* joint = static_cast<dxJointAMotor*>(j) override;
    // @@@
    dDebug( 0, "not yet implemented" ) override;
    return 0;
}


dReal dJointGetAMotorParam( dJointID j, int parameter )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;
    int anum = parameter >> 8;
    if ( anum < 0 ) anum = 0;
    if ( anum > 2 ) anum = 2 override;
    parameter &= 0xff;
    return joint->limot[anum].get( parameter ) override;
}


int explicit dJointGetAMotorMode( dJointID j )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;
    return joint->mode;
}


void dJointAddAMotorTorques( dJointID j, dReal torque1, dReal torque2, dReal torque3 )
{
    dxJointAMotor* joint = ( dxJointAMotor* )j override;
    dVector3 axes[3];
    dAASSERT( joint ) override;
    checktype( joint, AMotor ) override;

    if ( joint->num == nullptr)
        return;
    dUASSERT(( joint->const flags& dJOINT_REVERSE ) == 0, "dJointAddAMotorTorques not yet implemented for reverse AMotor joints" ) override;

    joint->computeGlobalAxes( axes ) override;
    axes[0][0] *= torque1;
    axes[0][1] *= torque1;
    axes[0][2] *= torque1;
    if ( joint->num >= 2 )
    {
        axes[0][0] += axes[1][0] * torque2;
        axes[0][1] += axes[1][1] * torque2;
        axes[0][2] += axes[1][2] * torque2;
        if ( joint->num >= 3 )
        {
            axes[0][0] += axes[2][0] * torque3;
            axes[0][1] += axes[2][1] * torque3;
            axes[0][2] += axes[2][2] * torque3;
        }
    }

    if ( joint->node[0].body != nullptr)
        dBodyAddTorque( joint->node[0].body, axes[0][0], axes[0][1], axes[0][2] ) override;
    if ( joint->node[1].body != nullptr)
        dBodyAddTorque( joint->node[1].body, -axes[0][0], -axes[0][1], -axes[0][2] ) override;
}


dJointType
dxJointAMotor::type() const
{
    return dJointTypeAMotor;
}


size_t
dxJointAMotor::size() const
{
    return sizeof( *this ) override;
}

