#ifndef CHANNELINFO_H_
#define CHANNELINFO_H_

#include <list>
#include <map>
#include <omnetpp.h>

#include "INETDefs.h"
#include "DetailedRadioFrame.h"

/**
 * @brief This class is used by the BasePhyLayer to keep track of the AirFrames
 * on the channel.
 *
 * ChannelInfo is able to return every AirFrame which intersects with a
 * specified interval. This is mainly used to get the noise for a received
 * signal.
 *
 * ChannelInfo is a passive class meaning the user has to tell it when a new
 * AirFrame starts and an existing ends.
 *
 * Once an AirFrame has been added to the ChannelInfo the ChannelInfo holds the
 * ownership of this AirFrame even if the AirFrame is removed again from the
 * ChannelInfo. This is necessary because the ChannelInfo has to be able to
 * store also the AirFrames which are over but still intersect with an currently
 * running AirFrame.
 *
 * Note: ChannelInfo assumes that the AirFrames are added and removed
 * 		 chronologically. This means every time you add an AirFrame with a
 * 		 specific start time ChannelInfo assumes that start time as the current
 * 		 time and assumes that every following action happens after that moment.
 * 		 The same goes for "removeAirFrame". When removing an AirFrames,
 * 		 ChannelInfo assumes the start time plus the duration of the AirFrame as
 * 		 the current time.
 * 		 This also affects "getAirFrames" in the way that you may only ask for
 * 		 intervals which lie before the "current time" of ChannelInfo.
 *
 * A state-machine-diagram for Radio, RadioStateAnalogueModel and ChannelInfo
 * showing how they work together under control of BasePhyLayer as well as some
 * documentation on how RadioStateAnalogueModel works is available
 * in @ref phyLayer.
 *
 * @ingroup phyLayer
 */
class INET_API ChannelInfo {
public:
	/** @brief Invalid simulation time point, will be used for return value of emptiness. */
	static const_simtime_t invalidSimTime;

	/** @brief Functor which can be used as to filter for specific air frames.
	 */
	struct airframe_filter_fctr {
		virtual ~airframe_filter_fctr() {}

		/** @brief Return true if the air frame fits your criteria.
		 *
		 * @param a The air frame which should be checked.
		*/
		virtual bool pass(const DetailedRadioFrame* a) const {
			return true;
		}
    };
protected:
	/**
	 * The AirFrames are stored in a Matrix with start- and end time as
	 * dimensions.
	 */
	typedef std::map<simtime_t, std::multimap<simtime_t, DetailedRadioFrame*> > AirFrameMatrix;

	struct c_min_start_time_fctr {
		bool operator() (const AirFrameMatrix::value_type& a, const AirFrameMatrix::value_type& b) {
			return a.second.key_comp()(a.second.begin()->first, b.second.begin()->first);
		}
	};

	/**
	 * @brief Iterator for every intersection of a specific interval in a
	 * AirFrameMatrix.
	 *
	 * A time interval A_start to A_end intersects with another interval B_start
	 * to B_end iff the following two conditions are fulfilled:
	 *
	 * 		1. A_end >= B_start.
	 * 		2. A_start <= B_end and
	 *
	 * Or the defined by the
	 * opposite: The two intervals do not intersect iff A_end < B_start or
	 * A_start > B_end.
	 *
	 * To iterate over a two dimensional (end-time x start-time) container
	 * of AirFrames this class iterates over all AirFrame-lists whose end-time
	 * is bigger or equal than the interval start (intersect condition 1) and in
	 * each of these lists iterates over all entries but only stops at and
	 * returns entries which start-time is smaller or equal than the interval
	 * end (intersection condition 2).
	 *
	 * In template form to work as const- and non-const iterator.
	 */
	template<typename C, typename ItMatrix = typename C::const_iterator, typename ItSubMatrix = typename C::mapped_type::const_iterator>
	class BaseIntersectionIterator
	{
	public:
		typedef typename ItSubMatrix::value_type value_type;
		typedef typename ItSubMatrix::reference  reference;
		typedef typename ItSubMatrix::pointer    pointer;

		typedef BaseIntersectionIterator<C, ItMatrix, ItSubMatrix> iterator;

		typedef std::bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t                       difference_type;

	protected:
		friend class ChannelInfo;
		typedef BaseIntersectionIterator<C, ItMatrix, ItSubMatrix> _Self;
		//typedef _Rb_tree_node_base::_Const_Base_ptr _Base_ptr;
		//typedef const _Rb_tree_node<_Tp>*           _Link_type;

		/** @brief Pointer to the matrix holding the intervals.*/
		C* intervals;

		/** @brief Point in time to start iterating over intersections.*/
		simtime_t_cref from;

		/** @brief Point in time to end iterating over intersections.*/
		simtime_t_cref to;

