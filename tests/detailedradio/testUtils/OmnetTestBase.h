#ifndef OMNETTEST_H_
#define OMNETTEST_H_

#include "asserts.h"
#include "INETDefs.h"

/**
 * @brief Utility class to providing basic functionality for every test module

 * @author Karl Wessel
 */
class TestModuleBase {
protected:
	typedef std::map<std::string, std::string> TestPlan;
	typedef std::map<std::string, bool> CheckList;
	CheckList executedTests;
	TestPlan plannedTests;

public:
	TestModuleBase()
		: executedTests()
		, plannedTests()
	{}
	virtual ~TestModuleBase() {}
	/**
	 * @brief Executes a previously planned test case with the passed name.
	 *
	 * @param name the name of the test case which has been executed.
	 * @return the test message associated with the test case
	 */
	std::string executePlannedTest(std::string name);

	/**
	 * @brief Tells SimpleTest a planned test case with the passed name and
	 * description.
	 *
	 * The passed name should than be passed to the "testForX()" method call
	 * which actually executes the test case.
	 * @param name A short identifier for this test case.
	 * @param description A description what this test case covers.
	 */
	void planTest(std::string name, std::string description);

	/**
	 * @brief Marks the previously planned test with the passed name as executed
	 * and passed.
	 *
	 * Meant to be used for "Meta"-tests which do not have a particular
	 * assertion to pass but consist of a set of other tests.
	 *
	 * @param name The identifier of the planned test to execute
	 */
	void testPassed(std::string name);

	/**
	 * @brief Marks the previously planned test with the passed name as executed
	 * but failed.
	 *
	 * Meant to be used for "Meta"-tests which do not have a particular
	 * assertion to pass but consist of a set of other tests.
	 *
	 * @param name The identifier of the planned test to execute
	 */
	void testFailed(std::string name);

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed boolean as true.
	 *
	 * Analog to "assertTrue" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param v The value which should be true for the test to succeed
	 */
	void testForTrue(std::string name, bool v);

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed boolean as false.
	 *
	 * Analog to "assertFalse" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param v The value which should be false for the test to succeed
	 */
	void testForFalse(std::string name, bool v);

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed "actual" value equal to the passed "expected" value.
	 *
	 * Analog to "assertEqual" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param expected The expected value of the parameter "actual"
	 * @param actual The actual value which is asserted to be equal to the
	 * "expected" parameter
	 */
	template<class T1, class T2>
	void testForEqual(std::string name, T1 expected, T2 actual) {
		std::string desc = executePlannedTest(name);

		assertEqual(desc, expected, actual);
	}

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed "actual" value close to the passed "expected" value.
	 *
	 * Analog to "assertClose" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param expected The expected value of the parameter "actual"
	 * @param actual The actual value which is asserted to be close to the
	 * "expected" parameter
	 */
	template<class T1, class T2>
	void testForClose(std::string name, T1 expected, T2 actual) {
		std::string desc = executePlannedTest(name);

		assertClose(desc, expected, actual);
	}

	/**
	 * @brief Checks if every tests case planned with "planTest()" has been
	 * executed.
	 *
	 * @return returns true if the check is passed
	 */
	bool assertPlannedTestsExecuted();
};

/**
 * @brief Utility class to ease testing of OMNeT++ modules.
 *
 * Since most OMNeT++ modules need a network to be executed this module provides
 * a simple base module to ease generation of that network as well providing
 * some entry-points to execute simple UnitTests in (without having to care
 * initialize() or handleMessage().
 *
 * Tests should subclass this module. The tests cases should be executed by
 * implementing them in the "runTests()" method which is executed by
 * SimpleTest during initialisation.
 * After all tests have been executed the member "testsExecuted" should be set
 * to true. SimpleTest checks this member during "finish()" to make sure
 * that the tests have been actually executed.
 *
 * SimpleTest further provides a mechanism to plan test cases. This can be
 * useful for the following reasons:
 * - planning tests forces the author to pre-think about the cases which should
 * be covered by a test without getting distracted by actually implementing them
 * yet
 * - enumerating planned tests serves for documentation purposes so one can
 * easily see which cases are covered by the test
 * - by planning the test cases with SimpleTest it can automatically make
 * sure that every planned test is actually executed and implemented
 *
 * To plan tests using SimpleTest one can override the "planTests()" method
 * where one can then enumerate every tests case using the "planTest()" method.
 * To execute a previously planned test case one simply uses "testForX()"
 * methods instead of "assertX()". E.g. "testForTrue()" instead of
 * "assertTrue()" or "testForEqual()" instead of "assertEqual()".
 *
 * @author Karl Wessel
 */
class SimpleTest:public TestModuleBase,
				 public cSimpleModule
{
protected:
	bool testsExecuted;

protected:
	virtual void planTests() {}
	virtual void runTests() = 0;
	
public:
	SimpleTest()
		:testsExecuted(false)
	{}

	virtual ~SimpleTest() {
		testsExecuted = testsExecuted && assertPlannedTestsExecuted();

		assertTrue("Tests should have been executed!", testsExecuted, true);
	}
	
	virtual void initialize(int /*stage*/){
		planTests();
		runTests();
	}
};

#endif /*OMNETTEST_H_*/
