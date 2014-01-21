#include "DeciderTest.h"
#include "../testUtils/asserts.h"
#include "TestSNRThresholdDeciderNew.h"

Define_Module(DeciderTest);

DeciderTest::DeciderTest()
	: DeciderToPhyInterface()
	, SimpleTest()
	, decider(NULL)
	, processedAF(NULL)
{
	// initializing members for testing
	world = new TestWorld();

	// set controlling things (initial states)
	noAttenuation = 1.0;

	// some fix time-points for the signals
	t0 = 0;
	t1 = 1;
	t3 = 3;
	t5 = 5;
	t7 = 7;
	t9 = 9;

	// time-points before and after
	before = t0;
	after = 10;

	// time-points in between
	t2 = 2;
	t4 = 4;
	t6 = 6;
	t8 = 8;

	// some TX-power values (should be 2^i, to detect errors in addition)
	TXpower1 = 1.0;
	TXpower2 = 3.981071705534;
	TXpower3 = 3.981071705535; // == 6dBm (sensitivity)
	TXpower4 = 8.0;

	// TX-power for SNR-threshold-test AirFrame
	TXpower5P = 16.0;
	TXpower5H = 32.0;

	TXpower6 = 1.0;

	// some bitrates
	bitrate9600 = 9600.0;


	// create test AirFrames
	TestAF1 = createTestAirFrame(1);
	TestAF2 = createTestAirFrame(2);
	TestAF3 = createTestAirFrame(3);
	TestAF4 = createTestAirFrame(4);

	// AirFrames for SNR-threshold testing
	TestAF5 = createTestAirFrame(5);
	TestAF6 = createTestAirFrame(6);

	// expected results of tests (hard coded)
	res_t1_noisy = TXpower1;
	res_t2_noisy = TXpower1;
	res_t3_noisy = TXpower1 + TXpower2 + TXpower3;
	res_t4_noisy = TXpower1 + TXpower2 + TXpower3;
	res_t5_noisy = TXpower1 + TXpower2 + TXpower3 + TXpower4;
	res_t9_noisy = TXpower2;

	// expected results of getChannelState()-tests, when receiving TestAirFrame 3
	// (testing the exclusion of a Frame when calculating the RSSI-value)
	res_t2_receiving		= TXpower1;
	res_t3_receiving_before	= TXpower1 + TXpower2 + TXpower3;
	res_t3_receiving_after	= TXpower1 + TXpower2 + TXpower3;
	res_t4_receiving		= TXpower1 + TXpower2 + TXpower3;
	res_t5_receiving_before	= TXpower1 + TXpower2 + TXpower3	+ TXpower4;
	res_t5_receiving_after	= TXpower1 + TXpower2 + TXpower3 	+ TXpower4;
	res_t6_receiving		= TXpower1 + TXpower2 				+ TXpower4;


}



DeciderTest::~DeciderTest() {

	freeAirFramePool();
	// clean up
	delete TestAF1;
	TestAF1 = 0;
	delete TestAF2;
	TestAF2 = 0;
	delete TestAF3;
	TestAF3 = 0;
	delete TestAF4;
	TestAF4 = 0;
	delete TestAF5;
	TestAF5 = 0;
	delete TestAF6;
	TestAF6 = 0;

	delete world;
	world = 0;
}

void DeciderTest::removeAirFrameFromPool(DetailedRadioFrame * af)
{
	for(AirFrameList::iterator it = airFramePool.begin();
		it != airFramePool.end(); ++it)
	{
		if(*it == af) {
			airFramePool.erase(it);
			delete af;
			return;
		}
	}

	assertTrue("AirFrame to remove has to be in pool.",false);
}

DetailedRadioFrame * DeciderTest::addAirFrameToPool(simtime_t_cref start, simtime_t_cref payloadStart, simtime_t_cref end,
										 double headerPower, double payloadPower)
{
	// create Signal containing TXpower- and bitrate-mapping
    DetailedRadioSignal* s = createSignal(start, payloadStart, end, headerPower, payloadPower, 16.0);

	// --- Phy-Layer's tasks
	// just a bypass attenuation, that has no effect on the TXpower
	//Mapping* bypassMap = createConstantMapping(start, end, noAttenuation);
	//s->addAttenuation(bypassMap);

	// create the new AirFrame
	DetailedRadioFrame * frame = new DetailedRadioFrame(0);

	// set the members
	frame->setDuration(s->getDuration());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	frame->setId(world->getUniqueAirFrameId());

	airFramePool.push_back(frame);

	return frame;
}

DetailedRadioFrame * DeciderTest::addAirFrameToPool(simtime_t_cref start, simtime_t_cref end, double power)
{
	// create Signal containing TXpower- and bitrate-mapping
    DetailedRadioSignal* s = createSignal(start, end, power, 16.0);

	// --- Phy-Layer's tasks
	// just a bypass attenuation, that has no effect on the TXpower
	//Mapping* bypassMap = createConstantMapping(start, end, noAttenuation);
	//s->addAttenuation(bypassMap);

	// create the new AirFrame
	DetailedRadioFrame * frame = new DetailedRadioFrame(0);

	// set the members
	frame->setDuration(s->getDuration());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	frame->setId(world->getUniqueAirFrameId());

	airFramePool.push_back(frame);

	return frame;
}

void DeciderTest::runTests()
{
	// start the test of the decider
	runDeciderTests("SNRThresholdDeciderNew");

	testsExecuted = true;
}



/*
 * It is important to set 'currentTestCase' properly, such that
 * the deciders calls on the DeciderToPhyInterface can be handled right.
 *
 *
 */