		/** @brief Iterator over AirFrame end times.*/
		ItMatrix endIt;

		/** @brief Iterator over AirFrame start times.*/
		ItSubMatrix startIt;
		ItSubMatrix startItEnd;

		/** Jumps top next valid entry.
		 *
		 * ATTENTION: startIt and startItEnd must be set!
		 */
	    _Self& jumpToNextValid(ItMatrix endItEnd)
	    {
			for (; endIt != endItEnd; startIt = endIt->second.begin()) {
				// while there are entries left at the current end-time
				if (startIt != startItEnd) {
					// check if this entry fulfills the intersection condition
					// 2 (condition 1 is already fulfilled in the constructor by
					// the start-value of the end-time-iterator)
					return *this;
				}
				if (++endIt != endItEnd) {
					startItEnd = endIt->second.upper_bound(to);
				}
			}

			intervals = NULL;
			return *this;
	    }
	public:
		/**
		 * @brief Creates an iterator for the specified interval at the
		 * specified AirFrameMatrix.
		 */
		BaseIntersectionIterator(C* airFrames, simtime_t_cref from, simtime_t_cref to) :
			intervals(airFrames), from(from), to(to), endIt(), startIt(), startItEnd()
		{
			// begin at the smallest end-time-entry fulfilling the intersection
			// condition 1
			if (intervals) {
				endIt = intervals->lower_bound(from);
				if(endIt != intervals->end()) {
					startIt    = endIt->second.begin();
					startItEnd = endIt->second.upper_bound(to);
					jumpToNextValid(intervals->end());
				}
				else {
					intervals = NULL;
					startIt   = startItEnd;
				}
			}
			else {
				startIt = startItEnd;
			}
			//we are already pointing at the first unchecked interval
		}
		BaseIntersectionIterator(const iterator& __it) :
			intervals(__it.intervals), from(__it.from), to(__it.to), endIt(__it.endIt), startIt(__it.startIt),  startItEnd(__it.startItEnd)
		{ }

		reference
	    operator*() const
		{
			if(intervals == NULL || endIt == intervals->end() || startIt == startItEnd)
				return NULL;
			return startIt.operator*();
		}

		pointer
	    operator->() const
		{
			if(intervals == NULL || endIt == intervals->end() || startIt == startItEnd)
				return NULL;
			return startIt.operator->();
		}

		/**  @brief Increases the iterator to the next intersecting AirFrame.
		 */
		_Self&
		operator++()
		{
			if (!intervals)
				return *this;

			const ItMatrix endItEnd = intervals->end();
			if(endIt == endItEnd) {
				intervals = NULL;
				return *this;
			}

			if (startIt != startItEnd)
				++startIt;

			return jumpToNextValid(endItEnd);
		}

		/**  @brief Increases the iterator to the next intersecting AirFrame.
		 */
		_Self
		operator++(int)
	    {
			_Self __tmp = *this;
			++(*this);
			return __tmp;
	    }

		/**  @brief Compares two iterators for equality.
		 */
	    bool
	    operator==(const _Self& __x) const
	    {
	    	if (intervals == NULL) {
	    		if (__x.intervals == NULL)
	    			return true;
	    		return (__x.endIt == __x.intervals->end() || __x.startIt == __x.startItEnd); // we have no next element
	    	}
	    	if (__x.intervals == NULL) {
	    		return (endIt == intervals->end() || startIt == endIt->second.end()); // we have no next element
	    	}
	    	if( (endIt == intervals->end() && __x.endIt == __x.intervals->end()) || (startIt == startItEnd && __x.startIt == __x.startItEnd) )
	    		return true;
	    	return intervals == __x.intervals && from == __x.from && to == __x.to && endIt == __x.endIt && startIt == __x.startIt && startItEnd == __x.startItEnd;
	    }

		/**  @brief Compares two iterators for inequality.
		 */
	    bool
	    operator!=(const _Self& __x) const
	    { return !(*this == __x); }
	};

	/** @brief Type for a const-iterator over an AirFrame interval matrix.*/
	typedef BaseIntersectionIterator<const AirFrameMatrix,
									 AirFrameMatrix::const_iterator,
									 AirFrameMatrix::mapped_type::const_iterator> const_iterator;

	/** @brief The end iterator for the matrix.*/
	const static const_iterator cConstItEnd;

    /**
     *  Returns a read-only (constant) iterator that points one past the last
     *  pair in the AirFrameMatrix.  Iteration is done in ascending order according to
     *  the end, and start time.
     */
	const_iterator end() const {
		return cConstItEnd;
	}

	/** @brief Type for a iterator over an AirFrame interval matrix.*/
	typedef BaseIntersectionIterator<AirFrameMatrix,
									 AirFrameMatrix::iterator,
									 AirFrameMatrix::mapped_type::iterator> iterator;

