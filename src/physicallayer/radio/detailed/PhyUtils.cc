#include "PhyUtils.h"

#include "DetailedRadioFrame.h"

using namespace std;

void RadioStateAnalogueModel::filterSignal(DetailedRadioFrame* frame, const Coord& /*sendersPos*/, const Coord& /*receiverPos*/)
{
    DetailedRadioSignal&      signal     = frame->getSignal();
	RSAMMapping* attMapping = new RSAMMapping(this, signal.getReceptionStart(), signal.getReceptionEnd());

	signal.addAttenuation(attMapping);
}

void RadioStateAnalogueModel::cleanUpUntil(simtime_t_cref t)
{
	// assert that list is not empty
	assert(!radioStateAttenuation.empty());

	/* the list contains at least one element */

	// CASE: t is smaller or equal the timepoint of the first element ==> nothing to do, return
	if ( t <= radioStateAttenuation.front().getTime() )
	{
		return;
	}

	// CASE: t is greater than the timepoint of the last element
	// ==> clear complete list except the last element, return
	if ( t > radioStateAttenuation.back().getTime() )
	{
		radioStateAttenuation.erase(radioStateAttenuation.begin(), --radioStateAttenuation.end());
		return;
	}

	/*
	 * preconditions from now on:
	 * 1. list contains at least two elements, since 2. + 3.
	 * 2. t > first_timepoint
	 * 3. t <= last_timepoint
	 */

	// get an iterator and set it to the first timepoint >= t
	time_attenuation_collection_type::iterator it =
	        lower_bound(radioStateAttenuation.begin(), radioStateAttenuation.end(), t);

	// CASE: list contains an element with exactly the given key
	if ( it != radioStateAttenuation.end() && !(t < *it) )
	{
		radioStateAttenuation.erase(radioStateAttenuation.begin(), it);
		return;
	}

	// CASE: t is "in between two elements"
	// ==> set the iterators predecessors time to t, it becomes the first element
	--it; // go back one element, possible since this one has not been the first one

	it->setTime(t); // set this elements time to t
	radioStateAttenuation.erase(radioStateAttenuation.begin(), it); // and erase all previous elements
}

void RadioStateAnalogueModel::writeRecvEntry(simtime_t_cref time, Argument::mapped_type_cref value)
{
	// bugfixed on 08.04.2008
	assert( (radioStateAttenuation.empty()) || (time >= radioStateAttenuation.back().getTime()) );

	radioStateAttenuation.push_back(ListEntry(time, value));

	if (!currentlyTracking)
	{
		cleanUpUntil(time);

		assert(radioStateAttenuation.back().getTime() == time);
	}
}

MiximRadio::MiximRadio(int numRadioStates,
			 bool recordStats,
			 int initialState,
			 Argument::mapped_type_cref minAtt, Argument::mapped_type_cref maxAtt,
			 int nbChannels):
	radioStates(), radioChannels(), state(initialState), nextState(initialState),
	numRadioStates(numRadioStates),
	swTimes(NULL),
	minAtt(minAtt), maxAtt(maxAtt),
	rsam(mapStateToAtt(initialState)),
	nbChannels(nbChannels)
{
	assert(nbChannels > 0);

	radioStates.setName("RadioState");
	radioStates.setEnabled(recordStats);
	radioStates.record(initialState);

	// allocate memory for one dimension
	swTimes = new simtime_t* [numRadioStates];

	// go through the first dimension and
	for (int i = 0; i < numRadioStates; i++)
	{
		// allocate memory for the second dimension
		swTimes[i] = new simtime_t[numRadioStates];
	}

	// initialize all matrix entries to 0.0
	for (int i = 0; i < numRadioStates; i++)
	{
		for (int j = 0; j < numRadioStates; j++)
		{
			swTimes[i][j] = 0;
		}
	}
}

MiximRadio::~MiximRadio()
{
	// delete all allocated memory for the switching times matrix
	for (int i = 0; i < numRadioStates; i++)
	{
		delete[] swTimes[i];
	}

	delete[] swTimes;
	swTimes = 0;
}

simtime_t MiximRadio::switchTo(int newState, simtime_t_cref now)
{
	// state to switch to must be in a valid range, i.e. 0 <= newState < numRadioStates
	assert(0 <= newState && newState < numRadioStates);

	// state to switch to must not be SWITCHING
	assert(newState != IRadio::RADIO_MODE_SWITCHING);

	// return error value if newState is the same as the current state
	// if (newState == state) return -1;

	// return error value if MiximRadio is currently switching
	if (state == IRadio::RADIO_MODE_SWITCHING) return -1;


	/* REGULAR CASE */

	// set the nextState to the newState and the current state to SWITCHING
	nextState = newState;
	int lastState = state;
	state = IRadio::RADIO_MODE_SWITCHING;
	radioStates.record(state);

	// make entry to RSAM
	makeRSAMEntry(now, state);

	// return matching entry from the switch times matrix
	return swTimes[lastState][nextState];
}

void MiximRadio::setSwitchTime(int from, int to, simtime_t_cref time)
{
	// assert parameters are in valid range
	assert(time >= 0.0);
	assert(0 <= from && from < numRadioStates);
	assert(0 <= to && to < numRadioStates);

	// it shall not be possible to set times to/from SWITCHING
	assert(from != IRadio::RADIO_MODE_SWITCHING && to != IRadio::RADIO_MODE_SWITCHING);

	swTimes[from][to] = time;
	return;
}

