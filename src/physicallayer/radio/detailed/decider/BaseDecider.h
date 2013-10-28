/*
 * BaseDecider.h
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#ifndef BASEDECIDER_H_
#define BASEDECIDER_H_

#include "INETDefs.h"
#include "Decider.h"

class Mapping;
class DeciderResult;

/**
 * @brief Provides some base functionality for most common deciders.
 *
 * Forwards the AirFrame from "processSignal" to "processNewSignal",
 * "processSignalHeader" or "processSignalEnd" depending on the
 * state for that AirFrame returned by "getSignalState".
 *
 * Provides answering of ChannelSenseRequests (instantaneous and over time).
 *
 * Subclasses should define when they consider the channel as idle by
 * calling "setChannelIdleStatus" because BaseDecider uses that to
 * answer ChannelSenseRequests.
 *
 * If a subclassing Decider only tries to receive one signal at a time
 * it can use BaseDeciders "currentSignal" member which is a pair of
 * the signal to receive and the state for that signal. The state
 * is then used by BaseDeciders "getSignalState" to decide to which
 * "process***" method to forward the signal.
 * If a subclassing Decider needs states for more than one Signal it
 * has to store these states by itself and should probably override
 * the "getSignalState" method.
 *
 * @ingroup decider
 * @ingroup baseModules
 */
class INET_API BaseDecider: public Decider {
public:
	/**
	 * @brief The kinds of ControlMessages this Decider sends.
	 *
	 * Sub-classing decider should begin their own kind enumeration
	 * at the value of "LAST_BASE_DECIDER_CONTROL_KIND".
	 */
	enum BaseDeciderControlKinds {
		/** @brief The phy has recognized a bit error in the packet.*/
		PACKET_DROPPED = 22100,
		/** @brief Sub-classing decider should begin their own kinds at this
		 * value.*/
		LAST_BASE_DECIDER_CONTROL_KIND
	};

protected:

	/** @brief The current state of processing for a signal*/
	enum eSignalState {
		/** @brief Signal is received the first time. */
		NEW,
		/** @brief Waiting for the header of the signal. */
		EXPECT_HEADER,
		/** @brief Waiting for the end of the signal. */
		EXPECT_END,
	};

    /** @name Tracked statistic values.*/
    /*@{*/
    unsigned long nbFramesWithInterference;
    unsigned long nbFramesWithoutInterference;

    unsigned long nbFramesWithInterferencePartial;
    unsigned long nbFramesWithoutInterferencePartial;

    unsigned long nbFramesWithInterferenceDropped;
    unsigned long nbFramesWithoutInterferenceDropped;
    /*@}*/

	/** @brief Sensitivity value for receiving an AirFrame if it <tt><= 0</tt> then no sensitivity check
	 *         will be done.
	 */
	double sensitivity;

	/** @brief Pair of a AirFrame and the state it is in. */
	typedef struct tProcessingSignal {
	    typedef DetailedRadioFrame* first_type;    /// @c first_type is the first bound type
	    typedef eSignalState   second_type;   /// @c second_type is the second bound type

	    first_type  first;  /// @c first is a copy of the first object
	    second_type second; /// @c second is a copy of the second object
	    std::size_t iInterferenceCnt;       ///< counted interference frames
	    simtime_t   busyUntilTime;    ///< the next idle time point

	    tProcessingSignal()
            : first(NULL)
            , second(NEW)
            , iInterferenceCnt(0)
	        , busyUntilTime(notAgain)
        {}
        tProcessingSignal(first_type f, second_type s)
            : first(f)
            , second(s)
            , iInterferenceCnt(0)
            , busyUntilTime(notAgain)
        {}
	    tProcessingSignal(const tProcessingSignal& o)
            : first(o.first)
            , second(o.second)
            , iInterferenceCnt(o.iInterferenceCnt)
            , busyUntilTime(o.busyUntilTime)
        {}
	    tProcessingSignal& operator=(const tProcessingSignal& copy)
        {
            first               = copy.first;
            second              = copy.second;
            iInterferenceCnt    = copy.iInterferenceCnt;
            busyUntilTime       = copy.busyUntilTime;
            return *this;
        }
        void swap(tProcessingSignal& s)
        {
            std::swap(first,               s.first);
            std::swap(second,              s.second);
            std::swap(iInterferenceCnt,    s.iInterferenceCnt);
            std::swap(busyUntilTime,       s.busyUntilTime);
        }
        std::size_t interferenceWith(const first_type& frame);
        void startProcessing(first_type frame, second_type state);
        std::size_t finishProcessing() {
            first  = NULL;
            second = NEW;
            return iInterferenceCnt;
        }
        bool isProcessing() const                     { return first != NULL; }
        /** @brief Returns the current interference count (how many other frames are on air on processing).
         *
         * If first is NULL than the interference count is the result from last processed packet.
         */
        std::size_t getInterferenceCnt() const        { return iInterferenceCnt; }
        simtime_t   getBusyEndTime() const            { return busyUntilTime; }
        void clear()                                  { first = NULL; second = NEW; iInterferenceCnt = 0; busyUntilTime = notAgain; }
	} ReceivedSignal;

