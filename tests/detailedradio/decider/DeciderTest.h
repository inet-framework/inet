#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <DeciderToPhyInterface.h>
#include <OmnetTestBase.h>

#include <list>
#include <Mapping.h>
#include <Decider.h>
#include <DetailedRadioSignal.h>
#include <ChannelSenseRequest_m.h>

class DeciderTest : public DeciderToPhyInterface, public SimpleTest {
private:
	/** @brief Copy constructor is not allowed.
	 */
	DeciderTest(const DeciderTest&);
	/** @brief Assignment operator is not allowed.
	 */
	DeciderTest& operator=(const DeciderTest&);

protected:

	Decider* decider;

	/** @brief The name of the currently tested decider (for logging purposes).*/
	std::string deciderName;

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	// member to store AirFrame on the channel (for current testing state)
	DeciderToPhyInterface::AirFrameVector airFramesOnChannel;

	/**
	 * @brief Used at initialisation to pass the parameters
	 * to the AnalogueModel and Decider
	 */
	typedef std::map<std::string, cMsgPar> ParameterMap;

	typedef std::list<DetailedRadioFrame *> AirFrameList;

	/** @brief Defines the all AirFrames for one test scenario which
	 * can appear on the channel.*/
	AirFrameList airFramePool;

	/**
	 * @brief Small service-class.
	 */
	class TestWorld
	{
	protected:

		/** @brief Provides a unique number for AirFrames per simulation */
		long airFrameId;

	public:

		TestWorld()
			: airFrameId(0)
		{}

		~TestWorld() {}

		/** @brief Returns an Id for an AirFrame, at the moment simply an incremented long-value */
	    long getUniqueAirFrameId(){

	    	// if counter has done one complete cycle and will be set to a value it already had
	    	if (airFrameId == -1){
	    		// print a warning
	    		EV_STATICCONTEXT;
	    		EV << "WARNING: AirFrameId-Counter has done one complete cycle."
	    		   << " AirFrameIds are repeating now and may not be unique anymore." << endl;
	    	}

	    	return airFrameId++;
	    }
	};

	// list containing simtime-to-double entries
	// typedef std::list<std::pair<simtime_t, double> > KeyList;

	virtual Decider* initDeciderTest(std::string name, ParameterMap& params);

	void runDeciderTests(std::string name);

	enum TestCaseIdentifier
	{
		//NOTE: The form of the comments and the position of the
		//commas results from the IDE not being able to show the comments
		//otherwise
		BEFORE_TESTS = 0,

		TEST_GET_CHANNELSTATE_EMPTYCHANNEL = 100,




		TEST_SNR_THRESHOLD_ACCEPT /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!


		TEST_SNR_THRESHOLD_DENY /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_SNR_THRESHOLD_PAYLOAD_DENY /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * Frame2             |-----------------|		(start: t3, length: t9-t3)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 * Frame6    |--|								(start: t0, length: t1-t0)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY/**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * Frame4                   |--|				(start: t5, length: t6-t5)
		 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
		 * (Frame5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!


		TEST_GET_CHANNELSTATE_NOISYCHANNEL /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * Frame2             |-----------------|		(start: t3, length: t9-t3)
		 * Frame3             |-----|					(start: t3, length: t5-t3)
		 * Frame4                   |--|				(start: t5, length: t6-t5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,	//<-------BEWARE!!!!!!! A COMMA!


		TEST_GET_CHANNELSTATE_RECEIVING /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * Frame2             |-----------------|		(start: t3, length: t9-t3)
		 * Frame3             |-----|					(start: t3, length: t5-t3)
		 * Frame4                   |--|				(start: t5, length: t6-t5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!


		TEST_CHANNELSENSE /**
		 * Frame1       |-----------------|				(start: t1, length: t7-t1)
		 * Frame2             |-----------------|		(start: t3, length: t9-t3)
		 * Frame3             |-----|					(start: t3, length: t5-t3)
		 * Frame4                   |--|				(start: t5, length: t6-t5)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_CHANNELSENSE_IDLE_CHANNEL /**
		 * Frame3             |-----|					(start: t3, length: t5-t3)
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_CHANNELSENSE_BUSY_CHANNEL /**
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

		TEST_CHANNELSENSE_CHANNEL_CHANGES_DURING_AIRFRAME /**
		 *           |  |  |  |  |  |  |  |  |  |  |
		 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
		 * where: t0=before, t10=after */
		,//<-------BEWARE!!!!!!!

	} currentTestCase;

	/**
	 * @brief Dispatchs test case execution to the correct
	 * "execute*TestCase" method depending of the current decider
	 * under test
	 *
	 * @param testCase the test case to execute
	 */
	void executeTestCase(TestCaseIdentifier testCase);
	/**
	 * @brief Executes test cases for SNRThresholdDecider.
	 */
	void executeSNRNewTestCase();






