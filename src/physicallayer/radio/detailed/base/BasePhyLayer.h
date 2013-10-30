#ifndef BASEPHYLAYER_
#define BASEPHYLAYER_

#include <map>
#include <vector>
#include <string>

#include "INETDefs.h"
#include "ChannelState.h"
#include "PhyUtils.h"
#include "DetailedRadioChannelAccess.h"
#include "DeciderToPhyInterface.h"
#include "ModuleAccess.h"
#include "ChannelInfo.h"

class AnalogueModel;
class Decider;
class cXMLElement;

/**
 * @brief The BasePhyLayer represents the physical layer of a nic.
 *
 * The BasePhyLayer is directly connected to the mac layer via
 * OMNeT channels and is able to send messages to other physical
 * layers through sub-classing from ChannelAcces.
 *
 * The BasePhyLayer encapsulates two sub modules.
 * The AnalogueModels, which are responsible for simulating
 * the attenuation of received signals and the Decider which
 * provides the main functionality of the physical layer like
 * signal classification (noise or not noise) and demodulation
 * (calculating transmission errors).
 *
 * The BasePhyLayer itself is responsible for the OMNeT
 * depended parts of the physical layer which are the following:
 *
 * Module initialization:
 * - read ned-parameters and initialize module, Decider and
 *   AnalogueModels.
 *
 * Message handling:
 * - receive messages from mac layer and hand them to the Decider
 *   or directly send them to the channel
 * - receive AirFrames from the channel, hand them to the
 *   AnalogueModels for filtering, simulate delay and transmission
 *   duration, hand it to the Decider for evaluation and send
 *   received packets to the mac layer
 * - keep track of currently active AirFrames on the channel
 *   (see ChannelInfo)
 *
 * The actual evaluation of incoming signals is done by the
 * Decider.
 *
 * base class ConnectionManagerAccess:
 * - provides access to the channel via the ConnectionManager
 *
 * base class DeciderToPhyInterface:
 * - interface for the Decider
 *
 * base class MacToPhyInterface:
 * - interface for the Mac
 *
 * @ingroup phyLayer
 * @ingroup baseModules
 */
class INET_API BasePhyLayer: public DetailedRadioChannelAccess, public DeciderToPhyInterface
{

protected:
    /**
     * @brief Message kinds used by every phy layer.
     *
     * Physical layers should begin their additional kinds
     * at the value of LAST_BASE_PHY_KIND.
     */
    enum BasePhyMessageKinds {
        /** @brief Indicates the end of a send transmission. */
        TX_OVER = 22000,
        /** @brief Indicates the end of a radio switch. */
        RADIO_SWITCHING_OVER,
        /** @brief Channel sense control message between Mac and Phy.*/
        CHANNEL_SENSE_REQUEST,
        /** @brief AirFrame kind */
        AIR_FRAME,
        /** @brief Stores the id on which classes extending BasePhy should
         * continue their own kinds.*/
        LAST_BASE_PHY_KIND,
    };

	enum ProtocolIds {
		GENERIC = 0,
	};

	int protocolId;

	/** @brief Defines the scheduling priority of AirFrames.
	 *
	 * AirFrames use a slightly higher priority than normal to ensure
	 * channel consistency. This means that before anything else happens
	 * at a time point t every AirFrame which ended at t has been removed and
	 * every AirFrame started at t has been added to the channel.
	 *
	 * An example where this matters is a ChannelSenseRequest which ends at
	 * the same time as an AirFrame starts (or ends). Depending on which message
	 * is handled first the result of ChannelSenseRequest would differ.
	 */
	static short airFramePriority;

	/** @brief Defines the strength of the thermal noise.*/
	ConstantSimpleConstMapping* thermalNoise;

    /** @brief The transmission power */
    double txPower;

	/** @brief The maximum transmission power a message can be send with */
	double maxTXPower;

	/** @brief The sensitivity describes the minimum strength a signal must have to be received.*/
	double sensitivity;

	/** @brief Stores if tracking of statistics (esp. cOutvectors) is enabled.*/
	bool recordStats;

	/**
	 * @brief Channel info keeps track of received AirFrames and provides information about
	 * currently active AirFrames at the channel.
	 */
	ChannelInfo channelInfo;

	/** @brief The state machine storing the current radio state (TX, RX, SLEEP).*/
	MiximRadio* radio;

	/** @brief Pointer to the decider module. */
	Decider* decider;

	/** @brief Used to store the AnalogueModels to be used as filters.*/
	typedef std::vector<AnalogueModel*> AnalogueModelList;

	/** @brief List of the analogue models to use.*/
	AnalogueModelList analogueModels;

	/**
	 * @brief Self message scheduled to the point in time when the
	 * switching process of the radio is over.
	 */
	cMessage* radioSwitchingOverTimer;

