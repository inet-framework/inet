#ifndef PHYUTILS_H_
#define PHYUTILS_H_

#include <cassert>
#include <list>
#include <omnetpp.h>

#include "INETDefs.h"
#include "IRadio.h"
#include "AnalogueModel.h"
#include "Mapping.h"

class RSAMMapping;

/**
 * @brief This special AnalogueModel provides filtering of a Signal
 * according to the actual RadioStates the Radio were in during
 * the Signal's time interval
 *
 * This AnalogueModel models the effects of the radio state on the
 * received signal. This effects are for example total attenuation
 * during the time the radio is not in receiving state. And no
 * attenuation at all if it is in receiving state.
 *
 * This way it is assured that the received power calculated for
 * a Signal by use of the AnalogueModels really is the power received,
 * means if the radio is not in receiving state then this is reflected
 * by the received power of the signal. Therefore the Decider which
 * evaluates the receiving power doesn't have to care about if the
 * radio was in correct state to actually receive a signal. It
 * just has to check if the receiving power was/is high enough.
 *
 * A state-machine-diagram for Radio, RadioStateAnalogueModel and ChannelInfo showing
 * how they work together under control of BasePhyLayer as well as some documentation
 * on how RadioStateAnalogueModel works is available in @ref phyLayer.
 *
 * @ingroup phyLayer
 */
class INET_API RadioStateAnalogueModel : public AnalogueModel
{
	friend class RSAMMapping;
	friend class RSAMConstMappingIterator;

protected:

	/**
	 * @brief Data structure for the list elements.
	 *
	 * Consists basically of a pair of a simtime_t and a double
	 * value (simple time-stamp).
	 */
	class ListEntry
	{
	protected:
		/** @brief The pair representing the time stamp.*/
		std::pair<simtime_t, Argument::mapped_type> basicTimestamp;

	public:
		/** @brief Initializes the entry with the passed values.*/
		ListEntry(simtime_t_cref time, Argument::mapped_type_cref value)
			: basicTimestamp(time, value)
		{}

		virtual ~ListEntry() {}

		/** @brief Returns the time of the entry.*/
		simtime_t_cref getTime() const {
			return basicTimestamp.first;
		}

		/** @brief Sets the time of the entry.*/
		void setTime(simtime_t_cref time) {
			basicTimestamp.first = time;
		}

		/** @brief Returns the value of the entry.*/
		Argument::mapped_type_cref getValue() const {
			return basicTimestamp.second;
		}

		/** @brief Sets the value of the entry.*/
		void setValue(Argument::mapped_type_cref value) {
			basicTimestamp.second = value;
		}

		/**
		 * @brief overload of operator < for class ListEntry to be able to use the STL Algorithms
		 * "lower_bound" and "upper_bound"
		 */
		friend bool operator<(const ListEntry& e, simtime_t_cref t) {
			return (e.getTime() < t);
		}

		/**
		 * @brief overload of operator < for class ListEntry to be able to use the STL Algorithms
		 * "lower_bound" and "upper_bound"
		 */
		friend bool operator<(simtime_t_cref t, const ListEntry& e) {
			return (t < e.getTime());
		}

		/**
		 * @brief overload of operator < for class ListEntry to be able to use the STL Algorithms
		 * "lower_bound" and "upper_bound"
		 */
		friend bool operator<(const ListEntry& left, const ListEntry& right) {
			return (left.getTime() < right.getTime());
		}
	};


	/**
	 * @brief Indicator variable whether we are currently tracking changes
	 */
	bool currentlyTracking;

public:
	/** @brief The type to hold the attenuation's over time. */
	typedef std::list<ListEntry> time_attenuation_collection_type;
	/** @brief Data structure to track the Radios attenuation over time.*/
	time_attenuation_collection_type radioStateAttenuation;

public:

	/**
	 * @brief Standard constructor for a RadioStateAnalogueModel instance
	 *
	 * Default setting is: tracking off
	 */
	RadioStateAnalogueModel( Argument::mapped_type_cref initValue
	                       , bool                       currentlyTracking = false
	                       , simtime_t_cref             initTime = SIMTIME_ZERO )
		: AnalogueModel()
		, currentlyTracking(currentlyTracking)
		, radioStateAttenuation()
	{
		// put the initial time-stamp to the list
		radioStateAttenuation.push_back(ListEntry(initTime, initValue));
	}

	virtual ~RadioStateAnalogueModel() {}