	/** @brief Pointer to the currently received AirFrame */
	ReceivedSignal currentSignal;

	/** @brief Data about an currently ongoing ChannelSenseRequest. */
	typedef struct tCSRInfo {
        typedef ChannelSenseRequest* first_type;    /// @c first_type is the first bound type
        typedef simtime_t            second_type;   /// @c second_type is the second bound type

        first_type  first;  /// @c first is a copy of the first object
        second_type second; /// @c second is a copy of the second object

		simtime_t   canAnswerAt;

		tCSRInfo()
			: first(NULL)
			, second()
			, canAnswerAt()
		{}
		tCSRInfo(const tCSRInfo& o)
			: first(o.first)
			, second(o.second)
			, canAnswerAt(o.canAnswerAt)
		{}
		tCSRInfo& operator=(const tCSRInfo& copy)
		{
			first       = copy.first;
			second      = copy.second;
			canAnswerAt = copy.canAnswerAt;
			return *this;
		}
		void swap(tCSRInfo& s)
		{
			std::swap(first,       s.first);
			std::swap(second,      s.second);
			std::swap(canAnswerAt, s.canAnswerAt);
		}

		ChannelSenseRequest *const getRequest() const { return first; }
		void setRequest(ChannelSenseRequest* request) { first = request; }
		simtime_t_cref getSenseStart() const          { return second; }
		void setSenseStart(simtime_t_cref start)      { second = start; }
		simtime_t_cref getAnswerTime() const          { return canAnswerAt; }
		void setAnswerTime(simtime_t_cref answerAt)   { canAnswerAt = answerAt; }
		void clear()                                  { first = NULL; second = canAnswerAt = Decider::notAgain; }
	} CSRInfo;

	/** @brief pointer to the currently running ChannelSenseRequest and its
	 * start-time */
	CSRInfo currentChannelSenseRequest;

	/** @brief index for this Decider-instance given by Phy-Layer (mostly
	 * Host-index) */
	int myIndex;

public:
	/**
	 * @brief Initializes the decider with the passed values.
	 *
	 * Needs a pointer to its physical layer, the sensitivity, the index of the host.
	 */
	BaseDecider( DeciderToPhyInterface* phy
	           , double                 sensitivity
	           , int                    myIndex)
		: Decider(phy)
		, nbFramesWithInterference(0)
		, nbFramesWithoutInterference(0)
		, nbFramesWithInterferencePartial(0)
		, nbFramesWithoutInterferencePartial(0)
		, nbFramesWithInterferenceDropped(0)
		, nbFramesWithoutInterferenceDropped(0)
		, sensitivity(sensitivity)
		, currentSignal(NULL, NEW)
		, currentChannelSenseRequest()
		, myIndex(myIndex)
	{
		currentChannelSenseRequest.clear();
	}

	virtual ~BaseDecider() {}

public:
	/**
	 * @brief Processes an AirFrame given by the PhyLayer
	 *
	 * Returns the time point when the decider wants to be given the AirFrame
	 * again.
	 */
	virtual simtime_t processSignal(DetailedRadioFrame* frame);

    /** @brief Cancels processing a AirFrame.
     */
    virtual void cancelProcessSignal() {
        currentSignal.finishProcessing();
    }