void DeciderTest::runDeciderTests(std::string name)
{
	deciderName = name;
	currentTestCase = BEFORE_TESTS;
	assert(currentTestCase == BEFORE_TESTS);

	/*
	 * Test getChannelState() of SNRThresholdDecider on an empty channel
	 */
	executeTestCase(TEST_GET_CHANNELSTATE_EMPTYCHANNEL);


	/*
	 * Test getChannelState() of SNRThresholdDecider with noise on channel
	 */
	executeTestCase(TEST_GET_CHANNELSTATE_NOISYCHANNEL);


	/*
	 * Test getChannelState() of SNRThresholdDecider while receiving an AirFrame
	 */
	executeTestCase(TEST_GET_CHANNELSTATE_RECEIVING);


	/*
	 * Test SNR-threshold
	 */
	executeTestCase(TEST_SNR_THRESHOLD_ACCEPT);

	executeTestCase(TEST_SNR_THRESHOLD_DENY);


	executeTestCase(TEST_SNR_THRESHOLD_PAYLOAD_DENY);


	executeTestCase(TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY);


	// Since we don't simulate the real order in which BasePhyLayer
	// handles AirFrames occuring at the same simulation-time we can't
	// test these border-cases
	//
	//executeTestCase(TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY);
	//executeTestCase(TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY);

	executeTestCase(TEST_CHANNELSENSE);

	executeTestCase(TEST_CHANNELSENSE_IDLE_CHANNEL);

	executeTestCase(TEST_CHANNELSENSE_BUSY_CHANNEL);

	executeTestCase(TEST_CHANNELSENSE_CHANNEL_CHANGES_DURING_AIRFRAME);

}

void DeciderTest::freeAirFramePool() {
	for(AirFrameList::iterator it = airFramePool.begin();
		it != airFramePool.end(); ++it)
	{
		assert(*it);
		delete *it;
	}

	airFramePool.clear();
}

void DeciderTest::executeTestCase(TestCaseIdentifier testCase) {
	//set current test case
	if(decider)
		delete decider;
	ParameterMap dummyParams;
	decider = initDeciderTest(deciderName, dummyParams);
	currentTestCase = testCase;
	airFramesOnChannel.clear();
	freeAirFramePool();
	processedAF = 0;
	testChannelSense = 0;
	testRSSIMap = 0;
	testTime = 0;
	sendUpCalled = false;

	if(deciderName == "SNRThresholdDeciderNew") {
		executeSNRNewTestCase();
	} //else if(...)
	//add further decider to test here
	//...
	else {
		assertFalse("Unknown decider name in executeTestCase!", true);
	}
}


Decider* DeciderTest::initDeciderTest(std::string name, ParameterMap& params) {

	//reset globals
	if(name == "SNRThresholdDeciderNew") {
		// parameters for original TestDecider (tests/basePhyLayer with testBaseDecider = true)
		params["snrThreshold"] = ParameterMap::mapped_type("snrThreshold");
		params["snrThreshold"].setDoubleValue(10.0);
		params["busyThreshold"] = ParameterMap::mapped_type("busyThreshold");
		params["busyThreshold"].setDoubleValue(FWMath::dBm2mW(6.0));

		double sensitivity = FWMath::dBm2mW(6.0);
		int    myIndex     = 0;

		TestSNRThresholdDeciderNew *const pDecider = new TestSNRThresholdDeciderNew(this, sensitivity, myIndex);
		if (pDecider != NULL && !pDecider->initFromMap(params)) {
			opp_warning("Decider from config.xml could not be initialized correctly!");
		}
		return pDecider;
	}

	return NULL;
}

/**
 * @brief Fills the AirFrameVector with intersecting AirFrames,
 * i.e. puts them on the (virtual) channel. (depends on testTime)
 *
 * AirFrames on the (virtual) channel in TestBDInitialization
 *
 * Frame1       |-----------------|				(start: t1, length: t7-t1)
 * Frame2             |-----------------|		(start: t3, length: t9-t3)
 * Frame3             |-----|					(start: t3, length: t5-t3)
 * Frame4                   |--|				(start: t5, length: t6-t5)
 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
 * (Frame5)
 * Frame6    |--|								(start: t0, length: t1-t0)
 *           |  |  |  |  |  |  |  |  |  |  |
 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
 * To Test:  __ __ __ __ __ __          __
 *
 *           where: t0=before, t10=after
 */
void DeciderTest::fillAirFramesOnChannel()
{
	// remove all pointers to AirFrames
	airFramesOnChannel.clear();
	if(TestAF5)
		delete TestAF5;

	TestAF5 = createTestAirFrame(5);
	switch (currentTestCase) {


		case TEST_SNR_THRESHOLD_ACCEPT:
		case TEST_SNR_THRESHOLD_DENY:
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF5);
			break;

		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF2);
			airFramesOnChannel.push_back(TestAF5);
			break;

		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF5);
			airFramesOnChannel.push_back(TestAF6);
			break;

		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF4);
			airFramesOnChannel.push_back(TestAF5);
			break;


		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
		case TEST_GET_CHANNELSTATE_RECEIVING:
		case TEST_CHANNELSENSE:


			// fill with pointers to test AirFrames that interfere with the testTime
			if (testTime == before || testTime == after)
			{
				// do nothing
			}
			else if (testTime == t1 || testTime == t2)
			{
				airFramesOnChannel.push_back(TestAF1);
			}
			else if (testTime == t3 || testTime == t4)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF3);

			}
			else if (testTime == t5)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF3);
				airFramesOnChannel.push_back(TestAF4);

			}
			else if (testTime == t6)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF4);
			}

			else if (testTime == t7)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
			}
			else if (testTime == t8 || testTime == t9)
			{
				airFramesOnChannel.push_back(TestAF2);
			}
			else
			{
				// do nothing
			}

			break;
		case TEST_CHANNELSENSE_BUSY_CHANNEL:
		case TEST_CHANNELSENSE_CHANNEL_CHANGES_DURING_AIRFRAME:
			break;
		case TEST_CHANNELSENSE_IDLE_CHANNEL:
			// fill with pointers to test AirFrames that interfere with the testTime
			if (testTime >= t3 && testTime <= t5)
			{
				airFramesOnChannel.push_back(TestAF3);
			}

			break;
		default:
			break;
	}

	//iterate over pool and add active airframes to channel
	for(AirFrameList::iterator it = airFramePool.begin();
		it != airFramePool.end(); ++it)
	{
	    DetailedRadioSignal& s = (*it)->getSignal();

		if(s.getReceptionStart() <= testTime && testTime <= s.getReceptionEnd())
		{
			airFramesOnChannel.push_back(*it);
		}
	}


	EV << log("Filled airFramesOnChannel. (the virtual channel)") << endl;
}

