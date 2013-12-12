#include "TestMacLayer.h"

#include <sstream>

#include "PhyControlInfo_m.h"
#include "DeciderToPhyInterface.h"
#include "FindModule.h"

Define_Module(TestMacLayer);

//---omnetpp part----------------------

//---intialisation---------------------
void TestMacLayer::initialize(int stage) {
	if(stage == 0) {
		if (simulation.getSystemModule()->par("showPassed"))
			displayPassed = true;
		else
			displayPassed = false;

		myIndex = findContainingNode(this)->getIndex();

		dataOut = findGate("lowerLayerOut");
		dataIn = findGate("lowerLayerIn");

		controlOut = findGate("lowerControlOut");
		controlIn = findGate("lowerControlIn");

		run = simulation.getSystemModule()->par("run");

		init("mac" + toString(myIndex));

		testPhy = FindModule<TestPhyLayer*>::findSubModule(this->getParentModule());
		phy = testPhy;

	}
}



//---test handling and redirection------------

/**
 * Redirects test handling to the "testRunx()"-methods
 * dependend on the current run.
 */
void TestMacLayer::runTests(int run, int state, const cMessage* msg)
{
	Enter_Method_Silent();

	//testRun1 and testRun6 are called by BasePhyTests directly

	if(run == 2)
		testRun2(state, msg);
	else if(run == 3)
		testRun3(state, msg);
	else if (run == 5)
		testRun5(state, msg);
	else
		fail("Unknown or duplicate test run: " + toString(run));
}

void TestMacLayer::testRun1(int stage, const cMessage* /*msg*/){
	Enter_Method_Silent();
	if(stage == 0)
	{
//planTest("1", "Check correct passing of channel state from decider to mac "
//			  	"by phy. For this a sequence of predefined states is created "
//		  		"by the decider.");
		ChannelState state = phy->getChannelState();
		testForTrue("1.1", state.isIdle());
		testForClose("1.2", 1.0, state.getRSSI());

		state = phy->getChannelState();
		testForFalse("1.3", state.isIdle());
		testForClose("1.4", 2.0, state.getRSSI());

		state = phy->getChannelState();
		testForTrue("1.5", state.isIdle());
		testForClose("1.6", 3.0, state.getRSSI());

		state = phy->getChannelState();
		testForFalse("1.7", state.isIdle());
		testForClose("1.8", 4.0, state.getRSSI());

		testPassed("1");

//planTest("2", "Check correct handling of radio switches.");
//planTest("2.1", "Radio starts in SLEEP mode.");
		int radioState = phy->getRadioMode();
		testForEqual("2.1", IRadio::RADIO_MODE_SLEEP, radioState);

//planTest("2.2", "Switch SLEEP to RX.");
		simtime_t switchTime = phy->setRadioState(IRadio::RADIO_MODE_RECEIVER);
		assertEqual("Correct switch time to RX.", simtime_t(3.0), switchTime);

		assertMessage(	"SWITCH_OVER message at phy.", BasePhyLayer::RADIO_SWITCHING_OVER,
						simTime() + switchTime,
						"phy0");
		waitForMessage(	"SWITCH_OVER message.",
						BasePhyLayer::RADIO_SWITCHING_OVER,
						simTime() + switchTime);

//planTest("2.4", "Try switching during ongoing switching.");
		switchTime = phy->setRadioState(IRadio::RADIO_MODE_RECEIVER);
		testForTrue("2.4", switchTime < 0.0);
	}
	else if(stage == 1) {
//planTest("2.2", "Switch SLEEP to RX.");
		int state = phy->getRadioMode();
		testForEqual("2.2", IRadio::RADIO_MODE_RECEIVER, state);

//planTest("2.3", "Switch RX to SLEEP.");
		simtime_t switchTime = phy->setRadioState(IRadio::RADIO_MODE_SLEEP);
		assertEqual("Correct switch time to SLEEP.", simtime_t(1.5), switchTime);

		assertMessage(	"SWITCH_OVER message at phy.", BasePhyLayer::RADIO_SWITCHING_OVER,
						simTime() + switchTime,
						"phy0" );
		waitForMessage(	"SWITCH_OVER message.",
						BasePhyLayer::RADIO_SWITCHING_OVER,
						simTime() + switchTime);
	}else if(stage == 2) {
		int state = phy->getRadioMode();
		testForEqual("2.3", IRadio::RADIO_MODE_SLEEP, state);
		testPassed("2");

//planTest("3", "Check correct forwarding of ChannelSenseRequests from mac to"
//			  	" decider by phy layer.");
		ChannelSenseRequest* req = new ChannelSenseRequest();
		req->setKind(BasePhyLayer::CHANNEL_SENSE_REQUEST);
		req->setSenseTimeout(0.5);
		req->setSenseMode(UNTIL_TIMEOUT);
		send(req, dataOut);
		testForChannelSenseRequest("3", req);

	} else if(stage == 3) {
//planTest("4", "Test if sending of packet while radio is not in TX state is "
//		  		"correctly handled by phy (throws error).");
		int state = phy->getRadioMode();
		assertNotEqual("Radio is not in TX.", IRadio::RADIO_MODE_TRANSMITTER, state);

		MACFrameBase * pkt = createMacPkt(1.0);
		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");

		manager->testForError("4");
	} else {
		fail("Invalid stage for run 1:" + toString(stage));
	}
}