	/**
	 * @brief A function that returns information about the channel state
	 *
	 * It is an alternative for the MACLayer in order to obtain information
	 * immediately (in contrast to sending a ChannelSenseRequest,
	 * i.e. sending a cMessage over the OMNeT-control-channel)
	 */
	virtual ChannelState getChannelState() const;

	/**
	 * @brief This function is called by the PhyLayer to hand over a
	 * ChannelSenseRequest.
	 *
	 * The MACLayer is able to send a ChannelSenseRequest to the PhyLayer
	 * that calls this function with it and is returned a time point when to
	 * re-call this function with the specific ChannelSenseRequest.
	 *
	 * The Decider puts the result (ChannelState) to the ChannelSenseRequest
	 * and "answers" by calling the "sendControlMsg"-function on the
	 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
	 */
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);

	/**
	 * @brief Called by phy layer to indicate that the channel this radio
	 * currently listens to has changed.
	 *
	 * Sub-classing deciders which support multiple channels should override
	 * this method to handle the effects of channel changes on ongoing
	 * receptions.
	 *
	 * @param newChannel The new channel the radio has changed to.
	 */
	virtual void channelChanged(int newChannel);

	/**
	 * @brief Method to be called by an OMNeT-module during its own finish(),
	 * to enable a decider to do some things.
	 */
	virtual void finish();

