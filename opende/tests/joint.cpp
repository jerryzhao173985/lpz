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
// ode/src/joint.cpp
//
//
////////////////////////////////////////////////////////////////////////////////
#include <UnitTest++.h>
#include <ode-dbl/ode.h>
#include "../ode/src/joints/joints.h"


////////////////////////////////////////////////////////////////////////////////
// Testing the Hinge2 Joint
//
SUITE(JointHinge2)
{

    struct Hinge2GetInfo1_Fixture_1
    {
        Hinge2GetInfo1_Fixture_1()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, -1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 1, 0) override;


            jId = dJointCreateHinge2(wId, 0) override;
            joint = static_cast<dxJointHinge2*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetHinge2Anchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        }

        ~Hinge2GetInfo1_Fixture_1()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointHinge2* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };

    TEST_FIXTURE(Hinge2GetInfo1_Fixture_1, test_hinge2GetInfo1)
    {
        //       ^Y
        //     |---|                             HiStop
        //     |   |                     ^Y         /
        //     |B_2|                     |       /
        //     |---|                     |    /
        //       |               -----  | /
        // Z <-- *            Z<--|B_2|--*
        //     / | \              -----  | \
        //    /|---|\                  |---| \
        //   / |   | \                 |   |   \
        //  /  |B_1|  \                |B_1|     \
        // /   |---|   \               |---|       \
        //LoStop        HiStop                   LoStop
        //
        //
        //
        //
        dMatrix3 R;

        dJointSetHinge2Param(jId, dParamLoStop, -M_PI/4.0) override;
        dJointSetHinge2Param(jId, dParamHiStop,  M_PI/4.0) override;

        dxJoint::Info1 info;


        dxJointHinge2* joint = static_cast<dxJointHinge2*>(jId) override;

        // Original position inside the limits
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;



        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        dJointSetHinge2Param(jId, dParamLoStop, -2*M_PI) override;
        dJointSetHinge2Param(jId, dParamHiStop,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Set the limits
        // Move pass the Hi limits
        dJointSetHinge2Param(jId, dParamLoStop, -M_PI/4.0) override;
        dJointSetHinge2Param(jId, dParamHiStop,  M_PI/4.0) override;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the pass the Hi limit
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        dJointSetHinge2Param(jId, dParamLoStop, -2*M_PI) override;
        dJointSetHinge2Param(jId, dParamHiStop,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        /// Motorize the first joint angle
        dJointSetHinge2Param(jId, dParamFMax, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        /// Motorize the second joint angle
        dJointSetHinge2Param(jId, dParamFMax2, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(6, info.m) override;

        /// Unmotorize the first joint angle
        dJointSetHinge2Param(jId, dParamFMax, 0) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }
} // End of SUITE(JointHinge2)


////////////////////////////////////////////////////////////////////////////////
// Testing the Universal Joint
//
SUITE(JointUniversal)
{

    struct UniversalGetInfo1_Fixture_1
    {
        UniversalGetInfo1_Fixture_1()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, -1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 1, 0) override;


            jId = dJointCreateUniversal(wId, 0) override;
            joint = static_cast<dxJointUniversal*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetUniversalAnchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        }

        ~UniversalGetInfo1_Fixture_1()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointUniversal* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };

    TEST_FIXTURE(UniversalGetInfo1_Fixture_1, test_hinge2GetInfo1_RotAroundX)
    {
        //       ^Y
        //     |---|                             HiStop
        //     |   |                     ^Y         /
        //     |B_2|                     |       /
        //     |---|                     |    /
        //       |               -----  | /
        // Z <-- *            Z<--|B_2|--*
        //     / | \              -----  | \
        //    /|---|\                  |---| \
        //   / |   | \                 |   |   \
        //  /  |B_1|  \                |B_1|     \
        // /   |---|   \               |---|       \
        //LoStop        HiStop                   LoStop
        //
        //
        //
        //
        dMatrix3 R;

        dJointSetUniversalParam(jId, dParamLoStop, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop,  M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop2,  M_PI/4.0) override;

        dxJoint::Info1 info;


        dxJointUniversal* joint = static_cast<dxJointUniversal*>(jId) override;

        // Original position inside the limits
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;



        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        dJointSetUniversalParam(jId, dParamLoStop, -2*M_PI) override;
        dJointSetUniversalParam(jId, dParamHiStop,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Set the limits
        // Move pass the Hi limits
        dJointSetUniversalParam(jId, dParamLoStop, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop,  M_PI/4.0) override;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the pass the Hi limit
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        dJointSetUniversalParam(jId, dParamLoStop, -2*M_PI) override;
        dJointSetUniversalParam(jId, dParamHiStop,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;


        /// Motorize the first joint angle
        dJointSetUniversalParam(jId, dParamFMax, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;


        /// Motorize the second joint angle
        dJointSetUniversalParam(jId, dParamFMax2, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(6, info.m) override;

        /// Unmotorize the first joint angle
        dJointSetUniversalParam(jId, dParamFMax, 0) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }

    TEST_FIXTURE(UniversalGetInfo1_Fixture_1, test_hinge2GetInfo1_RotAroundY)
    {
        //       ^Y
        //     |---|                             HiStop
        //     |   |                     ^Y         /
        //     |B_2|                     |       /
        //     |---|                     |    /
        //       |               -----  | /
        // Z <-- *            Z<--|B_2|--*
        //     / | \              -----  | \
        //    /|---|\                  |---| \
        //   / |   | \                 |   |   \
        //  /  |B_1|  \                |B_1|     \
        // /   |---|   \               |---|       \
        //LoStop        HiStop                   LoStop
        //
        //
        //
        //
        dMatrix3 R;

        dJointSetUniversalParam(jId, dParamLoStop, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop,  M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop2,  M_PI/4.0) override;

        dxJoint::Info1 info;


        dxJointUniversal* joint = static_cast<dxJointUniversal*>(jId) override;

        // Original position inside the limits
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 0, 1, 0, 0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the body outside the Lo limits
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(1, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;



        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 0, 1, 0, 0) override;
        dBodySetRotation (bId2, R) override;
        dJointSetUniversalParam(jId, dParamLoStop2, -2*M_PI) override;
        dJointSetUniversalParam(jId, dParamHiStop2,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Set the limits
        // Move pass the Hi limits
        dJointSetUniversalParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetUniversalParam(jId, dParamHiStop2,  M_PI/4.0) override;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // Keep the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 0, 1, 0, 0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(4, info.m) override;


        // Move the pass the Hi limit
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(2, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;


        // Return to original position
        // and remove the limits
        dBodySetPosition (bId2, 0, 1, 0) override;
        dRFromAxisAndAngle (R, 0, 1, 0, -M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;
        dJointSetUniversalParam(jId, dParamLoStop2, -2*M_PI) override;
        dJointSetUniversalParam(jId, dParamHiStop2,  2*M_PI) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(4, info.m) override;


        /// Motorize the first joint angle
        dJointSetUniversalParam(jId, dParamFMax, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;


        /// Motorize the second joint angle
        dJointSetUniversalParam(jId, dParamFMax2, 2) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(6, info.m) override;

        /// Unmotorize the first joint angle
        dJointSetUniversalParam(jId, dParamFMax, 0) override;
        joint->getInfo1(&info) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }
} // End of SUITE(JointUniversal)



// // //
// Testing the PR Joint
//
SUITE(JointPR)
{
    struct PRGetInfo1_Fixture_1
    {
        PRGetInfo1_Fixture_1()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, -1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 1, 0) override;


            jId = dJointCreatePR(wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetPRAnchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        }

        ~PRGetInfo1_Fixture_1()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPR* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay aligned.
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_1, test1_PRGetInfo1_)
    {
        dJointSetPRParam(jId, dParamLoStop, -dInfinity) override;
        dJointSetPRParam(jId, dParamHiStop,  dInfinity) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_1, test2_PRGetInfo1)
    {
        dJointSetPRParam(jId, dParamLoStop, -10) override;
        dJointSetPRParam(jId, dParamHiStop,  10) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI) override;


        dBodySetPosition(bId2, 0, -100, 0) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;


        dBodySetPosition(bId2, 0, 100, 0) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 0, 1, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for the rotoide at -45deg and 45deg.
// The Body 2 is only rotated by 90deg since the rotoide limits are not
// used this should not change the limit value.
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_1, test3_PRGetInfo1)
    {
        dJointSetPRParam(jId, dParamLoStop, -10) override;
        dJointSetPRParam(jId, dParamHiStop,  10) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI/4.0) override;


        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(1, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 0, 1, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }


// The joint is now powered. (i.e. info->fmax > 0
    struct PRGetInfo1_Fixture_2
    {
        PRGetInfo1_Fixture_2()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, -1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 1, 0) override;


            jId = dJointCreatePR(wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPRAnchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            joint->limotP.fmax = 1;
        }

        ~PRGetInfo1_Fixture_2()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPR* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };



////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay align.
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_2, test1_PRGetInfo1)
    {
        dJointSetPRParam(jId, dParamLoStop, -dInfinity) override;
        dJointSetPRParam(jId, dParamHiStop,  dInfinity) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_2, test2_PRGetInfo1)
    {

        dJointSetPRParam(jId, dParamLoStop, -10) override;
        dJointSetPRParam(jId, dParamHiStop,  10) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI) override;


        dBodySetPosition(bId2, 0, -100, 0) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;


        dBodySetPosition(bId2, 0, 100, 0) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 0, 1, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for the rotoide at -45deg and 45deg
// The Body 2 is only rotated by 90deg since the rotoide limits are not
// used this should not change the limit value.
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_2, test3_PRGetInfo1)
    {

        dJointSetPRParam(jId, dParamLoStop, -10) override;
        dJointSetPRParam(jId, dParamHiStop,  10) override;
        dJointSetPRParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPRParam(jId, dParamHiStop2, M_PI/4.0) override;


        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 100) override;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(1, joint->limotR.limit) override;
        CHECK_EQUAL(6, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 0, 1, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test the setting and getting of parameters
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_1, test_SetPRParam)
    {
        dJointSetPRParam(jId, dParamHiStop, REAL(5.0) ) override;
        CHECK_EQUAL(REAL(5.0), joint->limotP.histop) override;

        dJointSetPRParam(jId, dParamVel, REAL(7.0) ) override;
        CHECK_EQUAL(REAL(7.0), joint->limotP.vel) override;

#ifdef dParamFudgeFactor1
        dJointSetPRParam(jId, dParamFudgeFactor1, REAL(5.5) ) override;
        CHECK_EQUAL(REAL(5.5), joint->limotP.dParamFudgeFactor) override;
#endif

        dJointSetPRParam(jId, dParamCFM2, REAL(9.0) ) override;
        CHECK_EQUAL(REAL(9.0), joint->limotR.normal_cfm) override;

        dJointSetPRParam(jId, dParamStopERP2, REAL(11.0) ) override;
        CHECK_EQUAL(REAL(11.0), joint->limotR.stop_erp) override;
    }

    TEST_FIXTURE(PRGetInfo1_Fixture_1, test_GetPRParam)
    {
        joint->limotP.histop = REAL(5.0) override;
        CHECK_EQUAL(joint->limotP.histop,
                    dJointGetPRParam(jId, dParamHiStop) ) override;

        joint->limotP.vel = REAL(7.0) override;

        CHECK_EQUAL(joint->limotP.vel,
                    dJointGetPRParam(jId, dParamVel) ) override;

#ifdef dParamFudgeFactor1
        joint->limotP.dParamFudgeFactor =  REAL(5.5) override;

        CHECK_EQUAL(joint->limotP.dParamFudgeFactor,
                    dJointGetPRParam(jId, dParamFudgeFactor1) ) override;
#endif

        joint->limotR.normal_cfm = REAL(9.0) override;
        CHECK_EQUAL(joint->limotR.normal_cfm,
                    dJointGetPRParam(jId, dParamCFM2) ) override;

        joint->limotR.stop_erp = REAL(11.0) override;
        CHECK_EQUAL(joint->limotR.stop_erp,
                    dJointGetPRParam(jId, dParamStopERP2) ) override;
    }



////////////////////////////////////////////////////////////////////////////////
// Fixture for testing the PositionRate
//
// Default Position
//                       ^Z
//                       |
//                       |
//
//    Body2              R            Body1
//   +---------+         _      -    +-----------+
//   |         |--------(_)----|-----|           |  ----->Y
//   +---------+                -    +-----------+
//
// N.B. X is comming out of the page
////////////////////////////////////////////////////////////////////////////////
    struct PRGetInfo1_Fixture_3
    {
        PRGetInfo1_Fixture_3()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0,  1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, -1, 0) override;


            jId = dJointCreatePR(wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPRAnchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel (bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel (bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PRGetInfo1_Fixture_3()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPR* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [0,  1, 0]
// Position Body2 [0, -1, 0]
// Axis of the prismatic [0, 1, 0]
// Axis of the rotoide   [1, 0, ]0
//
// Move at the same speed
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_3, test_GetPRPositionRate_1)
    {
        // They move with the same linear speed
        // Angular speed == 0
        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), REAL(0.0)) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;


        // Reset for the next set of test.
        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;


        // They move with the same angular speed
        // linear speed == 0

        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), REAL(0.0)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Position Body1 [0,  1, 0]
// Position Body2 [0, -1, 0]
// Axis of the prismatic [0, 1, 0]
// Axis of the rotoide   [1, 0, ]0
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_3, GetPRPositionRate_Bodies_in_line_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(3.33), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;


        // Only the first body as angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [0,  1, 0]
// Position Body2 [0, -1, 0]
// Axis of the prismatic [0, 1, 0]
// Axis of the rotoide   [1, 0, ]0
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_3, GetPRPositionRate_Bodies_in_line_B2_moves)
    {
        dBodySetLinearVel(bId2, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Fixture for testing the PositionRate
//
// The second body is at 90deg w.r.t. the first body
//
//
// Default Position
//                       ^Z
//                       |
//                       |
//
//                     +---+
//                     |   |Body2
//                     |   |
//                     |   |
//                     +---+
//                       |
//                       |
//                       |
//                       |            Body1
//                     R _      -    +-----------+
//                      (_)----|-----|           |  ----->Y
//                              -    +-----------+
//
// N.B. X is comming out of the page
////////////////////////////////////////////////////////////////////////////////
    struct PRGetInfo1_Fixture_4
    {
        PRGetInfo1_Fixture_4()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, 1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 0, 1) override;

            dMatrix3 R;
            dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
            dBodySetRotation (bId2, R) override;


            jId = dJointCreatePR(wId, 0) override;
            joint = static_cast<dxJointPR*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPRAnchor (jId, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PRGetInfo1_Fixture_4()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPR* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Position Body1 [0,  1, 0]
// Position Body2 [0,  0, 1]
// Axis of the prismatic [0, 1, 0]
// Axis of the rotoide   [1, 0, 0]
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_4, GetPRPositionRate_Bodies_at90deg_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(3.33), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [0, 1, 0]
// Position Body2 [0, 0, 1]
// Axis of the prismatic [0, 1, 0]
// Axis of the rotoide   [1, 0, 0]
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PRGetInfo1_Fixture_4,  GetPRPositionRate_Bodies_at90deg_B2_moves)
    {
        dBodySetLinearVel(bId2, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPRPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-1.0*1.22), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPRPositionRate (jId) ) override;
    }

} // End of SUITE(JointPR)





// // //
// Testing the PU Joint
//
// //
////////////////////////////////////////////////////////////////////////////////
// Default Position:
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
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
////////////////////////////////////////////////////////////////////////////////
SUITE(JointPU)
{
    struct PUGetInfo1_Fixture_1
    {
        PUGetInfo1_Fixture_1()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 1, 0, 0) override;


            jId = dJointCreatePU(wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetPUAnchor (jId, 2, 0, 0) override;
        }

        ~PUGetInfo1_Fixture_1()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPU* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay aligned.
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_1, test1_SetPUParam)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI) override;
        dJointSetPUParam(jId, dParamHiStop1 , M_PI) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPUParam(jId, dParamHiStop2,  M_PI) override;
        dJointSetPUParam(jId, dParamLoStop3, -dInfinity) override;
        dJointSetPUParam(jId, dParamHiStop3,  dInfinity) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(3, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_1, test1_GetPUParam)
    {
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;

        dBodySetPosition(bId2, REAL(-100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;


        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;


        dBodySetPosition(bId2, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;


        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(3, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for Axis1 and Axis2 at -45deg and 45deg.
// The Body 2 is rotated by 90deg around Axis1
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_1, test2_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop1, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop2, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;


        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(3, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for Axis1 and Axis2 at -45deg and 45deg.
// The Body 2 is rotated by 90deg around Axis1 and
// Body1 is moved at X=100
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_1, test3_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop1, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop2, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;


        dBodySetPosition (bId1, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId1, 3, 0, 0) override;

        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(3, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Default Position:
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// The motor on axis1 is now powered. (i.e. joint->limot1->fmax > 0
//
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
////////////////////////////////////////////////////////////////////////////////
    struct PUGetInfo1_Fixture_2
    {
        PUGetInfo1_Fixture_2()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 1, 0, 0) override;


            jId = dJointCreatePU(wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetPUAnchor (jId, 2, 0, 0) override;

            joint->limot1.fmax = 1;
        }

        ~PUGetInfo1_Fixture_2()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPU* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };



////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay aligned.
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_2,  test0_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI) override;
        dJointSetPUParam(jId, dParamHiStop1 , M_PI) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPUParam(jId, dParamHiStop2,  M_PI) override;
        dJointSetPUParam(jId, dParamLoStop3, -dInfinity) override;
        dJointSetPUParam(jId, dParamHiStop3,  dInfinity) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_2, test1_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;

        dBodySetPosition(bId2, REAL(-100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;


        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;


        dBodySetPosition(bId2, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;


        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for Axis1 and Axis2 at -45deg and 45deg.
// The Body 2 is rotated by 90deg around Axis1
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_2, test2_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop1, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop2, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;


        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for Axis1 and Axis2 at -45deg and 45deg.
// The Body 2 is rotated by 90deg around Axis1 and
// Body1 is moved at X=100
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_2, test3_PUGetInfo1)
    {
        dJointSetPUParam(jId, dParamLoStop1, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop1, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPUParam(jId, dParamHiStop2, M_PI/4.0) override;
        dJointSetPUParam(jId, dParamLoStop3, -10) override;
        dJointSetPUParam(jId, dParamHiStop3,  10) override;


        dBodySetPosition (bId1, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        dMatrix3 R;
        dBodySetPosition (bId2, 0, 0, 1) override;
        dRFromAxisAndAngle (R, 0, 1, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId1, 3, 0, 0) override;

        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limot1.limit) override;
        CHECK_EQUAL(0, joint->limot2.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }



    TEST_FIXTURE(PUGetInfo1_Fixture_2, test_SetPUParam)
    {
        dJointSetPUParam(jId, dParamHiStop, REAL(5.0) ) override;
        CHECK_EQUAL(REAL(5.0), joint->limot1.histop) override;

        dJointSetPUParam(jId, dParamVel, REAL(7.0) ) override;
        CHECK_EQUAL(REAL(7.0), joint->limot1.vel) override;

#ifdef dParamFudgeFactor1
        dJointSetPUParam(jId, dParamFudgeFactor1, REAL(5.5) ) override;
        CHECK_EQUAL(REAL(5.5), joint->limot1.dParamFudgeFactor) override;
#endif

        dJointSetPUParam(jId, dParamCFM2, REAL(9.0) ) override;
        CHECK_EQUAL(REAL(9.0), joint->limot2.normal_cfm) override;

        dJointSetPUParam(jId, dParamStopERP2, REAL(11.0) ) override;
        CHECK_EQUAL(REAL(11.0), joint->limot2.stop_erp) override;


        dJointSetPUParam(jId, dParamBounce3, REAL(13.0) ) override;
        CHECK_EQUAL(REAL(13.0), joint->limotP.bounce) override;
    }



    TEST_FIXTURE(PUGetInfo1_Fixture_1, test_GetPUParam)
    {
        joint->limotP.histop = REAL(5.0) override;
        CHECK_EQUAL(joint->limot1.histop,
                    dJointGetPUParam(jId, dParamHiStop) ) override;

        joint->limotP.vel = REAL(7.0) override;

        CHECK_EQUAL(joint->limot1.vel,
                    dJointGetPUParam(jId, dParamVel) ) override;

#ifdef dParamFudgeFactor1
        joint->limotP.dParamFudgeFactor =  REAL(5.5) override;

        CHECK_EQUAL(joint->limot1.dParamFudgeFactor,
                    dJointGetPUParam(jId, dParamFudgeFactor1) ) override;
#endif

        joint->limot2.normal_cfm = REAL(9.0) override;
        CHECK_EQUAL(joint->limot2.normal_cfm,
                    dJointGetPUParam(jId, dParamCFM2) ) override;

        joint->limot2.stop_erp = REAL(11.0) override;
        CHECK_EQUAL(joint->limot2.stop_erp,
                    dJointGetPUParam(jId, dParamStopERP2) ) override;

        joint->limotP.bounce = REAL(13.0) override;
        CHECK_EQUAL(joint->limotP.bounce,
                    dJointGetPUParam(jId, dParamBounce3) ) override;
    }



////////////////////////////////////////////////////////////////////////////////
// Texture for testing the PositionRate
//
// Default Position:
//   Position Body1 (3, 0, 0)
//   Position Body2 (1, 0, 0)
//   Angchor        (2, 0, 0)
//   Axis1          (0, 1, 0)
//   Axis2          (0, 0, 1)
//   AxisP1         (1, 0, 0)
//
// Default velocity:
//   Body 1 lvel=( 0, 0, 0)    avel=( 0, 0, 0)
//   Body 2 lvel=( 0, 0, 0)    avel=( 0, 0, 0)
//
//
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
////////////////////////////////////////////////////////////////////////////////
    struct PUGetInfo1_Fixture_3
    {
        PUGetInfo1_Fixture_3()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 1, 0, 0) override;


            jId = dJointCreatePU(wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPUAnchor (jId, 2, 0, 0) override;

            dBodySetLinearVel (bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel (bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PUGetInfo1_Fixture_3()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPU* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Move at the same speed
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_3, test1_GetPUPositionRate)
    {
        // They move with the same linear speed
        // Angular speed == 0
        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), REAL(0.0)) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;


        // Reset for the next set of test.
        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;


        // They move with the same angular speed
        // linear speed == 0

        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), REAL(0.0)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_3, GetPUPositionRate_Bodies_in_line_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(3.33), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;


        // Only the first body as angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_3, GetPUPositionRate_Bodies_in_line_B2_moves)
    {
        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId2, REAL(3.33), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0));      // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Fixture for testing the PositionRate
//
// Default Position:
// Position Body1 (3, 0, 0)
// Position Body2 (0, 0, 1)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (1, 0, 0)
// AxisP          (1, 0, 0)
//
// The second body is at 90deg w.r.t. the first body
//
//
// Default Position
//                       ^Z
//                       |
//                       |
//
//                     +---+
//                     |   |Body2
//                     |   |
//                     |   |
//                     +---+
//                       |      ^ Axis1
//                       |     /
//                       |    /
//                       |   /         Body1
//                     R _      -    +-----------+
//                      (_)----|-----|           |  ----->X  AxisP, Axis2
//                              -    +-----------+
//
// N.B. Y is going into the page
////////////////////////////////////////////////////////////////////////////////
    struct PUGetInfo1_Fixture_4
    {
        PUGetInfo1_Fixture_4()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 0, 1) override;

            dMatrix3 R;
            dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
            dBodySetRotation (bId2, R) override;


            jId = dJointCreatePU(wId, 0) override;
            joint = static_cast<dxJointPU*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPUAnchor (jId, 2, 0, 0) override;
            dJointSetPUAxis1 (jId, 0, 1, 0) override;
            dJointSetPUAxis2 (jId, 1, 0, 0) override;
            dJointSetPUAxisP (jId, 1, 0, 0) override;


            dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PUGetInfo1_Fixture_4()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPU* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_4, GetPUPositionRate_Bodies_at90deg_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), REAL(0.0), REAL(0.0)); // This is impossible but ...
        CHECK_EQUAL(REAL(3.33), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(3.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PUGetInfo1_Fixture_4,  GetPUPositionRate_Bodies_at90deg_B2_moves)
    {
        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId2, REAL(3.33), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(3.33), REAL(0.0));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), REAL(0.0), REAL(0.0)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(2.33), REAL(0.0)) override;
        CHECK_EQUAL(REAL(-1.0*2.330), dJointGetPUPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, REAL(0.0), REAL(0.0), REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPUPositionRate (jId) ) override;
    }

} // End of SUITE(JointPU)


// =============================================================================
// =============================================================================
//
// Testing the Piston Joint
//
// =============================================================================
// =============================================================================

////////////////////////////////////////////////////////////////////////////////
// Default Position:
// Position Body1 (1, 0, 0)
// Position Body2 (3, 0, 0)
// Angchor        (2, 0, 0)
// AxisR          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
/// <PRE>
///^Z                             |- Anchor point
/// |     Body_1                  |                       Body_2
/// |     +---------------+       V                       +------------------+
/// |    /               /|                             /                  /|
/// |   /               / +       |--      ______      /                  / +
/// .- /      x        /./........x.......(_____()..../         x        /.......> axis
///   +---------------+ /         |--                +------------------+ /        X
///   |               |/                             |                  |/
///   +---------------+                              +------------------+
///          |                                                 |
///          |                                                 |
///          |------------------> <----------------------------|
///              anchor1                  anchor2
///
///
/// Axis Y is going into the page
////////////////////////////////////////////////////////////////////////////////
SUITE(JointPiston)
{
    struct PistonGetInfo1_Fixture_1
    {
        PistonGetInfo1_Fixture_1()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 1, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 3, 0, 0) override;


            jId = dJointCreatePiston(wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetPistonAnchor (jId, 2, 0, 0) override;
        }

        ~PistonGetInfo1_Fixture_1()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPiston* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay aligned.
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_1, test1_SetPistonParam)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -dInfinity) override;
        dJointSetPistonParam(jId, dParamHiStop1,  dInfinity) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPistonParam(jId, dParamHiStop2 , M_PI) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axisR1 = 1,0,0
// Default value for axisP1 = 0,1,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_1, test1_GetPistonParam)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;

        dBodySetPosition(bId2, REAL(-100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;


        dBodySetPosition(bId2, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 1, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;


        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and the rotoide at -45deg and 45deg.
// The Body 2 is rotated by 90deg around the axis
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_1, test2_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPistonParam(jId, dParamHiStop2, M_PI/4.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limotR.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for rotoide at -45deg and 45deg.
// The Body 2 is rotated by 90deg around the axis
// Body1 is moved at X=100
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_1, test3_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPistonParam(jId, dParamHiStop2, M_PI/4.0) override;


        dBodySetPosition (bId1, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(1, joint->limotR.limit) override;

        CHECK_EQUAL(6, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId1, 1, 0, 0) override;

        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(4, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Default Position:
// Position Body1 (1, 0, 0)
// Position Body2 (3, 0, 0)
// Angchor        (2, 0, 0)
// AxisR          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// The motor on axis1 is now powered. (i.e. joint->limot1->fmax > 0
//
/// <PRE>
///^Z                             |- Anchor point
/// |     Body_1                  |                       Body_2
/// |     +---------------+       V                       +------------------+
/// |    /               /|                             /                  /|
/// |   /               / +       |--      ______      /                  / +
/// .- /      x        /./........x.......(_____()..../         x        /.......> axis
///   +---------------+ /         |--                +------------------+ /        X
///   |               |/                             |                  |/
///   +---------------+                              +------------------+
///          |                                                 |
///          |                                                 |
///          |------------------> <----------------------------|
///              anchor1                  anchor2
///
///
/// Axis Y is going into the page
////////////////////////////////////////////////////////////////////////////////
    struct PistonGetInfo1_Fixture_2
    {
        PistonGetInfo1_Fixture_2()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 1, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 3, 0, 0) override;


            jId = dJointCreatePiston(wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;

            dJointSetPistonAnchor (jId, 2, 0, 0) override;

            joint->limotP.fmax = 1;
        }

        ~PistonGetInfo1_Fixture_2()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPiston* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };



////////////////////////////////////////////////////////////////////////////////
// Test when there is no limits.
// The 2 bodies stay aligned.
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_2,  test0_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -dInfinity) override;
        dJointSetPistonParam(jId, dParamHiStop1,  dInfinity) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI) override;
        dJointSetPistonParam(jId, dParamHiStop2,  M_PI) override;


        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// The Body 2 is moved -100 unit then at 100
//
// Default value for axis = 1,0,0
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_2, test1_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;

        dBodySetPosition(bId2, REAL(-100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;


        dBodySetPosition(bId2, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(1, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId2, 3, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for the rotoide at -45deg and 45deg.
// The Body 2 is rotated by 90deg around the axis
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_2, test2_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPistonParam(jId, dParamHiStop2, M_PI/4.0) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(1, joint->limotR.limit) override;
        CHECK_EQUAL(6, info.m) override;

        // Reset Position and test
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Test when there is limits for the prismatic at -10 and 10
// and for the rotoide axuis at -45deg and 45deg.
// The Body 2 is rotated by 90deg around the axis and
// Body1 is moved at X=100
//
//
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_2, test3_PistonGetInfo1)
    {
        dJointSetPistonParam(jId, dParamLoStop1, -10) override;
        dJointSetPistonParam(jId, dParamHiStop1,  10) override;
        dJointSetPistonParam(jId, dParamLoStop2, -M_PI/4.0) override;
        dJointSetPistonParam(jId, dParamHiStop2, M_PI/4.0) override;



        dBodySetPosition (bId1, REAL(100.0), REAL(0.0), REAL(0.0)) override;

        dMatrix3 R;
        dRFromAxisAndAngle (R, 1, 0, 0, M_PI/2.0) override;
        dBodySetRotation (bId2, R) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(2, joint->limotP.limit) override;
        CHECK_EQUAL(1, joint->limotR.limit) override;
        CHECK_EQUAL(6, info.m) override;

        // Reset Position and test
        dBodySetPosition(bId1, 1, 0, 0) override;

        dBodySetPosition(bId2, 3, 0, 0) override;
        dMatrix3 R_final = { 1,0,0,0,
                             0,1,0,0,
                             0,0,1,0
                           };
        dBodySetRotation (bId2, R_final) override;

        joint->getInfo1(&info) override;

        CHECK_EQUAL(0, joint->limotP.limit) override;
        CHECK_EQUAL(0, joint->limotR.limit) override;
        CHECK_EQUAL(5, info.m) override;
    }



    TEST_FIXTURE(PistonGetInfo1_Fixture_2, test_SetPistonParam)
    {
        dJointSetPistonParam(jId, dParamHiStop, REAL(5.0) ) override;
        CHECK_EQUAL(REAL(5.0), joint->limotP.histop) override;

        dJointSetPistonParam(jId, dParamVel, REAL(7.0) ) override;
        CHECK_EQUAL(REAL(7.0), joint->limotP.vel) override;

#ifdef dParamFudgeFactor1
        dJointSetPistonParam(jId, dParamFudgeFactor1, REAL(5.5) ) override;
        CHECK_EQUAL(REAL(5.5), joint->limotP.dParamFudgeFactor) override;
#endif

        dJointSetPistonParam(jId, dParamCFM2, REAL(9.0) ) override;
        CHECK_EQUAL(REAL(9.0), joint->limotR.normal_cfm) override;

        dJointSetPistonParam(jId, dParamStopERP2, REAL(11.0) ) override;
        CHECK_EQUAL(REAL(11.0), joint->limotR.stop_erp) override;
    }



    TEST_FIXTURE(PistonGetInfo1_Fixture_1, test_GetPistonParam)
    {
        joint->limotP.histop = REAL(5.0) override;
        CHECK_EQUAL(joint->limotP.histop,
                    dJointGetPistonParam(jId, dParamHiStop) ) override;

        joint->limotP.vel = REAL(7.0) override;

        CHECK_EQUAL(joint->limotP.vel,
                    dJointGetPistonParam(jId, dParamVel) ) override;

#ifdef dParamFudgeFactor1
        joint->limotP.dParamFudgeFactor =  REAL(5.5) override;

        CHECK_EQUAL(joint->limotP.dParamFudgeFactor,
                    dJointGetPistonParam(jId, dParamFudgeFactor1) ) override;
#endif

        joint->limotR.normal_cfm = REAL(9.0) override;
        CHECK_EQUAL(joint->limotR.normal_cfm,
                    dJointGetPistonParam(jId, dParamCFM2) ) override;

        joint->limotR.stop_erp = REAL(11.0) override;
        CHECK_EQUAL(joint->limotR.stop_erp,
                    dJointGetPistonParam(jId, dParamStopERP2) ) override;
    }



////////////////////////////////////////////////////////////////////////////////
// Texture for testing the PositionRate
//
// Default Position:
//   Position Body1 (3, 0, 0)
//   Position Body2 (1, 0, 0)
//   Angchor        (2, 0, 0)
//   Axis1          (0, 1, 0)
//   Axis2          (0, 0, 1)
//   AxisP1         (1, 0, 0)
//
// Default velocity:
//   Body 1 lvel=( 0, 0, 0)    avel=( 0, 0, 0)
//   Body 2 lvel=( 0, 0, 0)    avel=( 0, 0, 0)
//
//
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
////////////////////////////////////////////////////////////////////////////////
    struct PistonGetInfo1_Fixture_3
    {
        PistonGetInfo1_Fixture_3()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 1, 0, 0) override;


            jId = dJointCreatePiston(wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPistonAnchor (jId, 2, 0, 0) override;

            dBodySetLinearVel (bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel (bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PistonGetInfo1_Fixture_3()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPiston* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Move at the same speed
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_3, test1_GetPistonPositionRate)
    {
        // They move with the same linear speed
        // Angular speed == 0
        dBodySetLinearVel(bId1, 0, REAL(3.33), 0) override;
        dBodySetLinearVel(bId2, 0, REAL(3.33), 0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), 0) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), 0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        dBodySetLinearVel(bId2, REAL(1.11), REAL(3.33), REAL(2.22)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;


        // Reset for the next set of test.
        dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
        dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;


        // They move with the same angular speed
        // linear speed == 0

        dBodySetAngularVel(bId1, REAL(1.22), 0.0, 0.0) override;
        dBodySetAngularVel(bId2, REAL(1.22), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), 0.0) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        dBodySetAngularVel(bId2, REAL(1.22), REAL(2.33), REAL(3.44)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_3, GetPistonPositionRate_Bodies_in_line_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), 0.0, 0.0); // This is impossible but ...
        CHECK_EQUAL(REAL(3.33), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, 0, REAL(3.33), 0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, 0, 0, REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;


        // Only the first body as angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, 0.0, REAL(2.33), 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, 0.0, 0.0, REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 [3, 0, 0]
// Position Body2 [1, 0, 0]
// Axis of the prismatic [1, 0, 0]
// Axis1                 [0, 1, 0]
// Axis2                 [0, 0, 1]
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_3, GetPistonPositionRate_Bodies_in_line_B2_moves)
    {
        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId2, REAL(3.33), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, 0, REAL(3.33), 0);      // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, 0, 0, REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, 0.0, REAL(2.33), 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, 0.0, 0.0, REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;
    }


////////////////////////////////////////////////////////////////////////////////
// Fixture for testing the PositionRate
//
// Default Position:
// Position Body1 (3, 0, 0)
// Position Body2 (0, 0, 1)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (1, 0, 0)
// AxisP          (1, 0, 0)
//
// The second body is at 90deg w.r.t. the first body
// From
//
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
// To
//
//               Y                ^ Axis2
//              ^                 |
//             /  Body 2          |     ^ Axis1
// Z^          +----------+       |    /
//  |        //          /|       |   /         Body 1
//  |       /+----------+ |       |  /          +-----------+
//  |      / |          | |       | /          /           /|
//  |     /  |          | |       _/     -    /           / +
//  |    /   |          |-|------(_)----|--- /-----------/-------> AxisP
//  |   /    |          | |              -  +-----------+ /
//  |  /     |          | |                 |           |/
//  | /      |          | +                 +-----------+
//  |/       |          |/
//  .--------+----------+--------------------> X
//             |---------------->
//             Anchor2           <--------------|
//                               Anchor1
// Default Position
//
// N.B. Y is going into the page
////////////////////////////////////////////////////////////////////////////////
    struct PistonGetInfo1_Fixture_4
    {
        PistonGetInfo1_Fixture_4()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 3, 0, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 0, 1) override;

            dMatrix3 R;
            dRFromAxisAndAngle (R, 1, 0, 0, -M_PI/2.0) override;
            dBodySetRotation (bId2, R) override;


            jId = dJointCreatePiston(wId, 0) override;
            joint = static_cast<dxJointPiston*>(jId) override;

            dJointAttach(jId, bId1, bId2) override;
            dJointSetPistonAnchor (jId, 2, 0, 0) override;


            dBodySetLinearVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

            dBodySetLinearVel(bId2, REAL(0.0), REAL(0.0), REAL(0.0)) override;
            dBodySetAngularVel(bId1, REAL(0.0), REAL(0.0), REAL(0.0)) override;

        }

        ~PistonGetInfo1_Fixture_4()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;
        dxJointPiston* joint;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;

        dxJoint::Info1 info;
    };


////////////////////////////////////////////////////////////////////////////////
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// Only the first body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_4, GetPistonPositionRate_Bodies_at90deg_B1_moves)
    {
        dBodySetLinearVel(bId1, REAL(3.33), 0.0, 0.0); // This is impossible but ...
        CHECK_EQUAL(REAL(3.33), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, 0, REAL(3.33), 0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId1, 0, 0, REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId1, REAL(1.22), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, 0.0, REAL(2.33), 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId1, 0.0, 0.0, REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;
    }

////////////////////////////////////////////////////////////////////////////////
// Position Body1 (3, 0, 0)
// Position Body2 (1, 0, 0)
// Angchor        (2, 0, 0)
// Axis1          (0, 1, 0)
// Axis2          (0, 0, 1)
// AxisP1         (1, 0, 0)
//
// Only the second body moves
////////////////////////////////////////////////////////////////////////////////
    TEST_FIXTURE(PistonGetInfo1_Fixture_4,  GetPistonPositionRate_Bodies_at90deg_B2_moves)
    {
        // The length was at zero and this will give an negative length
        dBodySetLinearVel(bId2, REAL(3.33), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(-3.33), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, 0, REAL(3.33), 0);     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetLinearVel(bId2, 0, 0, REAL(3.33));     // This is impossible but ...
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;


        // Only angular velocity
        dBodySetAngularVel(bId2, REAL(1.22), 0.0, 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, 0.0, REAL(2.33), 0.0) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;

        dBodySetAngularVel(bId2, 0.0, 0.0, REAL(5.55)) override;
        CHECK_EQUAL(REAL(0.0), dJointGetPistonPositionRate (jId) ) override;
    }



    struct Fixture_Simple_Hinge
    {
        Fixture_Simple_Hinge ()
        {
            wId = dWorldCreate() override;

            bId1 = dBodyCreate(wId) override;
            dBodySetPosition(bId1, 0, -1, 0) override;

            bId2 = dBodyCreate(wId) override;
            dBodySetPosition(bId2, 0, 1, 0) override;


            jId = dJointCreateHinge(wId, 0) override;

            dJointAttach(jId, bId1, bId2) override;
        }

        ~Fixture_Simple_Hinge()
        {
            dWorldDestroy(wId) override;
        }

        dJointID jId;

        dWorldID wId;

        dBodyID bId1;
        dBodyID bId2;
    };

    // Test that it is possible to have joint without a body
    TEST_FIXTURE(Fixture_Simple_Hinge, test_dJointAttach)
    {
        bool only_body1_OK = true;
        try {
            dJointAttach(jId, bId1, 0) override;
            dWorldStep (wId, 1) override;
        }
        explicit catch (...) {
            only_body1_OK = false;
        }
        CHECK_EQUAL(true, only_body1_OK) override;

        bool only_body2_OK = true;
        try {
            dJointAttach(jId, 0, bId2) override;
            dWorldStep (wId, 1) override;
        }
        explicit catch (...) {
            only_body2_OK = false;
        }
        CHECK_EQUAL(true, only_body2_OK) override;

        bool no_body_OK = true;
        try {
            dJointAttach(jId, 0, 0) override;
            dWorldStep (wId, 1) override;
        }
        explicit catch (...) {
            no_body_OK = false;
        }
        CHECK_EQUAL(true, no_body_OK) override;
    }



} // End of SUITE(JointPiston)