/**
 * Testhandling for run 2:
 * - check sending on already sending
 */
void TestMacLayer::testRun2(int stage, const cMessage* /*msg*/){
	switch(stage) {
	case 0:
		waitForTX();
		break;
	case 1:{
		int state = phy->getRadioMode();
		assertEqual("Radio is in TX.", IRadio::RADIO_MODE_TRANSMITTER, state);

		MACFrameBase * pkt = createMacPkt(1.0);
		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
		//we don't assert any txOver message because we asume that the run
		//has been canceled before
		continueIn(0.5);
		break;
	}
	case 2: {
		MACFrameBase * pkt = createMacPkt(1.0);
		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
		//the run should be canceled after the asserted message, this is checked
		//indirectly by the following assertion
		manager->assertError("Phy should throw an error if we are trying to "
							 "to send more than one packet at once.");
		break;
	}
	default:
		fail("Invalid stage for run 2:" + toString(stage));
		break;
	}
}

/**
 * Testhandling for run 3:
 * - check valid sending of a packet to 3 recipients
 * - check getChannelInfo
 */
void TestMacLayer::testRun3(int stage, const cMessage* msg){
	switch(myIndex) {
	case 0:
		testSending1(stage, msg);
		break;
	case 1:
	case 2:
	case 3:
		testChannelInfo(stage);

		break;
	default:
		fail("No handling for test run 3 for this host index: " + toString(myIndex));
		break;
	}
}

/**
 * Testhandling for run 5:
 *
 * - check getChannelState()
 * - check channel sensing
 *
 * TODO: for now the methods called here are empty
 *
 * Testing SNRThresholdDecider is done in initialize of the TestPhyLayer
 * without using the simulation so far, i.e. among other things TestPhyLayer
 * overriding the methods of DeciderToPhyInterface implemented by
 * BasePhyLayer. This way we check whether SNRThresholdDecider makes correct calls
 * on the Interface and return specific testing values to SNRThresholdDecider.
 *
 * Sending real AirFrames over the channel that SNRThresholdDecider can obtain
 * by calling the DeciderToPhyInterface will be done later.
 */
void TestMacLayer::testRun5(int stage, const cMessage* /*msg*/)
{
	switch (stage) {
		case 0:
			testGetChannelStateWithBD();
			testChannelSenseWithBD();
			break;
		case 1:
			// testSwitchRadio(stage);
			testChannelSenseWithBD();
			break;
		default:
			break;
	}
}

