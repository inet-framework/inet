/***************************************************************************
                          CHannelInfoTest.cc  -  description
                             -------------------
    begin                : Fri Jan 11 2008
    copyright            : (C) 2007 by wessel
    email                : wessel@tkn.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <omnetpp.h>
#include <ChannelInfo.h>
#include <asserts.h>
#include <OmnetTestBase.h>

/**
 * Unit test for isInBoundary method of class Coord
 *
 * - test with one AirFrame
 * - test with removed AirFrame
 */
void testIntersections() {
	
	ChannelInfo testChannel;

	//test with one AirFrame
	DetailedRadioFrame * frame1 = new DetailedRadioFrame();
	frame1->setDuration(2.0);
	
	testChannel.addAirFrame(frame1, 1.0);
	
	ChannelInfo::AirFrameVector v;
	testChannel.getAirFrames(0.0, 0.9, v);	
	assertTrue("No intersecting AirFrames before single AirFrame.", v.empty());
	
	v.clear();
	testChannel.getAirFrames(3.1, 3.9, v);	
	assertTrue("No intersecting AirFrames after single AirFrame.", v.empty());
	
	v.clear();
	testChannel.getAirFrames(0.5, 1.5, v);	
	assertFalse("Cut with start should intersect.", v.empty());
	assertEqual("Cut with start should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(2.5, 3.5, v);	
	assertFalse("Cut with end should intersect.", v.empty());
	assertEqual("Cut with end should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(1.5, 2.5, v);	
	assertFalse("Interval total in AirFrame duration should intersect.", v.empty());
	assertEqual("Interval total in AirFrame duration should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(0.5, 3.5, v);	
	assertFalse("AirFrame total in interval duration should intersect.", v.empty());
	assertEqual("AirFrame total in interval duration should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(3.0, 3.9, v);	
	assertFalse("Upper border should count as intersect.", v.empty());
	assertEqual("Upper border intersection should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(0.0, 1.0, v);	
	assertFalse("Lower border should count as intersect.", v.empty());
	assertEqual("Lower border intersection should return single AirFrame.", frame1, v.front());
	
	
	//add another AirFrame
	DetailedRadioFrame * frame2 = new DetailedRadioFrame();
	frame2->setDuration(1.0);
	testChannel.addAirFrame(frame2, 2.5);
	
	v.clear();
	testChannel.getAirFrames(0.5, 2.44, v);	
	assertEqual("Interval before second AirFrame should return only first.", 1u, v.size());
	assertEqual("Interval before second AirFrame should return the first AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(3.05, 4.0, v);	
	assertEqual("Interval after first AirFrame should return only second.", 1u, v.size());
	assertEqual("Interval after first AirFrame should return the second AirFrame.", frame2, v.front());
	
	v.clear();
	testChannel.getAirFrames(2.44, 2.55, v);	
	assertEqual("Interval inside both AirFrames should return both.", 2u, v.size());
	bool bothReturned =    (v.front() == frame1 && v.back() == frame2) 
						|| (v.front() == frame2 && v.back() == frame1);
	assertTrue("Interval inside both AirFrame should return both.", bothReturned);
	assertEqual("Earliest info point should be from frame1", 1.0, SIMTIME_DBL(testChannel.getEarliestInfoPoint()));
	
	//remove first one
	testChannel.removeAirFrame(frame1);
	assertEqual("Earliest info point should be always from frame1 after delete.", 1.0, SIMTIME_DBL(testChannel.getEarliestInfoPoint()));
	
	v.clear();
	testChannel.getAirFrames(2.51, 2.9, v);	
	assertEqual("Interval inside both AirFrame should return also the deleted.", 2u, v.size());
	bothReturned =    (v.front() == frame1 && v.back() == frame2) 
							|| (v.front() == frame2 && v.back() == frame1);
	assertTrue("Interval inside both AirFrame should return also the deleted.", bothReturned);
	
	//add another AirFrame which intersects with second but not with first
	
	DetailedRadioFrame * frame3 = new DetailedRadioFrame();
	frame3->setDuration(1.5);
	testChannel.addAirFrame(frame3, 3.5);
	
	v.clear();
	testChannel.getAirFrames(2.51, 3.5, v);	
	assertEqual("Interval inside all AirFrame should return all.", 3u, v.size());
	
	//remove second AirFrame
	testChannel.removeAirFrame(frame2);
	
	v.clear();
	testChannel.getAirFrames(1.51, 2.0, v);	
	assertTrue("Interval before second frame should be empty (first one is deleted).", v.empty());
	assertEqual("Earliest info point should be now from frame2 after delete.", 2.5, SIMTIME_DBL(testChannel.getEarliestInfoPoint()));
	
	v.clear();
	testChannel.getAirFrames(3.5, 3.6, v);	
	assertEqual("Interval inside second and third AirFrame should return also the second (deleted).", 2u, v.size());
	bothReturned =    (v.front() == frame2 && v.back() == frame3) 
							|| (v.front() == frame3 && v.back() == frame2);
	assertTrue("Interval inside both AirFrame should return also the deleted.", bothReturned);
	
	//remove third AirFrame
	testChannel.removeAirFrame(frame3);
	
	v.clear();
	testChannel.getAirFrames(0.0, 10.0, v);	
	assertTrue("There shouldn't be anymore AirFrames.", v.empty());
	
	
	//add two airframes with same start and end
	DetailedRadioFrame * frame4 = new DetailedRadioFrame();
	frame4->setDuration(1.0);
	DetailedRadioFrame * frame4b = new DetailedRadioFrame();
	frame4b->setDuration(1.0);
	testChannel.addAirFrame(frame4b, 15.0);	
	testChannel.addAirFrame(frame4, 15.0);
	
	v.clear();
	testChannel.getAirFrames(14.5, 15.0, v);	
	assertEqual("Check for simultaneous airframes", 2u, v.size());
	bothReturned =    (v.front() == frame4b && v.back() == frame4) 
								|| (v.front() == frame4 && v.back() == frame4b);
	assertTrue("Check for simultaneous airframes.", bothReturned);
	
	
	//remove one of them
	testChannel.removeAirFrame(frame4);
	
	v.clear();
	testChannel.getAirFrames(14.5, 15.0, v);	
	assertEqual("Check for simultaneous airframes after remove of one.", 2u, v.size());
	bothReturned =    (v.front() == frame4b && v.back() == frame4) 
								|| (v.front() == frame4 && v.back() == frame4b);
	assertTrue("Check for simultaneous airframes after remove of one.", bothReturned);
	
	//add another airframe which starts at the end of the previous ones.
	DetailedRadioFrame * frame5 = new DetailedRadioFrame();
	frame5->setDuration(2.0);	
	testChannel.addAirFrame(frame5, 16.0);
	
	v.clear();
	testChannel.getAirFrames(16.0, 17.0, v);	
	assertEqual("Aiframes with same start and end are intersecting.", 3u, v.size());
	
	//remove the second of the simultaneous AirFrames
	testChannel.removeAirFrame(frame4b);
	
	v.clear();
	testChannel.getAirFrames(16.0, 17.0, v);	
	assertEqual("Should intersect still with both removed simultaneous AirFrames.", 3u, v.size());
	
	v.clear();
	testChannel.getAirFrames(16.1, 17.0, v);	
	assertEqual("Interval after simultaneous should return only third AirFrame.", 1u, v.size());
	assertEqual("Interval after simultaneous should return only third AirFrame.", frame5, v.front());
	
	//create another AirFrame with same start as previous but later end
	DetailedRadioFrame * frame6 = new DetailedRadioFrame();
	frame6->setDuration(3.0);
	testChannel.addAirFrame(frame6, 16.0);
	
	v.clear();
	testChannel.getAirFrames(16.1, 16.1, v);	
	assertEqual("Interval at start of both AirFrames should return both.", 2u, v.size());
	bothReturned =    (v.front() == frame5 && v.back() == frame6) 
								|| (v.front() == frame6 && v.back() == frame5);
	assertTrue("Interval at start of both AirFrames should return both.", bothReturned);
	
	v.clear();
	testChannel.getAirFrames(18.1, 19.0, v);	
	assertEqual("Interval after shorter AirFrame shouldn't return the shorter.", 1u, v.size());
	assertEqual("Interval after shorter AirFrame shouldn't return the shorter.", frame6, v.front());
	
	//remove shorter AirFrame with same start
	testChannel.removeAirFrame(frame5);
	
	v.clear();
	testChannel.getAirFrames(16.1, 16.1, v);	
	assertEqual("Nothing should have changed after deletion of shorter AirFrame.", 2u, v.size());
	bothReturned =    (v.front() == frame5 && v.back() == frame6) 
								|| (v.front() == frame6 && v.back() == frame5);
	assertTrue("Nothing should have changed after deletion of shorter AirFrame.", bothReturned);
	
	v.clear();
	testChannel.getAirFrames(18.1, 19.0, v);	
	assertEqual("Nothing should have changed after deletion of shorter AirFrame.", 1u, v.size());
	assertEqual("Nothing should have changed after deletion of shorter AirFrame.", frame6, v.front());
	
	//add another one with same end as previous but later start
	DetailedRadioFrame * frame7 = new DetailedRadioFrame();
	frame7->setDuration(0.5);
	testChannel.addAirFrame(frame7, 18.5);
	
	v.clear();
	testChannel.getAirFrames(16.1, 16.1, v);	
	assertEqual("Interval before newly added should not return newly added.", 2u, v.size());
	bothReturned =    (v.front() == frame5 && v.back() == frame6) 
								|| (v.front() == frame6 && v.back() == frame5);
	assertTrue("Interval before newly added should not return newly added.", bothReturned);
	
	v.clear();
	testChannel.getAirFrames(18.1, 18.4, v);	
	assertEqual("Interval before newly added should not return newly added.", 1u, v.size());
	assertEqual("Interval before newly added should not return newly added.", frame6, v.front());
	
	v.clear();
	testChannel.getAirFrames(18.5, 18.5, v);	
	assertEqual("Newly added should be returned together with the other one.", 2u, v.size());
	bothReturned =    (v.front() == frame6 && v.back() == frame7) 
								|| (v.front() == frame7 && v.back() == frame6);
	assertTrue("Newly added should be returned together with the other one.", bothReturned);
	
	v.clear();
	testChannel.getAirFrames(14.5, 15.0, v);	
	assertEqual("Our simultaneous AirFrames should be still there", 2u, v.size());
	bothReturned =    (v.front() == frame4b && v.back() == frame4) 
								|| (v.front() == frame4 && v.back() == frame4b);
	assertTrue("Our simultaneous AirFrames should be still there.", bothReturned);
	
	//remove the only AirFrame still intersecting with the simultaneous ones and the shorter version
	testChannel.removeAirFrame(frame6);
	
	v.clear();
	testChannel.getAirFrames(14.5, 15.0, v);	
	assertEqual("Simultaneous AirFrames should be deleted now.", 0u, v.size());
	
	v.clear();
	testChannel.getAirFrames(16.0, 16.0, v);	
	assertEqual("Only longer AirFrame should be still there.", 1u, v.size());
	assertEqual("Only longer AirFrame should be still there.", frame6, v.front());
	
	v.clear();
	testChannel.getAirFrames(18.5, 18.5, v);	
	assertEqual("Last mans standing: last added and the long AirFrame.", 2u, v.size());
	bothReturned =    (v.front() == frame6 && v.back() == frame7) 
								|| (v.front() == frame7 && v.back() == frame6);
	assertTrue("Last mans standing: last added and the long AirFrame.", bothReturned);
	
	//remove last AirFrame
	testChannel.removeAirFrame(frame7);
	v.clear();
	testChannel.getAirFrames(18.5, 18.5, v);	
	assertEqual("Should be empty now..", 0u, v.size());
}


class ChannelInfoTest:public SimpleTest {
protected:
	void planTests() {
		//tests for record flag of ChannelInfo
		planTest("1.1", "Start recording on empty ChannelInfo");
		planTest("1.2", "Start recording after AirFrame starts");
		planTest("1.3", "Start recording before AirFrame starts");

		planTest("2.1", "Forwarding recording time on empty ChannelInfo");
		planTest("2.2", "Forwarding recording time before AirFrame ends.");
		planTest("2.3", "Forwarding recording time after AirFrame ends");

		planTest("3.1", "Stop recording on empty ChannelInfo");
		planTest("3.2.1", "Stop recording after AirFrame removed but not "
						  "cleared.");
		planTest("3.2.2", "Stop recording after AirFrame removed but not "
						  "cleared and interfering active AirFrame.");
		planTest("3.3", "Stop recording after AirFrame removed and cleared.");

		planTest("4.1", "Remove AirFrame during recording.");
		planTest("4.2", "Remove AirFrame before recording with interfering "
						  "active AirFrame.");

		planTest("5.", "Remove AirFrame during recording which frees a inactive "
					  "one.");

		planTest("6.1", "Result of ChannelInfo::isRecording() before "
						"recording.");
		planTest("6.2", "Result of ChannelInfo::isRecording() while "
						"recording.");
		planTest("6.3", "Result of ChannelInfo::isRecording() after "
						"recording.");

	}

	void testRecordingFlag() {
		ChannelInfo testChannel;

		//planTest("6.1", "Result of ChannelInfo::isRecording() before "
		//				  "recording.");
		testForFalse("6.1", testChannel.isRecording());

		//planTest("1.1", "Start recording on empty ChannelInfo");
		testChannel.startRecording(0.0);
		testForTrue("1.1", testChannel.isChannelEmpty());

		//planTest("6.2", "Result of ChannelInfo::isRecording() while "
		//				  "recording.");
		testForTrue("6.2", testChannel.isRecording());

		//planTest("1.3", "Start recording before AirFrame starts");
		DetailedRadioFrame * frame1 = new DetailedRadioFrame();
		frame1->setDuration(2.0);
		testChannel.addAirFrame(frame1, 1.0);
		testForFalse("1.3", testChannel.isChannelEmpty());

		//planTest("4.1", "Remove AirFrame during recording.");
		testChannel.removeAirFrame(frame1);
		testForFalse("4.1", testChannel.isChannelEmpty());

		//planTest("2.2", "Forwarding recording time before AirFrame ends.");
		testChannel.startRecording(3.0);
		testForFalse("2.2", testChannel.isChannelEmpty());

		//planTest("2.3", "Forwarding recording time after AirFrame ends");
		testChannel.startRecording(3.1);
		testForTrue("2.3", testChannel.isChannelEmpty());

		//planTest("2.1", "Forwarding recording time on empty ChannelInfo");
		testChannel.startRecording(1.0);
		testForTrue("2.1", testChannel.isChannelEmpty());

		//planTest("3.1", "Stop recording on empty ChannelInfo");
		testChannel.stopRecording();
		testForTrue("3.1", testChannel.isChannelEmpty());

		//planTest("3.3", "Stop recording after AirFrame removed and cleared.");
		testForTrue("3.3", testChannel.isChannelEmpty());

		//planTest("6.3", "Result of ChannelInfo::isRecording() after "
		//				  "recording.");
		testForFalse("6.3", testChannel.isRecording());

		//planTest("1.2", "Start recording after AirFrame starts");
		frame1 = new DetailedRadioFrame();
		frame1->setDuration(2.0);
		testChannel.addAirFrame(frame1, 1.0);
		testChannel.startRecording(3.0);
		testForFalse("1.2", testChannel.isChannelEmpty());

		//planTest("4.1", "Remove AirFrame during recording.");
		testChannel.removeAirFrame(frame1);
		testForFalse("4.1", testChannel.isChannelEmpty());

		//planTest("3.2.1", "Stop recording after AirFrame removed but not "
		//				    "cleared.");
		testChannel.stopRecording();
		testForTrue("3.2.1", testChannel.isChannelEmpty());

		//planTest("4.2", "Remove AirFrame before recording with interfering "
		//				  "active AirFrame.");
		frame1 = new DetailedRadioFrame();
		frame1->setDuration(1.0);
		testChannel.addAirFrame(frame1, 1.0);
		DetailedRadioFrame * frame2 = new DetailedRadioFrame();
		frame2->setDuration(1.0);
		testChannel.addAirFrame(frame2, 2.0);
		testChannel.removeAirFrame(frame1);
		testChannel.startRecording(2.1);
		testForEqual("4.2", 2, numAirFramesOnChannel(testChannel));

		//planTest("3.2.2", "Stop recording after AirFrame removed but not "
		//				    "cleared and interfering active AirFrame.");
		testChannel.stopRecording();
		testForEqual("3.2.2", 2, numAirFramesOnChannel(testChannel));

		//planTest("5.", "Remove AirFrame during recording which frees a inactive "
		//			    "one.");
		testChannel.startRecording(2.1);
		testChannel.removeAirFrame(frame2);
		testForEqual("5.", 1, numAirFramesOnChannel(testChannel));

		//planTest("2.3", "Forwarding recording time after AirFrame ends");
		testChannel.startRecording(4);
		testForTrue("2.3", testChannel.isChannelEmpty());

		//planTest("3.3", "Stop recording after AirFrame removed and cleared.");
		testChannel.stopRecording();
		testForTrue("3.3", testChannel.isChannelEmpty());

	}

	int numAirFramesOnChannel(ChannelInfo& ch,
							  simtime_t_cref from = SIMTIME_ZERO, simtime_t_cref to = 999999.0)
	{
		ChannelInfo::AirFrameVector v;
		ch.getAirFrames(from, to, v);
		return v.size();
	}

	void runTests() {
		testIntersections();

		testRecordingFlag();
		testsExecuted = true;
	}
	virtual ~ChannelInfoTest() {}
};

Define_Module(ChannelInfoTest);