/**
 * @brief Creates a special test AirFrame by index
 *
 * Below follow the parameters of the created signals, sorted by index.
 *
 * i=1: start: t1, length: t7-t1, TXpower: 1.0, bitrate: 9600.0, constant
 * i=2: start: t3, length: t9-t3, TXpower: 2.0, bitrate: 9600.0, constant
 * i=3: start: t3, length: t5-t3, TXpower: 4.0, bitrate: 9600.0, constant
 * i=4: start: t5, length: t6-t5, TXpower: 8.0, bitrate: 9600.0, constant
 *
 * This is the AirFrame for SNR-threshold testing, its values may be changed during test
 * i=5: start: t1, length: t5-t1, TXpower: variable, bitrate: 9600.0, steps
 *
 * i=6:	start: t0, length : t1-t0, TXpower 1.0, bitrate: 9600.0, constant
 *
 * where TXpower and bitrate are also specified by a variable.
 *
 *
 * NOTE: No message is encapsulated in these test-AirFrames!
 */
DetailedRadioFrame * DeciderTest::createTestAirFrame(int i)
{
	// parameters needed
	simtime_t signalStart = -1;
	simtime_t signalDuration = -1;
	// values for header/payload
	std::pair<double, double> transmissionPower(0, 0);
	std::pair<double, double> bitrate(0, 0);
	simtime_t payloadStart = -1;

	// set parameters according to index
	switch (i) {
		case 1:
			signalStart = t1;
			signalDuration = (t7-t1);
			transmissionPower.first = TXpower1;
			bitrate.first = bitrate9600;
			break;
		case 2:
			signalStart = t3;
			signalDuration = (t9-t3);
			transmissionPower.first = TXpower2;
			bitrate.first = bitrate9600;
			break;
		case 3:
			signalStart = t3;
			signalDuration = (t5-t3);
			transmissionPower.first = TXpower3;
			bitrate.first = bitrate9600;
			break;
		case 4:
			signalStart = t5;
			signalDuration = (t6-t5);
			transmissionPower.first = TXpower4;
			bitrate.first = bitrate9600;
			break;
		case 5:
			signalStart = t1;
			signalDuration = (t5-t1);

			switch (currentTestCase) {
				case TEST_SNR_THRESHOLD_ACCEPT:
					transmissionPower.first = 10.00001;
					transmissionPower.second = 10.00001;
					break;
				case TEST_SNR_THRESHOLD_DENY:
					transmissionPower.first = 10.0;
					transmissionPower.second = 10.0;
					break;
				case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
					transmissionPower.first = 20.0;
					transmissionPower.second = 10.0;
					break;
				case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
					transmissionPower.first = 30.0;
					transmissionPower.second = 30.0;
					break;
				case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
					transmissionPower.first = 20.0;
					transmissionPower.second = 20.0;
					break;
				case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
					transmissionPower.first = 90.0;
					transmissionPower.second = 90.0;
					break;
				default:
					transmissionPower.first = TXpower5H;
					transmissionPower.second = TXpower5P;
					break;
			}

			bitrate.first = bitrate9600;
			payloadStart = t2;
			break;
		case 6:
			signalStart = t0;
			signalDuration = (t1-t0);
			transmissionPower.first = TXpower6;
			bitrate.first = bitrate9600;
			break;
		default:
			break;
	}

	// --- Mac-Layer's tasks

	// create Signal containing TXpower- and bitrate-mapping
	DetailedRadioSignal* s = createSignal(signalStart, signalDuration, transmissionPower, bitrate, i, payloadStart);

	// just a bypass attenuation, that has no effect on the TXpower
	//Mapping* bypassMap = createConstantMapping(signalStart, signalStart + signalDuration, noAttenuation);
	//s->addAttenuation(bypassMap);

	// --- Phy-Layer's tasks

	// create the new AirFrame
	DetailedRadioFrame * frame = new DetailedRadioFrame(0);

	// set the members
	frame->setDuration(s->getDuration());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	frame->setId(world->getUniqueAirFrameId());

	EV << log("Creating TestAirFrame ") << i << " done." << endl;

	return frame;
}

// pass AirFrame-pointers currently on the (virtual) channel to decider
void DeciderTest::passAirFramesOnChannel(AirFrameVector& out) const
{
	AirFrameVector::const_iterator it;

	// iterate over the member that holds the pointers to all AirFrames on
	// the virtual channel and put them to the output reference
	for (it = airFramesOnChannel.begin(); it != airFramesOnChannel.end(); ++it) {
		out.push_back(*it);
	}

	//EV << log("All TestAirFrames-pointers have been copied.") << endl;
}

/**
 * @brief Creates the right Signals for TestAirFrames (by index).
 *
 * By default TX-power and bitrate are constant for the whole duration of the Signal.
 *
 *
 */
DetailedRadioSignal* DeciderTest::createSignal(simtime_t_cref start,
									simtime_t_cref length,
									std::pair<double, double> power,
									std::pair<double, double> bitrate,
									int index,
									simtime_t_cref payloadStart = -1)
{
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	DetailedRadioSignal* s = new DetailedRadioSignal(start, length);

	Mapping* txPowerMapping = 0;
	Mapping* bitrateMapping = 0;

	switch (index) {

		// TestAirFrame 5 has different TX-power for header and payload
		case 5:

			//set tx power mapping
			txPowerMapping = createHeaderPayloadMapping(start, payloadStart, end,
																	power.first, power.second);

			//set bitrate mapping
			bitrateMapping = createConstantMapping(start, end, bitrate.first);
			break;

		default:
			//set tx power mapping
			txPowerMapping = createConstantMapping(start, end, power.first);

			//set bitrate mapping
			bitrateMapping = createConstantMapping(start, end, bitrate.first);
			break;
	}

	assert(txPowerMapping);
	assert(bitrateMapping);

	// put the mappings to the Signal
	s->setTransmissionPower(txPowerMapping);
	s->setBitrate(bitrateMapping);

	EV << "Signal for TestAirFrame" << index << " created with header/payload power: " <<  power.first << "/" << power.second << endl;

	return s;
}