	/**
	 * @brief Filters the AirFrame's Signal according to the RadioState (passively),
	 * i.e. adding an appropriate instance of RSAMMapping to the Signal
	 *
	 * The Signal is added a new RSAMMapping that has a pointer to
	 * this instance RadioStateAnalogueModel, hence the pointer is valid as long
	 * as the Radio instance exists that has this RSAM as a member.
	 */
	virtual void filterSignal(DetailedRadioFrame*, const Coord&, const Coord&);

	/**
	 * @brief sets tracking mode
	 */
	void setTrackingModeTo(bool b) {
		currentlyTracking = b;
	}

	/**
	 * @brief Cleans up all stored information strictly before the given time-point,
	 * i.e. all elements with their time-point strictly smaller than given key. That
	 * means multiple entries with same time are preserved.
	 *
	 * Intended to be used by the PhyLayer
	 *
	 * THIS SHOULD BE THE ONLY WAY TO DELETE ENTRIES IN THE RECEIVING LIST
	 */
	void cleanUpUntil(simtime_t_cref t);

	/**
	 * @brief Stores an entry of the form "time-point/attenuation (from now on)"
	 *
	 * Intended to be used by the Radio
	 */
	void writeRecvEntry(simtime_t_cref time, Argument::mapped_type_cref value);

};
// end class RadioStateAnalogueModel

/**
 * @brief The class that represents the Radio as a state machine.
 *
 * The Radio creates and updates its corresponding RadioStateAnalogueModel, that
 * is a filter representing the radios attenuation to a Signal depending on the
 * RadioState over time.
 *
 * For this basic version we assume a minimal attenuation when the Radio is in
 * state RX, and a maximum attenuation otherwise.
 *
 * A state-machine-diagram for Radio, RadioStateAnalogueModel and ChannelInfo showing
 * how they work together under control of BasePhyLayer as well as some documentation
 * on how RadioStateAnalogueModel works is available in @ref phyLayer.
 *
 * @ingroup phyLayer
 */
class INET_API MiximRadio
{
  protected:

	/** @brief Output vector for radio states.*/
	cOutVector radioStates;
	/** @brief Output vector for radio channels.*/
	cOutVector radioChannels;

	/** @brief The current state the radio is in.*/
	int state;
	/** @brief The state the radio is currently switching to.*/
	int nextState;

	/** @brief The number of radio states this Radio can be in.*/
	const int numRadioStates;
	/** @brief Array for storing switch-times between states.*/
	simtime_t** swTimes;

	/** @brief Constant to store the minimum attenuation for a Radio instance.*/
	const Argument::mapped_type minAtt;
	/** @brief Constant to store the maximum attenuation for a Radio instance.*/
	const Argument::mapped_type maxAtt;

	/**
	 * @brief The corresponding RadioStateAnalogueModel.
	 *
	 * Depends on the characteristics of the radio
	 */
	RadioStateAnalogueModel rsam;

	/** @brief Number of available channels. */
	int nbChannels;

private:
	/** @brief Copy constructor is not allowed.
	 */
	MiximRadio(const MiximRadio&);
	/** @brief Assignment operator is not allowed.
	 */
	MiximRadio& operator=(const MiximRadio&);

public:

	/**
	 * @brief Creates a new instance of this class.
	 *
	 * Since Radio hasn't a public constructor this is the only
	 * way to create an instance of this class.
	 *
	 * This method assures that the radio is initialized with the
	 * correct number of radio states. Sub classing Radios should also
	 * define a factory method like this instead of an public constructor.
	 */
	static MiximRadio* createNewRadio(bool recordStats = false,
                                     int initialState = IRadio::RADIO_MODE_OFF,
                                     Argument::mapped_type_cref minAtt = Argument::MappedOne,
                                     Argument::mapped_type_cref maxAtt = Argument::MappedZero,
                                     int nbChannels=1)
	{
		return new MiximRadio(IRadio::RADIO_MODE_SWITCHING + 1,
                                 recordStats,
                                 initialState,
                                 minAtt, maxAtt,
                                 nbChannels);
	}

	/**
	 * @brief Destructor for the Radio class
	 */
	virtual ~MiximRadio();

	/**
	 * @brief A function called by the Physical Layer to start the switching process to a new RadioState
	 *
	 * @return	-1: Error code if the Radio is currently switching
	 * 			else: switching time from the current RadioState to the new RadioState
	 *
	 *
	 * The actual simtime must be passed, to create properly RSAMEntry
	 */
	virtual simtime_t switchTo(int newState, simtime_t_cref now);

	/**
	 * @brief function called by PhyLayer in order to make an entry in the switch times matrix,
	 * i.e. set the time for switching from one state to another
	 *
	 */
	virtual void setSwitchTime(int from, int to, simtime_t_cref time);

