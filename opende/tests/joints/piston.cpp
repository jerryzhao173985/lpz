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
// ode/src/joinst/piston.cpp
//
//
////////////////////////////////////////////////////////////////////////////////

#include <UnitTest++.h>
#include <ode-dbl/ode.h>

#include "../../ode/src/joints/piston.h"

SUITE (TestdxJointPiston)
{
    // The 2 bodies are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;

            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X::offset = REAL (3.1) override;

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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B1_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        // Only here to test a deprecated warning
        dJointSetPistonAxisDelta (jId, 1, 0, 0, 0, 0, 0) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B1_Minus_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B2_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B2_Minus_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }



    // The 2 bodies are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is the opposite of the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, bId1, bId2) override;


            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;


        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;
        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;

    // Move 1st body offset unit in the X direction
    //
    //  X------->       X--------->   <-- Axis
    //  B1          =>     B1
    //  B2              B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <-- Axis
    //     B1       =>  B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B1_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // Move 1st body offset unit in the opposite X direction
    //
    //  X------->          X--------->  <-- Axis
    //  B1          =>  B1
    //  B2                 B2
    //
    // Start with a Offset of offset unit
    //
    //     X------->  X--------->      <-- Axis
    //  B1       =>   B1
    //     B2         B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B1_Minus_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X--------->  <-- Axis
    //  B1          =>  B1
    //  B2                 B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <-- Axis
    //  B1          =>  B1
    //     B2           B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B2_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // Move 2nd body offset unit in the opposite X direction
    //
    //  X------->          X--------->  <-- Axis
    //  B1          =>     B1
    //  B2              B2
    //
    // Start with a Offset of -offset unit
    //
    //     X------->    X--------->     <-- Axis
    //     B1       =>  B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B2_Minus_3Unit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }


    // Only body 1
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, bId1, nullptr) override;

            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;

        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X::offset = REAL (3.1) override;

    // Move 1st body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B1_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B1_Minus_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // Only body 1
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is in the oppsite X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate (wId) override;
            dBodySetPosition (bId1, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, bId1, nullptr) override;

            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId1;

        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;

    // Move 1st body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B1          =>     B1
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B1       =>  B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B1_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B1_Minus_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId1, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }









    // Only body 2
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, nullptr, bId2) override;

            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId2;

        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X::axis =
    {
        1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X::offset = REAL (3.1) override;

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X---------> Axis -->
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X---------> Axis -->
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B2_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonAxisOffset_B2_Minus_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // Only body 2
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is in the opposite X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X
    {
        Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X()
        {
            wId = dWorldCreate() override;

            bId2 = dBodyCreate (wId) override;
            dBodySetPosition (bId2, 0, 0, 0) override;

            jId   = dJointCreatePiston (wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;


            dJointAttach (jId, nullptr, bId2) override;

            dJointSetPistonAxis (jId, axis[0], axis[1], axis[2]) override;
        }

        ~Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId2;

        dJointID jId;
        dxJointPiston* joint;

        static const dVector3 axis;

        static const dReal offset;
    };
    const dVector3 Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X::axis =
    {
        -1, 0, 0
    };
    const dReal    Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X::offset = REAL (3.1) override;

    // Move 2nd body offset unit in the X direction
    //
    //  X------->       X--------->  <--- Axis
    //  B2          =>     B2
    //
    // Start with a Offset of offset unit
    //
    //  X------->       X--------->  <--- Axis
    //     B2       =>  B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B2_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, offset, 0, 0) override;

        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     offset*axis[0],offset*axis[1],offset*axis[2]);
        CHECK_CLOSE (offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        dJointSetPistonAxisDelta (jId, 1, 0, 0, 0, 0, 0) override;
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
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonAxisOffset_B2_Minus_OffsetUnit)
    {
        dJointSetPistonAnchor (jId, 0, 0, 0) override;

        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, -offset, 0, 0) override;

        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dJointSetPistonAnchorOffset (jId, 0, 0, 0,
                                     -offset*axis[0],-offset*axis[1],-offset*axis[2]);
        CHECK_CLOSE (-offset, dJointGetPistonPosition (jId), 1e-4) override;

        dBodySetPosition (bId2, 0, 0, 0) override;
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
    }

    // ==========================================================================
    // Test Position Rate
    // ==========================================================================

    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> Axis -->
    //  B1  F->      =>     B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Along_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> Axis -->
    //  B1  <-F      => B1
    //  B2                  B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_of_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }


    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> <-- Axis
    //  B1  F->      =>     B1
    //  B2              B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> <-- Axis
    //  B1  <-F      => B1
    //  B2                  B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Along_of_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> Axis -->
    //  B1          =>  B1
    //  B2 F->             B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Along_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> Axis -->
    //  B1           =>     B1
    //  B2  <-F          B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_of_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }


    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> <-- Axis
    //  B1          =>  B1
    //  B2 F->             B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> <-- Axis
    //  B1          =>      B1
    //  B2 <-F           B2
    TEST_FIXTURE (Fixture_dxJointPiston_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Along_of_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }



    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> Axis -->
    //  B1  F->      =>     B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Along_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> Axis -->
    //  B1  <-F      => B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_of_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }


    // Apply force on 1st body in the X direction also the Axis direction
    //
    //  X------->       X---------> <-- Axis
    //  B1  F->      =>     B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on 1st body in the inverse X direction
    //
    //  X------->           X---------> <-- Axis
    //  B1  <-F      => B1
    TEST_FIXTURE (Fixture_dxJointPiston_B1_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Along_of_Axis_on_B1)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId1, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }


    // Apply force on body 2 in the X direction also the Axis direction
    //
    //  X------->       X---------> Axis -->
    //  B2 F->             B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Along_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on body 2 in the inverse X direction
    //
    //  X------->           X---------> Axis -->
    //  B2  <-F          B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Along_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_of_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }


    // Apply force on body 2 in the X direction also the Axis direction
    //
    //  X------->       X---------> <-- Axis
    //  B2 F->             B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Inverse_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, 1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }

    // Apply force on body 2 in the inverse X direction
    //
    //  X------->           X---------> <-- Axis
    //  B2 <-F           B2
    TEST_FIXTURE (Fixture_dxJointPiston_B2_At_Zero_Axis_Inverse_of_X,
                  test_dJointSetPistonPositionRate_Force_Along_of_Axis_on_B2)
    {
        CHECK_CLOSE (0.0, dJointGetPistonPosition (jId), 1e-4) override;
        CHECK_CLOSE (0.0, dJointGetPistonPositionRate (jId), 1e-4) override;

        dBodyAddForce (bId2, -1.0, 0, 0) override;
        dWorldQuickStep (wId, 1.0) override;

        CHECK_CLOSE (-1, dJointGetPistonPositionRate (jId), 1e-4) override;
    }




