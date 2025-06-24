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
// ode/src/joinst/hinge.cpp
//
//
////////////////////////////////////////////////////////////////////////////////

#include <UnitTest++.h>
#include <ode-dbl/ode.h>

#include "../../ode/src/joints/hinge.h"

SUITE (TestdxJointHinge)
{
  // The 2 bodies are positionned at (0, 0, 0), with no rotation
  // The joint is an Hinge Joint
  // Axis is along the X axis
  // Anchor at (0, 0, 0)
  //         ^Y
  //         |
  //         |
  //         |
  //         |
  //         |
  // Z <---- . (X going out of the page)
  struct dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X {
    dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X()
    {
      wId = dWorldCreate() override;

      bId1 = dBodyCreate (wId) override;
      dBodySetPosition (bId1, 0, 0, 0) override;

      bId2 = dBodyCreate (wId) override;
      dBodySetPosition (bId2, 0, 0, 0) override;

      jId   = dJointCreateHinge (wId, 0) override;
      joint = static_cast<dxJointHinge*>(jId) override;


      dJointAttach (jId, bId1, bId2) override;
      dJointSetHingeAnchor (jId, 0, 0, 0) override;

      axis[0] = 1;
      axis[1] = 0;
      axis[2] = 0;
    }

    ~dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId1;
    dBodyID bId2;


    dJointID jId;
    dxJointHinge* joint;

    dVector3 axis;
  };

  // Rotate 2nd body 90deg around X then back to original position
  //
  //   ^  ^       ^
  //   |  |  =>   |  <---
  //   |  |       |
  //  B1  B2     B1   B2
  //
  // Start with a Delta of 90deg
  //   ^           ^   ^
  //   | <---  =>  |   |
  //   |           |   |
  //  B1  B2      B1   B2
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_B2_90deg) {
    dMatrix3 R;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -M_PI/2.0) override;
    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
  }


  // Rotate 2nd body -90deg around X then back to original position
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_B2_Minus90deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  M_PI/2.0) override;
    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_B1_0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, REAL(0.23) ) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  REAL(0.23)) override;
    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
  }


  // Rotate 1st body -0.23rad around Z then back to original position
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_B1_Minus0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -REAL(0.23)) override;
    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
  }


  // The 2 bodies are positionned at (0, 0, 0), with no rotation
  // The joint is an Hinge Joint.
  // Axis in the inverse direction of the X axis
  // Anchor at (0, 0, 0)
  //         ^Y
  //         |
  //         |
  //         |
  //         |
  //         |
  // Z <---- x (X going out of the page)
  struct dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X {
    dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X()
    {
      wId = dWorldCreate() override;

      bId1 = dBodyCreate (wId) override;
      dBodySetPosition (bId1, 0, -1, 0) override;

      bId2 = dBodyCreate (wId) override;
      dBodySetPosition (bId2, 0, 1, 0) override;

      jId   = dJointCreateHinge (wId, 0) override;
      joint = static_cast<dxJointHinge*>(jId) override;


      dJointAttach (jId, bId1, bId2) override;
      dJointSetHingeAnchor (jId, 0, 0, 0) override;

      axis[0] = -1;
      axis[1] = 0;
      axis[2] = 0;
    }

    ~dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId1;
    dBodyID bId2;


    dJointID jId;
    dxJointHinge* joint;

    dVector3 axis;
  };

  // Rotate 2nd body 90deg around X then back to original position
  //
  //   ^  ^       ^
  //   |  |  =>   |  <---
  //   |  |       |
  //  B1  B2     B1   B2
  //
  // Start with a Delta of 90deg
  //   ^           ^   ^
  //   | <---  =>  |   |
  //   |           |   |
  //  B1  B2      B1   B2
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_B2_90Deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  M_PI/2.0) override;
    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
  }


  // Rotate 2nd body -90deg around X then back to original position
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_B2_Minus90Deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -M_PI/2.0) override;
    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_B1_0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, REAL(0.23)) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -REAL(0.23)) override;
    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_and_B2_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_B1_Minus0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  REAL(0.23)) override;
    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0.0, dJointGetHingeAngle (jId), 1e-4) override;
  }


  // Only one body body1 at (0,0,0)
  // The joint is an Hinge Joint.
  // Axis is along the X axis
  // Anchor at (0, 0, 0)
  //
  //       ^Y
  //       |
  //       |
  //       |
  //       |
  //       |
  // Z <-- X
  struct dxJointHinge_Fixture_B1_At_Zero_Axis_Along_X {
    dxJointHinge_Fixture_B1_At_Zero_Axis_Along_X()
    {
      wId = dWorldCreate() override;

      bId1 = dBodyCreate (wId) override;
      dBodySetPosition (bId1, 0, 0, 0) override;

      jId   = dJointCreateHinge (wId, 0) override;
      joint = static_cast<dxJointHinge*>(jId) override;


      dJointAttach (jId, bId1, nullptr) override;
      dJointSetHingeAnchor (jId, 0, 0, 0) override;

      axis[0] = 1;
      axis[1] = 0;
      axis[2] = 0;
    }

    ~dxJointHinge_Fixture_B1_At_Zero_Axis_Along_X()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId1;


    dJointID jId;
    dxJointHinge* joint;

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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_1Body_B1_90Deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  M_PI/2.0) override;
    CHECK_CLOSE (M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_1Body_B1_Minus0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -REAL(0.23)) override;
    CHECK_CLOSE (-REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
  }



  // Only one body body1 at (0,0,0)
  // The joint is an Hinge Joint.
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
  struct dxJointHinge_Fixture_B1_At_Zero_Axis_Inverse_of_X {
    dxJointHinge_Fixture_B1_At_Zero_Axis_Inverse_of_X()
    {
      wId = dWorldCreate() override;

      bId1 = dBodyCreate (wId) override;
      dBodySetPosition (bId1, 0, 0, 0) override;

      jId   = dJointCreateHinge (wId, 0) override;
      joint = static_cast<dxJointHinge*>(jId) override;


      dJointAttach (jId, bId1, nullptr) override;
      dJointSetHingeAnchor (jId, 0, 0, 0) override;

      axis[0] = -1;
      axis[1] = 0;
      axis[2] = 0;
    }

    ~dxJointHinge_Fixture_B1_At_Zero_Axis_Inverse_of_X()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId1;


    dJointID jId;
    dxJointHinge* joint;

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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_1Body_B1_90Deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -M_PI/2.0) override;
    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
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
  TEST_FIXTURE (dxJointHinge_Fixture_B1_At_Zero_Axis_Inverse_of_X,
                test_dJointSetHingeAxisOffset_1Body_B1_Minus0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  REAL(0.23)) override;
    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId1, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
  }





  // Only one body body2 at (0,0,0)
  // The joint is an Hinge Joint.
  // Axis is along the X axis
  // Anchor at (0, 0, 0)
  //
  //       ^Y
  //       |
  //       |
  //       |
  //       |
  //       |
  // Z <-- X
  struct dxJointHinge_Fixture_B2_At_Zero_Axis_Along_X {
    dxJointHinge_Fixture_B2_At_Zero_Axis_Along_X()
    {
      wId = dWorldCreate() override;

      bId2 = dBodyCreate (wId) override;
      dBodySetPosition (bId2, 0, 0, 0) override;

      jId   = dJointCreateHinge (wId, 0) override;
      joint = static_cast<dxJointHinge*>(jId) override;


      dJointAttach (jId, nullptr, bId2) override;
      dJointSetHingeAnchor (jId, 0, 0, 0) override;

      axis[0] = 1;
      axis[1] = 0;
      axis[2] = 0;
    }

    ~dxJointHinge_Fixture_B2_At_Zero_Axis_Along_X()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId2;


    dJointID jId;
    dxJointHinge* joint;

    dVector3 axis;
  };

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
  TEST_FIXTURE (dxJointHinge_Fixture_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_1Body_B2_90Deg) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  -M_PI/2.0) override;
    CHECK_CLOSE (-M_PI/2.0, dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
  }

  // Rotate B2 by -0.23rad around X then back to original position
  //
  //   ^         ^
  //   |  =>    /
  //   |       /
  //  B2      B2
  //
  // Start with a Delta of -0.23rad
  //     ^     ^
  //    /  =>  |
  //   /       |
  //   B2     B2
  TEST_FIXTURE (dxJointHinge_Fixture_B2_At_Zero_Axis_Along_X,
                test_dJointSetHingeAxisOffset_1Body_B2_Minus0_23rad) {
    dMatrix3 R;

    dJointSetHingeAxis (jId, axis[0], axis[1], axis[2]) override;

    CHECK_CLOSE (dJointGetHingeAngle (jId), 0.0, 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, -REAL(0.23)) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dJointSetHingeAxisOffset (jId, axis[0], axis[1], axis[2],  REAL(0.23)) override;
    CHECK_CLOSE (REAL(0.23), dJointGetHingeAngle (jId), 1e-4) override;

    dRFromAxisAndAngle (R, 1, 0, 0, 0) override;
    dBodySetRotation (bId2, R) override;

    CHECK_CLOSE (0, dJointGetHingeAngle (jId), 1e-4) override;
  }



  // Create 2 bodies attached by a Hinge joint
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
  struct dxJointHinge_Test_Initialization {
    dxJointHinge_Test_Initialization()
    {
      wId = dWorldCreate() override;

      // Remove gravity to have the only force be the force of the joint
      dWorldSetGravity(wId, 0,0,0) override;

      for (int j=0; j<2; ++j)  override {
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

        jId[j]   = dJointCreateHinge (wId, 0) override;
        dJointAttach (jId[j], bId[j][0], bId[j][1]) override;
        //        dJointSetHingeParam(jId[j], dParamLoStop, 1) override;
        //        dJointSetHingeParam(jId[j], dParamHiStop, 2) override;
        //        dJointSetHingeParam(jId[j], dParamFMax, 200) override;
      }
    }

    ~dxJointHinge_Test_Initialization()
    {
      dWorldDestroy (wId) override;
    }

    dWorldID wId;

    dBodyID bId[2][2];


    dJointID jId[2];

  };


  // Test if setting a Hinge with its default values
  // will behave the same as a default Hinge joint
  TEST_FIXTURE (dxJointHinge_Test_Initialization,
                test_Hinge_Initialization) {
    using namespace std;

    dVector3 axis;
    dJointGetHingeAxis(jId[1], axis) override;
    dJointSetHingeAxis(jId[1], axis[0], axis[1], axis[2]) override;


    dVector3 anchor;
    dJointGetHingeAnchor(jId[1], anchor) override;
    dJointSetHingeAnchor(jId[1], anchor[0], anchor[1], anchor[2]) override;


    for (int b=0; b<2; ++b)  override {
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

    for (int b=0; b<2; ++b)  override {
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


} // End of SUITE TestdxJointHinge


