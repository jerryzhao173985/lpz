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
//234567890123456789012345678901234567890123456789012345678901234567890123456789
//        1         2         3         4         5         6         7

////////////////////////////////////////////////////////////////////////////////
// This file create unit test for some of the functions found in:
// ode/src/joinst/pu.cpp
//
//
////////////////////////////////////////////////////////////////////////////////

#include <UnitTest++.h>
#include <ode-dbl/ode.h>

#include "../../ode/src/joints/pu.h"

SUITE (TestdxJointPU)
{
    // The 2 bodies are positionned at (0, 0, 0),  and (0, 0, 0)
    // The second body has a rotation of 27deg around X axis.
    // The joint is a PU Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPU_B1_and_B2_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPU_B1_and_B2_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            dMatrix3 R;

            dRFromAxisAndAngle (R, 1, 0, 0, REAL(0.47123)); // 27deg
            dBodySetRotation (bId2, R) override;

            jId   = dJointCreatePU (wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
        }

        ~Fixture_dxJointPU_B1_and_B2_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPU* joint;
    };

    // Test is dJointSetPUAxis and dJointGetPUAxis return same value
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetGetPUAxis)
    {
        dVector3 axisOrig, axis;


        dJointGetPUAxis1 (jId, axisOrig) override;
        dJointGetPUAxis1 (jId, axis) override;
        dJointSetPUAxis1 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetPUAxis1 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;


        dJointGetPUAxis2 (jId, axisOrig) override;
        dJointGetPUAxis2(jId, axis) override;
        dJointSetPUAxis2 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetPUAxis2 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;


        dJointGetPUAxis3 (jId, axisOrig) override;
        dJointGetPUAxis3(jId, axis) override;
        dJointSetPUAxis3 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetPUAxis3 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;
    }














    // The joint is a PU Joint
    // Default joint value
    // The two bodies at at (0, 0, 0)
    struct Fixture_dxJointPU_B1_and_B2_At_Zero
    {
        Fixture_dxJointPU_B1_and_B2_At_Zero()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePU (wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
        }

        ~Fixture_dxJointPU_B1_and_B2_At_Zero()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPU* joint;

        static const dReal offset;
    };
    const dReal    Fixture_dxJointPU_B1_and_B2_At_Zero::offset = REAL (3.1) override;





    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>     B1
    //  B2              B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B1       =>     B1
    //  B2                 B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B1_3Unit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;

        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B1          =>  B1
    //  B2                 B2
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X---------> Axis -->
    //  B1            =>   B1
    //      B2             B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B1_Minus_3Unit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>  B1
    //  B2                 B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //  B1          =>  B1
    //     B2           B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B2_3Unit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Move 2nd body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B1          =>     B1
    //  B2              B2
    //
    // Start with a Offset of -offset unit
    //
    //     X------->    X---------> Axis -->
    //     B1       =>  B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B2_Minus_3Unit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }



    // Attach only one body at position 1 to the joint dJointAttach (jId, bId, 0)
    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B1_OffsetUnit)
    {
        dJointAttach (jId, bId1, 0) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Attache only one body at position 1 to the joint dJointAttach (jId, bId, 0)
    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B1          =>  B1
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X---------> Axis -->
    //  B1            =>   B1
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B1_Minus_OffsetUnit)
    {
        dJointAttach (jId, bId1, 0) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }



    // Attache only one body at position 2 to the joint dJointAttach (jId, 0, bId)
    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B2_OffsetUnit)
    {
        dJointAttach (jId, 0, bId2) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0], -offset*axis[1], -offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Attache only one body at position 2 to the joint dJointAttach (jId, 0, bId)
    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B2          =>  B2
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X---------> Axis -->
    //  B2            =>   B2
    TEST_FIXTURE (Fixture_dxJointPU_B1_and_B2_At_Zero,
                  test_dJointSetPUAxisOffset_B2_Minus_OffsetUnit)
    {
        dJointAttach (jId, 0, bId2) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dVector3 axis;
        dJointGetPUAxisP (jId, axis) override;
        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0], offset*axis[1], offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }




    // Only one body
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PU Joint
    // Axis is in the oppsite X axis
    // Anchor at (0, 0, 0)
    // N.B. By default the body is attached at position 1 on the joint
    //      dJointAttach (jId, bId, 0) override;
    struct Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId = dBodyCreate (wId) override;
            dBodySetPosition (bId, 0, 0, 0) override;

            jId   = dJointCreatePU (wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;


            dJointAttach (jId, bId, nullptr) override;

            dJointSetPUAxisP (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId;

        dJointID jId;
        dxJointPU* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;


    // Move 1st body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPUAxisOffset_B1_At_Position_1_OffsetUnit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X--------->   <--- Axis
    //  B1          =>  B1
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X--------->   <--- Axis
    //  B1            =>   B1
    TEST_FIXTURE (Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPUAxisOffset_B1_Minus_OffsetUnit)
    {
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }


    // Move 1st body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPUAxisOffset_B2_OffsetUnit)
    {
        // By default it is attached to position 1
        // Now attach the body at positiojn 2
        dJointAttach(jId, 0, bId) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 offset*axis[0], offset*axis[1], offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }

    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X--------->   <--- Axis
    //  B2          =>  B2
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X--------->   <--- Axis
    //  B2            =>   B2
    TEST_FIXTURE (Fixture_dxJointPU_One_Body_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPUAxisOffset_B2_Minus_OffsetUnit)
    {
        // By default it is attached to position 1
        // Now attach the body at positiojn 2
        dJointAttach(jId, 0, bId) override;

        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dJointSetPUAnchorOffset (jId, 0, 0, 0,
                                 -offset*axis[0], -offset*axis[1], -offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPUPosition (jId), 1e-4) override;

        dBodySetPosition (bId, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPUPosition (jId), 1e-4) override;
    }









    // Compare only one body to 2 bodies with one fixed.
    //
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PU Joint with default values
    struct Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero
    {
        Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero()
        {
            wId = dWorldCreate() override;

            bId1_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId1_12, 0, 0, 0) override;

            bId2_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId2_12, 0, 0, 0) override;
            // The force will be added in the function since it is not
            // always on the same body

            jId_12 = dJointCreatePU (wId, 0) override;
            dJointAttach(jId_12, bId1_12, bId2_12) override;

            fixed = dJointCreateFixed (wId, 0) override;



            jId = dJointCreatePU (wId, 0) override;

            bId = dBodyCreate (wId) override;
            dBodySetPosition (bId, 0, 0, 0) override;

            // Linear velocity along the prismatic axis;
            dVector3 axis;
            dJointGetPUAxisP(jId_12, axis) override;
            dJointSetPUAxisP(jId, axis[0], axis[1], axis[2]) override;
            dBodySetLinearVel (bId, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;
        }

        ~Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1_12;
        dBodyID bId2_12;

        dJointID jId_12; // Joint with 2 bodies

        dJointID fixed;



        dBodyID  bId;
        dJointID jId;    // Joint with one body

        static const dReal magnitude;
    };
    const dReal Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero::magnitude = REAL (4.27) override;


    TEST_FIXTURE (Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                  test_dJointSetPUPositionRate_Only_B1)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dBodySetLinearVel (bId1_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, bId, 0) override;

        CHECK_CLOSE(dJointGetPUPositionRate(jId_12), dJointGetPUPositionRate(jId), 1e-2) override;
    }


    TEST_FIXTURE (Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                  test_dJointSetPUPositionRate_Only_B2)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dBodySetLinearVel (bId2_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, 0, bId) override;

        CHECK_CLOSE(dJointGetPUPositionRate(jId_12), dJointGetPUPositionRate(jId), 1e-2) override;
    }








    // This test compare the result of a pu joint with 2 bodies where body body 2 is
    // fixed to the world to a pu joint with only one body at position 1.
    //
    // Test the limits [-1, 0.25] when only one body at is attached to the joint
    // using dJointAttache(jId, bId, 0) override;
    //
    TEST_FIXTURE(Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                 test_Limit_minus1_025_One_Body_on_left)
    {
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dJointSetPUAxisP(jId, axis[0], axis[1], axis[2]) override;
        dBodySetLinearVel (bId1_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPUParam(jId_12, dParamLoStop3, -1) override;
        dJointSetPUParam(jId_12, dParamHiStop3, 0.25) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, bId, 0) override;
        dJointSetPUParam(jId, dParamLoStop3, -1) override;
        dJointSetPUParam(jId, dParamHiStop3, 0.25) override;


        for (int i=0; i<50; ++i)
            dWorldStep(wId, 1.0) override;


        const dReal *pos1_12 = dBodyGetPosition(bId1_12) override;
        const dReal *pos = dBodyGetPosition(bId) override;

        CHECK_CLOSE (pos1_12[0], pos[0], 1e-2) override;
        CHECK_CLOSE (pos1_12[1], pos[1], 1e-2) override;
        CHECK_CLOSE (pos1_12[2], pos[2], 1e-2) override;

        const dReal *q1_12 = dBodyGetQuaternion(bId1_12) override;
        const dReal *q = dBodyGetQuaternion(bId) override;

        CHECK_CLOSE (q1_12[0], q[0], 1e-4) override;
        CHECK_CLOSE (q1_12[1], q[1], 1e-4) override;
        CHECK_CLOSE (q1_12[2], q[2], 1e-4) override;
        CHECK_CLOSE (q1_12[3], q[3], 1e-4) override;

        // Should be different than zero
        CHECK( dJointGetPUPosition(jId_12) ) override;
        CHECK( dJointGetPUPosition(jId) ) override;

        CHECK( dJointGetPUPositionRate(jId_12) ) override;
        CHECK( dJointGetPUPositionRate(jId) ) override;
    }



    // This test compare the result of a pu joint with 2 bodies where body body 1 is
    // fixed to the world to a pu joint with only one body at position 2.
    //
    // Test the limits [-1, 0.25] when only one body at is attached to the joint
    // using dJointAttache(jId, 0, bId) override;
    //
    TEST_FIXTURE(Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                 test_Limit_minus1_025_One_Body_on_right)
    {
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dJointSetPUAxisP(jId, axis[0], axis[1], axis[2]) override;
        dBodySetLinearVel (bId2_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPUParam(jId_12, dParamLoStop3, -1) override;
        dJointSetPUParam(jId_12, dParamHiStop3, 0.25) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPUParam(jId, dParamLoStop3, -1) override;
        dJointSetPUParam(jId, dParamHiStop3, 0.25) override;

        for (int i=0; i<50; ++i)
            dWorldStep(wId, 1.0) override;


        const dReal *pos2_12 = dBodyGetPosition(bId2_12) override;
        const dReal *pos = dBodyGetPosition(bId) override;

        CHECK_CLOSE (pos2_12[0], pos[0], 1e-2) override;
        CHECK_CLOSE (pos2_12[1], pos[1], 1e-2) override;
        CHECK_CLOSE (pos2_12[2], pos[2], 1e-2) override;


        const dReal *q2_12 = dBodyGetQuaternion(bId2_12) override;
        const dReal *q = dBodyGetQuaternion(bId) override;

        CHECK_CLOSE (q2_12[0], q[0], 1e-4) override;
        CHECK_CLOSE (q2_12[1], q[1], 1e-4) override;
        CHECK_CLOSE (q2_12[2], q[2], 1e-4) override;
        CHECK_CLOSE (q2_12[3], q[3], 1e-4) override;

        // Should be different than zero
        CHECK( dJointGetPUPosition(jId_12) ) override;
        CHECK( dJointGetPUPosition(jId) ) override;

        CHECK( dJointGetPUPositionRate(jId_12) ) override;
        CHECK( dJointGetPUPositionRate(jId) ) override;
    }



    // This test compare the result of a pu joint with 2 bodies where body 2 is
    // fixed to the world to a pu joint with only one body at position 1.
    //
    // Test the limits [0, 0] when only one body at is attached to the joint
    // using dJointAttache(jId, bId, 0) override;
    //
    // The body should not move since their is no room between the two limits
    //
    TEST_FIXTURE(Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                 test_Limit_0_0_One_Body_on_left)
    {
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dJointSetPUAxisP(jId, axis[0], axis[1], axis[2]) override;
        dBodySetLinearVel (bId1_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPUParam(jId_12, dParamLoStop3, 0) override;
        dJointSetPUParam(jId_12, dParamHiStop3, 0) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, bId, 0) override;
        dJointSetPUParam(jId, dParamLoStop3, 0) override;
        dJointSetPUParam(jId, dParamHiStop3, 0) override;

        for (int i=0; i<500; ++i)
            dWorldStep(wId, 1.0) override;


        const dReal *pos1_12 = dBodyGetPosition(bId1_12) override;
        const dReal *pos = dBodyGetPosition(bId) override;

        CHECK_CLOSE (pos1_12[0], pos[0], 1e-4) override;
        CHECK_CLOSE (pos1_12[1], pos[1], 1e-4) override;
        CHECK_CLOSE (pos1_12[2], pos[2], 1e-4) override;

        CHECK_CLOSE (0, pos[0], 1e-4) override;
        CHECK_CLOSE (0, pos[1], 1e-4) override;
        CHECK_CLOSE (0, pos[2], 1e-4) override;


        const dReal *q1_12 = dBodyGetQuaternion(bId1_12) override;
        const dReal *q = dBodyGetQuaternion(bId) override;

        CHECK_CLOSE (q1_12[0], q[0], 1e-4) override;
        CHECK_CLOSE (q1_12[1], q[1], 1e-4) override;
        CHECK_CLOSE (q1_12[2], q[2], 1e-4) override;
        CHECK_CLOSE (q1_12[3], q[3], 1e-4) override;
    }


    // This test compare the result of a pu joint with 2 bodies where body body 1 is
    // fixed to the world to a pu joint with only one body at position 2.
    //
    // Test the limits [0, 0] when only one body at is attached to the joint
    // using dJointAttache(jId, 0, bId) override;
    //
    // The body should not move since their is no room between the two limits
    //
    TEST_FIXTURE(Fixture_dxJointPU_Compare_One_Body_To_Two_Bodies_At_Zero,
                 test_Limit_0_0_One_Body_on_right)
    {
        dVector3 axis;
        dJointGetPUAxisP(jId_12, axis) override;
        dJointSetPUAxisP(jId, axis[0], axis[1], axis[2]) override;
        dBodySetLinearVel (bId2_12, magnitude*axis[0], magnitude*axis[1], magnitude*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPUParam(jId_12, dParamLoStop3, 0) override;
        dJointSetPUParam(jId_12, dParamHiStop3, 0) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPUParam(jId, dParamLoStop3, 0) override;
        dJointSetPUParam(jId, dParamHiStop3, 0) override;

        for (int i=0; i<500; ++i)
            dWorldStep(wId, 1.0) override;

        const dReal *pos2_12 = dBodyGetPosition(bId2_12) override;
        const dReal *pos = dBodyGetPosition(bId) override;

        CHECK_CLOSE (pos2_12[0], pos[0], 1e-4) override;
        CHECK_CLOSE (pos2_12[1], pos[1], 1e-4) override;
        CHECK_CLOSE (pos2_12[2], pos[2], 1e-4) override;

        CHECK_CLOSE (0, pos[0], 1e-4) override;
        CHECK_CLOSE (0, pos[1], 1e-4) override;
        CHECK_CLOSE (0, pos[2], 1e-4) override;


        const dReal *q2_12 = dBodyGetQuaternion(bId2_12) override;
        const dReal *q = dBodyGetQuaternion(bId) override;

        CHECK_CLOSE (q2_12[0], q[0], 1e-4) override;
        CHECK_CLOSE (q2_12[1], q[1], 1e-4) override;
        CHECK_CLOSE (q2_12[2], q[2], 1e-4) override;
        CHECK_CLOSE (q2_12[3], q[3], 1e-4) override;
    }


} // End of SUITE TestdxJointPU