	/** @brief The end iterator for the matrix.*/
	const static iterator cItEnd;
    /**
     *  Returns a read/write iterator that points one past the last
     *  pair in the AirFrameMatrix.  Iteration is done in ascending order according to
     *  the end, and start time.
     */
	iterator end() {
		return cItEnd;
	}

    /**
     *  @brief Erases an element from a AirFrameMatrix.
     *  @param  position  An iterator pointing to the element to be erased.
     *
     *  This function erases an element, pointed to by the given
     *  iterator, from a AirFrameMatrix.  Note that this function only erases
     *  the element, and that if the element is itself a pointer,
     *  the pointed-to memory is not touched in any way.  Managing
     *  the pointer is the user's responsibility.
     */
	void erase(const iterator& __position) {
		assert(__position.endIt   != __position.intervals->end());
		assert(__position.startIt != __position.startItEnd);

		//erase AirFrame from list
		__position.endIt->second.erase(__position.startIt);

		//check if we deleted the only entry in the list
		if(__position.endIt->second.empty()) {
			__position.intervals->erase(__position.endIt); //delete list from map
		}
	}

	/**
	 * @brief Stores the currently active AirFrames.
	 *
	 * This means every AirFrame which was added but not yet removed.
	 */
	AirFrameMatrix activeAirFrames;

	/**
	 * @brief Stores inactive AirFrames.
	 *
	 * This means every AirFrame which has been already removed but still is
	 * needed because it intersect with one or more active AirFrames.
	 */
	AirFrameMatrix inactiveAirFrames;

	/** @brief Type for a map of AirFrame pointers to their start time.*/
	typedef std::map<long, simtime_t> AirFrameStartMap;

	/** @brief Stores the start time of every AirFrame.*/
	AirFrameStartMap airFrameStarts;

	/** @brief Stores a point in history up to which we need to keep all channel
	 * information stored (value should be less than 0.0).*/
	simtime_t recordStartTime;

public:
	/**
	 * @brief Type for a container of AirFrames.
	 *
	 * Used as out type for "getAirFrames" method.
	 */
	typedef std::list<DetailedRadioFrame*> AirFrameVector;

protected:
	/**
	 * @brief Asserts that every inactive AirFrame is still intersecting with at
	 * least one active airframe or with the current record start time.
	 */
	void assertNoIntersections() const;


	/**
	 * @brief Returns every AirFrame of an AirFrameMatrix which intersect with a
	 * given interval.
	 *
	 * The intersecting AirFrames are stored in the AirFrameVector reference
	 * passed as parameter.
	 */
	static void getIntersections( const AirFrameMatrix&     airFrames
                                , simtime_t_cref            from
                                , simtime_t_cref            to
                                , AirFrameVector&           outVector
                                , airframe_filter_fctr *const fctrFilter = NULL)
	{
	    const_iterator itEnd = cConstItEnd;
	    for (const_iterator it(&airFrames, from, to); it != itEnd; ++it) {
	        if (fctrFilter != NULL) {
	            if (!fctrFilter->pass(it->second))
	                continue;
	        }
	        outVector.push_back(it->second);
	    }
	}

	/**
	 * @brief Returns true if there is at least one AirFrame in the passed
	 * AirFrameMatrix which intersect with the given interval.
	 */
	static bool isIntersecting( const AirFrameMatrix& airFrames,
	                            simtime_t_cref from, simtime_t_cref to );

	/**
	 * @brief Moves a previously active AirFrame to the inactive AirFrames.
	 *
	 * This methods checks if there are some inactive AirFrames which can be
	 * deleted because the AirFrame to in-activate was the last one they
	 * intersected with.
	 * It also checks if the AirFrame to in-activate still intersect with at
	 * least one active AirFrame before it is moved to inactive AirFrames.
	 */
	void addToInactives(DetailedRadioFrame* a, simtime_t_cref startTime, simtime_t_cref endTime);

	/**
	 * @brief Deletes an AirFrame from an AirFrameMatrix.
	 */
	static void deleteAirFrame(AirFrameMatrix& airFrames,
	                           DetailedRadioFrame*  a,
	                           simtime_t_cref  startTime, simtime_t_cref endTime);

	/**
	 * @brief Returns the start time of the odlest AirFrame on the channel.
	 */
	simtime_t findEarliestInfoPoint(simtime_t_cref returnTimeIfEmpty = invalidSimTime) const;

	/**
	 * @brief Checks if any information inside the passed interval can be
	 * discarded.
	 *
	 * This method should be called every time the information for a certain
	 * interval changes (AirFrame is removed or record time changed).
	 *
	 * @param startTime The start of the interval to check
	 * @param endTime The end of the interval to check
	 */
	void checkAndCleanInterval(simtime_t_cref startTime, simtime_t_cref endTime);

