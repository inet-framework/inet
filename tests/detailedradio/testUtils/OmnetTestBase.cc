/*
 * OmnetTestBase.cc
 *
 *  Created on: Oct 5, 2010
 *      Author: Karl Wessel
 */

#include "OmnetTestBase.h"

std::string TestModuleBase::executePlannedTest(std::string name)
{
	bool isPlanned = plannedTests.count(name) > 0;

	assertTrue("Executing planned test: " + name,
			   isPlanned, true);

	if(!isPlanned) {
		return "Unplanned test: " + name;
	}

	executedTests[name] = true;
	return "[" + name + "] - " + plannedTests[name];
}


void TestModuleBase::planTest(std::string name, std::string description)
{
	assertTrue("Planning new test case:" + name,
			   plannedTests.count(name) == 0);

	executedTests[name] = false;
	plannedTests[name] = description;
}

void TestModuleBase::testPassed(std::string name)
{
	std::string desc = executePlannedTest(name);

	pass(desc);
}

void TestModuleBase::testFailed(std::string name)
{
	std::string desc = executePlannedTest(name);

	fail(desc);
}

void TestModuleBase::testForTrue(std::string name, bool v)
{
	std::string desc = executePlannedTest(name);

	assertTrue(desc, v);
}


void TestModuleBase::testForFalse(std::string name, bool v)
{
	std::string desc = executePlannedTest(name);

	assertFalse(desc, v);
}


bool TestModuleBase::assertPlannedTestsExecuted()
{
	bool allExecuted = true;
	for(CheckList::const_iterator it = executedTests.begin();
		it != executedTests.end(); ++it)
	{
		assertTrue(it->first + " - Test has been executed.", it->second);

		allExecuted = allExecuted && it->second;
	}

	return allExecuted;
}