void TestMacLayer::testForChannelSenseRequest(std::string test,
											  ChannelSenseRequest* req)
{
	assertMessage(	"ChannelSense at phy layer.",
	                BasePhyLayer::CHANNEL_SENSE_REQUEST,
					simTime(), "phy" + toString(myIndex));
	assertMessage(	"ChannelSense at decider.",
					BasePhyLayer::CHANNEL_SENSE_REQUEST,
					simTime(), "decider" + toString(myIndex));
	assertMessage(	"Scheduled sense request at phy.",
					BasePhyLayer::CHANNEL_SENSE_REQUEST,
					simTime() + req->getSenseTimeout(),
					"phy" + toString(myIndex));
	assertMessage(	"Scheduled sense request.",
					BasePhyLayer::CHANNEL_SENSE_REQUEST,
					simTime() + req->getSenseTimeout(),
					"decider" + toString(myIndex));
	testAndWaitForMessage(	test, BasePhyLayer::CHANNEL_SENSE_REQUEST,
							simTime() + req->getSenseTimeout());
}

void TestMacLayer::testRun6(int stage, const cMessage* msg)
{
	Enter_Method_Silent();

	if(stage == 0) {
// planTest("1.", "Host1 sends AirFrame A to Host2");
		waitForTX();
	} else if(stage == 1) {
		MACFrameBase * pkt = createMacPkt(1.0);
		sendDown(pkt);
		testForMessage("1.", TEST_MACPKT, simTime(), "phy0");

// planTest("2.", "Host2 starts receiving AirFrame A");
		testForMessage("2.", BasePhyLayer::AIR_FRAME, simTime(), "phy1");
		waitForMessage("First process of AirFrame at Decider",
					   BasePhyLayer::AIR_FRAME,
					   3.5, "decider1");
//planTest("3.", "Host2 starts a ChannelSense");
	} else if(stage == 2) {
		ChannelSenseRequest* req = new ChannelSenseRequest();
		req->setKind(BasePhyLayer::CHANNEL_SENSE_REQUEST);
		req->setSenseTimeout(0.5);
		req->setSenseMode(UNTIL_TIMEOUT);

		send(req, dataOut);
		testForChannelSenseRequest("3.", req);

	} else if(stage == 3) {
//planTest("3.1", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during "
//				  "ChannelSense duration which should return AirFrame A");
		const ChannelSenseRequest* answer
				= dynamic_cast<const ChannelSenseRequest*>(msg);
		assertTrue("Received ChannelSenseRequest answer.", answer != NULL);
		testForFalse("3.1", answer->getResult().isIdle());

//planTest("3.2", "Host2 starts a ChannelSense");
		ChannelSenseRequest* req = new ChannelSenseRequest();
		req->setKind(BasePhyLayer::CHANNEL_SENSE_REQUEST);
		req->setSenseTimeout(1.0);
		req->setSenseMode(UNTIL_TIMEOUT);
		send(req, dataOut);
		testForChannelSenseRequest("3.2", req);

// planTest("4.", "Host2 completes reception of AirFrame A");
		assertMessage("Transmission over message at phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 0.5, "phy0");
		assertMessage("Transmission over message from phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 0.5, "mac0");
		testForMessage("4.", BasePhyLayer::AIR_FRAME, simTime() + 0.5, "phy1");

	} else if(stage == 4) {
// planTest("5.", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames"
//		   		  "during ChannelSense duration which should return AirFrame A");
		//decider will return busy if AirFrame was returned otherwise idle
		const ChannelSenseRequest* answer
				= dynamic_cast<const ChannelSenseRequest*>(msg);
		assertTrue("Received ChannelSenseRequest answer.", answer != NULL);
		testForFalse("5.", answer->getResult().isIdle());

// planTest("6.", "Host2 starts a ChannelSense");
		ChannelSenseRequest* req = new ChannelSenseRequest();
		req->setKind(BasePhyLayer::CHANNEL_SENSE_REQUEST);
		req->setSenseTimeout(0.5);
		req->setSenseMode(UNTIL_TIMEOUT);
		send(req, dataOut);
		testForChannelSenseRequest("6.", req);

	} else if(stage == 5) {
// planTest("7.", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during "
//  			 "ChannelSense duration which should return none");
		const ChannelSenseRequest* answer
				= dynamic_cast<const ChannelSenseRequest*>(msg);
		assertTrue("Received ChannelSenseRequest answer.", answer != NULL);
		testForTrue("7.", answer->getResult().isIdle());
	}


	/*
	planTest("1.", "Host1 sends AirFrame A to Host2");
	planTest("2.", "Host2 starts receiving AirFrame A");
	planTest("3.", "Host2 starts a ChannelSense");
	planTest("4.", "Host2 completes reception of AirFrame A");
	planTest("5.", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during "
				   "ChannelSense duration which should return AirFrame A");
	*/
}

void TestMacLayer::testRun7(int stage, const cMessage* /*msg*/)
{
	Enter_Method_Silent();

	if(stage == 0) {
//planTest("1.2", "Host A1 sends packet 1.");
		waitForTX();
	} else if(stage == 1) {
		MACFrameBase * pkt = createMacPkt(5.0);
		sendDown(pkt);

		assertMessage("Transmission over message at phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 5.0, "phy0");
		assertMessage("Transmission over message from phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 5.0, "mac0");

		testForMessage("1.2", TEST_MACPKT, simTime(), "phy0");


//planTest("1.4", "Packet 1 arrives only at decider A2.");
		testForMessage("1.4", BasePhyLayer::AIR_FRAME, simTime(), "decider1");

//planTest("1.3", "Packet 1 arrives at phy of A2, B1 and B2.");
		assertMessage("Packet 1 at phy of A2", BasePhyLayer::AIR_FRAME, simTime(), "phy1");
		assertMessage("Packet 1 at phy of B2", BasePhyLayer::AIR_FRAME, simTime(), "phy3");
		assertMessage("End of Packet 1 at phy of B1", BasePhyLayer::AIR_FRAME,
					  simTime() + 5.0, "phy2");
		assertMessage("End of Packet 1 at phy of B2", BasePhyLayer::AIR_FRAME,
					  simTime() + 5.0, "phy3");
		testForMessage("1.3", BasePhyLayer::AIR_FRAME,
					   	   	  simTime(), "phy2");

		continueIn(0.5);
	} else if(stage == 2) {
//planTest("1.5.1", "Packet 1 is still active.");
		testPassed("1.5.1"); //this is implicitly true since if not we would
							 //get an unasserted TX over until now
//planTest("1.5.2", "Host B1 sends packet 2.");
		waitForTX();
	} else if(stage == 3) {
		MACFrameBase * pkt = createMacPkt(5.0);
		sendDown(pkt);

		assertMessage("Transmission over message at phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 5.0, "phy2");
		assertMessage("Transmission over message from phy",
					  BasePhyLayer::TX_OVER,
					  simTime() + 5.0, "mac2");

		testForMessage("1.5.2", TEST_MACPKT, simTime(), "phy2");


//planTest("1.7", "Packet 2 arrives only at decider B2.");
		testForMessage("1.7", BasePhyLayer::AIR_FRAME, simTime(), "decider3");

//planTest("1.6", "Packet 2 arrives at phy of A1, A2 and B2.");
		assertMessage("Packet 2 at phy of A1", BasePhyLayer::AIR_FRAME, simTime(), "phy0");
		assertMessage("Packet 2 at phy of A2", BasePhyLayer::AIR_FRAME, simTime(), "phy1");
		assertMessage("End of Packet 1 at phy of A1", BasePhyLayer::AIR_FRAME,
					  simTime() + 5.0, "phy0");
		assertMessage("End of Packet 1 at phy of A2", BasePhyLayer::AIR_FRAME,
					  simTime() + 5.0, "phy1");
		testForMessage("1.6", BasePhyLayer::AIR_FRAME,
					   	   	  simTime(), "phy3");

//planTest("1.9", "Interference for Packet 1 at decider A2 contains packet 2.");
		waitForMessage( "End of Packet 1 at decider of A2",
						BasePhyLayer::AIR_FRAME, simTime() + 1.0, "decider1");
	}
}

//---run 3 tests----------------------------

void TestMacLayer::testChannelInfo(int stage) {
	switch(stage) {
	case 0: {
		DeciderToPhyInterface::AirFrameVector v;
		testPhy->getChannelInfo(0.0, simTime(), v);

		assertTrue("No AirFrames on channel.", v.empty());
		break;
	}
	case 4:
	case 2:
	case 3:{
		if(myIndex ==( stage - 2 + 1)) {
			DeciderToPhyInterface::AirFrameVector v;
			testPhy->getChannelInfo(0.0, simTime(), v);

			assertFalse("AirFrames on channel.", v.empty());
		}
		break;
	}
	case 1:
	    break;
	default:
		std::stringstream sBuf;

		sBuf << "TestChannelInfo: Unknown stage (" << stage << ").";
		fail(sBuf.str());
		break;
	}
	//displayPassed = false;
}

void TestMacLayer::testSending1(int stage, const cMessage* /*lastMsg*/) {
	switch(stage) {
	case 0: {
		waitForTX();

		break;
	}
	case 1:{
		MACFrameBase * pkt = createMacPkt(1.0);

		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");

		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy1");
		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy2");
		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy3");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy1");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy2");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy3");

		waitForMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider1");
		waitForMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider2");
		waitForMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider3");

		assertMessage("Transmission over message at phy", BasePhyLayer::TX_OVER, simTime() + 1.0, "phy0");
		assertMessage("Transmission over message from phy", BasePhyLayer::TX_OVER, simTime() + 1.0);
		break;
	}
	case 2:
	case 3:
	case 4:{
		//TestMacLayer* mac = manager->getModule<TestMacLayer>("mac" + toString((stage - 2)+ 1));
		//mac->onAssertedMessage(stage, 0);
		break;
	}
	default:
		std::stringstream sBuf;

		sBuf << "Unknown stage (" << stage << ").";
		fail(sBuf.str());
		break;
	}
}

//---run 5 tests----------------------------

void TestMacLayer::testChannelSenseWithBD()
{
	// empty for now

}

void TestMacLayer::testGetChannelStateWithBD()
{
	// empty for now
}

//---utilities------------------------------

void TestMacLayer::continueIn(simtime_t_cref time){
	scheduleAt(simTime() + time, new cMessage(0, 23242));
	waitForMessage(	"Waiting for " + toString(time) + "s.",
					23242,
					simTime() + time);
}

void TestMacLayer::waitForTX() {
	simtime_t switchTime = phy->setRadioState(IRadio::RADIO_MODE_TRANSMITTER);
	assertTrue("A valid switch time.", switchTime >= 0.0);


	assertMessage(	"SWITCH_OVER to TX message at phy.",
					BasePhyLayer::RADIO_SWITCHING_OVER,
					simTime() + switchTime,
					"phy" + toString(myIndex));
	waitForMessage(	"SWITCH_OVER to TX message.",
					BasePhyLayer::RADIO_SWITCHING_OVER,
					simTime() + switchTime);
}

void TestMacLayer::sendDown(MACFrameBase * pkt) {
	send(pkt, dataOut);
}

MACFrameBase * TestMacLayer::createMacPkt(simtime_t_cref length) {
    double bitrate = 2E+6;
    PhyControlInfo *controlInfo = new PhyControlInfo();
    controlInfo->setBitrate(bitrate);
	MACFrameBase * res = new MACFrameBase();
	res->setKind(TEST_MACPKT);
	res->setDuration(length);
	res->setBitLength(bitrate * length.dbl());
    res->setControlInfo(controlInfo);
	return res;
}
