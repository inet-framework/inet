#ifndef TESTMODULE_H_
#define TESTMODULE_H_

#include "TestManager.h"
#include "asserts.h"
#include "OmnetTestBase.h"
#include <string>

class TestModule;

/**
 * @brief Class representation of the "assertMessage()" command.
 *
 * It describes the asserted message together with a description text.
 *
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMessage {
protected:
	std::string msg;
	bool continuesTests;
	/** @brief Stores if this assert executes a test case planned by a
	 * "planTest" call.*/
	bool isPlannedFlag;

public:
	AssertMessage(std::string msg,
				  bool isPlanned = false,
				  bool continuesTests = false):
		msg(msg),
		continuesTests(continuesTests),
		isPlannedFlag(isPlanned)
	{}
	
	virtual ~AssertMessage() {}

	/**
	 * @brief Returns true if this assert executes a previously planned test
	 * case.
	 *
	 * @return true if this executes a planned test case
	 */
	virtual bool isPlanned() const { return isPlannedFlag; }
		
	/**
	 * @brief Returns true if the passed message is the message expected by this
	 * AssertMessage.
	 *
	 * Has to be implemented by every subclass.
	 */
	virtual bool isMessage(cMessage* msg) = 0;
	
	/**
	 * @brief Returns the message or in case of a planned test the name for this
	 * test case.
	 *
	 * @return message or name of test case
	 */
	virtual std::string getMessage() const { return msg; }

	/**
	 * @brief Appends the description text of this AssertMessage to the passed
	 * outstream.
	 *
	 * Should be overridden/extended by subclasses.
	 */
	virtual std::ostream& concat(std::ostream& o) const {
		return o;
	}
	
	/**
	 * @brief Returns true if this message has been waited for to continue test
	 * run.
	 */
	bool continueTests() const {
		return continuesTests;
	}
	
	/**
	 * @brief Needed by the "toString()" method to convert this class into a
	 * string.
	 */
	friend std::ostream& operator<<(std::ostream& o, const AssertMessage& m) {
		return m.concat(o);
	}
};

/**
 * @brief Asserts a message with a specific kind at a specific time.
 *
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMsgKind:public AssertMessage {
protected:
	int kind;
	simtime_t arrival;
public:
	AssertMsgKind(std::string msg, int kind, simtime_t arrival,
				  bool isPlanned = false,
				  bool continuesTests = false):
		AssertMessage(msg, isPlanned, continuesTests),
		kind(kind),
		arrival(arrival)
	{}
		
	virtual ~AssertMsgKind() {}
	
	/**
	 * @brief Returns true if the passed message has the kind and arrival time
	 * this AssertMsgKind expects.
	 */
	virtual bool isMessage(cMessage* msg) {
		return msg->getKind() == kind && msg->getArrivalTime() == arrival;
	}
	
	/**
	 * @brief Concatenates the description text together with expected kind and
	 * arrival time to an out stream.
	 */
	virtual std::ostream& concat(std::ostream& o) const{
		o << ": kind = " << kind; // << ",  = " << arrival << "s";
		return o;
	}
};

/**
 * @brief Asserts a message with a specific kind in a specific time interval.
 *
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMsgInterval:public AssertMsgKind {
protected:
	simtime_t intvStart;
	simtime_t intvEnd;
	simtime_t msgArrivalTime;
public:
	AssertMsgInterval(std::string msg, int kind,
					  simtime_t intvStart, simtime_t intvEnd,
					  bool isPlanned = false,
					  bool continuesTests = false):
		AssertMsgKind(msg, kind, intvStart, isPlanned, continuesTests),
		intvStart(intvStart),
		intvEnd(intvEnd),
		msgArrivalTime(0)
	{}

	virtual ~AssertMsgInterval() {}

	/**
	 * @brief Returns true if the passed message has the kind and arrival time
	 * this AssertMsgInterval expects.
	 */
	virtual bool isMessage(cMessage* msg) {
		if (msg->getKind() == kind) {
			msgArrivalTime = msg->getArrivalTime();
			return    intvStart      <= msgArrivalTime
				   && msgArrivalTime <= intvEnd;
		}
		return false;
	}

	/**
	 * @brief Concatenates the description text together with expected kind and
	 * arrival time to an out stream.
	 */
	virtual std::ostream& concat(std::ostream& o) const{
		if (intvStart <= msgArrivalTime && msgArrivalTime <= intvEnd) {
			o << ": kind = " << kind << ", " << "arrival time was in range";
		}
		else {
			o << ": kind = " << kind << ", " << "arrival = (" << intvStart << "<=t<=" << intvEnd << ") t was " << msgArrivalTime;
		}
		return o;
	}
};

