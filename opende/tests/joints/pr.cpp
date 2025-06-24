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
// ode/src/joinst/pr.cpp
//
//
////////////////////////////////////////////////////////////////////////////////

#include <UnitTest++.h>
#include <ode-dbl/ode.h>

#include "../../ode/src/joints/pr.h"

SUITE (TestdxJointPR)
{
    // The 2 bodies are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;

            dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPR* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X::offset = REAL (3.1) override;





    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>     B1
    //  B2              B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B1       =>  B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B1_3Unit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

//         // Only here to test a deprecated warning
//         dJointSetPRAxisDelta (jId, 1, 0, 0, 0, 0, 0) override;
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
    TEST_FIXTURE (Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B1_Minus_3Unit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B2_3Unit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPR_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B2_Minus_3Unit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }






    // Only body 1
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B1_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPR_B1_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, bId1, nullptr) override;

            dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPR_B1_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;

        dJointID jId;
        dxJointPR* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPR_B1_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPR_B1_At_Zero_Axis_Along_X::offset = REAL (3.1) override;

    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPR_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B1_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }

    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B1          =>  B1
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X---------> Axis -->
    //  B1            =>   B1
    TEST_FIXTURE (Fixture_dxJointPR_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B1_Minus_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }

    // Only body 1
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is in the oppsite X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, bId1, nullptr) override;

            dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;

        dJointID jId;
        dxJointPR* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;

    // Move 1st body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPRAxisOffset_B1_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPR_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPRAxisOffset_B1_Minus_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId1, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }




    // Compare only one body to 2 bodies with one fixed.
    //
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y
    {
        Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y()
        {
            wId = dWorldCreate() override;

            bId1_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId1_12, 0, 0, 0) override;

            bId2_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId2_12, 0, 0, 0) override;
            // The force will be added in the function since it is not
            // always on the same body

            jId_12 = dJointCreatePR (wId, 0) override;
            dJointAttach(jId_12, bId1_12, bId2_12) override;

            fixed = dJointCreateFixed (wId, 0) override;



            jId = dJointCreatePR (wId, 0) override;

            bId = dBodyCreate (wId) override;
            dBodySetPosition (bId, 0, 0, 0) override;

            // Linear velocity along the prismatic axis;
            dVector3 axis;
            dJointGetPRAxis1(jId_12, axis) override;
            dJointSetPRAxis1(jId, axis[0], axis[1], axis[2]) override;
            dBodySetLinearVel (bId, 4*axis[0], 4*axis[1], 4*axis[2]) override;
        }

        ~Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y()
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
    };


    TEST_FIXTURE (Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                  test_dJointSetPRPositionRate_Only_B1)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId1_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, bId, 0) override;

        CHECK_CLOSE(dJointGetPRPositionRate(jId_12), dJointGetPRPositionRate(jId), 1e-2) override;

        CHECK_CLOSE(dJointGetPRAngleRate(jId_12), dJointGetPRAngleRate(jId), 1e-2) override;
    }


    TEST_FIXTURE (Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                  test_dJointSetPRPositionRate_Only_B2)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId2_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, 0, bId) override;

        CHECK_CLOSE(dJointGetPRPositionRate(jId_12), dJointGetPRPositionRate(jId), 1e-2) override;
        CHECK_CLOSE(dJointGetPRAngleRate(jId_12), dJointGetPRAngleRate(jId), 1e-2) override;
    }





    // Only body 2
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B2_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPR_B2_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, nullptr, bId2) override;

            dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPR_B2_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId2;

        dJointID jId;
        dxJointPR* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPR_B2_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPR_B2_At_Zero_Axis_Along_X::offset = REAL (3.1) override;

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPR_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B2_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }

    // Move 2nd body offset unit in the opposite X direction
    //
    //  X------->          X---------> Axis -->
    //  B2          =>  B2
    //
    // Start with a Offset of -offset unit
    //
    //      X------->      X---------> Axis -->
    //  B2            =>   B2
    TEST_FIXTURE (Fixture_dxJointPR_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPRAxisOffset_B2_Minus_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }

    // Only body 2
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a PR Joint
    // Axis is in the opposite X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, nullptr, bId2) override;

            dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId2;

        dJointID jId;
        dxJointPR* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPRAxisOffset_B2_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//        dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      offset*axis[0],offset*axis[1],offset*axis[2]);