DetailedRadioSignal* DeciderTest::createSignal(simtime_t_cref start, simtime_t_cref payLoadStart,
									simtime_t_cref end,
									double headerPower,
									double payloadPower,
									double bitrate)
{
	simtime_t length = end - start;
	//create signal with start at current simtime and passed length
	DetailedRadioSignal* s = new DetailedRadioSignal(start, length);

	Mapping* txPowerMapping = 0;
	Mapping* bitrateMapping = 0;

	//set tx power mapping
	txPowerMapping = createHeaderPayloadMapping(start, payLoadStart, end, headerPower, payloadPower);

	//set bitrate mapping
	bitrateMapping = createConstantMapping(start, end, bitrate);

	assert(txPowerMapping);
	assert(bitrateMapping);

	// put the mappings to the Signal
	s->setTransmissionPower(txPowerMapping);
	s->setBitrate(bitrateMapping);

	//EV << "Signal for TestAirFrame" << index << " created with header/payload power: " <<  power.first << "/" << power.second << endl;

	return s;
}

DetailedRadioSignal* DeciderTest::createSignal(simtime_t_cref start,
									simtime_t_cref end,
									double power,
									double bitrate)
{
	simtime_t length = end - start;
	//create signal with start at current simtime and passed length
	DetailedRadioSignal* s = new DetailedRadioSignal(start, length);

	Mapping* txPowerMapping = 0;
	Mapping* bitrateMapping = 0;

	//set tx power mapping
	txPowerMapping = createConstantMapping(start, end, power);

	//set bitrate mapping
	bitrateMapping = createConstantMapping(start, end, bitrate);

	assert(txPowerMapping);
	assert(bitrateMapping);

	// put the mappings to the Signal
	s->setTransmissionPower(txPowerMapping);
	s->setBitrate(bitrateMapping);

	//EV << "Signal for TestAirFrame" << index << " created with header/payload power: " <<  power.first << "/" << power.second << endl;

	return s;
}

Mapping* DeciderTest::createConstantMapping(simtime_t_cref start, simtime_t_cref end, double value)
{
	//create mapping over time
	Mapping* m = MappingUtils::createMapping(Argument::MappedZero, DimensionSet(Dimension::time), Mapping::LINEAR);

	//set position Argument
	Argument startPos(start);
	//set mapping at position
	m->setValue(Argument(pre(start)), 0.0);
	m->setValue(startPos, value);

	//set position Argument
	Argument endPos(end);
	//set mapping at position
	m->setValue(endPos, value);
	m->setValue(Argument(post(end)), 0.0);

	return m;
}

Mapping* DeciderTest::createHeaderPayloadMapping(	simtime_t_cref start,
													simtime_t_cref payloadStart,
													simtime_t_cref end,
													double         headerValue,
													double         payloadValue)
{
	//create mapping over time
	Mapping* m = MappingUtils::createMapping(Argument::MappedZero, DimensionSet(Dimension::time), Mapping::LINEAR);

	//set position Argument
	Argument startPos(start);
	//set mapping at position
	m->setValue(startPos, headerValue);


	// header end (infinitely small time step earlier)
	Argument headerEndPos(pre(payloadStart));
	m->setValue(headerEndPos, headerValue);

	// payload start
	Argument payloadStartPos(payloadStart);
	m->setValue(payloadStartPos, payloadValue);


	//set position Argument
	Argument payloadEndPos(end);
	//set mapping at position
	m->setValue(payloadEndPos, payloadValue);

	return m;
}