/**
 * @brief This class provides some testing utilities for Omnet modules.
 *
 * TestModules only implement and provide the methods which implement the parts
 * of a test run which have to be executed by this module. The start of a test
 * run and the order in which these methods are called is done by the
 * TestManager module. @see TestManager for details.
 *
 * Besides the normal assertions and tests like "assertTrue" and "testForTrue"
 * an Omnet modules which extends this class has further access to the following
 * commands:
 * 
 * - (assert-/testFor-)Message(	description, expected kind,
 * 								expected arrival, destination module)
 * - (waitFor-/testAndWaitFor-)Message(	description, expected kind,
 * 										expected arrival, destination module)
 * 
 * (see doc of these methods for details.)
 * 
 * Note: To be able to work the extending module has to to the following things:
 * 
 * - call method "init(name)" at first initialization
 * - call method "announceMessage(msg)" at begin of "handleMessage()"
 * - call method "finalize()" at end of "finish()"
 *
 * - TestManager has to be present as global module in simulation.
 * 
 * @author Karl Wessel
 */
class TestModule {
private:
	/** @brief Copy constructor is not allowed.
	 */
	TestModule(const TestModule&);
	/** @brief Assignment operator is not allowed.
	 */
	TestModule& operator=(const TestModule&);

protected:
	/** @brief Unique name of this module.*/
	std::string name;

	/** @brief Pointer to the TestManager module.*/
	TestManager* manager;
	
	typedef std::list<AssertMessage*> MessageDescList;
	
	/** @brief List of message this module expects to arrive.*/
	MessageDescList expectedMsgs;
	
private:
	/**
	 * @brief Proceeds the message assert to the correct destination.
	 */
	void assertNewMessage(AssertMessage* assert, std::string destination);
	
protected:
	/**
	 * @brief Plans test case with the passed name and description.
	 *
	 * The passed name should then be passed to the "testForX()" method call
	 * which actually executes the test case.
	 *
	 * @param name A short identifier for this test case.
	 * @param description A description what this test case covers.
	 */
	void planTest(std::string name, std::string description) {
		manager->planTest(name, description);
	}

	/**
	 * @brief Executes a previously planned test case with the passed name.
	 *
	 * @param name the name of the test case which has been executed.
	 * @return the test message associated with the test case
	 */
	std::string executePlannedTest(std::string name) {
		return manager->executePlannedTest(name);
	}

	/**
	 * @brief Register the TestModule with the TestManager with the passed name.
	 *
	 * This method has to be called by the subclassing Omnet module during
	 * initialization.
	 * 
	 * Note: The name has to be unique in simulation.
	 */ 
	void init(const std::string& name);
	
	/**
	 * @brief Checks if the passed message is expected.
	 *
	 * This method has to be called at the beginning of "handleMessage()"
	 * or whenever a new message arrives at the module.
	 * The passed message should be the newly arrived message.
	 */
	void announceMessage(cMessage* msg);
	
	/**
	 * @brief Checks if there are still message which have been expected but
	 * haven't arrived.
	 *
	 * This method has to be called at the end of the "finish()"-method.
	 */
	void finalize();
	
	/**
	 * @brief Return a string with the pattern "[module name] - passed text"
	 */
	std::string log(std::string msg);
	
	/**
	 * Asserts the arrival of a message described by the passed AssertMessage object 
	 * at the module with the passed name. If the module name is ommited the message is
	 * expected at this module.
	 * This method should be used if you want to write your own AssertMessage-Descriptor.
	 */
	void assertMessage(AssertMessage* assert, std::string destination = "");
	
	
	/**
	 * Asserts the arrival of a message with the specified kind at the specified
	 * time at module with the passed name. If the module name is ommited the message is
	 * expected at this module.
	 */
	void assertMessage(std::string msg, int kind, simtime_t_cref arrival, std::string destination = "");
	