	/**
	 * @brief Self message scheduled to the point in time when the
	 * transmission of an AirFrame is over.
	 */
	cMessage* txOverTimer;

	/** @brief The states of the receiving process for AirFrames.*/
	enum eAirFrameStates {
		/** @brief Start of actual receiving process of the AirFrame. */
		START_RECEIVE = 1,
		/** @brief AirFrame is being received. */
		RECEIVING,
		/** @brief Receiving process over */
		END_RECEIVE
	};

	/** @brief Stores the length of the phy header in bits. */
	long headerLength;

private:

	/**
	 * @brief Utility function. Reads the parameters of a XML element
	 * and stores them in the passed ParameterMap reference.
	 */
	void getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap) const;

	/**
	 * @brief Initializes the AnalogueModels with the data from the
	 * passed XML-config data.
	 */
	void initializeAnalogueModels(cXMLElement* xmlConfig);

	/**
	 * @brief Initializes the Decider with the data from the
	 * passed XML-config data.
	 */
	void initializeDecider(cXMLElement* xmlConfig);

protected:

	/**
	 * @brief Reads and returns the parameter with the passed name.
	 *
	 * If the parameter couldn't be found the value of defaultValue
	 * is returned.
	 *
	 * @param parName 		- the name of the ned-parameter
	 * @param defaultValue 	- the value to be returned if the parameter
	 * 				  		  couldn't be found
	 */
	template<class T> T readPar(const char* parName, const T defaultValue) const;

	/**
	 * @brief OMNeT++ initialization function.
	 *
	 * Read simple parameters.
	 * Read and parse xml file for decider and analogue models
	 * configuration.
	 */
	virtual void initialize(int stage);

	/**
	 * @brief OMNeT++ handle message function.
	 *
	 * Classify and forward message to subroutines.
	 * - AirFrames from channel
	 * - self scheduled AirFrames
	 * - MacPackets from MAC layer
	 * - ControllMesasges from MAC layer
	 * - self messages like TX_OVER and RADIO_SWITCHED
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * @brief Initializes and returns the radio class to use.
	 *
	 * Can be overridden by sub-classing phy layers to use their
	 * own Radio implementations.
	 */
	virtual MiximRadio* initializeRadio() const;

	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 * specified name.
	 *
	 * The returned AnalogueModel has to be
	 * generated with the "new" command. The BasePhyLayer
	 * keeps the ownership of the returned AnalogueModel.
	 *
	 * This method is used by the BasePhyLayer during
	 * initialization to load the AnalogueModels which
	 * has been specified in the ned file.
	 *
	 * This method has to be overridden if you want to be
	 * able to load your own AnalogueModels.
	 *
	 * Is able to initialize the following AnalogueModels:
	 * - RadioStateAnalogueModel
	 */
	virtual AnalogueModel* getAnalogueModelFromName(const std::string& name, ParameterMap& params) const;

	/**
	 * @brief Creates and returns an instance of the analogue model with the specified
	 *        _AMODEL_CLASS_.
	 *
	 * The returned Decider has to be generated with
	 * the "new" command.
	 *
	 * @return Instance of the Decider with the specified class type.
	 */
	template <class _AMODEL_CLASS_>
	_AMODEL_CLASS_* createAnalogueModel(const ParameterMap& params) const {
		_AMODEL_CLASS_ *const pAnalogueModel = new _AMODEL_CLASS_();
		if (pAnalogueModel != NULL && !pAnalogueModel->initFromMap(params)) {
			opp_warning("Analog model from config.xml could not be initialized correctly!");
		}
		return pAnalogueModel;
	}

	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 * name.
	 *
	 * The returned Decider has to be generated with
	 * the "new" command. The BasePhyLayer keeps the ownership
	 * of the returned Decider.
	 *
	 * This method is used by the BasePhyLayer during
	 * Initialization to load the decider which has been
	 * specified in the config.xml file.
	 *
	 * This method has to be overridden if you want to be
	 * able to load your own decider.
	 *
	 * @return Instance of the decider with the specified name.
	 */
	virtual Decider* getDeciderFromName(const std::string& name, ParameterMap& params);

	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 *        _DECIDER_CLASS_.
	 *
	 * The returned Decider has to be generated with
	 * the "new" command.
	 *
	 * @return Instance of the Decider with the specified class type.
	 */
	template <class _DECIDER_CLASS_>
	_DECIDER_CLASS_* createDecider(const ParameterMap& params) {
		_DECIDER_CLASS_ *const pDecider = new _DECIDER_CLASS_(this, sensitivity, findContainingNode(this)->getIndex());
		if (pDecider != NULL && !pDecider->initFromMap(params)) {
			opp_warning("Decider from config.xml could not be initialized correctly!");
		}
		return pDecider;
	}

	/**
	 * @name Handle Messages
	 **/
	/*@{ */
	/**
	 * @brief Handles messages received from the channel (probably AirFrames).
	 */
	virtual void handleAirFrame(DetailedRadioFrame* frame);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * data gate.
	 */
	virtual void handleUpperMessage(cMessage* msg);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * control gate.
	 */
	virtual void handleUpperControlMessage(cMessage* msg);

	/**
	 * @brief Handles self scheduled messages.
	 */
	virtual void handleSelfMessage(cMessage* msg);

	/**
	 * @brief Handles reception of a ChannelSenseRequest by forwarding it
	 * to the decider and scheduling it to the point in time
	 * returned by the decider.
	 */
	virtual void handleChannelSenseRequest(cMessage* msg);

	/**
	 * @brief Handles incoming AirFrames with the state FIRST_RECEIVE.
	 */
	void handleAirFrameFirstReceive(DetailedRadioFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state START_RECEIVE.
	 */
	virtual void handleAirFrameStartReceive(DetailedRadioFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state RECEIVING.
	 */
	virtual void handleAirFrameReceiving(DetailedRadioFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state END_RECEIVE.
	 */
	virtual void handleAirFrameEndReceive(DetailedRadioFrame* msg);

	/*@}*/

	/**
	 * @name Send Messages
	 **/
	/*@{ */

	/**
	 * @brief Sends the passed control message to the upper layer.
	 */
	void sendControlMessageUp(cMessage* msg);

	/**
	 * @brief Sends the passed MacPkt to the upper layer.
	 */
	void sendMacPktUp(cMessage* pkt);

	/**
	 * @brief Sends the passed AirFrame to the channel
	 */
	void sendMessageDown(DetailedRadioFrame* pkt);

	/**
	 * @brief Schedule self message to passed point in time.
	 */
	void sendSelfMessage(cMessage* msg, simtime_t_cref time);

	/*@}*/

	/**
	 * @brief This function encapsulates messages from the upper layer into an
	 * AirFrame and sets all necessary attributes.
	 */
	virtual DetailedRadioFrame* encapsMsg(cPacket *msg);

	/**
	 * @brief Filters the passed AirFrame's Signal by every registered AnalogueModel.
	 */
	virtual void filterSignal(DetailedRadioFrame* frame);

	/**
	 * @brief Called the moment the simulated switching process of the MiximRadio is finished.
	 *
	 * The Radio is set the new RadioState and the MAC Layer is sent
	 * a confirmation message.
	 *
	 * @param bSendCtrlMsg Flag for sending control message to MAC (in case of zero switch time
	 *                     this flag maybe false).
	 */
	virtual void finishRadioSwitching(bool bSendCtrlMsg = true);

	/**
	 * @brief Returns the identifier of the protocol this phy uses to send
	 * messages.
	 *
	 * @return An integer representing the identifier of the used protocol.
	 */
	virtual int myProtocolId() const { return protocolId; }

	/**
	 * @brief Returns true if the protocol with the passed identifier is
	 * decodeable by the decider.
	 *
	 * If the protocol with the passed id is not understood by this phy layers
	 * decider the according AirFrame is not passed to the it but only is added
	 * to channel info to be available as interference to the decider.
	 *
	 * Default implementation checks only if the passed id is the same as the
	 * one returned by "myProtocolId()".
	 *
	 * @param id The identifier of the protocol of an AirFrame.
	 * @return Returns true if the passed protocol id is supported by this phy-
	 * layer.
	 */
	virtual bool isKnownProtocolId(int id) const { return id == myProtocolId(); }

private:
	/** @brief Copy constructor is not allowed.
	 */
	BasePhyLayer(const BasePhyLayer&);
	/** @brief Assignment operator is not allowed.
	 */
	BasePhyLayer& operator=(const BasePhyLayer&);

public:
	BasePhyLayer();

	/**
	 * Free the pointer to the decider and the AnalogueModels and the Radio.
	 */
	virtual ~BasePhyLayer();

	/** @brief Only calls the deciders finish method.*/
	virtual void finish();

	//---------MacToPhyInterface implementation-----------
	/**
	 * @name MacToPhyInterface implementation
	 * @brief These methods implement the MacToPhyInterface.
	 **/
	/*@{ */
    virtual void setRadioMode(RadioMode radioMode);

    virtual void setRadioChannel(int radioChannel);

	/**
	 * @brief Returns the current state the radio is in.
	 *
	 * See RadioState for possible values.
	 *
	 * This method is mainly used by the mac layer.
	 */
	virtual int getRadioState() const;

	/**
	 * @brief Returns the true if the radio is in RX state.
	 */
	virtual bool isRadioInRX() const;

	/**
	 * @brief Tells the BasePhyLayer to switch to the specified
	 * radio state.
	 *
	 * The switching process can take some time depending on the
	 * specified switching times in the ned file.
	 *
	 * @return Decider::notAgain: Error code if the Radio is currently switching
	 *			else: switching time from the current RadioState to the new RadioState
	 */
	virtual simtime_t setRadioState(int rs);

	/**
	 * @brief Returns the current state of the channel.
	 *
	 * See ChannelState for details.
	 */
	virtual ChannelState getChannelState() const;

	virtual void updateRadioChannelState();

	/**
	 * @brief Returns the length of the phy header in bits.
	 *
	 * Both the MAC and the PHY needs the header length.
	 */
	virtual long getPhyHeaderLength() const;

	/** @brief Returns the number of channels available on this radio. */
	virtual int getNbRadioChannels() const;

	/*@}*/

	//---------DeciderToPhyInterface implementation-----------
	/**
	 * @name DeciderToPhyInterface implementation
	 * @brief These methods implement the DeciderToPhyInterface.
	 **/
	/*@{ */

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) const;

	/**
	 * @brief Returns a Mapping which defines the thermal noise in
	 * the passed time frame (in mW).
	 *
	 * The implementing class of this method keeps ownership of the
	 * Mapping.
	 *
	 * This implementation returns a constant mapping with the value
	 * of the "thermalNoise" module parameter
	 *
	 * Override this method if you want to define a more complex
	 * thermal noise.
	 */
	virtual ConstMapping* getThermalNoise(simtime_t_cref from, simtime_t_cref to);

	/**
	 * @brief Called by the Decider to send a control message to the MACLayer
	 *
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 *
	 */
	virtual void sendControlMsgToMac(cMessage* msg);

	/**
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 *
	 * When a packet is completely received and not noise, the Decider
	 * call this function to send the packet together with
	 * the corresponding DeciderResult up to MACLayer
	 *
	 */
	virtual void sendUp(DetailedRadioFrame* packet, DeciderResult* result);

	/**
	 * @brief Returns the current simulation time
	 */
	virtual simtime_t getSimTime() const;

	/**
	 * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it doesn't need to handle an AirFrame or
	 * ControlMessage again anymore.
	 */
	virtual void cancelScheduledMessage(cMessage* msg);

	/**
	 * @brief Tells the PhyLayer to reschedule a message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or an control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void rescheduleMessage(cMessage* msg, simtime_t_cref t);

	/**
	 * @brief Records a double into the scalar result file.
	 *
	 * Implements the method from DeciderToPhyInterface, method-calls are forwarded
	 * to OMNeT-method 'recordScalar'.
	 */
	void recordScalar(const char *name, double value, const char *unit=NULL);

	/*@}*/

	/**
	 * @brief Attaches a "control info" (PhyToMac) structure (object) to the message pMsg.
	 *
	 * This is most useful when passing packets between protocol layers
	 * of a protocol stack, the control info will contain the decider result.
	 *
	 * The "control info" object will be deleted when the message is deleted.
	 * Only one "control info" structure can be attached (the second
	 * setL3ToL2ControlInfo() call throws an error).
	 *
	 * @param pMsg		The message where the "control info" shall be attached.
	 * @param pSrcAddr	The MAC address of the message receiver.
	 */
	virtual cObject* setUpControlInfo(cMessage *const pMsg, DeciderResult *const pDeciderResult);

  protected:
	virtual DetailedRadioSignal* createSignal(cPacket *macPkt);

    /**
     * @brief Creates a simple Signal defined over time with the
     * passed parameters.
     *
     * Convenience method to be able to create the appropriate
     * Signal for the MacToPhyControlInfo without needing to care
     * about creating Mappings.
     *
     * NOTE: The created signal's transmission-power is a rectangular function.
     * This method uses MappingUtils::addDiscontinuity to represent the discontinuities
     * at the beginning and end of this rectangular function.
     * Because of this the created mapping which represents the signal's
     * transmission-power is still zero at the exact start and end.
     * Please see the method MappingUtils::addDiscontinuity for the reason.
     */
    virtual DetailedRadioSignal* createSignal(simtime_t_cref start, simtime_t_cref length, double power, double bitrate);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value.
     *
     * Used by "createSignal" to create the bitrate mapping.
     */
    Mapping* createConstantMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value and discontinuities at the boundaries.
     *
     * Used by "createSignal" to create the power mapping.
     */
    Mapping* createRectangleMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a Mapping defined over time and frequency with
     * constant power in a certain frequency band.
     */
    ConstMapping* createSingleFrequencyMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref centerFreq, Argument::mapped_type_cref bandWith, Argument::mapped_type_cref value);
};

#endif /*BASEPHYLAYER_*/