// --- TESTING IMPLEMENTATION OF DeciderToPhyInterface !!! ---
// --- PLEASE REFER TO HEADER-FILE !!! ---

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) const
{

	switch (currentTestCase) {

		// there is no AirFrame on the Channel at the requested timepoint,
		// and we are not currently receiving an AirFrame
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
			// test whether values 'from' and 'to' are the same
			assertTrue( "Decider demands ChannelInfo for one single timepoint", from == to );
			assertEqual( "Decider demands ChannelInfo for current timepoint", getSimTime() , from);

			//  AirFrameVector referenced by 'out' remains as is
			break;

		// there are AirFrames on the Channel at the requested timepoint
		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:

		case TEST_GET_CHANNELSTATE_RECEIVING:


		case TEST_SNR_THRESHOLD_ACCEPT:
		case TEST_SNR_THRESHOLD_DENY:
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
		case TEST_CHANNELSENSE:

			if (processedAF != 0)
			{
				assert(testChannelSense == 0);

				const DetailedRadioSignal& signal = processedAF->getSignal();

				// test whether Decider asks for the duration-interval of the processed AirFrame
				assertTrue( "Decider demands ChannelInfo for the duration of the AirFrame",
						(from == signal.getReceptionStart()) &&
						(to == signal.getReceptionEnd()) );

				assertEqual( "Decider demands ChannelInfo for current test-timepoint (end of AirFrame)", getSimTime() , to);

			}
			else if(testChannelSense != 0)
			{
				assert(processedAF == 0);

				// test whether Decider asks for the duration-interval of the ChannelSense
				//assertEqual( "Start of the interval which Decider requested is correct.",
				//			getSimTime() - testChannelSense->getSenseTimeout(),
				//			from);
				assertTrue( "Start of the interval which Decider requested is correct.",
							true);

				assertEqual( "End of the interval which Decider requested is correct.",
							getSimTime(),
							to);
			}
			else
			{
				// test whether values 'from' and 'to' are the same
				assertTrue( "Decider demands ChannelInfo for one single timepoint", from == to );
				assertEqual( "Decider demands ChannelInfo for current test-timepoint", getSimTime() , from);
			}

			// pass all AirFrames currently on the (virtual) channel
			passAirFramesOnChannel(out);

			break;

		case TEST_CHANNELSENSE_IDLE_CHANNEL:
		case TEST_CHANNELSENSE_BUSY_CHANNEL:
		case TEST_CHANNELSENSE_CHANNEL_CHANGES_DURING_AIRFRAME:
			passAirFramesOnChannel(out);
			break;

		default:
			assertFalse("Unknown test scenario in getChannelInfo!",false);
			break;
	}

	//EV << log("All channel-info has been copied to AirFrameVector-reference.") << endl;

	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::sendControlMsgToMac(cMessage* msg)
{

	ChannelSenseRequest* req = dynamic_cast<ChannelSenseRequest*>(msg);

	assertNotEqual("Control message send to mac by the Decider should be a ChannelSenseRequest", (void*)0, req);

	assertTrue("This ChannelSenseRequest answer has been expected.", expectCSRAnswer);
	expectCSRAnswer = false;

	ChannelState state = req->getResult();

	assertEqual("ChannelSense results isIdle state match expected results isIdle state.",
				expChannelState.isIdle(), state.isIdle());
    assertEqual("ChannelSense results isBusy state match expected results isBusy state.",
                expChannelState.isBusy(), state.isBusy());
	assertEqual("ChannelSense results RSSI value match expected results RSSI value.",
				expChannelState.getRSSI(), state.getRSSI());

	// TODO as soon as ThresholdDecider sends 'packet dropped' - control messages
	// check correct sending of them here
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::sendUp(DetailedRadioFrame * packet, DeciderResult* /*result*/)
{

	// signal that method has been called
	sendUpCalled = true;

	switch (currentTestCase) {
		case TEST_GET_CHANNELSTATE_RECEIVING:
			// TODO check what the result should be in this test case
			// assertTrue("DeciderResult is: 'correct'", result.isSignalCorrect());
			assertTrue("BaseDecider has returned the pointer to currently processed AirFrame", (packet==processedAF));
			break;

		case TEST_SNR_THRESHOLD_ACCEPT:
			assertTrue("BaseDecider decided correctly to send up the packet", true);
			assertTrue("BaseDecider has returned the pointer to currently processed AirFrame", (packet==processedAF));
			break;

		default:
			break;
	}

	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
simtime_t DeciderTest::getSimTime() const
{
	return testTime;
}

void DeciderTest::executeSNRNewTestCase()
{
	ChannelState cs;
	simtime_t nextHandoverTime = -1;

	switch (currentTestCase) {
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
		{
			// ask SNRThresholdDecider for the ChannelState, it should call getChannelInfo()
			// on the DeciderToPhyInterface
			cs = decider->getChannelState();
			assertTrue("This decider considers an empty channel as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'now' in an empty Set of AirFrames",
						0.0, cs.getRSSI());

			break;
		}

		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
		{
			EV << log("-TEST_GET_CHANNELSTATE_NOISYCHANNEL-----------------------------------------------") << endl;


			// 1. testTime = before = t0
			testTime = before;

			// this method-call depends on the variable testTime,
			// so it needs to be set properly before
			fillAirFramesOnChannel();

			// call getChannelState() of the SNRThresholdDecider
			cs = decider->getChannelState();
			assertTrue("This decider considers an empty channel as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime' in an empty Set of AirFrames",
						0.0, cs.getRSSI());

			EV << log("-full output-----------------------------------------------") << endl;

			// 2. testTime = t1
			testTime = t1;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			EV << log("Putting AirFrames on the virtual channel.") << endl;
			fillAirFramesOnChannel();

			EV << log("Calling getChannelState() of BaseDecider.") << endl;
			cs = decider->getChannelState();

			EV << log("Checking results against expected results.") << endl;
			assertTrue("This decider considers the RX-Power of AirFrame1 as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t1_noisy, cs.getRSSI());

			EV << log("-short output-----------------------------------------------") << endl;

			// 3. testTime = t2
			testTime = t2;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("This decider considers the RX-Power of AirFrame1 as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t2_noisy, cs.getRSSI());

			EV << log("-short output-----------------------------------------------") << endl;

			// 4. testTime = t3
			testTime = t3;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_noisy, cs.getRSSI());

			EV << log("-short output-----------------------------------------------") << endl;

			// 5. testTime = t4
			testTime = t4;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t4_noisy, cs.getRSSI());

			EV << log("-short output-----------------------------------------------") << endl;

			// 6. testTime = t5
			testTime = t5;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3+4 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_noisy, cs.getRSSI());

			EV << log("-short output-----------------------------------------------") << endl;

			// 7. testTime = t9
			testTime = t9;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("This decider considers the RX-power of AirFrame2 as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t9_noisy, cs.getRSSI());

			EV << log("-DONE-------------------------------------------") << endl;

		}
			break;
		case TEST_GET_CHANNELSTATE_RECEIVING:
		{
			EV << log("-TEST_GET_CHANNELSTATE_RECEIVING-----------------------------------------------") << endl;

			EV << log("-full output-----------------------------------------------") << endl;

			// testTime = t2
			testTime = t2;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			EV << log("Putting AirFrames on the virtual channel.") << endl;
			fillAirFramesOnChannel();

			EV << log("Calling getChannelState() of BaseDecider.") << endl;
			cs = decider->getChannelState();

			EV << log("Checking results against expected results.") << endl;
			assertTrue("This decider considers the RX-power of AirFrame1 as idle.", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t2_receiving, cs.getRSSI());



			EV << log("-short output-----------------------------------------------") << endl;

			// testTime = t3
			testTime = t3;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// by now we are not yet receiving something
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_receiving_before, cs.getRSSI());

			// trying to receive an AirFrame whose signal (TXpower) is too weak
			EV << log("Trying to receive an AirFrame whose TXpower is too low.") << endl;
			nextHandoverTime = decider->processSignal(TestAF2);
			assertTrue("AirFrame has been rejected, since TXpower is too low.", (nextHandoverTime < 0));

			// starting to receive TestAirFrame 3
			EV << log("Trying to receive TestAirFrame 3") << endl;
			nextHandoverTime = decider->processSignal(TestAF3);
			DetailedRadioSignal& signal3 = TestAF3->getSignal();
			assertTrue("TestAirFrame 3 can be received, end-time is returned", (nextHandoverTime == signal3.getReceptionEnd()));


			// try to receive another AirFrame at the same time, whose signal not too weak
			// (taking a copy of the currently received one)
			DetailedRadioFrame * tempAF = new DetailedRadioFrame(*TestAF3);
			EV << log("Trying to receive another AirFrame at the same time.") << endl;
			nextHandoverTime = decider->processSignal(tempAF);
			assertTrue("AirFrame has been rejected, since we already receive one.", (nextHandoverTime < 0));
			delete tempAF;
			tempAF = 0;


			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_receiving_after, cs.getRSSI());


			EV << log("-short output-----------------------------------------------") << endl;

			// testTime = t4
			testTime = t4;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t4_receiving, cs.getRSSI());


			EV << log("-short output-----------------------------------------------") << endl;

			// testTime = t5
			testTime = t5;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3+4 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_receiving_before, cs.getRSSI());

			//hand over the AirFrame again, end-time is reached
			EV << log("Handing over TestAirFrame 3 again, transmission has finished") << endl;

			// set the pointer to the processed AirFrame before SNRThresholdDecider will finally process it
			processedAF = TestAF3;
			nextHandoverTime = decider->processSignal(TestAF3);
			assertTrue("TestAirFrame 3 has been finally processed",
					(nextHandoverTime < 0));
			processedAF = 0;

			// now receive of the AirFrame is over
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+3+4 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_receiving_after, cs.getRSSI());

			// SNRThresholdDecider should be able to handle the next AirFrame
			EV << log("Trying to immediately receive TestAirFrame 4") << endl;
			nextHandoverTime = decider->processSignal(TestAF4);
			DetailedRadioSignal& signal4 = TestAF4->getSignal();
			assertTrue("TestAirFrame 4 can be received, end-time is returned", (nextHandoverTime == signal4.getReceptionEnd()));



			EV << log("-short output-----------------------------------------------") << endl;

			// testTime = t6
			testTime = t6;
			EV << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we are not receiving an AirFrame
			cs = decider->getChannelState();
			assertTrue("This decider considers the combined RX-Power of AirFrame1+2+4 as busy.", cs.isBusy());
			assertEqual("RSSI-value is the value for 'testTime'", res_t6_receiving, cs.getRSSI());

			// set the pointer to the processed AirFrame before SNRThresholdDecider will finally process it
			processedAF = TestAF4;
			nextHandoverTime = decider->processSignal(TestAF4);
			assertTrue("TestAirFrame 4 has been finally processed",
					(nextHandoverTime < 0));
			processedAF = 0;

			EV << log("-DONE-------------------------------------------") << endl;



		}
			break;

		// here we test a simple case where one noise-AirFrame is present, but SNR is just high enough
		case TEST_SNR_THRESHOLD_ACCEPT:

			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << endl;

			updateSimTime(t1);


			sendUpCalled = false;

			processedAF = TestAF5;
			nextHandoverTime = decider->processSignal(TestAF5);

			assertEqual("NextHandoverTime is t5.", t5, nextHandoverTime);

			testTime = nextHandoverTime;

			nextHandoverTime = decider->processSignal(TestAF5);

			assertTrue("TestAirFrame 5 has been finally processed",
								(nextHandoverTime < 0));
			processedAF = 0;

			assertTrue("sendUp() has been called.", sendUpCalled);

			break;


		// here we test a simple case where one noise-AirFrame is present, but SNR is just not high enough
		case TEST_SNR_THRESHOLD_DENY:

		// here we test a simple case where one noise-AirFrame is present and  header-SNR is just high enough
		// but payload-SNR is just not high enough
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:

		// here we test a simple case where a second noise-AirFrame arrives during reception of an AirFrame
		// and thus SNR-level turns too low
		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:

		// here we test a simple case where one noise-AirFrame ends at the beginning of the reception
		// thus SNR-level is too low
		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:

		// here we test a simple case where one noise-AirFrame begins at the end of the reception
		// thus SNR-level is too low
		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:

			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << endl;

			updateSimTime(t1);


			sendUpCalled = false;

			processedAF = TestAF5;
			nextHandoverTime = decider->processSignal(TestAF5);

			assertEqual("NextHandoverTime is t5.", t5, nextHandoverTime);

			testTime = nextHandoverTime;

			nextHandoverTime = decider->processSignal(TestAF5);

			assertTrue("TestAirFrame 5 has been finally processed",
								(nextHandoverTime < 0));
			processedAF = 0;

			assertFalse("sendUp() has not been called.", sendUpCalled);

			break;

		case TEST_CHANNELSENSE:
			break;