	/**
	 * @brief Returns the state the Radio is currently in
	 */
	virtual int getCurrentState() const {
		return state;
	}

	/**
	 * @brief called by PhyLayer when duration-time for the
	 * current switching process is up
	 *
	 * Radio checks whether it is in switching state (pre-condition)
	 * and switches to the target state
	 *
	 * The actual simtime must be passed, to create properly RSAMEntry
	 */
	virtual void endSwitch(simtime_t_cref now);

	/**
	 * @brief Returns a pointer to the RadioStateAnalogueModel
	 *
	 * This method is intended to be used by the PhyLayer to obtain a pointer
	 * to the corresponding RSAM to this Radio
	 */
	virtual RadioStateAnalogueModel* getAnalogueModel() {
		return (&rsam);
	}

	/**
	 * @brief discards information in the RadioStateAnalogueModel before given time-point
	 *
	 */
	virtual void cleanAnalogueModelUntil(simtime_t_cref t) {
		rsam.cleanUpUntil(t);
	}

	/**
	 * @brief sets tracking mode
	 */
	void setTrackingModeTo(bool b) {
		rsam.setTrackingModeTo(b);
	}

	/**
	 * @brief Returns the number of channels.
	 * @return The number of channels.
	 */
	int getNbChannels() const {
		return nbChannels;
	}

protected:
	/**
	 * @brief Intern constructor to initialize the radio.
	 *
	 * By defining no default constructor we assure that sub classing radios
	 * have to explicitly call this constructor which assures they pass
	 * the correct number of radio states.
	 *
	 * The protected constructor + factory method solution assures that
	 * while sub-classing Radios HAVE to explicitly pass their correct amount
	 * of radio states to this constructor, the user (creator) of the Radio
	 * doesn't has to pass it or even know about it (which wouldn't be possible
	 * with a public constructor).
	 * Therefore sub classing Radios which could be sub-classed further should
	 * also do it this way.
	 */
	MiximRadio(int numRadioStates,
		  bool recordStats,
		  int initialState = IRadio::RADIO_MODE_OFF,
		  Argument::mapped_type_cref minAtt = Argument::MappedOne, Argument::mapped_type_cref maxAtt = Argument::MappedZero,
		  int nbChannels = 1);

	/**
	 * @brief responsible for making entries to the RSAM
	 */
	virtual void makeRSAMEntry(simtime_t_cref time, int state)
	{
		rsam.writeRecvEntry(time, mapStateToAtt(state));
	}

	/**
	 * @brief maps RadioState to attenuation, the Radios receiving characteristic
	 *
	 */
	virtual Argument::mapped_type_cref mapStateToAtt(int state)
	{
		if (state == IRadio::RADIO_MODE_RECEIVER) {
			return minAtt;
		}
		else {
			return maxAtt;
		}
	}
};
// end class MiximRadio

/**
 * @brief ConstMapingIterator implementation for a RSAM
 *
 * @ingroup phyLayer
 */
class INET_API RSAMConstMappingIterator : public ConstMappingIterator
{
protected:

	/** @brief Pointer to the RSAM module.*/
	const RadioStateAnalogueModel* rsam;

	/** @brief List iterator pointing to the current position.*/
	RadioStateAnalogueModel::time_attenuation_collection_type::const_iterator it;

	/** @brief The current position of this iterator.*/
	Argument position;
	/** @brief The next position the iterator will jump to.*/
	Argument nextPosition;

	/** @brief The start time of the signal this iterators mapping attenuates.*/
	simtime_t signalStart;
	/** @brief The end time of the signal this iterators mapping attenuates.*/
	simtime_t signalEnd;

private:
	/** @brief Copy constructor is not allowed.
	 */
	RSAMConstMappingIterator(const RSAMConstMappingIterator&);
	/** @brief Assignment operator is not allowed.
	 */
	RSAMConstMappingIterator& operator=(const RSAMConstMappingIterator&);

public:

	/** @brief Initializes the iterator with the passed values.*/
	RSAMConstMappingIterator(const RadioStateAnalogueModel* rsam,
							 simtime_t_cref signalStart,
							 simtime_t_cref signalEnd);

	virtual ~RSAMConstMappingIterator() {}

	/**
	 * @brief Lets the iterator point to the passed position.
	 *
	 * The passed new position can be at arbitary places, jumping explicitly
	 * before signalStart is allowed.
	 */
	virtual void jumpTo(const Argument& pos);

	/**
	 * @brief Helper function that sets member nextPosition. Presumes that
	 * iterator it and member position are set correctly.
	 *
	 * This function does not care for zero time switches!
	 * This must be done before!
	 *
	 * Might be helpful if position of iterator it has not been set
	 * by upper_bound before (it has not just been standing on the "nextPosition").
	 */
	virtual void setNextPosition();

