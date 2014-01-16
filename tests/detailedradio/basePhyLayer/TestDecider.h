#ifndef TESTDECIDER_H_
#define TESTDECIDER_H_

#include <Decider.h>
#include <TestModule.h>
#include <BasePhyLayer.h>
#include "TestGlobals.h"

class TestDecider:public Decider, public TestModule {
protected:
	int myIndex;
	int run;
	int RECEIVING_STATE;

	bool state;
	double rssi;

	simtime_t senseStart;

protected:
	void assertMessage(std::string msg, int state, DetailedRadioFrame * frame, simtime_t_cref arrival, std::string dest = "") {
		TestModule::assertMessage(new AssertAirFrame(msg, state, arrival, frame), dest);
	}

public:
	TestDecider( DeciderToPhyInterface* phy
	           , double                 sensitivity
	           , int                    myIndex)
		: Decider(phy)
		, myIndex(myIndex)
		, run(0)
		, RECEIVING_STATE(0)
		, state(false)
		, rssi(0.0)
		, senseStart()
	{
		init("decider" + toString(myIndex));
	}

	/** @brief Initialize the decider from XML map data.
	 *
	 * This method should be defined for generic decider initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap& params) {
	    ParameterMap::const_iterator it;
	    bool                         bInitSuccess = true;

	    if((it = params.find("recvState")) != params.end()) {
	    	RECEIVING_STATE = ParameterMap::mapped_type(it->second).longValue();
	    }
	    else {
	        bInitSuccess = false;
	        opp_warning("No recvState defined in config.xml for TestDecider!");
	    }
	    if((it = params.find("runNumber")) != params.end()) {
	    	run = ParameterMap::mapped_type(it->second).longValue();
	    }
	    else {
	        bInitSuccess = false;
	        opp_warning("No recvState defined in config.xml for TestDecider!");
	    }
	    return Decider::initFromMap(params) && bInitSuccess;
	}

	virtual ChannelState getChannelState() const {
		(const_cast<TestDecider*>(this))->state = !state;
		(const_cast<TestDecider*>(this))->rssi += 1.0;
		return ChannelState(state, false, rssi);
	}

	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request) {
		announceMessage(request);

		simtime_t time = phy->getSimTime();
		if(request->getSenseTimeout() > 0.0) {
			senseStart = time;
			simtime_t next = time + request->getSenseTimeout();

			request->setSenseTimeout(0.0);
			return next;
		} else {

			DeciderToPhyInterface::AirFrameVector v;
			phy->getChannelInfo(senseStart, time, v);
			request->setResult(ChannelState(v.empty()));
			phy->sendControlMsgToMac(request);
			return -1.0;
		}
	}

	void testRun7(int stage, cMessage* msg)
	{

		DetailedRadioFrame * frame = dynamic_cast<DetailedRadioFrame *>(msg);
		assertTrue("Passed message is an AirFrame.", frame != NULL, true);

		const DetailedRadioSignal& s = frame->getSignal();

		simtime_t start = s.getReceptionStart();
		simtime_t end = s.getReceptionEnd();

		AirFrameVector interf;

		if(stage == 4) {
//planTest("1.9", "Interference for Packet 1 at decider A2 contains packet 2.");
			phy->getChannelInfo(start, end, interf);

			testForEqual("1.9", (unsigned int)2, interf.size());

//planTest("1.8", "Interference for Packet 2 at decider B2 contains packet 1.");
			waitForMessage( "End of Packet 2 at decider of B2",
							BasePhyLayer::AIR_FRAME,
							simTime() + 4.0, "decider3");
		} else if(stage == 5) {
			phy->getChannelInfo(start, end, interf);

			testForEqual("1.8", (unsigned int)2, interf.size());
		}
	}

	simtime_t processSignal(DetailedRadioFrame * frame) {
		announceMessage(frame);

		const DetailedRadioSignal& s = frame->getSignal();

		simtime_t time = phy->getSimTime();
		simtime_t end;

		if(run == 7)
		{
			end = s.getReceptionEnd();
		} else {
			end = s.getReceptionStart() + s.getDuration() * 0.1;
		}

		if(time == s.getReceptionStart()) {
			assertMessage("Scheduled AirFrame to end at phy.", RECEIVING_STATE,
						  frame, end, "phy" + toString(myIndex));
			assertMessage("Scheduled AirFrame to end.", RECEIVING_STATE,
						  frame, end);
			return end;

		} else if(time == end) {
			phy->sendUp(frame, new DeciderResult(true));
			TestModule::assertMessage("MacPkt at mac layer.", TEST_MACPKT,
						  time, "mac" + toString(myIndex));
			return -1;
		}

		fail("Simtime(" + toString(time) + ") was neither signal start ("
			 + toString(s.getReceptionStart()) + ") nor end(" + toString(end)
			 + ").");

		return -1;
	}

	virtual ~TestDecider() {
		finalize();
	}
};
#endif /*TESTDECIDER_H_*/