	/**
	 * @brief Returns true if all information inside the passed interval can be
	 * deleted.
	 *
	 * For example this method is used to check if information for the duration
	 * of an AirFrame is needed anymore and if not the AirFrame is deleted.
	 *
	 * @param startTime The start time of the interval (e.g. AirFrame start)
	 * @param endTime The end time of the interval (e.g. AirFrame end)
	 * @return returns true if any information for the passed interval can be
	 * discarded.
	 */
	bool canDiscardInterval(simtime_t_cref startTime, simtime_t_cref endTime);

	/**
	 * @brief Checks if any information up from the passed start time can be
	 * discarded.
	 *
	 * @param startTime The start of the interval to check
	 */
	void checkAndCleanFrom(simtime_t_cref start) {
		//nothing to do
		if(inactiveAirFrames.empty())
			return;

		//take last ended inactive airframe as end of interval
		checkAndCleanInterval(start, inactiveAirFrames.rbegin()->first);
	}

public:
	ChannelInfo()
		: activeAirFrames()
		, inactiveAirFrames()
		, airFrameStarts()
		, recordStartTime(invalidSimTime)
	{}

	virtual ~ChannelInfo() {}

	/**
	 * @brief Tells the ChannelInfo that an AirFrame has started.
	 *
	 * From this point ChannelInfo gets the ownership of the AirFrame.
	 *
	 * parameter startTime holds the time the receiving of the AirFrame has
	 * started in seconds.
	 */
	void addAirFrame(DetailedRadioFrame* a, simtime_t_cref startTime);

	/**
	 * @brief Tells the ChannelInfo that an AirFrame is over.
	 *
	 * This does not mean that it loses ownership of the AirFrame.
	 *
	 * @return The current time-point from on which information concerning
	 * AirFrames is needed to be stored.
	 */
	simtime_t removeAirFrame(DetailedRadioFrame* a, simtime_t_cref returnTimeIfEmpty = invalidSimTime);

	/**
	 * @brief Fills the passed AirFrameVector reference with the AirFrames which
	 * intersect with the given time interval.
	 *
	 * Note: Completeness of the list of AirFrames for specific interval can
	 * only be assured if start and end point of the interval lies inside the
	 * duration of at least one currently active AirFrame.
	 * An AirFrame is called active if it has been added but not yet removed
	 * from ChannelInfo.
	 */
	void getAirFrames( simtime_t_cref            from
                     , simtime_t_cref            to
                     , AirFrameVector&           out
                     , airframe_filter_fctr *const fctrFilter = NULL) const
	{
	    //check for intersecting inactive AirFrames
	    getIntersections(inactiveAirFrames, from, to, out, fctrFilter);

	    //check for intersecting active AirFrames
	    getIntersections(activeAirFrames, from, to, out, fctrFilter);
	}

	/**
	 * @brief Returns the current time-point from that information concerning
	 * AirFrames is needed to be stored.
	 */
	simtime_t getEarliestInfoPoint(simtime_t_cref returnTimeIfEmpty = invalidSimTime) const
	{
		return findEarliestInfoPoint(returnTimeIfEmpty);
	}

	/**
	 * @brief Tells ChannelInfo to keep from now on all channel information
	 * until the passed time in history.
	 *
	 * The passed start-time should be the current simulation time, otherwise
	 * ChannelInfo can't assure that it hasn't already thrown away some
	 * information for that passed time period.
	 *
	 * Subsequent calls to this method will update the recording start time and
	 * information from old start times is thrown away.
	 *
	 * @param start The point in time from which to keep all channel information
	 * stored.
	 */
	void startRecording(simtime_t_cref start)
	{
		// clean up until old record start
		if(recordStartTime >= SIMTIME_ZERO) {
			recordStartTime = start;
			checkAndCleanInterval(0, recordStartTime);
		} else {
			recordStartTime = start;
		}
	}

	/**
	 * @brief Tells ChannelInfo to stop recording Information from now on.
	 *
	 * Its up to ChannelInfo to decide when to actually throw away the
	 * information it doesn't need to store anymore now.
	 */
	void stopRecording()
	{
		if(recordStartTime >= SIMTIME_ZERO) {
			simtime_t old = recordStartTime;
			recordStartTime = invalidSimTime;
			checkAndCleanFrom(old);
		}
	}

	/**
	 * @brief Returns true if ChannelInfo is currently recording.
	 * @return true if ChannelInfo is recording
	 */
	bool isRecording() const
	{
		return recordStartTime >= SIMTIME_ZERO;
	}

	/**
	 * @brief Returns true if there are currently no active or inactive
	 * AirFrames on the channel.
	 */
	bool isChannelEmpty() const {
		return airFrameStarts.empty();
	}
};

#endif /*CHANNELINFO_H_*/
