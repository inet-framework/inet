#include "TestModule.h"
#include "FindModule.h"

void TestModule::init(const std::string& name) {
	this->name = name;
	
	manager = FindModule<TestManager*>::findGlobalModule();
	
	if(!manager) {
		fail("Could not find TestManager module.");
		exit(1);
	}
	
	manager->registerModule(name, this);
}

void TestModule::announceSignal(simsignal_t signalID) {
    SignalDescList::iterator it = expectedSignals.begin();
    while(it != expectedSignals.end()) {
        AssertSignal* exp = *it;
        EV << toString(*exp) << "\n";
        if(exp->isSignal(signalID)) {

            std::string testMsg = exp->getMessage();
            if(exp->isPlanned()) {
                testMsg = executePlannedTest(testMsg);
            }
            pass(log("Expected \"" + testMsg + "\"" + toString(*exp)));

            if(exp->continueTests()) {
                manager->continueTests(signalID);
            }

            delete exp;
            it = expectedSignals.erase(it);
            continue;
        }
        it++;
    }
}

void TestModule::announceMessage(cMessage* msg) {
	MessageDescList::iterator it = expectedMsgs.begin();
	bool foundMessage = false;
	while(it != expectedMsgs.end()) {
		
		AssertMessage* exp = *it;
		if(exp->isMessage(msg)) {
				
			std::string testMsg = exp->getMessage();
			if(exp->isPlanned()) {
				testMsg = executePlannedTest(testMsg);
			}
			pass(log("Expected \"" + testMsg + "\"" + toString(*exp)));

			if(exp->continueTests()) {
				manager->continueTests(msg);
			}

			foundMessage = true;

			delete exp;
			it = expectedMsgs.erase(it);
			continue;				
		}
		it++;
	}
	if(!foundMessage) {
		fail(log("Received not expected Message " + toString(msg) + " at " + toString(msg->getArrivalTime()) + "s"));
	}
	manager->onMessage(name, msg);
}

void TestModule::finalize() {
    for(MessageDescList::iterator it = expectedMsgs.begin();
		it != expectedMsgs.end(); it++) {
		
		AssertMessage* exp = *it;

		std::string testMsg = exp->getMessage();
		if(exp->isPlanned()) {
			testMsg = executePlannedTest(testMsg);
		}
		fail(log("Expected \"" + testMsg + "\"" + toString(*exp)));
		delete exp;
	}
	expectedMsgs.clear();

    for(SignalDescList::iterator it = expectedSignals.begin();
        it != expectedSignals.end(); it++) {

        AssertMessage* exp = *it;

        std::string testMsg = exp->getMessage();
        if(exp->isPlanned()) {
            testMsg = executePlannedTest(testMsg);
        }
        fail(log("Expected \"" + testMsg + "\"" + toString(*exp)));
        delete exp;
    }
    expectedSignals.clear();
}

std::string TestModule::log(std::string msg) {
	return "[" + name + "] - " + msg;
}	

void TestModule::assertNewSignal(AssertSignal* assert, std::string destination) {

    if(destination == "") {
        expectedSignals.push_back(assert);
    } else {
        TestModule* dest = manager->getModule<TestModule>(destination);
        if(!dest) {
            fail(log("No test module with name \"" + destination + "\" found."));
            return;
        }

        dest->expectedSignals.push_back(assert);
    }
}

void TestModule::assertNewMessage(AssertMessage* assert, std::string destination) {
	
	if(destination == "") {
		expectedMsgs.push_back(assert);
	} else {
		TestModule* dest = manager->getModule<TestModule>(destination);
		if(!dest) {
			fail(log("No test module with name \"" + destination + "\" found."));
			return;
		}
		
		dest->expectedMsgs.push_back(assert);
	}
}

void TestModule::assertSignal(std::string msg, simsignal_t signalID, simtime_t_cref arrival, std::string destination)
{
    assertNewSignal(new AssertSignal(msg, signalID, arrival), destination);
}

void TestModule::assertMessage(AssertMessage* assert, std::string destination) {
	assertNewMessage(assert, destination);
}

void TestModule::assertMessage(	std::string msg, 
								int kind, simtime_t_cref arrival,
								std::string destination)
{
	
	assertNewMessage(new AssertMsgKind(msg, kind, arrival),
					 destination);
}

void TestModule::assertMessage(	std::string msg,
								int kind,
								simtime_t_cref intvStart, simtime_t_cref intvEnd,
								std::string destination)
{

	assertNewMessage(new AssertMsgInterval(msg, kind, intvStart, intvEnd),
					 destination);
}

void TestModule::testForMessage(std::string testName,
								int kind, simtime_t_cref arrival,
								std::string destination)
{

	assertNewMessage(new AssertMsgKind(testName, kind, arrival, true),
					 destination);
}

void TestModule::testForMessage(std::string testName,
								int kind,
								simtime_t_cref intvStart, simtime_t_cref intvEnd,
								std::string destination)
{

	assertNewMessage(new AssertMsgInterval(	testName, kind,
											intvStart, intvEnd, true),
					 destination);
}

void TestModule::waitForSignal(std::string msg,
                                simsignal_t signalID, simtime_t_cref arrival,
                                std::string destination) {

    assertNewSignal(new AssertSignal(msg,
                                     signalID,
                                     arrival,
                                     false,
                                     true),
                     destination);
}

void TestModule::waitForMessage(std::string msg,
								int kind, simtime_t_cref arrival,
								std::string destination) {
	
	assertNewMessage(new AssertMsgKind(msg,
									   kind,
									   arrival,
									   false,
									   true),
					 destination);
}

void TestModule::testAndWaitForMessage(	std::string testName,
										int kind, simtime_t_cref arrival,
										std::string destination)
{

	assertNewMessage(new AssertMsgKind(testName, kind, arrival, true, true),
					 destination);
}

void TestModule::testAndWaitForMessage(	std::string testName,
										int kind,
										simtime_t_cref intvStart, simtime_t_cref intvEnd,
										std::string destination)
{

	assertNewMessage(new AssertMsgInterval(	testName, kind,
											intvStart, intvEnd, true, true),
					 destination);
}

