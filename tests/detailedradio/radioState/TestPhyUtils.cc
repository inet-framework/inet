#include "PhyUtils.h"
#include "Mapping.h"

#include <asserts.h>
#include <limits>
#include <cmath>
#include <OmnetTestBase.h>

static bool simTimeDblEquals(simtime_t_cref a, double b) {
    simtime_t    stB(b);
    const double dDelta( std::fabs(a.dbl() - b) );

    return ( a == stB || dDelta < std::numeric_limits<double>::epsilon() || dDelta < std::pow(10.0, simtime_t::getScaleExp()) );
}
static bool doubleEquals(double a, double b) {
    return a == b || std::fabs(a - b) < std::numeric_limits<double>::epsilon();
}
/* ------ Testing stuff for Radio ------ */

// global variables needed for tests
const int initialState =  IRadio::RADIO_MODE_RECEIVER;

static const double RX2TX       = 1.0;
static const double RX2SLEEP    = 2.0;
static const double TX2RX       = 3.0;
static const double TX2SLEEP    = 4.0;
static const double SLEEP2RX    = 5.0;
static const double SLEEP2TX    = 6.0;
static const double RX2RX       = 7.0;
static const double TX2TX       = 8.0;
static const double SLEEP2SLEEP = 9.0;

// test functions

void testRadioConstructor()
{
	MiximRadio* radio1 = MiximRadio::createNewRadio(); // default constructor

	// check members of radio1
	assertTrue("RadioState is correct", radio1->getCurrentState() == IRadio::RADIO_MODE_OFF);

	MiximRadio* radio2 = MiximRadio::createNewRadio(false, initialState); // contructor with argument

	//check members of radio2
	assertTrue("RadioState is correct", radio2->getCurrentState() == initialState);

	std::cout << "Constructor Test passed." << std::endl;

	delete radio1;
	delete radio2;

	return;
}

int testSwitching(MiximRadio& radio, int to, double refValue)
{

	// check whether radio is currently switching
	if (radio.getCurrentState() == IRadio::RADIO_MODE_SWITCHING) return -1;

	// call switchTo
	double result = SIMTIME_DBL(radio.switchTo(to, 0));

	assertFalse("Radio does return a valid switch time", result < 0);
	assertTrue("Radio is now switching", radio.getCurrentState() == IRadio::RADIO_MODE_SWITCHING);
	assertEqual("Radio has returned the correct switch time", refValue, result);

	result = SIMTIME_DBL(radio.switchTo(to, 0));
	assertTrue("Radio is switching, error value awaited", result < 0);

	// call endSwitch
	radio.endSwitch(0);

	assertTrue("Radio has switched to correct state", radio.getCurrentState() == to);

	return 0;
}