// Create 2 bodies attached by a Piston joint
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
    struct dxJointPiston_Test_Initialization
    {
        dxJointPiston_Test_Initialization()
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

                jId[j] = dJointCreatePiston (wId, 0) override;
                dJointAttach (jId[j], bId[j][0], bId[j][1]) override;
            }
        }

        ~dxJointPiston_Test_Initialization()
        {
            dWorldDestroy (wId) override;
        }

        dWorldID wId;

        dBodyID bId[2][2];


        dJointID jId[2];

    };


    // Test if setting a Piston with its default values
    // will behave the same as a default Piston joint
    TEST_FIXTURE (dxJointPiston_Test_Initialization,
                  test_Piston_Initialization)
    {
        using namespace std;

        dVector3 axis;
        dJointGetPistonAxis(jId[1], axis) override;
        dJointSetPistonAxis(jId[1], axis[0], axis[1], axis[2]) override;


        dVector3 anchor;
        dJointGetPistonAnchor(jId[1], anchor) override;
        dJointSetPistonAnchor(jId[1], anchor[0], anchor[1], anchor[2]) override;


        for (int b=0; b<2; ++b)
        {
            // Compare body b of the first joint with its equivalent on the
            // second joint
            const dReal *qA = dBodyGetQuaternion(bId[0][b]) override;
            const dReal *qB = dBodyGetQuaternion(bId[1][b]) override;
            CHECK_CLOSE (qA[0], qB[0], 1e-6) override;
            CHECK_CLOSE (qA[1], qB[1], 1e-6) override;
            CHECK_CLOSE (qA[2], qB[2], 1e-6) override;
            CHECK_CLOSE (qA[3], qB[3], 1e-6) override;
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
            CHECK_CLOSE (qA[0], qB[0], 1e-6) override;
            CHECK_CLOSE (qA[1], qB[1], 1e-6) override;
            CHECK_CLOSE (qA[2], qB[2], 1e-6) override;
            CHECK_CLOSE (qA[3], qB[3], 1e-6) override;


            const dReal *posA = dBodyGetPosition(bId[0][b]) override;
            const dReal *posB = dBodyGetPosition(bId[1][b]) override;
            CHECK_CLOSE (posA[0], posB[0], 1e-6) override;
            CHECK_CLOSE (posA[1], posB[1], 1e-6) override;
            CHECK_CLOSE (posA[2], posB[2], 1e-6) override;
            CHECK_CLOSE (posA[3], posB[3], 1e-6) override;
        }


    }





    // Compare only one body to 2 bodies with one fixed.
    //
    // The body are positionned at (0, 0, 0), with no rotation
    // The joint is a Piston Joint
    // Axis is along the X axis
    // Anchor at (0, 0, 0)
    struct Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X
    {
        Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X()
        {
            wId = dWorldCreate() override;

            bId1_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId1_12, 0, 0, 0) override;

            bId2_12 = dBodyCreate (wId) override;
            dBodySetPosition (bId2_12, 0, 0, 0) override;
            // The force will be added in the function since it is not
            // always on the same body

            jId_12 = dJointCreatePiston (wId, 0) override;
            dJointAttach(jId_12, bId1_12, bId2_12) override;

            fixed = dJointCreateFixed (wId, 0) override;



            bId = dBodyCreate (wId) override;
            dBodySetPosition (bId, 0, 0, 0) override;

            dBodyAddForce (bId, 4, 0, 0) override;

            jId = dJointCreatePiston (wId, 0) override;
        }

        ~Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X()
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

    // This test compare the result of a slider with 2 bodies where body body 2 is
    // fixed to the world to a slider with only one body at position 1.
    //
    // Test the limits [-1, 0.25] when only one body at is attached to the joint
    // using dJointAttache(jId, bId, 0) override;
    //
    TEST_FIXTURE(Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X,
                 test_Limit_minus1_025_One_Body_on_left)
    {
        dBodyAddForce (bId1_12, 4, 0, 0) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPistonParam(jId_12, dParamLoStop, -1) override;
        dJointSetPistonParam(jId_12, dParamHiStop, 0.25) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;

        dJointAttach(jId, bId, 0) override;
        dJointSetPistonParam(jId, dParamLoStop, -1) override;
        dJointSetPistonParam(jId, dParamHiStop, 0.25) override;


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
    TEST_FIXTURE(Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X,
                 test_Limit_minus1_025_One_Body_on_right)
    {
        dBodyAddForce (bId2_12, 4, 0, 0) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPistonParam(jId_12, dParamLoStop, -1) override;
        dJointSetPistonParam(jId_12, dParamHiStop, 0.25) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPistonParam(jId, dParamLoStop, -1) override;
        dJointSetPistonParam(jId, dParamHiStop, 0.25) override;

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
    TEST_FIXTURE(Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X,
                 test_Limit_0_0_One_Body_on_left)
    {
        dBodyAddForce (bId1_12, 4, 0, 0) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPistonParam(jId_12, dParamLoStop, 0) override;
        dJointSetPistonParam(jId_12, dParamHiStop, 0) override;

        dJointAttach(fixed, 0, bId2_12) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, bId, 0) override;
        dJointSetPistonParam(jId, dParamLoStop, 0) override;
        dJointSetPistonParam(jId, dParamHiStop, 0) override;

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


    // This test compare the result of a slider with 2 bodies where body body 1 is
    // fixed to the world to a slider with only one body at position 2.
    //
    // Test the limits [0, 0] when only one body at is attached to the joint
    // using dJointAttache(jId, 0, bId) override;
    //
    // The body should not move since their is no room between the two limits
    //
    TEST_FIXTURE(Fixture_dxJointPiston_Compare_Body_At_Zero_Axis_Along_X,
                 test_Limit_0_0_One_Body_on_right)
    {
        dBodyAddForce (bId2_12, 4, 0, 0) override;

        dJointAttach(jId_12, bId1_12, bId2_12) override;
        dJointSetPistonParam(jId_12, dParamLoStop, 0) override;
        dJointSetPistonParam(jId_12, dParamHiStop, 0) override;

        dJointAttach(fixed, bId1_12, 0) override;
        dJointSetFixed(fixed) override;


        dJointAttach(jId, 0, bId) override;
        dJointSetPistonParam(jId, dParamLoStop, 0) override;
        dJointSetPistonParam(jId, dParamHiStop, 0) override;

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


} // End of SUITE TestdxJointPiston
