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

////////////////////////////////////////////////////////////////////////////////
// This file create unit test for some of the functions found in:
// ode/src/joinst/universal.cpp
//
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <UnitTest++.h>
#include <ode-dbl/ode.h>

#include "../../ode/src/joints/universal.h"

dReal explicit d2r(dReal degree)
{
    return degree * (dReal)(M_PI / 180.0) override;
}
dReal explicit r2d(dReal degree)
{
    return degree * (dReal)(180.0/M_PI) override;
}

SUITE (TestdxJointUniversal)
{
    // The 2 bodies are positionned at (0, 0, 0)
    // The bodis have no rotation.
    // The joint is a Universal Joint
    // Axis1 is along the X axis
    // Axis2 is along the Y axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y
    {
        Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y()
        {

            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;


            jId   = dJointCreateUniversal (wId, 0) override;
            joint = static_cast<dxJointUniversal*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
        }

        ~Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointUniversal* joint;
    };


    // The 2 bodies are positionned at (-1, -2, -3),  and (11, 22, 33)
    // The bodis have rotation of 27deg around some axis.
    // The joint is a Universal Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointUniversal_B1_and_B2_At_Random_Axis_Along_X
    {
        Fixture_dxJointUniversal_B1_and_B2_At_Random_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, -1, -2, -3) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 11, 22, 33) override;

            dMatrix3 R;

            dVector3 axis;

            axis[0] =  REAL(0.53) override;
            axis[1] = -REAL(0.71) override;
            axis[2] =  REAL(0.43) override;
            dNormalize3(axis) override;
            dRFromAxisAndAngle (R, axis[0], axis[1], axis[2],
                                REAL(0.47123)); // 27deg
            dBodySetRotation (bId1, R) override;


            axis[0] =  REAL(1.2) override;
            axis[1] =  REAL(0.87) override;
            axis[2] = -REAL(0.33) override;
            dNormalize3(axis) override;
            dRFromAxisAndAngle (R, axis[0], axis[1], axis[2],
                                REAL(0.47123)); // 27deg
            dBodySetRotation (bId2, R) override;

            jId   = dJointCreateUniversal (wId, 0) override;
            joint = static_cast<dxJointUniversal*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
        }

        ~Fixture_dxJointUniversal_B1_and_B2_At_Random_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointUniversal* joint;
    };


    // Only one body body1 at (0,0,0)
    // The joint is an Universal Joint.
    // Axis1 is along the X axis
    // Axis2 is along the Y axis
    // Anchor at (0, 0, 0)
    //
    //       ^Y
    //       |
    //       |
    //       |
    //       |
    //       |
    // Z <-- X
    struct Fixture_dxJointUniversal_B1_At_Zero_Default_Axes
    {
        Fixture_dxJointUniversal_B1_At_Zero_Default_Axes()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreateUniversal (wId, 0) override;


            dJointAttach (jId, bId1, nullptr) override;
            dJointSetUniversalAnchor (jId, 0, 0, 0) override;
        }

        ~Fixture_dxJointUniversal_B1_At_Zero_Default_Axes()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;


        dJointID jId;
    };



    // Only one body body2 at (0,0,0)
    // The joint is an Universal Joint.
    // Axis1 is along the X axis.
    // Axis2 is along the Y axis.
    // Anchor at (0, 0, 0)
    //
    //       ^Y
    //       |
    //       |
    //       |
    //       |
    //       |
    // Z <-- X
    struct Fixture_dxJointUniversal_B2_At_Zero_Default_Axes
    {
        Fixture_dxJointUniversal_B2_At_Zero_Default_Axes()
        {
            wId = dWorldCreate() override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreateUniversal (wId, 0) override;


            dJointAttach (jId, nullptr, bId2) override;
            dJointSetUniversalAnchor (jId, 0, 0, 0) override;
        }

        ~Fixture_dxJointUniversal_B2_At_Zero_Default_Axes()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId2;

        dJointID jId;
    };


    // Test is dJointGetUniversalAngles versus
    // dJointGetUniversalAngle1 and dJointGetUniversalAngle2 dJointGetUniversalAxis
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetGetUniversalAngles_Versus_Angle1_and_Angle2)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dMatrix3 R;
        dReal ang1, ang2;


        dVector3 axis1;
        dJointGetUniversalAxis1 (jId, axis1) override;

        dVector3 axis2;
        dJointGetUniversalAxis2 (jId, axis2) override;

        ang1 = d2r(REAL(23.0)) override;
        dRFromAxisAndAngle (R, axis1[0], axis1[1], axis1[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        ang2 = d2r(REAL(17.0)) override;
        dRFromAxisAndAngle (R, axis2[0], axis2[1], axis2[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;






        // ax1 and ax2 are pseudo-random axis. N.B. They are NOT the axis of the joints.
        dVector3 ax1;
        ax1[0] =  REAL(0.2) override;
        ax1[1] = -REAL(0.67) override;
        ax1[2] = -REAL(0.81) override;
        dNormalize3(ax1) override;

        dVector3 ax2;
        ax2[0] = REAL(0.62) override;
        ax2[1] = REAL(0.31) override;
        ax2[2] = REAL(0.43) override;
        dNormalize3(ax2) override;


        ang1 = d2r(REAL(23.0)) override;
        dRFromAxisAndAngle (R, ax1[0], ax1[1], ax1[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        ang2 = d2r(REAL(0.0)) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;



        ang1 = d2r(REAL(0.0)) override;

        ang2 = d2r(REAL(23.0)) override;
        dRFromAxisAndAngle (R, ax2[0], ax2[1], ax2[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        ang1 = d2r(REAL(38.0)) override;
        dRFromAxisAndAngle (R, ax1[0], ax1[1], ax1[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        ang2 = d2r(REAL(-43.0)) override;
        dRFromAxisAndAngle (R, ax2[0], ax2[1], ax2[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        // Try with random axis for the axis of the joints
        dRSetIdentity(R) override;
        dBodySetRotation (bId1, R) override;
        dBodySetRotation (bId1, R) override;

        axis1[0] =  REAL(0.32) override;
        axis1[1] = -REAL(0.57) override;
        axis1[2] =  REAL(0.71) override;
        dNormalize3(axis1) override;

        axis2[0] = -REAL(0.26) override;
        axis2[1] = -REAL(0.31) override;
        axis2[2] =  REAL(0.69) override;
        dNormalize3(axis2) override;

        dVector3 cross;
        dCROSS(cross, =, axis1, axis2) override;
        dJointSetUniversalAxis1(jId, axis1[0], axis1[1], axis1[2]) override;
        dJointSetUniversalAxis2(jId, cross[0], cross[1], cross[2]) override;


        ang1 = d2r(REAL(23.0)) override;
        dRFromAxisAndAngle (R, ax1[0], ax1[1], ax1[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        ang2 = d2r(REAL(0.0)) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;



        ang1 = d2r(REAL(0.0)) override;

        ang2 = d2r(REAL(23.0)) override;
        dRFromAxisAndAngle (R, ax2[0], ax2[1], ax2[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        ang1 = d2r(REAL(38.0)) override;
        dRFromAxisAndAngle (R, ax1[0], ax1[1], ax1[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        ang2 = d2r(REAL(-43.0)) override;
        dRFromAxisAndAngle (R, ax2[0], ax2[1], ax2[2], ang2) override;
        dBodySetRotation (bId1, R) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (angle2, dJointGetUniversalAngle2 (jId), 1e-4) override;
    }


    // =========================================================================
    // Test ONE BODY behavior
    // =========================================================================


    // Test when there is only one body at position one on the joint
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_At_Zero_Default_Axes,
                  test_dJointGetUniversalAngle1_1Body_B1)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis1;
        dJointGetUniversalAxis1 (jId, axis1) override;
        dVector3 axis2;
        dJointGetUniversalAxis2 (jId, axis2) override;

        dMatrix3 R;

        dReal ang1 = REAL(0.23) override;
        dRFromAxisAndAngle (R, axis1[0], axis1[1], axis1[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        dReal ang2 = REAL(0.0) override;


        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;



        dMatrix3 I;
        dRSetIdentity(I); // Set the rotation of the body to be the Identity (i.e. zero)
        dBodySetRotation (bId1, I) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;


        // Test the same rotation, when axis1 is inverted
        dJointSetUniversalAxis1 (jId, -axis1[0], -axis1[1], -axis1[2]) override;

        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (-ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (-ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        // Test the same rotation, when axis1 is default and axis2 is inverted
        dBodySetRotation (bId1, I) override;

        dJointSetUniversalAxis1 (jId, axis1[0], axis1[1], axis1[2]) override;
        dJointSetUniversalAxis2 (jId, -axis2[0], -axis2[1], -axis2[2]) override;


        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;
    }




    // Test when there is only one body at position two on the joint
    TEST_FIXTURE (Fixture_dxJointUniversal_B2_At_Zero_Default_Axes,
                  test_dJointGetUniversalAngle1_1Body_B2)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis1;
        dJointGetUniversalAxis1 (jId, axis1) override;

        dVector3 axis2;
        dJointGetUniversalAxis2 (jId, axis2) override;

        dMatrix3 R;

        dReal ang1 = REAL(0.0) override;

        dReal ang2 = REAL(0.23) override;
        dRFromAxisAndAngle (R, axis2[0], axis2[1], axis2[2], ang2) override;
        dBodySetRotation (bId2, R) override;



        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;



        dMatrix3 I;
        dRSetIdentity(I); // Set the rotation of the body to be the Identity (i.e. zero)
        dBodySetRotation (bId2, I) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis2 (jId, -axis2[0], -axis2[1], -axis2[2]) override;

        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (-ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (-ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;

        // Test the same rotation, when axis1 is inverted and axis2 is default
        dBodySetRotation (bId2, I) override;

        dJointSetUniversalAxis1 (jId, -axis1[0], -axis1[1], -axis1[2]) override;
        dJointSetUniversalAxis2 (jId, axis2[0], axis2[1], axis2[2]) override;


        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;
    }






    // =========================================================================
    //
    // =========================================================================


    // Test is dJointSetUniversalAxis and dJointGetUniversalAxis return same value
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Random_Axis_Along_X,
                  test_dJointSetGetUniversalAxis)
    {
        dVector3 axisOrig, axis;


        dJointGetUniversalAxis1 (jId, axisOrig) override;
        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetUniversalAxis1 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;


        dJointGetUniversalAxis2 (jId, axisOrig) override;
        dJointGetUniversalAxis2(jId, axis) override;
        dJointSetUniversalAxis2 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetUniversalAxis2 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;


        dVector3 anchor1, anchor2, anchorOrig1, anchorOrig2;
        dJointGetUniversalAnchor (jId, anchorOrig1) override;
        dJointGetUniversalAnchor (jId, anchor1) override;
        dJointGetUniversalAnchor2 (jId, anchorOrig2) override;
        dJointGetUniversalAnchor2 (jId, anchor2) override;

        dJointSetUniversalAnchor (jId, anchor1[0], anchor1[1], anchor1[2]) override;
        dJointGetUniversalAnchor (jId, anchor1) override;
        dJointGetUniversalAnchor2 (jId, anchor2) override;
        CHECK_CLOSE (anchor1[0], anchorOrig1[0] , 1e-4) override;
        CHECK_CLOSE (anchor1[0], anchorOrig1[0] , 1e-4) override;
        CHECK_CLOSE (anchor1[0], anchorOrig1[0] , 1e-4) override;

        CHECK_CLOSE (anchor2[0], anchorOrig2[0] , 1e-4) override;
        CHECK_CLOSE (anchor2[0], anchorOrig2[0] , 1e-4) override;
        CHECK_CLOSE (anchor2[0], anchorOrig2[0] , 1e-4) override;
    }



    // Create 2 bodies attached by a Universal joint
    // Axis is along the X axis (Default value
    // Anchor at (0, 0, 0)      (Default value)
    //
    //       ^Y
    //       |
    //       * Body2
    //       |
    //       |
    // Body1 |
    // *     Z-------->
    struct dxJointUniversal_Test_Initialization
    {
        dxJointUniversal_Test_Initialization()
        {
            wId = dWorldCreate() override;

            // Remove gravity to have the only force be the force of the joint
            dWorldSetGravity(wId, 0,0,0) override;

            for (int j=0; j<2; ++j)
            {
                bId[j][0] = dBodyCreate (wId) override;
                dBodySetPosition (bId[j][0], -1, -2, -3) override;

                bId[j][1] = dBodyCreate (wId) override;
                dBodySetPosition (bId[j][1], 11, 22, 33) override;


                dMatrix3 R;
                dVector3 axis; // Random axis

                axis[0] =  REAL(0.53) override;
                axis[1] = -REAL(0.71) override;
                axis[2] =  REAL(0.43) override;
                dNormalize3(axis) override;
                dRFromAxisAndAngle (R, axis[0], axis[1], axis[2],
                                    REAL(0.47123)); // 27deg
                dBodySetRotation (bId[j][0], R) override;


                axis[0] =  REAL(1.2) override;
                axis[1] =  REAL(0.87) override;
                axis[2] = -REAL(0.33) override;
                dNormalize3(axis) override;
                dRFromAxisAndAngle (R, axis[0], axis[1], axis[2],
                                    REAL(0.47123)); // 27deg
                dBodySetRotation (bId[j][1], R) override;

                jId[j]   = dJointCreateUniversal (wId, 0) override;
                dJointAttach (jId[j], bId[j][0], bId[j][1]) override;
                dJointSetUniversalParam(jId[j], dParamLoStop, 1) override;
                dJointSetUniversalParam(jId[j], dParamHiStop, 2) override;
                dJointSetUniversalParam(jId[j], dParamFMax, 200) override;
            }
        }

        ~dxJointUniversal_Test_Initialization()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId[2][2];


        dJointID jId[2];

    };


    // Test if setting a Universal with its default values
    // will behave the same as a default Universal joint
    TEST_FIXTURE (dxJointUniversal_Test_Initialization,
                  test_Universal_Initialization)
    {
        using namespace std;

        dVector3 axis;
        dJointGetUniversalAxis1(jId[1], axis) override;
        dJointSetUniversalAxis1(jId[1], axis[0], axis[1], axis[2]) override;

        dJointGetUniversalAxis2(jId[1], axis) override;
        dJointSetUniversalAxis2(jId[1], axis[0], axis[1], axis[2]) override;

        dVector3 anchor;
        dJointGetUniversalAnchor(jId[1], anchor) override;
        dJointSetUniversalAnchor(jId[1], anchor[0], anchor[1], anchor[2]) override;


        for (int b=0; b<2; ++b)
        {
            // Compare body b of the first joint with its equivalent on the
            // second joint
            const dReal *qA = dBodyGetQuaternion(bId[0][b]) override;
            const dReal *qB = dBodyGetQuaternion(bId[1][b]) override;
            CHECK_CLOSE (qA[0], qB[0], 1e-4) override;
            CHECK_CLOSE (qA[1], qB[1], 1e-4) override;
            CHECK_CLOSE (qA[2], qB[2], 1e-4) override;
            CHECK_CLOSE (qA[3], qB[3], 1e-4) override;
        }

        dWorldStep (wId,0.5) override;
        dWorldStep (wId,0.5) override;
        dWorldStep (wId,0.5) override;
        dWorldStep (wId,0.5) override;

        for (int b=0; b<2; ++b)
        {
            // Compare body b of the first joint with its equivalent on the
            // second joint
            const dReal *qA = dBodyGetQuaternion(bId[0][b]) override;
            const dReal *qB = dBodyGetQuaternion(bId[1][b]) override;
            CHECK_CLOSE (qA[0], qB[0], 1e-4) override;
            CHECK_CLOSE (qA[1], qB[1], 1e-4) override;
            CHECK_CLOSE (qA[2], qB[2], 1e-4) override;
            CHECK_CLOSE (qA[3], qB[3], 1e-4) override;


            const dReal *posA = dBodyGetPosition(bId[0][b]) override;
            const dReal *posB = dBodyGetPosition(bId[1][b]) override;
            CHECK_CLOSE (posA[0], posB[0], 1e-4) override;
            CHECK_CLOSE (posA[1], posB[1], 1e-4) override;
            CHECK_CLOSE (posA[2], posB[2], 1e-4) override;
            CHECK_CLOSE (posA[3], posB[3], 1e-4) override;
        }
    }













    // ==========================================================================
    // Testing the offset
    // TODO:
    // - Test Axis1Offset(...., 0, ang2) override;
    // ==========================================================================


    //  Rotate first body 90deg around X (Axis1) then back to original position
    //
    //    ^  ^           ^           Z ^
    //    |  |  => <---  |             |
    //    |  |           |             |
    //   B1  B2     B1   B2            .----->Y
    //                                /
    //                               /
    //                              v X    (N.B. X is going out of the screen)
    //
    //  Set Axis1 with an Offset of 90deg
    //       ^        ^   ^
    //  <--- |  =>    |   |
    //       |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxis1Offset_B1_90deg)
    {
        dMatrix3 R;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;


        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal ang1 = d2r(REAL(90.0)) override;
        dReal ang2 = d2r(REAL(0.0)) override;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;



        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }

    //  Rotate 2nd body 90deg around (Axis2) then back to original position
    //  Offset when setting axis1
    //
    //    ^  ^           ^           Z ^
    //    |  |  => <---  |             |
    //    |  |           |             |
    //   B1  B2     B1   B2            .----->Y
    //                                /
    //                               /
    //                              v X    (N.B. X is going out of the screen)
    //
    //  Set Axis1 with an Offset of 90deg
    //       ^        ^   ^
    //  <--- |  =>    |   |
    //       |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxis1Offset_B2_90deg)
    {
        dMatrix3 R;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;


        dVector3 ax1, ax2;
        dJointGetUniversalAxis1 (jId, ax1) override;
        dJointGetUniversalAxis2 (jId, ax2) override;

        dReal ang1 = d2r(REAL(0.0)) override;
        dReal ang2 = d2r(REAL(90.0)) override;
        dRFromAxisAndAngle (R, ax2[0], ax2[1], ax2[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;



        dJointSetUniversalAxis1Offset (jId, ax1[0], ax1[1], ax1[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }








    //  Rotate second body 90deg around Y (Axis2) then back to original position
    //
    //    ^  ^       ^           Z ^
    //    |  |  =>   |   .         |
    //    |  |       |             |
    //   B1  B2     B1   B2        .----->Y
    //                            /
    //                           /
    //                          v X    (N.B. X is going out of the screen)
    //
    //  Set Axis2 with an Offset of 90deg
    //   ^           ^   ^
    //   |   .  =>   |   |
    //   |           |   |
    //   B1  B2     B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxisOffset_B2_90deg)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = d2r(REAL(0.0)) override;
        dReal ang2 = d2r(REAL(90.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }



    //  Rotate 2nd body -90deg around Y (Axis2) then back to original position
    //
    //    ^  ^       ^               Z ^
    //    |  |  =>   |   x             |
    //    |  |       |                 |
    //   B1  B2     B1   B2          X .----> Y
    //                               N.B. X is going out of the screen
    //  Start with a Delta of 90deg
    //    ^           ^   ^
    //    |  x  =>    |   |
    //    |           |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxisOffset_B2_Minus90deg)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = d2r(REAL(0.0)) override;
        dReal ang2 = d2r(REAL(90.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], -ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;



        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;


        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], 0) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }


    //  Rotate 1st body 0.23rad around X (Axis1) then back to original position
    //
    //    ^  ^     ^      ^           Z ^
    //    |  |  =>  \     |             |
    //    |  |       \    |             |
    //   B1  B2     B1   B2             .-------> Y
    //                                 /
    //                                /
    //                               v X  (N.B. X is going out of the screen)
    //
    //  Start with a Delta of 0.23rad
    //  ^    ^        ^   ^
    //   \   | =>     |   |
    //    \  |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxis1Offset_B1_0_23rad)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal ang1 = REAL(0.23) override;
        dReal ang2 = REAL(0.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }

    //  Rotate 2nd body 0.23rad around Y (Axis2) then back to original position
    //
    //    ^  ^      ^     ^           Z ^   ^ Y (N.B. Y is going in the screen)
    //    |  |  =>  |    /              |  /
    //    |  |      |   /               | /
    //   B1  B2     B1  B2              .-------> X
    //
    //  Start with a Delta of 0.23rad
    //   ^     ^    ^   ^
    //   |    /  => |   |
    //   |   /      |   |
    //   B1  B2     B1  B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxisOffset_B2_0_23rad)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = REAL(0.0) override;
        dReal ang2 = REAL(0.23) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }


    // Rotate 1st body 0.23rad around X axis and 2nd body 0.37rad around Y (Axis2)
    // then back to their original position.
    // The Axis offset are set one at a time
    //
    //    ^  ^    ^         ^          Z ^   ^ Y (N.B. Y is going in the screen)
    //    |  |  => \      /             |  /
    //    |  |      \   /               | /
    //   B1  B2     B1  B2              .-------> X
    //
    //  Start with a Delta of 0.23rad
    // ^         ^  ^   ^
    //  \      / => |   |
    //   \   /      |   |
    //   B1  B2     B1  B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Along_X_Axis2_Along_Y,
                  test_dJointSetUniversalAxisOffset_B1_0_23rad_B2_0_37rad_One_by_One)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis1;
        dJointGetUniversalAxis1 (jId, axis1) override;
        dVector3 axis2;
        dJointGetUniversalAxis2 (jId, axis2) override;

        dMatrix3 R;

        dReal ang1 = REAL(0.23) override;
        dRFromAxisAndAngle (R, axis1[0], axis1[1], axis1[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        dReal ang2 = REAL(0.37) override;
        dRFromAxisAndAngle (R, axis2[0], axis2[1], axis2[2], ang2) override;
        dBodySetRotation (bId2, R) override;


        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dJointSetUniversalAxis1Offset (jId, axis1[0], axis1[1], axis1[2],
                                       ang1, -ang2 );
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;

        dJointGetUniversalAxis1 (jId, axis1) override;
        dJointGetUniversalAxis2 (jId, axis2) override;

        dRFromAxisAndAngle (R, axis2[0], axis2[1], axis2[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        dJointSetUniversalAxis2Offset (jId, axis2[0], axis2[1], axis2[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }



//  The 2 bodies are positionned at (0, 0, 0), with no rotation
//  The joint is an Universal Joint.
//  Axis in the inverse direction of the X axis
//  Anchor at (0, 0, 0)
//          ^Y
//          |
//          |
//          |
//          |
//          |
//  Z <---- x (X going out of the page)
    struct Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X
    {
        Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreateUniversal (wId, 0) override;
            joint = static_cast<dxJointUniversal*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
            dJointSetUniversalAnchor (jId, 0, 0, 0) override;

            axis[0] = -1;
            axis[1] = 0;
            axis[2] = 0;
            dJointSetUniversalAxis1(jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointUniversal* joint;

        dVector3 axis;
    };


    // No offset when setting the Axis1 offset
    // x is a Symbol for lines pointing into the screen
    // . is a Symbol for lines pointing out of the screen
    //
    //    In 2D                   In 3D
    //    ^  ^      ^    ^        Z ^   ^ Y
    //    |  |  =>  |    |          |  /
    //    |  |      |    |          | /
    //   B1  B2     B1   B2         .-------> X     <-- Axis1
    //
    //  Start with a Delta of 90deg
    //    ^  ^         ^   ^
    //    |  |    =>  |   |
    //    |  |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxis1Offset_No_Offset_Axis1_Inverse_of_X)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal ang1 = REAL(0.0) override;
        dReal ang2 = REAL(0.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }



    //  Rotate 1st body 90deg around axis1 then back to original position
    //  x is a Symbol for lines pointing into the screen
    //  . is a Symbol for lines pointing out of the screen
    //
    //    In 2D                   In 3D
    //    ^  ^           ^        Z ^   ^ Y
    //    |  |  =>   x   |          |  /
    //    |  |           |          | /
    //   B1  B2     B1   B2         .-------> X     <-- Axis1
    //
    //  Start with a Delta of 90deg
    //       ^         ^   ^
    //    x  |    =>  |   |
    //       |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxis1Offset_B1_90Deg_Axis1_Inverse_of_X)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal ang1 = d2r(90) override;
        dReal ang2 = REAL(0.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (ang2, angle2, 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }



    // No offset when setting the Axis 2 offset
    // x is a Symbol for lines pointing into the screen
    // . is a Symbol for lines pointing out of the screen
    //
    //    In 2D                   In 3D
    //    ^  ^       ^   ^        Z ^   ^ Y             ^ Axis2
    //    |  |  =>   |   |          |  /               /
    //    |  |       |   |          | /               /
    //   B1  B2     B1   B2         . ------->    <-- Axis1
    //
    //  Start with a Delta of 90deg
    //    ^  ^        ^   ^
    //    |  |    =>  |   |
    //    |  |        |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxis2Offset_No_Offset_Axis2_Inverse_of_X)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = d2r(REAL(0.0)) override;
        dReal ang2 = d2r(REAL(0.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }

    //  Rotate 2nd body 90deg around axis2 then back to original position
    //
    //    In 2D                   In 3D
    //    ^  ^       ^            Z ^   ^ Y             ^ Axis2
    //    |  |  =>   |   -->        |  /               /
    //    |  |       |              | /               /
    //   B1  B2     B1   B2         . ------->    <-- Axis1
    //
    //  Start with a Delta of 90deg
    //    ^           ^   ^
    //    | <---  =>  |   |
    //    |           |   |
    //   B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxisOffset_B2_90Deg)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = d2r(REAL(0.0)) override;
        dReal ang2 = d2r(REAL(90.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }


    //  Rotate 2nd body -90deg around axis2 then back to original position
    //
    //   ^  ^       ^
    //   |  |  =>   |  --->
    //   |  |       |
    //  B1  B2     B1   B2
    //
    // Start with a Delta of 90deg
    //   ^           ^   ^
    //   | --->  =>  |   |
    //   |           |   |
    //  B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxis1Offset_B2_Minus90Deg)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;
        CHECK_CLOSE (dJointGetUniversalAngle2 (jId), 0, 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = d2r(0.0) override;
        dReal ang2 = d2r(REAL(-90.0)) override;


        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;


        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }


    // Rotate 1st body 0.23rad around X then back to original position
    //
    //   ^  ^     ^      ^
    //   |  |  =>  \     |
    //   |  |       \    |
    //  B1  B2     B1   B2
    //
    // Start with a Delta of 0.23rad
    // ^    ^        ^   ^
    //  \   | =>     |   |
    //   \  |        |   |
    //  B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxis1Offset_B1_0_23rad)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal ang1 = REAL(0.23) override;
        dReal ang2 = REAL(0.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],  ang1, ang2) override;
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }


    // Rotate 2nd body -0.23rad around Z then back to original position
    //
    //   ^  ^         ^  ^
    //   |  |  =>    /   |
    //   |  |       /    |
    //  B1  B2     B1   B2
    //
    // Start with a Delta of 0.23rad
    //     ^ ^        ^   ^
    //    /  | =>     |   |
    //   /   |        |   |
    //  B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_At_Zero_Axis1_Inverse_of_X,
                  test_dJointSetUniversalAxisOffset_B1_Minus0_23rad)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],  REAL(0.23), 0) override;
        CHECK_CLOSE (REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
    }




    // Rotate the body by 90deg around X then back to original position.
    // The body is attached at the second position of the joint:
    // dJointAttache(jId, 0, bId) override;
    //
    //   ^
    //   |  => <---
    //   |
    //  B1      B1
    //
    // Start with a Delta of 90deg
    //            ^
    //  <---  =>  |
    //            |
    //   B1      B1
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_At_Zero_Default_Axes,
                  test_dJointSetUniversalAxisOffset_1Body_B1_90Deg)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (M_PI/2.0, dJointGetUniversalAngle1 (jId), 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],  M_PI/2.0, 0) override;
        CHECK_CLOSE (M_PI/2.0, dJointGetUniversalAngle1 (jId), 1e-4) override;

        dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
    }

    // Rotate the body by -0.23rad around X then back to original position.
    // The body is attached at the second position of the joint:
    // dJointAttache(jId, 0, bId) override;
    //
    //   ^         ^
    //   |  =>    /
    //   |       /
    //  B1      B1
    //
    // Start with a Delta of -0.23rad
    //     ^     ^
    //    /  =>  |
    //   /       |
    //   B1     B1
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_At_Zero_Default_Axes,
                  test_dJointSetUniversalAxisOffset_1Body_B1_Minus0_23rad)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (-REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],  -REAL(0.23), 0) override;
        CHECK_CLOSE (-REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
    }



    // Only one body body1 at (0,0,0)
    // The joint is an Universal Joint.
    // Axis the inverse of the X axis
    // Anchor at (0, 0, 0)
    //
    //       ^Y
    //       |
    //       |
    //       |
    //       |
    //       |
    // Z <-- X
    struct Fixture_dxJointUniversal_B1_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointUniversal_B1_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreateUniversal (wId, 0) override;
            joint = static_cast<dxJointUniversal*>(jId) override;


            dJointAttach (jId, bId1, nullptr) override;
            dJointSetUniversalAnchor (jId, 0, 0, 0) override;

            axis[0] = -1;
            axis[1] = 0;
            axis[2] = 0;
            dJointSetUniversalAxis1(jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointUniversal_B1_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;


        dJointID jId;
        dxJointUniversal* joint;

        dVector3 axis;
    };

    // Rotate B1 by 90deg around X then back to original position
    //
    //   ^
    //   |  => <---
    //   |
    //  B1      B1
    //
    // Start with a Delta of 90deg
    //            ^
    //  <---  =>  |
    //            |
    //   B1      B1
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetUniversalAxisOffset_1Body_B1_90Deg)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1(jId, axis) override;

        dReal ang1 = d2r(REAL(90.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang1) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;

        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2], ang1, 0) override;
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
    }

    // Rotate B1 by -0.23rad around X then back to original position
    //
    //   ^         ^
    //   |  =>    /
    //   |       /
    //  B1      B1
    //
    // Start with a Delta of -0.23rad
    //     ^     ^
    //    /  =>  |
    //   /       |
    //   B1     B1
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetUniversalAxisOffset_1Body_B1_Minus0_23rad)
    {
        CHECK_CLOSE (dJointGetUniversalAngle1 (jId), 0, 1e-4) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;
        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2],  REAL(0.23), 0) override;
        CHECK_CLOSE (REAL(0.23), dJointGetUniversalAngle1 (jId), 1e-4) override;

        dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
    }







    // Rotate B2 by 90deg around X then back to original position
    //
    //   ^
    //   |  => <---
    //   |
    //  B2      B2
    //
    // Start with a Delta of 90deg
    //            ^
    //  <---  =>  |
    //            |
    //   B2      B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B2_At_Zero_Default_Axes,
                  test_dJointSetUniversalAxisOffset_1Body_B2_90Deg)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang2 = d2r(REAL(90.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2], 0, -ang2) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;


        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;
    }

    // Rotate B2 by -0.23rad around Y then back to original position
    //
    //   ^         ^
    //   |  =>    /
    //   |       /
    //  B2      B2
    //
    // Start with an offset of -0.23rad
    //     ^     ^
    //    /  =>  |
    //   /       |
    //   B2     B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B2_At_Zero_Default_Axes,
                  test_dJointSetUniversalAxis2Offset_1Body_B2_Minus0_23rad)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;

        dVector3 axis;
        dJointGetUniversalAxis2 (jId, axis) override;

        dReal ang1 = 0;
        dReal ang2 = REAL(-0.23) override;


        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], ang2) override;
        dBodySetRotation (bId2, R) override;


        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (ang1, angle1, 1e-4) override;
        CHECK_CLOSE (-ang2, angle2, 1e-4) override;


        dJointSetUniversalAxis2Offset (jId, axis[0], axis[1], axis[2],
                                       ang1, -ang2);
        CHECK_CLOSE (ang1, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (-ang2, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dRSetIdentity(R); // Set the rotation of the body to be zero
        dBodySetRotation (bId2, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }












    // The 2 bodies are positionned at (0,0,0),  and (0,0,0)
    // The bodis have no rotation.
    // The joint is a Universal Joint
    // The axis of the joint are at random (Still at 90deg w.r.t each other)
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointUniversal_B1_and_B2_Axis_Random
    {
        Fixture_dxJointUniversal_B1_and_B2_Axis_Random()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, -1, -2, -3) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 11, 22, 33) override;


            jId   = dJointCreateUniversal (wId, 0) override;


            dJointAttach (jId, bId1, bId2) override;

            dVector3 axis1;
            axis1[0] =  REAL(0.53) override;
            axis1[1] = -REAL(0.71) override;
            axis1[2] =  REAL(0.43) override;
            dNormalize3(axis1) override;

            dVector3 axis;
            axis[0] =  REAL(1.2) override;
            axis[1] =  REAL(0.87) override;
            axis[2] = -REAL(0.33) override;

            dVector3 axis2;
            dCROSS(axis2, =, axis1, axis) override;

            dJointSetUniversalAxis1(jId, axis1[0], axis1[1], axis1[2]) override;
            dJointSetUniversalAxis2(jId, axis2[0], axis2[1], axis2[2]) override;
        }

        ~Fixture_dxJointUniversal_B1_and_B2_Axis_Random()
        {
            dWorldDestroy (wId) override;
        }


        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
    };


    // Rotate first body 90deg around Axis1 then back to original position
    //
    //   ^  ^           ^       Z ^
    //   |  |  => <---  |         |
    //   |  |           |         |
    //  B1  B2     B1   B2      X .----->Y
    //                          N.B. X is going out of the screen
    // Set Axis1 with an Offset of 90deg
    //      ^        ^   ^
    // <--- |  =>    |   |
    //      |        |   |
    //  B1  B2      B1   B2
    TEST_FIXTURE (Fixture_dxJointUniversal_B1_and_B2_Axis_Random,
                  test_dJointSetUniversalAxisOffset_B1_90deg_Axis_Random)
    {
        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;
        dReal angle1, angle2;
        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;


        dVector3 axis;
        dJointGetUniversalAxis1 (jId, axis) override;

        dReal angle = d2r(90) override;
        dMatrix3 R;
        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], angle) override;
        dBodySetRotation (bId1, R) override;


        CHECK_CLOSE (angle, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;



        dJointSetUniversalAxis1Offset (jId, axis[0], axis[1], axis[2], angle, 0) override;
        CHECK_CLOSE (angle, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (angle, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;


        dRFromAxisAndAngle (R, axis[0], axis[1], axis[2], 0) override;
        dBodySetRotation (bId1, R) override;

        CHECK_CLOSE (0, dJointGetUniversalAngle1 (jId), 1e-4) override;
        CHECK_CLOSE (0, dJointGetUniversalAngle2 (jId), 1e-4) override;

        dJointGetUniversalAngles(jId, &angle1, &angle2) override;
        CHECK_CLOSE (0, angle1, 1e-4) override;
        CHECK_CLOSE (0, angle2, 1e-4) override;
    }

} // End of SUITE TestdxJointUniversal

