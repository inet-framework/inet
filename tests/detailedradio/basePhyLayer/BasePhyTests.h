//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __MIXIM_BASEPHYTESTS_H_
#define __MIXIM_BASEPHYTESTS_H_

#include <omnetpp.h>
#include "../testUtils/TestManager.h"

/**
 * @brief Starts and manages the tests for BasePhyLayer functionality.
 *
 * Test run 1: Some simple method checks and error handling.
 * Test run 2-5: TODO organize tests and document
 * Test run 6: tests the use of ChannelInfo's record-feature
 */
class BasePhyTests : public TestManager
{
protected:
	/**
	 * @brief Just forwards to the corresponding "planTestRunX()" method.
	 * @param run The test run to plan tests for.
	 */
	virtual void planTests(int run);
	/**
	 * @brief Just forwards to the corresponding "testRunX()" method.
	 * @param run The test run to execute.
	 */
	virtual void runTests(int run, int stage, cMessage* msg);

	/**
	 * @brief Plans tests to be executed in test run 1.
	 */
	void planTestRun1();
	/**
	 * @brief Executes test run 1 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - check getChannelState() method of MacToPhyInterface
	 * - check switchRadio() method of MacToPhyInterface
	 * - check channel sense request handling of phy
	 * - check sending on not TX mode (expect error)
	 */
    void testRun1(int stage, cMessage* msg);

	/**
	 * @brief Plans tests to be executed in test run 6.
	 */
    void planTestRun6();
	/**
	 * @brief Executes test run 6 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Test if decider has access to already ended AirFrames during a
	 * ChannelSenseRequest which started during the AirFrame but ended after
	 * the AirFrame. This tests the use of ChannelInfo's record-feature during
	 * ChannelSenseRequests.
	 */
	void testRun6(int stage, cMessage* msg);


	/**
	 * @brief Plans tests to be executed in test run 7.
	 */
    void planTestRun7();
	/**
	 * @brief Executes test run 7 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for phy protocol id implementation
	 * - only AirFrames with known protocol are transfered to decider
	 * - other protocols are only contained in interference for decider
	 */
	void testRun7(int stage, cMessage* msg);
};

#endif