	/**
	 * @brief Lets the iterator point to the begin of the function.
	 *
	 * The beginning of the function depends on the implementation.
	 */
	virtual void jumpToBegin() {
		jumpTo(signalStart);
	}

	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 */
	virtual void iterateTo(const Argument& pos);

	/**
	 * @brief Iterates to the next position of the function.
	 *
	 * The next position depends on the implementation of the
	 * Function.
	 */
	virtual void next() {
		iterateTo(nextPosition);
	}

	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 *
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 */
	virtual bool inRange() const;

	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range.
	 */
	virtual bool hasNext() const;

	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const {
		return position;
	}

	virtual const Argument& getNextPosition() const {
		return nextPosition;
	}

	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 */
	virtual argument_value_t getValue() const {
		return it->getValue();
	}

	/**
	 * @brief Iterates to valid entry for timepoint t over all zero-time switches
	 * starting from the current position of iterator it
	 */
	virtual void iterateToOverZeroSwitches(simtime_t_cref t);

}; // end class RSAMConstMappingIterator

/**
 * @brief This class represents the corresponding mapping
 * to a RadioStateAnalogueModel (RSAM).
 *
 * Since RSAM is modified dynamically over time,
 * RSAMMapping doesn't store the information itself, but
 * interacts with its RSAM to obtain information about the mapping.
 *
 * The relation to RSAM is very tight. RSAM creates an appropriate RSAMMapping
 * by passing a self-pointer to the constructor call.
 *
 * class RSAMMapping is a friend of class RadioStateAnalogueModel
 *
 *
 * @ingroup phyLayer
 */
class INET_API RSAMMapping : public ConstMapping
{
protected:

	/** @brief Pointer to the RSAM module.*/
	const RadioStateAnalogueModel* rsam;
	/** @brief Start of the signal this mapping defines attenuation for.*/
	simtime_t signalStart;
	/** @brief End of the signal this mapping defines attenuation for.*/
	simtime_t signalEnd;

public:
	/**
	 * @brief Copy constructor.
	 */
	RSAMMapping(const RSAMMapping& o)
		: ConstMapping(o)
		, rsam(o.rsam)
		, signalStart(o.signalStart)
		, signalEnd(o.signalEnd)
	{ }

	/**
	 *  @brief  %RSAMMapping assignment operator.
	 *  @param  copy  A %RSAMMapping of identical element and allocator types.
	 *
	 *  All the elements of @a copy are copied.
	 */
	RSAMMapping& operator=(const RSAMMapping& copy) {
		RSAMMapping tmp(copy);

		swap(tmp);
		return *this;
	}

	/**
	 *  @brief  Swaps data with another %RSAMMapping.
	 *  @param  s  A %RSAMMapping of the same element and allocator types.
	 *
	 *  This exchanges the elements between two RSAMMapping's in constant time.
	 *  Note that the global std::swap() function is specialized such that
	 *  std::swap(s1,s2) will feed to this function.
	 */
	void swap(RSAMMapping& s) {
		ConstMapping::swap(s);
		std::swap(rsam,        s.rsam);
		std::swap(signalStart, s.signalStart);
		std::swap(signalEnd,   s.signalEnd);
	}

public:
	/**
	 * @brief Constructor taking a pointer to the corresponding RSAM
	 *
	 */
	RSAMMapping(const RadioStateAnalogueModel* rsam,
				simtime_t_cref signalStart,
				simtime_t_cref signalEnd) :
		ConstMapping(),
		rsam(rsam),
		signalStart(signalStart),
		signalEnd(signalEnd)
	{
		assert(rsam);
		assert( !(signalStart < rsam->radioStateAttenuation.front().getTime()) );
	}

	virtual ~RSAMMapping() {}

	/**
	 * @brief Returns the value of this Function at position specified
	 * by the passed Argument. Zero-time-switches are ignored here,
	 * i.e. in case of multiple entries at the same time-point, the last one
	 * is significant.
	 *
	 * In this case we have a function: simtime_t -> attenuation
	 *
	 */
	virtual argument_value_t getValue(const Argument& pos) const;

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function.
	 */
	virtual ConstMappingIterator* createConstIterator() const
	{
		return new RSAMConstMappingIterator(rsam, signalStart, signalEnd);
	}

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function. The iterator starts at the passed position.
	 *
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) const;

	virtual ConstMapping* constClone() const {
		return new RSAMMapping(*this);
	}

}; // end class RSAMMapping


#endif /*PHYUTILS_H_*/