	std::string stateToString(int state)
	{
		switch (state) {
			case BEFORE_TESTS:
				return "BEFORE_TESTS";

			case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
				return "TEST_GET_CHANNELSTATE_EMPTYCHANNEL";

			case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
				return "TEST_GET_CHANNELSTATE_NOISYCHANNEL";
			case TEST_GET_CHANNELSTATE_RECEIVING:
				return "TEST_GET_CHANNELSTATE_RECEIVING";


			case TEST_SNR_THRESHOLD_ACCEPT:
				return "TEST_SNR_THRESHOLD_ACCEPT";
			case TEST_SNR_THRESHOLD_DENY:
				return "TEST_SNR_THRESHOLD_DENY";
			case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
				return "TEST_SNR_THRESHOLD_PAYLOAD_DENY";
			case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
				return "TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY";
			case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
				return "TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY";
			case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
				return "TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY";

			case TEST_CHANNELSENSE:
				return "TEST_CHANNELSENSE";

			default:
				assertFalse("Correct state found.", true);
				return "Unknown state.";
		}

	}

	// method to fill the member airFramesOnChannel with AirFrames
	// according to testing state
	void fillAirFramesOnChannel();

	// create a test AirFrame identified by an index
	DetailedRadioFrame * createTestAirFrame(int i);

	// pass AirFrames currently on the (virtual) channel to currently tested decider
	void passAirFramesOnChannel(AirFrameVector& out) const;

	/**
	 * @brief Creates a simple Signal defined over time with the
	 * passed parameters.
	 *
	 * Signals that are not constant over the whole duration can also be
	 * created.
	 *
	 * Convenience method to be able to create the appropriate
	 * Signal for the MacToPhyControlInfo without needing to care
	 * about creating Mappings.
	 */
	virtual DetailedRadioSignal* createSignal(simtime_t_cref start,
									simtime_t_cref length,
									std::pair<double, double> power,
									std::pair<double, double> bitrate,
									int index,
									simtime_t_cref payloadStart);

	/**
	 * @brief Creates a simple Signal defined over time with the
	 * passed parameters.
	 *
	 * Convenience method to be able to create the appropriate
	 * Signal for the MacToPhyControlInfo without needing to care
	 * about creating Mappings.
	 */
	virtual DetailedRadioSignal* createSignal(simtime_t_cref start,
									simtime_t_cref length,
									double power,
									double bitrate);

	virtual DetailedRadioSignal* createSignal(simtime_t_cref start,
								 simtime_t_cref payloadStart,
								 simtime_t_cref end,
								 double powerHeader,
								 double powerPayload,
								 double bitrate);

	/**
	 * @brief Creates a simple Mapping with a constant curve
	 * progression at the passed value.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 */
	Mapping* createConstantMapping(simtime_t_cref start, simtime_t_cref end, double value);


	/**
	 * @brief Creates a mapping with separate values for header and payload of an AirFrame.
	 *
	 * This is a 'quasi-step' mapping, using a linear interpolated mapping, i.e. a
	 * step is realized by two succeeding key-entries in the mapping with linear
	 * interpolation between them.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 *
	 */
	Mapping* createHeaderPayloadMapping(simtime_t_cref start,
										simtime_t_cref payloadStart,
										simtime_t_cref end,
										double headerValue,
										double payloadValue);



	// some fix time-points for the signals
	simtime_t t0;
	simtime_t t1;
	simtime_t t3;
	simtime_t t5;
	simtime_t t7;
	simtime_t t9;

	// time-points before and after
	simtime_t before;
	simtime_t after;

	// time-points in between
	simtime_t t2;
	simtime_t t4;
	simtime_t t6;
	simtime_t t8;

	simtime_t testTime;

	// some test AirFrames
	DetailedRadioFrame * TestAF1;
	DetailedRadioFrame * TestAF2;
	DetailedRadioFrame * TestAF3;
	DetailedRadioFrame * TestAF4;

	// AirFrames for SNR-threshold tests
	DetailedRadioFrame * TestAF5;
	DetailedRadioFrame * TestAF6;


	// Used for ChannelSenseRequest tests to define the expected result of a Test sense
	ChannelState expChannelState;

	/** @brief Stores if we are currently expecting an answer for a CSR from the decider.*/
	bool expectCSRAnswer;

	/** @brief Used for CSR tests to define the expected rescheduling time.*/
	simtime_t expRescheduleTime;

	simtime_t actualRescheduleTime;

	/** @brief Stores if we are currently expecting an reschedule for a CSR from the Decider.*/
	bool expectReschedule;

	// Represents the currently used ChannelSenseRequest which is tested
	ChannelSenseRequest*  testChannelSense;

	// pointer to the AirFrame that is currently processed by decider
	DetailedRadioFrame * processedAF;

	// value for no attenuation (in attenuation-mappings)
	double noAttenuation;


	// some TX-power values
	double TXpower1;
	double TXpower2;
	double TXpower3;
	double TXpower4;
	double TXpower5P;
	double TXpower5H;
	double TXpower6;

	// some bitrates
	double bitrate9600;

	// expected results of tests (hard coded)
	double res_t1_noisy;
	double res_t2_noisy;
	double res_t3_noisy;
	double res_t4_noisy;
	double res_t5_noisy;
	double res_t9_noisy;