void testRadioFunctionality()
{
	MiximRadio& radio1 = *MiximRadio::createNewRadio(false, IRadio::RADIO_MODE_RECEIVER);

	// call setSwTime() for every matrix entry
	//radio1.setSwitchTime(RX, RX, value);//not a regular case
	radio1.setSwitchTime(IRadio::RADIO_MODE_RECEIVER, IRadio::RADIO_MODE_TRANSMITTER, RX2TX);
	radio1.setSwitchTime(IRadio::RADIO_MODE_RECEIVER, IRadio::RADIO_MODE_SLEEP, RX2SLEEP);
	//radio1.setSwitchTime(RX, SWITCHING, value); //not a regular case

	radio1.setSwitchTime(IRadio::RADIO_MODE_TRANSMITTER, IRadio::RADIO_MODE_RECEIVER, TX2RX);
	//radio1.setSwitchTime(TX, TX, value);//not a regular case
	radio1.setSwitchTime(IRadio::RADIO_MODE_TRANSMITTER, IRadio::RADIO_MODE_SLEEP, TX2SLEEP);
	//radio1.setSwitchTime(TX, SWITCHING, value);//not a regular case

	radio1.setSwitchTime(IRadio::RADIO_MODE_SLEEP, IRadio::RADIO_MODE_RECEIVER, SLEEP2RX);
	radio1.setSwitchTime(IRadio::RADIO_MODE_SLEEP, IRadio::RADIO_MODE_TRANSMITTER, SLEEP2TX);
	//radio1.setSwitchTime(SLEEP, SLEEP, value);//not a regular case
	//radio1.setSwitchTime(SLEEP, SWITCHING, value);//not a regular case

	//radio1.setSwitchTime(SWITCHING, RX, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, TX, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, SLEEP, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, SWITCHING, value);//not a regular case

	// testing switching to the same state, this might not be needed
	radio1.setSwitchTime(IRadio::RADIO_MODE_RECEIVER, IRadio::RADIO_MODE_RECEIVER, RX2RX);
	radio1.setSwitchTime(IRadio::RADIO_MODE_TRANSMITTER, IRadio::RADIO_MODE_TRANSMITTER, TX2TX);
	radio1.setSwitchTime(IRadio::RADIO_MODE_SLEEP, IRadio::RADIO_MODE_SLEEP, SLEEP2SLEEP);

	int switchResult;

	// RX -> RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_RECEIVER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_RECEIVER, RX2RX);
	assertTrue("Switching test", switchResult == 0);

	// RX -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_RECEIVER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_TRANSMITTER, RX2TX);
	assertTrue("Switching test", switchResult == 0);

	// TX -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_TRANSMITTER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_TRANSMITTER, TX2TX);
	assertTrue("Switching test", switchResult == 0);

	// TX -> SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_TRANSMITTER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_SLEEP, TX2SLEEP);
	assertTrue("Switching test", switchResult == 0);

	// SLEEP -> SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_SLEEP);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_SLEEP, SLEEP2SLEEP);
	assertTrue("Switching test", switchResult == 0);

	// SLEEP -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_SLEEP);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_TRANSMITTER, SLEEP2TX);
	assertTrue("Switching test", switchResult == 0);

	// TX - > RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_TRANSMITTER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_RECEIVER, TX2RX);
	assertTrue("Switching test", switchResult == 0);

	// RX - > SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_RECEIVER);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_SLEEP, RX2SLEEP);
	assertTrue("Switching test", switchResult == 0);

	// SLEEP -> RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_SLEEP);
	switchResult = testSwitching(radio1, IRadio::RADIO_MODE_RECEIVER, SLEEP2RX);
	assertTrue("Switching test", switchResult == 0);

	assertTrue("RadioState is correct", radio1.getCurrentState() == IRadio::RADIO_MODE_RECEIVER);
	std::cout << "SetSwitchTime test passed." << std::endl;

	delete &radio1;

	return;
}

/* ------ Testing stuff for RadioStateAnalogueModel ------ */


// further global variables

// timepoints to store in the analogue model
static const double time1 = (0.2); // first timepoint
static const double time2 = (0.3);
static const double time3 = (0.51);
static const double time4 = (0.6); // last timepoint

// timepoint for testing purposes
static const double time5 = (0.5); // in between two timepoints, before duplicate
static const double time6 = (0.52); // in between two timepoints, after duplicate
static const double time7 = (0.01); // before first timepoint
//const simtime_t time7 = 0.19; // before first timepoint
static const double time8 = (0.65); // after last timepoint
static const double time9 = (0.59); // before last timepoint

static const Argument::mapped_type minAtt = (2.0);
static const Argument::mapped_type maxAtt = (0.0);

typedef std::list< std::pair<double, Argument::mapped_type> > RSList;

// just an inherited class to get access to the members
class DiagRSAM : public RadioStateAnalogueModel
{

public:

	DiagRSAM(double initValue) : RadioStateAnalogueModel(initValue) {}

	DiagRSAM(double initValue, bool currentlyTracking)
		: RadioStateAnalogueModel(initValue, currentlyTracking) {}