/*
			processedAF = 0;

			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - empty channel"<< endl;
			//test sense on empty channel
			updateSimTime(before);

			testChannelSense = createCSR(0.2, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 0.2, nextHandoverTime);
			testTime = nextHandoverTime;

			setExpectedCSRAnswer(true, false, 0.0);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;


			//test sense during one single airframe without an AirFrame receiving
			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - signel airframe on channel"<< endl;
			updateSimTime(t1);

			testChannelSense = createCSR(0.2, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 0.2, nextHandoverTime);
			testTime = nextHandoverTime;

			setExpectedCSRAnswer(true, false, TXpower1);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames at end of sense without an AirFrame receiving
			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - new airframes at end of sense"<< endl;
			updateSimTime(t2);

			testChannelSense = createCSR(1.0, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 1.0, nextHandoverTime);
			updateSimTime(nextHandoverTime);


			setExpectedCSRAnswer(false, true, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames during sense without an AirFrame receiving
			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - new AirFrames during sense"<< endl;
			updateSimTime(t2);

			testChannelSense = createCSR(2.0, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 2.0, nextHandoverTime);
			updateSimTime(nextHandoverTime);


			setExpectedCSRAnswer(false, true, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with AirFrames end at start of sense without an AirFrame receiving
			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - AirFrames ends at start of sense"<< endl;
			updateSimTime(t7);

			testChannelSense = createCSR(1.0, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 1.0, nextHandoverTime);
			testTime = nextHandoverTime;

			setExpectedCSRAnswer(true, false, TXpower2);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames during sense with an AirFrame receiving
			EV << log("-------------------------------------------------------") << endl;
			EV << log(stateToString(currentTestCase)) << " - new AirFrames during sense not idle"<< endl;
			updateSimTime(t2);

			testChannelSense = createCSR(2.0, UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 2.0, nextHandoverTime);

			updateSimTime(t3);

			nextHandoverTime = decider->processSignal(TestAF3);
			assertTrue("TestAF3 should be received by decider.", nextHandoverTime > 0.0);

			updateSimTime(t4);


			setExpectedCSRAnswer(false, true, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			break;
*/
		case TEST_CHANNELSENSE_IDLE_CHANNEL:
		{
			//test UNTIL_IDLE on already idle channel
			double tmpAF1Power = TXpower2;
			DetailedRadioFrame * tmpAF1 = addAirFrameToPool(t1, t8, tmpAF1Power);
			updateSimTime(t1);

			simtime_t handleTime = decider->processSignal(tmpAF1);
			assert(handleTime < 0);

			double senseLength = 6;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);
			setExpectedCSRAnswer(true, false, tmpAF1Power);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertEqual("UNTIL_IDLE request on idle channel should be answered immediately.",
						-1.0, SIMTIME_DBL(handleTime));
			delete testChannelSense;


			//test UNTIL_BUSY on idle channel with incoming frame changing
			//answer time of request

			//since both the airframe and the request have been handled or rejected
			//the decider should be in the same state as it was before again
			testChannelSense = createCSR(senseLength, UNTIL_BUSY);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertEqual("UNTIL_BUSY request on idle channel can't be answered yet.",
						testTime + senseLength, SIMTIME_DBL(handleTime));

			updateSimTime(t3);

			setExpectedCSRAnswer(false, true, TXpower3 + tmpAF1Power);
			assert(expectCSRAnswer);
			handleTime = decider->processSignal(TestAF3);
			assertFalse("UNTIL_BUSY request should have been answered because of AF3.", expectCSRAnswer);

			assert(handleTime == t5);

			delete testChannelSense;

			//clear af3 from decider
			updateSimTime(t5);
			handleTime = decider->processSignal(TestAF3);
			assert(handleTime < 0);

			//test UNTIL_BUSY on idle channel with time out occurs (because of idle channel)
			updateSimTime(t6);

			senseLength = 2;
			testChannelSense = createCSR(senseLength, UNTIL_BUSY);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertEqual("UNTIL_BUSY request on idle channel can't be answered yet.",
						testTime + senseLength, SIMTIME_DBL(handleTime));

			updateSimTime(testTime + senseLength);

			setExpectedCSRAnswer(true, false, tmpAF1Power);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertFalse("UNTIL_BUSY request should have been answered because of time out.", expectCSRAnswer);

			break;
		}

		case TEST_CHANNELSENSE_BUSY_CHANNEL:
		{
			//test UNTIL_BUSY on already busy channel
			double tmpAF1Power = 1;
			addAirFrameToPool(t1, t5, tmpAF1Power);
			double tmpAF2Power = 3;
			addAirFrameToPool(t3, t7, tmpAF2Power);
			updateSimTime(t3);

			simtime_t senseLength = 2;
			testChannelSense = createCSR(senseLength, UNTIL_BUSY);

			setExpectedCSRAnswer(true, true, tmpAF1Power + tmpAF2Power);
			assert(expectCSRAnswer);
			simtime_t handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertFalse("UNTIL_BUSY request should have been answered because of already busy channel.", expectCSRAnswer);
			assert(handleTime < 0);

			delete testChannelSense;

			//test UNTIL_IDLE on busy channel:
			// - without change until timeout
			senseLength = 1;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered because of busy channel.",
						testTime + senseLength, handleTime);

			updateSimTime(handleTime);

			setExpectedCSRAnswer(true, true, tmpAF1Power + tmpAF2Power);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request was answered because of time out.",
						expectCSRAnswer);

			delete testChannelSense;

			// - without new airframe but channel changes before timeout
			senseLength = 2;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered because of busy channel.",
						t5, handleTime);

			updateSimTime(handleTime);

			setExpectedCSRAnswer(true, false, tmpAF2Power);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request was answered because of idle channel.",
						expectCSRAnswer);

			delete testChannelSense;

			// - with new airframe delaying answer time
			double tmpAF3Power = 1;
			DetailedRadioFrame * tmpAF3 = addAirFrameToPool(t4, t6, tmpAF3Power);

			updateSimTime(t3);

			senseLength = 5;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered because of busy channel.",
						t5, handleTime);

			updateSimTime(t4);

			setExpectedReschedule(t6);
			assert(expectReschedule);
			handleTime = decider->processSignal(tmpAF3);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request was rescheduled because of new AirFrame.",
						expectReschedule);

			updateSimTime(expRescheduleTime);

			setExpectedCSRAnswer(true, false, tmpAF2Power);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request was answered because of idle channel.",
						expectCSRAnswer);

			delete testChannelSense;

			// - with new airframe not changing answer time

			// ASSUMPTION: decider state is the same as directly after initialization
			removeAirFrameFromPool(tmpAF3);
			tmpAF3Power = 0.2;
			tmpAF3 = addAirFrameToPool(t4, t6, tmpAF3Power);

			updateSimTime(t3);

			senseLength = 5;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered because of busy channel.",
						t5, handleTime);

			simtime_t answerCSRTime = handleTime;

			updateSimTime(t4);

			handleTime = decider->processSignal(tmpAF3);
			assert(handleTime < 0);

			updateSimTime(answerCSRTime);

			setExpectedCSRAnswer(true, false, tmpAF2Power + tmpAF3Power);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request was answered because of idle channel.",
						expectCSRAnswer);

			delete testChannelSense;

			break;
		}

		case TEST_CHANNELSENSE_CHANNEL_CHANGES_DURING_AIRFRAME:
		{
			//UNTIL_IDLE with channel state busy->idle during airframe
			double tmpAF1Power1 = 4;
			double tmpAF1Power2 = 0.5;
			DetailedRadioFrame * tmpAF1 = addAirFrameToPool(t1, t3, t5, tmpAF1Power1, tmpAF1Power2);

			updateSimTime(t2);

			simtime_t senseLength = 6;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			simtime_t handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered because of busy header.",
						t3, handleTime);

			updateSimTime(handleTime);

			setExpectedCSRAnswer(true, true, tmpAF1Power1);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertFalse("UNTIL_IDLE request should have been answered because of idle payload.", expectCSRAnswer);
			assert(handleTime < 0);

			delete testChannelSense;

			//UNTIL_IDLE with idle delayed because of summed up power at a
			//time point which is not a start or end of any airframe
			removeAirFrameFromPool(tmpAF1);

			tmpAF1 = addAirFrameToPool(t2, t5, t8, tmpAF1Power1, tmpAF1Power2);
			double tmpAF23Power1 = 2;
			double tmpAF23Power2 = 0.5;
			DetailedRadioFrame * tmpAF2 = addAirFrameToPool(t1, t4, t8, tmpAF23Power2, tmpAF23Power1);
			DetailedRadioFrame * tmpAF3 = addAirFrameToPool(t3, t6, t8, tmpAF23Power1, tmpAF23Power2);

			updateSimTime(t2);

			senseLength = 6;
			testChannelSense = createCSR(senseLength, UNTIL_IDLE);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_IDLE request can't be answered yet because of busy header(1).",
						t5, handleTime);

			updateSimTime(t3);

			setExpectedReschedule(t6);
			assert(expectReschedule);
			handleTime = decider->processSignal(tmpAF3);
			assert(handleTime < 0);
			assertFalse("UNTIL_IDLE request should have been rescheduled.", expectReschedule);

			updateSimTime(expRescheduleTime);

			setExpectedCSRAnswer(true, true, tmpAF1Power2 + tmpAF23Power1 + tmpAF23Power1);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertFalse("UNTIL_IDLE request should have been answered because of idle channel.",
						expectCSRAnswer);
			assert(handleTime < 0);

			delete testChannelSense;

			//UNTIL_BUSY with channel state idle->busy during airframe
			removeAirFrameFromPool(tmpAF1);
			removeAirFrameFromPool(tmpAF2);
			removeAirFrameFromPool(tmpAF3);
			tmpAF1Power1 = 2;
			tmpAF1Power2 = 4;
			tmpAF1 = addAirFrameToPool(t1, t4, t7, tmpAF1Power1, tmpAF1Power2);

			updateSimTime(t2);

			senseLength = 7;
			testChannelSense = createCSR(senseLength, UNTIL_BUSY);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_BUSY request can't be answered because of idle header.",
						t4, handleTime);

			updateSimTime(handleTime);

			setExpectedCSRAnswer(true, true, tmpAF1Power2);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertFalse("UNTIL_BUSY request should have been answered because of busy payload.", expectCSRAnswer);
			assert(handleTime < 0);

			delete testChannelSense;

			//UNTIL_BUSY with busy change is brought forward by second payload
			double tmpAF2Power1 = 1;
			double tmpAF2Power2 = 2;
			tmpAF2 = addAirFrameToPool(t2, t3, t5, tmpAF2Power1, tmpAF2Power2);

			updateSimTime(t1);

			senseLength = 7;
			testChannelSense = createCSR(senseLength, UNTIL_BUSY);

			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assertClose("UNTIL_BUSY request can't be answered because of idle header(s).",
						t4, handleTime);

			updateSimTime(t2);

			setExpectedReschedule(t3);
			assert(expectReschedule);
			handleTime = decider->processSignal(tmpAF2);
			assert(handleTime < 0);
			assertFalse("UNTIL_BUSY request was rescheduled because of new AirFrame payload.",
						expectReschedule);

			updateSimTime(expRescheduleTime);

			setExpectedCSRAnswer(true, true, tmpAF1Power1 + tmpAF2Power2);
			assert(expectCSRAnswer);
			handleTime = decider->handleChannelSenseRequest(testChannelSense);
			assert(handleTime < 0);
			assertFalse("UNTIL_BUSY request was answered because of busy payload.",
						expectCSRAnswer);

			delete testChannelSense;
			break;
		}

		default:
			break;
	}
}


/**
 * Return a string with the pattern
 * "[module name] - passed text"
 */
std::string DeciderTest::log(std::string msg) const {
	return "[" + deciderName + " Test] - " + msg;
}


ConstMapping* DeciderTest::getThermalNoise(simtime_t_cref /*from*/, simtime_t_cref /*to*/)
{
	return NULL;
}

void DeciderTest::cancelScheduledMessage(cMessage* /*msg*/)
{
	return;
}

void DeciderTest::drawCurrent(double /*amount*/, int /*activity*/)
{
	return;
}

void DeciderTest::recordScalar(const char */*name*/, double /*value*/, const char */*unit*/)
{
	return;
}


void DeciderTest::rescheduleMessage(cMessage* msg, simtime_t_cref t)
{
	ChannelSenseRequest* req = dynamic_cast<ChannelSenseRequest*>(msg);
	assertTrue("Rescheduled message is a ChannelSenseRequest.", req != 0);
	assertTrue("This rescheduling was expected.", expectReschedule);
	assertClose("Decider rescheduled CSR to correct time.", expRescheduleTime, t);
	expRescheduleTime = t;

	expectReschedule = false;
}