protected:
	/**
	 * @brief Calculates the receive power of given frame.
	 *
	 * Default implementation use only the arrival time point
	 * for signal receive power calculation.
	 */
	virtual double getFrameReceivingPower(DetailedRadioFrame* frame) const;

	/**
	 * @brief Returns the next signal state (END, HEADER, NEW).
	 *
	 * @param CurState The current signal state.
	 * @return The next signal state.
	 */
	virtual eSignalState getNextSignalState(eSignalState CurState) const {
	    switch(CurState) {
            case NEW:           return EXPECT_END; break;
            case EXPECT_HEADER: return EXPECT_END; break;
            default:            return NEW;        break;
	    }
	    return NEW;
	}

	/**
	 * @brief Returns the next handle time for scheduler.
	 *
	 * @param frame The current frame which is in processing.
	 * @return The next scheduler handle time.
	 */
	virtual simtime_t getNextSignalHandleTime(const DetailedRadioFrame* frame) const;

	/**
	 * @brief Processes a new Signal. Returns the time it wants to
	 * handle the signal again.
	 *
	 * Default implementation checks if the signals receiving power
	 * is above the sensitivity of the radio and we are not already trying
	 * to receive another AirFrame. If thats the case it waits for the end
	 * of the signal.
	 */
	virtual simtime_t processNewSignal(DetailedRadioFrame* frame);

	/**
	 * @brief Processes the end of the header of a received Signal.
	 *
	 * Returns the time it wants to handle the signal again.
	 *
	 * Default implementation does not handle signal headers.
	 */
	virtual simtime_t processSignalHeader(DetailedRadioFrame* /*frame*/) {
		opp_error("BaseDecider does not handle Signal headers!");
		return notAgain;
	}

	/** @brief Creates the DeciderResult from frame.
	 *
	 * @param frame The processed frame.
	 * @return The result for frame.
	 */
	virtual DeciderResult* createResult(const DetailedRadioFrame* frame) const;

	/**
	 * @brief Processes the end of a received Signal.
	 *
	 * Returns the time it wants to handle the signal again
	 * (most probably notAgain).
	 *
	 * Default implementation just decides every signal as correct and passes it
	 * to the upper layer.
	 */
	virtual simtime_t processSignalEnd(DetailedRadioFrame* frame);

	/**
	 * @brief Processes any Signal for which no state could be found.
	 * (is an error case).
	 */
	virtual simtime_t processUnknownSignal(DetailedRadioFrame* frame);

	/**
	 * @brief Returns the SignalState for the passed AirFrame.
	 *
	 * The default implementation checks if the passed AirFrame
	 * is the "currentSignal" and returns its state or if not
	 * "NEW".
	 */
	virtual eSignalState getSignalState(const DetailedRadioFrame* frame) const;
	virtual eSignalState setSignalState(const DetailedRadioFrame* frame, eSignalState newState);

	/**
	 * @brief Handles a new incoming ChannelSenseRequest and returns the next
	 * (or latest) time to handle the request again.
	 */
	virtual simtime_t handleNewSenseRequest(ChannelSenseRequest* request);

	/**
	 * @brief Handles the timeout or end of a ChannelSenseRequest by calculating
	 * the ChannelState and returning the request to the mac layer.
	 *
	 * If this handler is reached the decider has to be able to answer the
	 * request. Either because the timeout is reached or because the
	 * channel state changed accordingly.
	 */
	virtual void handleSenseRequestEnd(CSRInfo& requestInfo);

	/**
	 * @brief Returns point in time when the ChannelSenseRequest of the passed
	 * CSRInfo can be answered (e.g. because channel state changed or timeout
	 * is reached).
	 */
	virtual simtime_t canAnswerCSR(const CSRInfo& requestInfo) const;

	/** @brief Return type of BaseDecider::calcChannelSenseRSSI function.
	 *
	 *  The pair consists in first part the RSSI value and in second part
	 *  the maximum reception time of all air frames in requested range.
	 */
	typedef std::pair<double, simtime_t> channel_sense_rssi_t;
	/**
	 * @brief Calculates the RSSI value for the passed interval.
	 *
	 * This method is called by BaseDecider when it answers a
	 * ChannelSenseRequest or calculates the channel state. Can be overridden
	 * by sub classing Deciders.
	 *
	 * Default implementation returns the maximum RSSI value inside the
	 * passed interval.
	 */
	virtual channel_sense_rssi_t calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) const;

	/**
	 * @brief Answers the ChannelSenseRequest (CSR) from the passed CSRInfo.
	 *
	 * Calculates the rssi value and the channel idle state and sends the CSR
	 * together with the result back to the mac layer.
	 */
	virtual void answerCSR(CSRInfo& requestInfo);

	/**
	 * @brief Checks if the changed channel state enables us to answer
	 * any ongoing ChannelSenseRequests.
	 *
	 * This method is ment to update only an already ongoing
	 * ChannelSenseRequests it can't handle a new one.
	 */
	virtual void channelStateChanged();

	/**
	 * @brief Collects the AirFrame on the channel during the passed interval.
	 *
	 * Forwards to DeciderToPhyInterfaces "getChannelInfo" method.
	 * Subclassing deciders can override this method to filter the returned
	 * AirFrames for their own criteria.
	 *
	 * @param[in]  start The start of the interval to collect AirFrames from.
	 * @param[in]  end   The end of the interval to collect AirFrames from.
	 * @param[out] out   The output vector in which to put the AirFrames.
	 */
	virtual void getChannelInfo(simtime_t_cref start, simtime_t_cref end, AirFrameVector& out) const;

	//------Utility methods------------

	/**
	 * @brief Calculates a SNR-Mapping for a Signal.
	 *
	 * A Noise-Strength-Mapping is calculated (by using the
	 * "calculateRSSIMapping()"-method) for the time-interval
	 * of the Signal and the Signal-Strength-Mapping is divided by the
	 * Noise-Strength-Mapping.
	 *
	 * Note: 'divided' means here the special element-wise operation on
	 * mappings.
	 */
	virtual Mapping* calculateSnrMapping(const DetailedRadioFrame* frame) const;

	/** @brief Return type of BaseDecider::calculateRSSIMapping function.
	 *
	 *  The pair consists in first part the RSSI map pointer and in second part
	 *  the maximum reception time of all air frames in requested range.
	 */
	typedef std::pair<Mapping*, channel_sense_rssi_t::second_type> rssi_mapping_t;

	/**
	 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a
	 * Signal.
	 *
	 * This method can be used to calculate a RSSI-Mapping in case the parameter
	 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
	 * AirFrame of the received Signal is passed as parameter exclude.
	 *
	 * @return The mapping and the maximum reception end of all air frames in rang [start,end].
	 */
	virtual rssi_mapping_t calculateRSSIMapping( simtime_t_cref       start
	                                           , simtime_t_cref       end
	                                           , const DetailedRadioFrame* exclude = NULL) const;
};

#endif /* BASEDECIDER_H_ */