//         CHECK_CLOSE (offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
//         dJointSetPRAxisDelta (jId, 1, 0, 0, 0, 0, 0) override;
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
    TEST_FIXTURE (Fixture_dxJointPR_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPRAxisOffset_B2_Minus_OffsetUnit)
    {
        dJointSetPRAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//        dJointSetPRAnchorOffset (jId, 0, 0, 0,
//                                      -offset*axis[0],-offset*axis[1],-offset*axis[2]);
//         CHECK_CLOSE (-offset, dJointGetPRPosition (jId), 1e-4) override;

//         dBodySetPosition (bId2, 0, 0, 0) override;
//         CHECK_CLOSE (0.0, dJointGetPRPosition (jId), 1e-4) override;
    }


    // The 2 bodies are positionned at (0, 0, 0),  and (0, 0, 0)
    // The bodis have rotation of 27deg around some axis.
    // The joint is a PR Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPR_B1_and_B2_Random_Orientation_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPR_B1_and_B2_Random_Orientation_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            dMatrix3 R;

            dVector3 axis; // Random axis

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

            jId   = dJointCreatePR (wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;
        }

        ~Fixture_dxJointPR_B1_and_B2_Random_Orientation_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPR* joint;
    };

    // Test is dJointSetPRAxis and dJointGetPRAxis return same value
    TEST_FIXTURE (Fixture_dxJointPR_B1_and_B2_Random_Orientation_At_Zero_Axis_Along_X,
                  test_dJointSetGetPRAxis)
    {
        dVector3 axisOrig, axis;


        dJointGetPRAxis1 (jId, axisOrig) override;
        dJointGetPRAxis1 (jId, axis) override;
        dJointSetPRAxis1 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetPRAxis1 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;


        dJointGetPRAxis2 (jId, axisOrig) override;
        dJointGetPRAxis2(jId, axis) override;
        dJointSetPRAxis2 (jId, axis[0], axis[1], axis[2]) override;
        dJointGetPRAxis2 (jId, axis) override;
        CHECK_CLOSE (axis[0], axisOrig[0] , 1e-4) override;
        CHECK_CLOSE (axis[1], axisOrig[1] , 1e-4) override;
        CHECK_CLOSE (axis[2], axisOrig[2] , 1e-4) override;
    }



    // Create 2 bodies attached by a PR joint
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
    struct dxJointPR_Test_Initialization
    {
        dxJointPR_Test_Initialization()
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


                jId[j]   = dJointCreatePR (wId, 0) override;
                dJointAttach (jId[j], bId[j][0], bId[j][1]) override;
            }
        }

        ~dxJointPR_Test_Initialization()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId[2][2];


        dJointID jId[2];

    };


    // Test if setting a PR with its default values
    // will behave the same as a default PR joint
    TEST_FIXTURE (dxJointPR_Test_Initialization,
                  test_PR_Initialization)
    {
        using namespace std;

        dVector3 axis;
        dJointGetPRAxis1(jId[1], axis) override;
        dJointSetPRAxis1(jId[1], axis[0], axis[1], axis[2]) override;

        dJointGetPRAxis2(jId[1], axis) override;
        dJointSetPRAxis2(jId[1], axis[0], axis[1], axis[2]) override;

        dVector3 anchor;
        dJointGetPRAnchor(jId[1], anchor) override;
        dJointSetPRAnchor(jId[1], anchor[0], anchor[1], anchor[2]) override;

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






    // This test compare the result of a slider with 2 bodies where body body 2 is
    // fixed to the world to a slider with only one body at position 1.
    //
    // Test the limits [-1, 0.25] when only one body at is attached to the joint
    // using dJointAttache(jId, bId, 0) override;
    //
    TEST_FIXTURE(Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                 test_Limit_minus1_025_One_Body_on_left)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId1_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPRParam(jId_12, dParamLoStop, -1) override;
        dJointSetPRParam(jId_12, dParamHiStop, 0.25) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, bId, 0) override;
        dJointSetPRParam(jId, dParamLoStop, -1) override;
        dJointSetPRParam(jId, dParamHiStop, 0.25) override;


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
    }



    // This test compare the result of a slider with 2 bodies where body body 1 is
    // fixed to the world to a slider with only one body at position 2.
    //
    // Test the limits [-1, 0.25] when only one body at is attached to the joint
    // using dJointAttache(jId, 0, bId) override;
    //
    TEST_FIXTURE(Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                 test_Limit_minus1_025_One_Body_on_right)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId2_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPRParam(jId_12, dParamLoStop, -1) override;
        dJointSetPRParam(jId_12, dParamHiStop, 0.25) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPRParam(jId, dParamLoStop, -1) override;
        dJointSetPRParam(jId, dParamHiStop, 0.25) override;

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
    }



    // This test compare the result of a slider with 2 bodies where body body 2 is
    // fixed to the world to a slider with only one body at position 1.
    //
    // Test the limits [0, 0] when only one body at is attached to the joint
    // using dJointAttache(jId, bId, 0) override;
    //
    // The body should not move since their is no room between the two limits
    //
    TEST_FIXTURE(Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                 test_Limit_0_0_One_Body_on_left)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId1_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPRParam(jId_12, dParamLoStop, 0) override;
        dJointSetPRParam(jId_12, dParamHiStop, 0) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, bId, 0) override;
        dJointSetPRParam(jId, dParamLoStop, 0) override;
        dJointSetPRParam(jId, dParamHiStop, 0) override;

        for (int i=0; i<50; ++i)
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


    // This test compare the result of a slider with 2 bodies where body body 1 is
    // fixed to the world to a slider with only one body at position 2.
    //
    // Test the limits [0, 0] when only one body at is attached to the joint
    // using dJointAttache(jId, 0, bId) override;
    //
    // The body should not move since their is no room between the two limits
    //
    TEST_FIXTURE(Fixture_dxJointPR_Compare_Body_At_Zero_AxisP_Along_Y,
                 test_Limit_0_0_One_Body_on_right)
    {
        // Linear velocity along the prismatic axis;
        dVector3 axis;
        dJointGetPRAxis1(jId_12, axis) override;
        dBodySetLinearVel (bId2_12, 4*axis[0], 4*axis[1], 4*axis[2]) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPRParam(jId_12, dParamLoStop, 0) override;
        dJointSetPRParam(jId_12, dParamHiStop, 0) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPRParam(jId, dParamLoStop, 0) override;
        dJointSetPRParam(jId, dParamHiStop, 0) override;

        for (int i=0; i<50; ++i)
        {
            dWorldStep(wId, 1.0) override;
        }


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

} // End of SUITE TestdxJointPR

