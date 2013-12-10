#ifndef TESTMANAGER_H_
#define TESTMANAGER_H_

#include <omnetpp.h>
#include <map>
#include "OmnetTestBase.h"

class TestModule;

/**
 * @brief Central starting and intermediate point for every test run.
 *
 * Tests for OMNeT modules should implement and contain one global instance of
 * this module. For this at least the method "runTests()" should be overridden
 * by forwarding test execution to the according TestModule depending on the
 * current run and test stage.
 * If test planning is used the planning should be done in the overridden
 * "planTests()" method.
 * Modules executing the test runs have to implement the "TestModule" class.
 *
 * A test can contain several runs (OMNeT runs). Each run can further consist
 * of several stages where each stage indicates a point where test execution had
 * to be interrupted because it had to wait for a certain message or had to
 * continue at another module (often also caused by a message at that module).
 *
 * This module is meant to centralize testing of OMNeT modules. It serves as
 * singular and central module where every test run starts, ends and has to
 * come back to if the module executing a test has to change.
 * It means to organize some problems which occur when testing OMNeT modules.
 * The main problem is that execution code for a test run often is not very
 * linear because it has to wait for certain messages or has to be executed in
 * different modules jumping back and forth between them. This makes writing,
 * tracking and understanding test cases very hard.
 * With the test manager one has a single module where every test starts and
 * every change in the module currently executing the test run happens. It also
 * keeps track of the current stage of the test as well as the planned tests as
 * well as their state of execution.
 *
 * @author Karl Wessel
 */
class TestManager:public cSimpleModule,
				  public TestModuleBase
{
protected:
	typedef std::map<std::string, TestModule*> ModuleMap;
	typedef std::set<std::string> PlannedModules;
	
	/** @brief Modules which are part of test execution. */
	ModuleMap modules;

	/** @brief Stores the modules expected to be present for the current test
	 * case as well as their description.*/
	PlannedModules plannedModules;

	/** @brief The current run number. */
	int run;

	/** @brief The current stage of the current test run.*/
	int stage;

	/** @brief Set by finish(), read by destructor to check if simulation ended
	 * normally. */
	bool finishCalled;

	/** @brief Stores the assertion message if "assertError()" was called.*/
	std::string errorExpected;

	/** @brief Iterator pointing to the planned test set by "testForError()".*/
	TestPlan::iterator testForErrorTest;

protected:
	/**
	 * @brief Handles execution of the test run with the passed number at the
	 * passed stage.
	 *
	 * This method has to be implemented by actual tests. It should forward
	 * test execution to the correct TestModule depending on the current run
	 * and its current stage. Or it can execute the tests itself if possible.
	 *
	 * @param run The number of the currently executed test run.
	 * @param stage The current stage of the current test run to be executed.
	 * @param msg A pointer to the message the current stage had been waiting
	 * for, or NULL.
	 */
	virtual void runTests(int run, int stage, cMessage* msg) {(void)run;(void)stage;(void)msg;};

	/**
	 * @brief Plans the tests to be executed for the passed test run.
	 *
	 * Has to be implemented by the actual test. In this method it should place
	 * all its "planTest()" calls.
	 *
	 * @param run The test run to plan the tests for
	 */
	virtual void planTests(int run) {(void)run;};

	/**
	 * @brief Calles whenever a message arrived at a test module.
	 *
	 * Can be used to handle messages which do not have influence on test
	 * execution but are needed for evaluation.
	 * @param module The module the message arrived on.
	 * @param msg The message arrived.
	 */
	virtual void onTestModuleMessage(std::string module, cMessage* msg) {(void)module;(void)msg;}

	/**
	 * @brief Reads current run number from ned parameter and calls planTests()
	 * followed by "runTests()" in stage number two.
	 *
	 * @param stage The current initialization stage (Three stages: 0, 1, 2).
	 */
	virtual void initialize(int stage);

	/**
	 * @brief If finish is called we assume the simulation ended normally.
	 *
	 * "assertError()" and "testForError()" fail if this method is called.
	 */
	virtual void finish();

	/**
	 * @brief Checks if all modules planned with "planTestModule()" are present
	 * and registered.
	 */
	void checkPlannedModules();


public:
	/** @brief Initializes members for expected errors. */
	TestManager();

	/**
	 * @brief Last but important step of a test run.
	 *
	 * Checks whether the simulation ended normally and if all planned tests
	 * were executed.
	 */
	virtual ~TestManager();

	/** @brief To make sure everything is initialized the tests start in init
	 * stage 2.*/
	virtual int numInitStages() const { return 3; }

	/**
	 * @brief Registers the passed TestModule with the passed name at the
	 * database.
	 *
	 * @param name Name to register the module with
	 * @param module Pointer to the TestModule to register
	 */
	void registerModule(const std::string& name, TestModule* module);
	
	/**
	 * @brief Returns a pointer to the TestModule with the passed name.
	 *
	 * Returns NULL if no module with this name is registered.
	 *
	 * @param name The name the module to return is registered with
	 * @return Pointer to the module or NULL
	 */
	template<typename T>
	T* getModule(const std::string& name) const
	{
		ModuleMap::const_iterator it = modules.find(name);
		if(it == modules.end())
			return NULL;
		else
		{
			T* m = dynamic_cast<T*>(it->second);
			assertTrue("Module with passed name and type exists.", m != NULL, true);
			return m;
		}
	}

	/**
	 * @brief Continues the current test run with the next stage.
	 *
	 * Called after a message arrives which the test run has waited for.
	 *
	 * @param msg A pointer to the message which continued the test run
	 */
	void continueTests(cMessage* msg);

	/**
	 * @brief Asserts the simulation to not terminate normally.
	 *
	 * This is checked by testing if finish is called in the end, if not the
	 * simulation is assumed to have finished with an error.
	 *
	 * @param msg The message for this assertion.
	 */
	void assertError(std::string msg);

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the simulation to not end normally.
	 *
	 * Analog to "assertError" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 */
	void testForError(std::string name);

	/**
	 * @brief Tells the manager that a module with the passed id is expected to
	 * be present for this test.
	 *
	 * @param id The id of the module which is expected to be present for this
	 * test.
	 * @param description A description what the module does or for what its
	 * needed. Is displayed if the test for the presence of this module failes.
	 */
	void planTestModule(std::string id, std::string description);

	/**
	 * @brief Is called whenever a message arrives at a TestModule.
	 *
	 * Forwards the message to "onTestModuleMessage" which can be overridden
	 * by tests to react to certain messages although they do not influence the
	 * test execution.
	 *
	 * @param module The test module the message arrived on.
	 * @param msg The message arrived at a TestModule.
	 */
	void onMessage(std::string module, cMessage* msg);
};

#endif /*TESTMANAGER_H_*/