	DiagRSAM(double initValue, bool currentlyTracking, simtime_t_cref initTime)
			: RadioStateAnalogueModel(initValue, currentlyTracking, initTime) {}

	bool getTrackingFlag() const { return currentlyTracking; }


	int getRecvListSize() const { return ((int) radioStateAttenuation.size()); }

	ListEntry getFirstRecvListEntry() const { return radioStateAttenuation.front(); }


	bool compareRecvLists(const RSList& refRecvList) const
	{
		if ( radioStateAttenuation.size() != refRecvList.size() )
		    return false;

		time_attenuation_collection_type::const_iterator it1;
        time_attenuation_collection_type::const_iterator it1End = radioStateAttenuation.end();
		RSList::const_iterator                           it2;

		for (it1 = radioStateAttenuation.begin(), it2 = refRecvList.begin(); it1 != it1End; ++it1, ++it2) {
			if ( !simTimeDblEquals(it1->getTime(), it2->first) ) return false;
			if ( !doubleEquals(it1->getValue(), it2->second) )   return false;
		}

		return true;
	}
    void printLists(const RSList& refRecvList) const
    {
        time_attenuation_collection_type::const_iterator it1;
        time_attenuation_collection_type::const_iterator it1End = radioStateAttenuation.end();
        RSList::const_iterator                           it2;
        RSList::const_iterator                           it2End = refRecvList.end();
        unsigned                                         iCnt   = 0;

        for (it1 = radioStateAttenuation.begin(), it2 = refRecvList.begin(); it1 != it1End || it2 != it2End; ++iCnt) {
            if ( it1 != it1End ) {
                std::cout << "#" << iCnt << " Time1: " << it1->getTime() << ", Value1: " << it1->getValue();
                ++it1;
            }
            else {
                std::cout << "#" << iCnt << " Time1: \t\t, Value1: ";
            }
            std::cout << std::endl;
            if ( it2 != it2End ) {
                std::cout << "   Time2: " << it2->first << ", Value2: " << it2->second;
                ++it2;
            }
            else {
                std::cout << "   Time2: \t\t, Value1: ";
            }
            std::cout << std::endl;
        }
    }
};



class RadioStateTest:public SimpleTest {
private:
	simtime_t initTime;
	simtime_t offset;

	// reference lists
	RSList emptyList;
	RSList oneElemList;
	RSList manyElemList;
	RSList variableList;

public:
	RadioStateTest()
		: SimpleTest()
		, initTime(0.1)
		, offset(0.1)
		, emptyList()
		, oneElemList()
		, manyElemList()
		, variableList()
	{}
	virtual ~RadioStateTest() {}

private:

	void fillReferenceLists()
	{
		// contains only last timestamp
		// (time4,minAtt)
		oneElemList.push_back( std::make_pair(time4, minAtt) );

		// contains all timestamps
		// (initTime, minAtt)--(time1,minAtt)--(time2,minAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)
		manyElemList.push_back( std::make_pair(initTime.dbl(), minAtt) );
		manyElemList.push_back( std::make_pair(time1, minAtt) );
		manyElemList.push_back( std::make_pair(time2, minAtt) );
		manyElemList.push_back( std::make_pair(time3, maxAtt) );
		manyElemList.push_back( std::make_pair(time3, minAtt) );
		manyElemList.push_back( std::make_pair(time4, minAtt) );
	}

	//test constructor with all possible arguments (attenuation, initTime)
	void testRSAMConstructor()
	{
		std::cout << "---testRSAMConstructor" << std::endl;

		DiagRSAM m = DiagRSAM(minAtt);
		assertFalse("Default constructor sets tracking off.", m.getTrackingFlag());
		assertTrue("Default constructor sets initialTime to 0.", m.getFirstRecvListEntry().getTime() == 0);
		assertTrue("Default constructor creates one-elem list.", m.getRecvListSize() == 1);

		m = DiagRSAM(minAtt, false);
		assertFalse("DiagRSAM(false) sets tracking off.", m.getTrackingFlag());
		assertTrue("DiagRSAM(false) creates one-elem list.", m.getRecvListSize() == 1);

		m = DiagRSAM(minAtt, true);
		assertTrue("DiagRSAM(true) sets tracking on.", m.getTrackingFlag());
		assertTrue("DiagRSAM(true) creates one-elem list.", m.getRecvListSize() == 1);

		m = DiagRSAM(minAtt, true, initTime);
		assertTrue("Initial time in receive-list is correct.", m.getFirstRecvListEntry().getTime() == initTime);
		assertTrue("Initial Value in receive-list is correct.", m.getFirstRecvListEntry().getValue() == minAtt);

	}