void MiximRadio::endSwitch(simtime_t_cref now)
{
	// make sure we are currently switching
	assert(state == IRadio::RADIO_MODE_SWITCHING);

	// set the current state finally to the next state
	state = nextState;
	radioStates.record(state);

	// make entry to RSAM
	makeRSAMEntry(now, state);

	return;
}

RSAMConstMappingIterator::RSAMConstMappingIterator( const RadioStateAnalogueModel* rsam,
                                                    simtime_t_cref                 signalStart,
                                                    simtime_t_cref                 signalEnd )
	: ConstMappingIterator()
	, rsam(rsam)
	, it()
	, position()
	, nextPosition()
	, signalStart(signalStart)
	, signalEnd(signalEnd)
{
	assert(rsam);

	assert( !(signalStart < rsam->radioStateAttenuation.front().getTime()) );

	jumpToBegin();
}

void RSAMConstMappingIterator::jumpTo(const Argument& pos)
{
	// extract the time-component from the argument
	simtime_t_cref t = pos.getTime();

	assert( !(rsam->radioStateAttenuation.empty()) &&
			!(t < rsam->radioStateAttenuation.front().getTime()) );

	// current position is already correct
	if( t == position.getTime() )
		return;

	// this automatically goes over all zero time switches
	it = upper_bound(rsam->radioStateAttenuation.begin(), rsam->radioStateAttenuation.end(), t);

	--it;
	position.setTime(t);
	setNextPosition();
}

void RSAMConstMappingIterator::setNextPosition()
{
	if (hasNext()) // iterator it does not stand on last entry and next entry is before signal end
	{
		if(position.getTime() < signalStart) //signal start is our first key entry
		{
			nextPosition.setTime(signalStart);
		} else
		{
		    RadioStateAnalogueModel::time_attenuation_collection_type::const_iterator it2 = it;
			++it2;

			assert(it->getTime() <= position.getTime() && position.getTime() < it2->getTime());

			//point in time for the "pre step" of the next real key entry
			simtime_t_cref preTime = MappingUtils::pre(it2->getTime());

			if(position.getTime() == preTime) {
				nextPosition.setTime(it2->getTime());
			}
			else {
				nextPosition.setTime(preTime);
			}
		}

	} else // iterator it stands on last entry or next entry whould be behind signal end
	{
		nextPosition.setTime(MappingUtils::incNextPosition(position.getTime()));
	}
}

void RSAMConstMappingIterator::iterateTo(const Argument& pos)
{
	// extract the time component from the passed Argument
	simtime_t_cref t = pos.getTime();

	// ERROR CASE: iterating to a position before (time) the beginning of the mapping is forbidden
	assert( !(rsam->radioStateAttenuation.empty()) &&
			!(t < rsam->radioStateAttenuation.front().getTime()) );

	assert( !(t < position.getTime()) );

	// REGULAR CASES:
	// t >= position.getTime();

	// we are already exactly there
	if( t == position.getTime() )
		return;

	// we iterate there going over all zero time switches
	iterateToOverZeroSwitches(t);

	// update current position
	position.setTime(t);
	setNextPosition();
}

bool RSAMConstMappingIterator::inRange() const
{
	simtime_t_cref t             = position.getTime();
	simtime_t_cref lastEntryTime = std::max(rsam->radioStateAttenuation.back().getTime(), signalStart);

	return 	signalStart <= t
			&& t <= signalEnd
			&& t <= lastEntryTime;

}

bool RSAMConstMappingIterator::hasNext() const
{
	assert( !(rsam->radioStateAttenuation.empty()) );

	RadioStateAnalogueModel::time_attenuation_collection_type::const_iterator it2 = it;
	if (it2 != rsam->radioStateAttenuation.end())
	{
		++it2;
	}

	return 	position.getTime() < signalStart
			|| (it2 != rsam->radioStateAttenuation.end() && it2->getTime() <= signalEnd);
}

void RSAMConstMappingIterator::iterateToOverZeroSwitches(simtime_t_cref t)
{
    RadioStateAnalogueModel::time_attenuation_collection_type::const_iterator itEnd = rsam->radioStateAttenuation.end();
	if( it != itEnd && !(t < it->getTime()) )
	{
		// and go over (ignore) all zero-time-switches, to the next greater entry (time)
		while( it != itEnd && !(t < it->getTime()) )
			++it;

		// go back one step, here the iterator 'it' is placed right
		--it;
	}
}

RSAMMapping::argument_value_t RSAMMapping::getValue(const Argument& pos) const
{
	// extract the time-component from the argument
	simtime_t_cref t = pos.getTime();

	// assert that t is not before the first timepoint in the RSAM
	// and receiving list is not empty
	assert( !(rsam->radioStateAttenuation.empty()) &&
			!(t < rsam->radioStateAttenuation.front().getTime()) );

	/* receiving list contains at least one entry */

	// set an iterator to the first entry with timepoint > t
	RadioStateAnalogueModel::time_attenuation_collection_type::const_iterator it;
	it = upper_bound(rsam->radioStateAttenuation.begin(), rsam->radioStateAttenuation.end(), t);

	// REGULAR CASE: it points to an element that has a predecessor
	--it; // go back one entry, this one is significant!

	return it->getValue();
}

ConstMappingIterator* RSAMMapping::createConstIterator(const Argument& pos) const
{
	RSAMConstMappingIterator* rsamCMI
		= new RSAMConstMappingIterator(rsam, signalStart, signalEnd);

	rsamCMI->jumpTo(pos);

	return rsamCMI;
}
