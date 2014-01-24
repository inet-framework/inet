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

#include "BasePhyTests.h"
#include "TestMacLayer.h"

Define_Module(BasePhyTests);

void BasePhyTests::planTestRun1()
{
	planTestModule("mac0", "Mac layer executing all of test run 1");
	planTest("0", "Test initialisation of phy layer.");
	planTest("1", "Check correct passing of channel state from decider to mac "
				  "by phy. For this a sequence of predefined states is created "
				  "by the decider.");
	planTest("1.1", "First channel idle state is true.");
	planTest("1.2", "First channel rssi is 1.0");
	planTest("1.3", "Second channel idle state is false.");
	planTest("1.4", "Second channel rssi is 2.0");
	planTest("1.5", "Third channel idle state is true.");
	planTest("1.6", "Third channel rssi is 3.0");
	planTest("1.7", "4th channel idle state is false.");
	planTest("1.8", "4th channel rssi is 4.0");

	planTest("2", "Check correct handling of radio switches.");
	planTest("2.1", "Radio starts in SLEEP mode.");
	planTest("2.2", "Switch SLEEP to RX.");
	planTest("2.3", "Switch RX to SLEEP.");
	planTest("2.4", "Try switching during ongoing switching.");

	planTest("3", "Check correct forwarding of ChannelSenseRequests from mac to"
				  " decider by phy layer.");

	planTest("4", "Test if sending of packet while radio is not in TX state is "
				  "correctly handled by phy (throws error).");
}

void BasePhyTests::testRun1(int stage, cMessage* msg)
{
	if(stage == 0) {
		getModule<TestPhyLayer>("phy0")->testInitialisation();
	}
	getModule<TestMacLayer>("mac0")->testRun1(stage, msg);
}




void BasePhyTests::planTestRun6()
{
	planTestModule("mac0", "Mac layer of Host0 (Sender)");
	planTestModule("mac1", "Mac layer of Host1 (receiver)");
    planTest("1.", "Host1 sends AirFrame A to Host2");
    planTest("2.", "Host2 starts receiving AirFrame A");
    planTest("3.", "Host2 starts a ChannelSense");
    planTest("3.1", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during ChannelSense duration which should return AirFrame A");
    planTest("3.2", "Host2 starts a ChannelSense");
    planTest("4.", "Host2 completes reception of AirFrame A");
    planTest("5.", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during ChannelSense duration which should return AirFrame A");
    planTest("6.", "Host2 starts a ChannelSense");
    planTest("7.", "Host2 ends ChannelSense and asks ChannelInfo for AirFrames during ChannelSense duration which should return none");
}

void BasePhyTests::testRun6(int stage, cMessage* msg)
{
	if(stage < 2) {
		//planTest("1.", "Host1 sends AirFrame A to Host2");
		//planTest("2.", "Host2 starts receiving AirFrame A");
		getModule<TestMacLayer>("mac0")->testRun6(stage, msg);
	} else {
		//planTest("3.", "Host2 starts a ChannelSense");
		//planTest("3.1", "Host2 ends ChannelSense and asks ChannelInfo for"
		//				  "AirFrames during ChannelSense duration which "
		//				  "should return AirFrame A");
		//planTest("3.2", "Host2 starts a ChannelSense");
		//planTest("4.", "Host2 completes reception of AirFrame A");
		//planTest("5.", "Host2 ends ChannelSense and asks ChannelInfo for "
		//				 "AirFrames during ChannelSense duration which "
		//				 "should return AirFrame A");
		//planTest("6.", "Host2 starts a ChannelSense");
		//planTest("7.", "Host2 ends ChannelSense and asks ChannelInfo for "
		//				 "AirFrames during ChannelSense duration which "
		//				 "should return none");
		getModule<TestMacLayer>("mac1")->testRun6(stage, msg);
	}
}

void BasePhyTests::planTestRun7()
{
	planTestModule("mac0", "Mac layer of host A1");
	planTestModule("phy0", "Phy layer of host A1");
	planTestModule("phy1", "Phy layer of host A2");
	planTestModule("mac2", "Mac layer of host B1");
	planTestModule("phy2", "Phy layer of host B1");
	planTestModule("phy3", "Phy layer of host B2");
	planTest("1.1.1", "2 Hosts protocol A (A1 and A2)");
	planTest("1.1.2", "2 hosts protocol B (B1 and B2).");
	planTest("1.2", "Host A1 sends packet 1.");
	planTest("1.3", "Packet 1 arrives at phy of A2, B1 and B2.");
	planTest("1.4", "Packet 1 arrives only at decider A2.");

	planTest("1.5.1", "Packet 1 is still active.");
	planTest("1.5.2", "Host B1 sends packet 2.");
	planTest("1.6", "Packet 2 arrives at phy of A1, A2 and B2.");
	planTest("1.7", "Packet 2 arrives only at decider B2.");
	planTest("1.8", "Interference for Packet 2 at decider B2 contains packet 1.");
	planTest("1.9", "Interference for Packet 1 at decider A2 contains packet 2.");
}

void BasePhyTests::testRun7(int stage, cMessage* msg)
{
	if(stage == 0)
	{
//planTest("1.1.1", "2 Hosts protocol A (A1 and A2)");
		TestPhyLayer* tmp = getModule<TestPhyLayer>("phy0");
		assertEqual("Host A1 uses protocol 1",
					1, tmp->par("protocol").longValue());
		tmp = getModule<TestPhyLayer>("phy1");
		testForEqual("1.1.1", 1, tmp->par("protocol").longValue());

//planTest("1.1.2", "2 hosts protocol B (B1 and B2).");
		tmp = getModule<TestPhyLayer>("phy2");
		assertEqual("Host B1 uses protocol 2",
					2, tmp->par("protocol").longValue());
		tmp = getModule<TestPhyLayer>("phy3");
		testForEqual("1.1.2", 2, tmp->par("protocol").longValue());

//planTest("1.2", "Host A1 sends packet 1.");
		getModule<TestMacLayer>("mac0")->testRun7(stage, msg);
	} else if(stage == 1){
		getModule<TestMacLayer>("mac0")->testRun7(stage, msg);
	} else if(stage == 2 || stage == 3){
		getModule<TestMacLayer>("mac2")->testRun7(stage, msg);
	} else if(stage == 4) {
//planTest("1.9", "Interference for Packet 1 at decider A2 contains packet 2.");
		getModule<TestDecider>("decider1")->testRun7(stage, msg);
	} else if(stage == 5) {
//planTest("1.8", "Interference for Packet 2 at decider B2 contains packet 1.");
		getModule<TestDecider>("decider3")->testRun7(stage, msg);
	}
}


void BasePhyTests::planTests(int run)
{
	//getModule<TestMacLayer>("mac0")->planTests(run);
    if(run == 1)
    {
        planTestRun1();
    } else if(run == 6)
    {
        planTestRun6();
    } else if(run == 7)
    {
        planTestRun7();
    }
    else if (run > 7)
    	assertFalse("Unknown test run number: " + run, true);
}

void BasePhyTests::runTests(int run, int stage, cMessage* msg)
{
	if(run == 1)
	{
		testRun1(stage, msg);
	}
	else if(run == 6)
	{
		testRun6(stage, msg);
	}
	else if(run == 7)
	{
		testRun7(stage, msg);
	}
	else {
		getModule<TestMacLayer>("mac0")->runTests(run, stage, msg);
		if(getModule<TestMacLayer>("mac1"))
			getModule<TestMacLayer>("mac1")->runTests(run, stage, msg);
	}
}