	void testRSAMModification()
	{
		std::cout << "---testRSAMModification" << std::endl;


		// empty map, tracking off
		DiagRSAM m = DiagRSAM(minAtt, false, initTime);

		// call writeRecvEntry, should not write to list
		m.writeRecvEntry(time1, minAtt);
		assertTrue("Nothing has been written to list.", m.getRecvListSize() == 1);


		m.setTrackingModeTo(true);
		assertTrue("Tracking is on.", m.getTrackingFlag());

		m.setTrackingModeTo(false);
		assertFalse("Tracking is off.", m.getTrackingFlag());


		// tests for lists storing many entries

		// add the elements to the model
		// should create:

		// (initTime, minAtt)--(time1,minAtt)--(time2,minAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)

		m = DiagRSAM(minAtt, true, initTime);

		m.setTrackingModeTo(true);

		const simtime_t stime1(time1);
		const simtime_t stime2(time2);
		const simtime_t stime3(time3);
		const simtime_t stime4(time4);
		const simtime_t stime5(time5);
		const simtime_t stime6(time6);
		const simtime_t stime7(time7);
		const simtime_t stime8(time8);
		const simtime_t stime9(time9);

		m.writeRecvEntry(stime1, minAtt);
		m.writeRecvEntry(stime2, minAtt);
		m.writeRecvEntry(stime3, maxAtt);
		m.writeRecvEntry(stime3, minAtt);
		m.writeRecvEntry(stime4, minAtt);

		assertTrue("Elements have been written to list.", m.getRecvListSize() == (1+5));

		// compare lists
		assertTrue("Lists are equal. (many entries)", m.compareRecvLists(manyElemList));

		// make copies for cleanup-tests

		// special cases
		DiagRSAM temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint before the first entry
		temp.cleanUpUntil(stime7); // nothing should happen
		assertTrue("Lists are equal. (many entries), cleanUp before first entry", temp.compareRecvLists(manyElemList));

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint after the last entry
		temp.cleanUpUntil(stime8); // complete list should be cleaned, except last entry
		assertTrue("Lists are equal. (many entries), cleanUp after last entry", temp.compareRecvLists(oneElemList));

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint equal to the first entry
		temp.cleanUpUntil(initTime); // nothing should happen
		assertTrue("Lists are equal. (many entries), cleanUp exactly first entry", temp.compareRecvLists(manyElemList));

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint equal to the last entry
		temp.cleanUpUntil(stime4); // only last entry should remain
		assertTrue("Lists are equal. (many entries), cleanUp exactly last entry", temp.compareRecvLists(oneElemList));

		// expected regular cases

		// (time3,maxAtt)--(time3,minAtt)--(time4,minAtt)
		variableList.clear();
		variableList.push_back( std::make_pair(time3, maxAtt) );
		variableList.push_back( std::make_pair(time3, minAtt) );
		variableList.push_back( std::make_pair(time4, minAtt) );

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint equal to the first entry of
		// a bunch with same timepoint
		temp.cleanUpUntil(stime3); // all entries with timepoint >= time3 should remain
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));

		// (time5,minAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)
		variableList.clear();
		variableList.push_back( std::make_pair(time5, minAtt) );
		variableList.push_back( std::make_pair(time3, maxAtt) );
		variableList.push_back( std::make_pair(time3, minAtt) );
		variableList.push_back( std::make_pair(time4, minAtt) );

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint in between two timepoint, one entry must be modified,
		// all others before that one must be deleted
		temp.cleanUpUntil(stime5); // (time2, minAtt) must become (time5, minAtt), (time1, minAtt) must be deleted
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));

		// (time2,minAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)
		variableList.clear();
		variableList.push_back( std::make_pair(time2, minAtt) );
		variableList.push_back( std::make_pair(time3, maxAtt) );
		variableList.push_back( std::make_pair(time3, minAtt) );
		variableList.push_back( std::make_pair(time4, minAtt) );

		temp = DiagRSAM(m);
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with exact timepoint of one entry in the middle
		temp.cleanUpUntil(stime2); // all entries before (time2, minAtt) must be deleted
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));

		// (time9,minAtt)--(time4,minAtt)
		variableList.clear();
		variableList.push_back( std::make_pair(time9, minAtt) );
		variableList.push_back( std::make_pair(time4, minAtt) );

		temp = DiagRSAM(m);
		if (!temp.compareRecvLists(manyElemList)) {
		    temp.printLists(manyElemList);
		}
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
		// called with timepoint before a single entry
		temp.cleanUpUntil(stime9); // (time3, minAtt) must become (time9, minAtt) , all before must be deleted
		if (!temp.compareRecvLists(variableList)) {
		    temp.printLists(variableList);
		}
		assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));

		std::cout << "END" << std::endl;

		// tests for lists storing exactly one entry
		DiagRSAM m2 = DiagRSAM(minAtt, true, initTime);
		// (initTime, minAtt)


		m2.writeRecvEntry(time4, minAtt);
		// (initTime, minAtt)--(time4, minAtt)

		assertTrue("Element has been written to list.", m2.getRecvListSize() == (1+1));

		m2.cleanUpUntil(stime4);
		// (time4, minAtt)

        if (!m2.compareRecvLists(oneElemList)) {
            m2.printLists(oneElemList);
        }

		assertTrue("Lists are equal. (one entry)", m2.compareRecvLists(oneElemList));

		DiagRSAM temp2 = DiagRSAM(m2);
		assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
		// called with timepoint before entry
		temp2.cleanUpUntil(stime9); // nothing should happen
		assertTrue("Lists are equal. (one entry), cleanUp before entry", temp2.compareRecvLists(oneElemList));

		temp2 = DiagRSAM(m2);
		assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
		// called with exactly the timepoint contained
		temp2.cleanUpUntil(stime4); // nothing should happen
		assertTrue("Lists are equal. (one entry), cleanup exactly on entry", temp2.compareRecvLists(oneElemList));

		temp2 = DiagRSAM(m2);
		assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
		// called with timepoint after entry
		temp2.cleanUpUntil(stime8); // list should be cleaned
		assertTrue("Lists are equal. (one entry), cleanUp after entry", temp2.compareRecvLists(oneElemList));

		std::cout << "END" << std::endl;
	}

	/* ------ Testing stuff for RSAMMapping ------ */
	void testGetValue()
	{

		std::cout << "---testGetValue" << std::endl;

		// create empty RSAM and mapping
		RadioStateAnalogueModel rsam = RadioStateAnalogueModel(minAtt, true, initTime);
		RSAMMapping mapping = RSAMMapping( &rsam, initTime, time4 );

		// argument with time = 0.0
		Argument pos = Argument();


		// update of RSAM, leads to:
		// (initTime, minAtt)--(time1,minAtt)--(time2,maxAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)
		rsam.writeRecvEntry(time1, minAtt);
		rsam.writeRecvEntry(time2, maxAtt);
		rsam.writeRecvEntry(time3, maxAtt);
		rsam.writeRecvEntry(time3, minAtt);
		rsam.writeRecvEntry(time4, minAtt);

		std::cout << "(initTime, minAtt)--(time1,minAtt)--(time2,maxAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,minAtt)" << std::endl;
		std::cout << "--------------------------------------------------------------------------------------------------" << std::endl;
		assertEqual("Entry at init time", mapping.getValue(Argument(initTime)), minAtt);
		assertEqual("Entry in between two time-points", mapping.getValue(Argument(initTime + 0.5*(initTime+time1))), minAtt);
		assertEqual("Entry at exactly a time-point (time1)", mapping.getValue(Argument(time1)), minAtt);
		assertEqual("Entry at exactly a time-point (time2)", mapping.getValue(Argument(time2)), maxAtt);
		assertEqual("Entry at time-point with zero-time-switches (time3)", mapping.getValue(Argument(time3)), minAtt);
		assertEqual("Entry at last time-point (time4)", mapping.getValue(Argument(time4)), minAtt);
		assertEqual("Entry after last time-point", mapping.getValue(Argument(time4+offset)), mapping.getValue(Argument(time4)));
	}

	/* ------ Testing stuff for RSAMConstMappingIterator ------ */

	void checkIterator(std::string msg, const ConstMappingIterator& it,
					   bool hasNext, bool inRange,
					   const Argument& arg, const Argument& nextArg,
					   double val, const ConstMapping& f) const {
		assertEqual(msg + ": hasNext()", hasNext, it.hasNext());
		assertEqual(msg + ": inRange()", inRange, it.inRange());
		assertEqual(msg + ": currentPos()", arg, it.getPosition());
		assertEqual(msg + ": nextPos()", nextArg, it.getNextPosition());
		assertClose(msg + ": getValue()", val, it.getValue());
		assertClose(msg + ": Equal with function", f.getValue(it.getPosition()), it.getValue());
	}

	void checkNext(std::string msg, ConstMappingIterator& it,
				   bool hasNext, bool inRange,
				   Argument arg, Argument nextArg,
				   double val, ConstMapping& f) {
		it.next();
		checkIterator(msg, it, hasNext, inRange, arg, nextArg, val, f);
	}

	simtime_t incrTime(simtime_t_cref t)
	{
		return MappingUtils::incNextPosition(t);
	}

	void testRSAMConstMappingIterator()
	{
		std::cout << "---test RSAMConstMappingiterator" << endl;

		const simtime_t stime1(time1);
		const simtime_t stime2(time2);
		const simtime_t stime3(time3);
		const simtime_t stime4(time4);
		const simtime_t stime5(time5);
		const simtime_t stime6(time6);
		const simtime_t stime7(time7);
		const simtime_t stime8(time8);
		const simtime_t stime9(time9);

		// create empty RSAM and mapping
		RadioStateAnalogueModel rsam = RadioStateAnalogueModel(minAtt, true, initTime);
		RSAMMapping mapping = RSAMMapping( &rsam, initTime, time4 );
		RSAMConstMappingIterator* rsamCMI;


		// Constructor tests with rsam storing only the entry (initTime, minAtt)
		// rsamCMI stands on initTime
		simtime_t t0(initTime);
		simtime_t t0Next(MappingUtils::incNextPosition(t0));
		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());
		checkIterator("default Constructor", *rsamCMI, false, true, Argument(t0), Argument(t0Next), minAtt, mapping);
		delete rsamCMI;

		// Constructor tests with rsam storing only one the entry (initTime, minAtt) and Argument(initTime)
		// rsamCMI stands on initTime
		simtime_t t1(initTime);
		simtime_t t1Next(MappingUtils::incNextPosition(t1));
		Argument pos1(t1);
		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator(pos1));
		checkIterator("constructor with position exactly on beginning of initial rsam",
				*rsamCMI, false, true, Argument(t1), Argument(t1Next), minAtt, mapping);
		delete rsamCMI;


		// Constructor tests with rsam storing only one the entry (initTime, minAtt) and Argument(initTime+offset)
		// rsamCMI stands on initTime+offset
		simtime_t t2(initTime+offset);
		simtime_t t2Next(MappingUtils::incNextPosition(t2));
		Argument  pos2(t2);
		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator(pos2));
		checkIterator("constructor with position after beginning of initial rsam",
				*rsamCMI, false, false, pos2, Argument(t2Next), minAtt, mapping);
		delete rsamCMI;

		//--- End of constructor tests

		//--- Begin iterator movement tests
		rsam = RadioStateAnalogueModel(minAtt, true, initTime);
		mapping = RSAMMapping( &rsam, initTime, stime4 );

		// update of RSAM, leads to:
		// (initTime, minAtt)--(time1,minAtt)--(time2,maxAtt)--(time3,maxAtt)--(time3,minAtt)--(time4,maxAtt)
		rsam.writeRecvEntry(stime1, minAtt);
		rsam.writeRecvEntry(stime2, maxAtt);
		rsam.writeRecvEntry(stime3, maxAtt);
		rsam.writeRecvEntry(stime3, minAtt);
		rsam.writeRecvEntry(stime4, maxAtt);

		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());

		checkIterator("Iterator at beginning of filled RSAM",
				*rsamCMI, true, true, Argument(initTime), Argument(MappingUtils::pre(stime1)), minAtt, mapping);

		// --- test iterateTo() method by iterating through some interesting points

		std::cout << "--- testing iterateTo()" << std::endl;

		simtime_t t3(stime1 + (stime2-stime1)/2);
		rsamCMI->iterateTo(Argument(t3));
		checkIterator("Iterator between time1 and time2",
					*rsamCMI, true, true, Argument(t3), Argument(MappingUtils::pre(stime2)), minAtt, mapping);

		rsamCMI->iterateTo(Argument(MappingUtils::pre(stime2)));
		checkIterator("Iterator on pre(time2)",
					*rsamCMI, true, true, Argument(MappingUtils::pre(stime2)), Argument(stime2), minAtt, mapping);

		rsamCMI->iterateTo(Argument(stime2));
		checkIterator("Iterator on time2",
					*rsamCMI, true, true, Argument(stime2), Argument(MappingUtils::pre(stime3)), maxAtt, mapping);


		rsamCMI->iterateTo(Argument(stime3));
		checkIterator("Iterator on time3",
					*rsamCMI, true, true, Argument(stime3), Argument(MappingUtils::pre(stime4)), minAtt, mapping);

		rsamCMI->iterateTo(Argument(stime4));
		simtime_t time4Next = incrTime(stime4);
		checkIterator("Iterator on time4",
					*rsamCMI, false, true, Argument(stime4), Argument(time4Next), maxAtt, mapping);


		simtime_t t5     = stime4+offset;
		simtime_t t5Next = incrTime(t5);
		rsamCMI->iterateTo(Argument(t5));
		checkIterator("Iterator on time4+offset",
					*rsamCMI, false, false, Argument(t5), Argument(t5Next), maxAtt, mapping);

		// --- test jumpTo() method by jumping to some interesting points

		std::cout << "--- testing jumpTo() forewards" << endl;

		// reset iterator to the beginning of the mapping
		rsamCMI->jumpToBegin();

		checkIterator("Iterator at beginning of filled RSAM",
					*rsamCMI, true, true, Argument(initTime), Argument(MappingUtils::pre(stime1)), minAtt, mapping);

		rsamCMI->jumpTo(Argument(t3));
		checkIterator("Iterator between time1 and time2",
					*rsamCMI, true, true, Argument(t3), Argument(MappingUtils::pre(stime2)), minAtt, mapping);

		rsamCMI->jumpTo(Argument(MappingUtils::pre(stime2)));
		checkIterator("Iterator on pre(time2)",
					*rsamCMI, true, true, Argument(MappingUtils::pre(stime2)), Argument(stime2), minAtt, mapping);

		rsamCMI->jumpTo(Argument(stime2));
		checkIterator("Iterator on time2",
					*rsamCMI, true, true, Argument(stime2), Argument(MappingUtils::pre(time3)), maxAtt, mapping);


		rsamCMI->jumpTo(Argument(stime3));
		checkIterator("Iterator on time3",
					*rsamCMI, true, true, Argument(stime3), Argument(MappingUtils::pre(stime4)), minAtt, mapping);

		rsamCMI->jumpTo(Argument(stime4));
		checkIterator("Iterator on time4",
					*rsamCMI, false, true, Argument(stime4), Argument(time4Next), maxAtt, mapping);

		rsamCMI->jumpTo(Argument(t5));
		checkIterator("Iterator on time4+offset",
					*rsamCMI, false, false, Argument(t5), Argument(t5Next), maxAtt, mapping);

		std::cout << "--- testing jumpTo() backwards" << endl;

		rsamCMI->jumpTo(Argument(stime4));
		checkIterator("Iterator on time4",
					*rsamCMI, false, true, Argument(stime4), Argument(time4Next), maxAtt, mapping);

		rsamCMI->jumpTo(Argument(stime3));
		checkIterator("Iterator on time3",
					*rsamCMI, true, true, Argument(stime3), Argument(MappingUtils::pre(stime4)), minAtt, mapping);


		rsamCMI->jumpTo(Argument(stime2));
		checkIterator("Iterator on time2",
					*rsamCMI, true, true, Argument(stime2), Argument(MappingUtils::pre(stime3)), maxAtt, mapping);

		rsamCMI->jumpTo(Argument(t3));
		checkIterator("Iterator between time1 and time2",
					*rsamCMI, true, true, Argument(t3), Argument(MappingUtils::pre(stime2)), minAtt, mapping);

		// some tests on multiple entries at first timepoint
		std::cout << "--- some tests on multiple entries at first timepoint" << endl;

		// clean the list and add a zero time switch at initTime
		//rsam.cleanUpUntil(initTime);
		rsam = RadioStateAnalogueModel(minAtt, true, initTime);
		mapping = RSAMMapping( &rsam, initTime, stime4 );

		t0 = initTime;
		t0Next = MappingUtils::incNextPosition(t0);

		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());
		checkIterator("Iterator at init time without zero time switch",
						*rsamCMI, false, true, Argument(t0), Argument(t0Next), minAtt, mapping);

		// write a zero time switch at init time
		rsam.writeRecvEntry(initTime, maxAtt);

		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());
		checkIterator("Iterator at init time with zero time switch",
						*rsamCMI, false, true, Argument(t0), Argument(t0Next), maxAtt, mapping);

		// now clean up until init time, only the last entry should survive
		rsam.cleanUpUntil(initTime);

		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());
			checkIterator("Iterator at init time without zero time switch",
							*rsamCMI, false, true, Argument(t0), Argument(t0Next), maxAtt, mapping);

		// write a zero time switch at init time
		rsam.writeRecvEntry(initTime, minAtt);

		rsamCMI = static_cast<RSAMConstMappingIterator*>(mapping.createConstIterator());
		checkIterator("Iterator at init time without zero time switch",
						*rsamCMI, false, true, Argument(t0), Argument(t0Next), minAtt, mapping);

		delete rsamCMI;
		rsamCMI = 0;
	}

protected:
	void runTests() {
		// Radio
		std::cout << "------ Testing stuff for the Radio ------" << std::endl;
		testRadioConstructor();
		testRadioFunctionality();

		// RadioStateAnalogueModel
		std::cout << "------ Testing stuff for the RadioStateAnalogueModel ------" << std::endl;
		fillReferenceLists();
		testRSAMConstructor();
		testRSAMModification();

		//RSAMMapping
		std::cout << "------ Testing stuff for RSAMMapping ------" << std::endl;
		testGetValue();

		//RSAMConstMappingIterator
		std::cout << "------ Testing stuff for RSAMConstMappingIterator ------" << std::endl;
		testRSAMConstMappingIterator();

		testsExecuted = true;
	}
};

Define_Module(RadioStateTest);
