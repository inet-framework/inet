#include "TestManager.h"

Define_Module(TestManager);

TestManager::TestManager()
	: cSimpleModule()
	, TestModuleBase()
	, modules()
	, plannedModules()
	, run(0)
	, stage(0)
	, finishCalled(false)
	, errorExpected("")
	, testForErrorTest()
{
	testForErrorTest = plannedTests.end();
}

TestManager::~TestManager()
{
	if(!finishCalled) {
		if(testForErrorTest != plannedTests.end()) {
			testPassed(testForErrorTest->first);
		} else if(errorExpected != ""){
			pass(errorExpected);
		} else {
			fail("Simulation should end normally.");
		}
	}

	assertPlannedTestsExecuted();
}

void TestManager::initialize(int stage)
{
	if(stage == 0) {
		run = simulation.getSystemModule()->par("run").longValue();
		this->stage = 0;
	}
	else if(stage == 2) {
		planTests(run);
		checkPlannedModules();
		runTests(run, 0, NULL);
	}
}

void TestManager::checkPlannedModules()
{
	for(PlannedModules::iterator it = plannedModules.begin();
		it != plannedModules.end(); ++it)
	{
		testForTrue(*it, modules.count(*it));
	}
}

void TestManager::finish()
{
	finishCalled = true;

	if(testForErrorTest != plannedTests.end()) {
		testFailed(testForErrorTest->first);
	} else if(errorExpected != ""){
		fail(errorExpected);
	}
}

void TestManager::registerModule(const std::string& name, TestModule* module)
{
	modules[name] = module;
}

void TestManager::continueTests(cMessage* msg)
{
	Enter_Method_Silent();
	runTests(run, ++stage, msg);
}

void TestManager::assertError(std::string msg)
{
	assertTrue("Only one error may be expected per test run.",
				errorExpected == "", true);
	errorExpected = msg;
}

void TestManager::testForError(std::string name)
{
	bool isPlanned = plannedTests.count(name) > 0;

	assertTrue("Error test " + name + " is planned.",
				isPlanned, true);

	testForErrorTest = plannedTests.find(name);
	assertError(testForErrorTest->second);
}

void TestManager::planTestModule(std::string id, std::string description)
{
	assertTrue(	"Module with the following id already planned:" + id,
				plannedModules.count(id) == 0, true);

	planTest(id, "Expected module - " + description);

	plannedModules.insert(id);
}

void TestManager::onMessage(std::string module, cMessage* msg) {
	Enter_Method_Silent();
	onTestModuleMessage(module, msg);
}