	/**
	 * Asserts the arrival of a message with the specified kind in the specified
	 * time interval at module with the passed name.
	 *
	 * If the module name is ommited the message is expected at this module.
	 */
	void assertMessage(	std::string msg, int kind,
						simtime_t_cref intvStart, simtime_t_cref intvEnd,
						std::string destination = "");

	/**
	 * @brief Analog for "assertMessage" method but for a previously planned
	 * test case.
	 *
	 * @param testName - name of the planned test case this test executes
	 * @param kind - message kind of the expected message
	 * @param arrival - arrival time of the expected message
	 * @param destination - destination where the message is expected
	 * 						if omitted this module is used as destination
	 */
	void testForMessage(std::string testName,
						int kind, simtime_t_cref arrival,
						std::string destination = "");
	/**
	 * @brief Analog for "assertMessage" method but for a previously planned
	 * test case.
	 *
	 * @param testName Name of the planned test case this test executes.
	 * @param kind The kind of the expected message.
	 * @param intvStart The start of the time interval in which to expect the
	 * 					message.
	 * @param intvEnd 	The end of the time interval in which to expect the
	 * 					message.
	 * @param destination The destination TestModule at which the message is
	 * 					  expected. Default is this module.
	 */
	void testForMessage(std::string testName,
						int kind,
						simtime_t_cref intvStart, simtime_t_cref intvEnd,
						std::string destination = "");

	/**
	 * @brief Does the same as "assertMessage" plus it calls the
	 * "continueTest()"-method when the message arrives.
	 */
	void waitForMessage(std::string msg, int kind, simtime_t_cref arrival, std::string destination = "");
	
	/**
	 * @brief Analog for "waitForMessage" method but for a previously planned
	 * test case.
	 *
	 * @param testName - name of the planned test case this test executes
	 * @param kind - message kind of the expected message
	 * @param arrival - arrival time of the expected message
	 * @param destination - destination where the message is expected
	 * 						if omitted this module is used as destination
	 */
	void testAndWaitForMessage(	std::string testName,
								int kind, simtime_t_cref arrival,
								std::string destination = "");

	/**
	 * @brief Analog for "waitForMessage" method but for a previously planned
	 * test case.
	 *
	 * @param testName - name of the planned test case this test executes
	 * @param kind - message kind of the expected message
	 * @param intvStart The start of the time interval in which to expect the
	 * 					message.
	 * @param intvEnd 	The end of the time interval in which to expect the
	 * 					message.
	 * @param destination - destination where the message is expected
	 * 						if omitted this module is used as destination
	 */
	void testAndWaitForMessage(	std::string testName,
								int kind,
								simtime_t_cref intvStart, simtime_t_cref intvEnd,
								std::string destination = "");

	/**
	 * @brief Marks the previously planned test with the passed name as executed
	 * and passed.
	 *
	 * Meant to be used for "Meta"-tests which do not have a particular
	 * assertion to pass but consist of a set of other tests.
	 *
	 * @param name The identifier of the planned test to execute
	 */
	void testPassed(std::string testName){
		manager->testPassed(testName);
	}

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed boolean as true.
	 *
	 * Analog to "assertTrue" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param v The value which should be true for the test to succeed
	 */
	void testForTrue(std::string testName, bool exp) {
		manager->testForTrue(testName, exp);
	}

	/**
	 * @brief Executes the previously planned test with the passed name by
	 * asserting the passed boolean as false.
	 *
	 * Analog to "assertFalse" but for planned test cases.
	 *
	 * @param name The identifier of the planned test to execute
	 * @param v The value which should be false for the test to succeed
	 */
	void testForFalse(std::string testName, bool exp) {
		manager->testForFalse(testName, exp);
	}

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
		manager->testForEqual(name, expected, actual);
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
		manager->testForClose(name, expected, actual);
	}
public:
	TestModule()
		: name()
		, manager(NULL)
		, expectedMsgs()
	{}
	virtual ~TestModule() {}
};

#endif /*TESTMODULE_H_*/