	double res_t2_receiving;
	double res_t3_receiving_before;
	double res_t3_receiving_after;
	double res_t4_receiving;
	double res_t5_receiving_before;
	double res_t5_receiving_after;
	double res_t6_receiving;

	// flag to signal whether sendUp() has been called by decider
	bool sendUpCalled;

	// minimal world for testing purposes
	TestWorld* world;


	/**
	 * @brief returns the closest value of simtime before passed value
	 *
	 * Works only for arguments t > 0;
	 */
	simtime_t pre(simtime_t_cref t)
	{
		if (SIMTIME_RAW(t) > 0)
			return MappingUtils::pre(t);
		return t;
	}
	/**
	 * @brief returns the closest value of simtime after passed value
	 *
	 * Works only for arguments t > 0;
	 */
	simtime_t post(simtime_t_cref t)
	{
		if (SIMTIME_RAW(t) > 0)
			return MappingUtils::post(t);
		return t;
	}

	/**
	 * @brief Updates the current test time to the passed value and
	 * updates the channel by calling "fillAirFramesOnChannel".
	 *
	 * @param t - the new test time
	 */
	void updateSimTime(simtime_t_cref t) {
		testTime = t;
		fillAirFramesOnChannel();
	}

	ChannelSenseRequest* createCSR(simtime_t_cref duration, SenseMode mode) {
		ChannelSenseRequest* tmp = new ChannelSenseRequest();
		tmp->setSenseMode(mode);
		tmp->setSenseTimeout(duration);
		return tmp;
	}

	DetailedRadioFrame * addAirFrameToPool(simtime_t_cref start, simtime_t_cref end, double power);
	DetailedRadioFrame * addAirFrameToPool(simtime_t_cref start, simtime_t_cref payloadStart, simtime_t_cref end,
								double headerPower, double payloadPower);
	void removeAirFrameFromPool(DetailedRadioFrame * af);

	void freeAirFramePool();

	void setExpectedCSRAnswer(bool expIsIdle, double expRSSI) {
		expectCSRAnswer = true;
		expChannelState = ChannelState(expIsIdle, expRSSI);
	}

	void setExpectedReschedule(simtime_t_cref newTime) {
		expectReschedule = true;
		expRescheduleTime = newTime;
	}
public:
	DeciderTest();

	virtual ~DeciderTest();


	//---------DeciderToPhyInterface implementation-----------
	// Taken from BasePhyLayer. Will be overridden to control
	// the deciders calls on the Interface during runDeciderTests().

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) const;

	/**
	 * @brief Called by the decider to send a control message to the MACLayer
	 *
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 *
	 */
	virtual void sendControlMsgToMac(cMessage* msg);

	/**
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 *
	 * When a packet is completely received and not noise, the decider
	 * calls this function to send the packet together with
	 * the corresponding DeciderResult up to MACLayer
	 *
	 */
	virtual void sendUp(DetailedRadioFrame * packet, DeciderResult* result);

	/**
	 * @brief Returns a special test-time
	 *
	 */
	virtual simtime_t getSimTime() const;

	/**
	 * Return a string with the pattern
	 * "[module name] - passed text"
	 */
	std::string log(std::string msg) const;

	/**
	 * @brief Returns a Mapping which defines the thermal noise in
	 * the passed time frame (in mW).
	 *
	 * The implementing class of this method keeps ownership of the
	 * Mapping.
	 */
	virtual ConstMapping* getThermalNoise(simtime_t_cref from, simtime_t_cref to);

	/**
	 * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or a control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void cancelScheduledMessage(cMessage* msg);

	/**
	 * @brief Enables the Decider to draw Power from the
	 * phy layers power accounts.
	 *
	 * Does nothing if no Battery module in simulation is present.
	 */
	virtual void drawCurrent(double amount, int activity);

	/**
	 * @brief Utility method to enable a Decider, which isn't an OMNeT-module, to
	 * use the OMNeT-method 'recordScalar' with the help of and through its interface to BasePhyLayer.
	 *
	 * The method-signature is taken from OMNeTs 'ccomponent.h' but made pure virtual here.
	 * See the original method-description below:
	 *
	 * Records a double into the scalar result file.
	 */
	virtual void recordScalar(const char *name, double value, const char *unit=NULL);

	/**
	 * @brief Tells the PhyLayer to reschedule a message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or an control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void rescheduleMessage(cMessage* msg, simtime_t_cref t);

	virtual int getCurrentRadioChannel() const { return -1; }

	virtual int getNbRadioChannels()     const { return 1;  }
	/**
	 * @brief Returns the current state the radio is in.
	 *
	 * See RadioState for possible values.
	 *
	 * This method is mainly used by the mac layer.
	 */
	virtual bool isRadioInRX() const {
		return true;
	}

	virtual long getPhyHeaderLength() const { return 1; }
	//---------SimpleTest implementation-----------

	virtual void runTests();
};

#endif /*TESTPHYLAYER_H_*/
