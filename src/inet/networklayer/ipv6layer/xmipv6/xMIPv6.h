//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund, Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#ifndef __INET_XMIPV6_H
#define __INET_XMIPV6_H

#include <map>
#include <vector>

#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"
#include "inet/networklayer/xmipv6/BindingUpdateList.h"
#include "inet/networklayer/xmipv6/MobilityHeader_m.h" // for HAOpt & RH2

namespace inet {

// Foreign declarations:
class BindingCache;
class BindingUpdate;
class NetworkInterface;
class Ipv6Header;
class Ipv6NeighbourDiscovery;
class Ipv6Tunneling;
class Ipv6RoutingTable;

// Keys for timer list (=message type)
#define KEY_BU               0 // Binding Update
#define KEY_HI               1 // HoTI
#define KEY_CI               2 // CoTI
#define KEY_BR               3 // Binding Refresh Request
#define KEY_BUL_EXP          4 // BUL entry expiry
#define KEY_BC_EXP           5 // BC entry expiry
#define KEY_HTOKEN_EXP       6 // home token expiry
#define KEY_CTOKEN_EXP       7 // care-of token expiry

#define TRANSMIT_TYPE_BU     51 // BuTransmitIfEntry
#define TRANSMIT_TYPE_TI     52 // TestInitTransmitIfEntry

#define EXPIRY_TYPE_BUL      61 // BulExpiryIfEntry
#define EXPIRY_TYPE_BC       62 // BcExpiryIfEntry

#define EXPIRY_TYPE_TOKEN    63 // {Home, CareOf}TokenExpiryIfEntry

/**
 * Implements RFC 3775 Mobility Support in Ipv6.
 */
class INET_API xMIPv6 : public cSimpleModule
{
  public:
    virtual ~xMIPv6();

  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    opp_component_ptr<Ipv6RoutingTable> rt6;
    ModuleRefByPar<BindingUpdateList> bul;
    ModuleRefByPar<BindingCache> bc;
    ModuleRefByPar<Ipv6Tunneling> tunneling;
    ModuleRefByPar<Ipv6NeighbourDiscovery> ipv6nd;

    // statistic collection
    cOutVector statVectorBUtoHA, statVectorBUtoCN, statVectorBUtoMN;
    // 1 means BA from HA, 2 means BA from CN
    cOutVector statVectorBAtoMN;
    // 1 means BA to register BU, 2 means BA to deregister BU message
    // 3 means invalid BA
    cOutVector statVectorBAfromHA, statVectorBAfromCN;

    cOutVector statVectorHoTItoCN, statVectorCoTItoCN;
    cOutVector statVectorHoTtoMN, statVectorCoTtoMN;
    cOutVector statVectorHoTfromCN, statVectorCoTfromCN;

    /**
     * The base class for all other timers that are used for retransmissions.
     */
    class INET_API TimerIfEntry {
      public:
        cMessage *timer; // pointer to the scheduled timer message
        virtual ~TimerIfEntry() {} // to make it a polymorphic base class

        Ipv6Address dest; // the address (HA or CN(s) for which the message is sent
        simtime_t ackTimeout; // timeout for the Ack
        simtime_t nextScheduledTime; // time when the corrsponding message is supposed to be sent
        NetworkInterface *ifEntry; // interface from which the message will be transmitted
    };

    class INET_API Key {
      public:
        int type; // type of the message (BU, HoTI, CoTI) stored in the map, indexed by this key
        int interfaceID; // ID of the interface over which the message is sent
        Ipv6Address dest; // the address of either the HA or the CN

        Key(Ipv6Address _dest, int _interfaceID, int _type)
        {
            dest = _dest;
            interfaceID = _interfaceID;
            type = _type;
        }

        bool operator<(const Key& b) const
        {
            if (type == b.type)
                return interfaceID == b.interfaceID ? dest < b.dest : interfaceID < b.interfaceID;
            else
                return type < b.type;
        }
    };

    typedef std::map<Key, TimerIfEntry *> TransmitIfList;
    TransmitIfList transmitIfList;

    /** holds the tuples of currently available {InterfaceID, CoA} pairs */
    typedef std::map<int, Ipv6Address> InterfaceCoAList;
    InterfaceCoAList interfaceCoAList;

    // A vector that will contain and maintain a list of all the CN(s) that the MN is in communication with. Although this is a quick fix, but this list should be populated and depopulated in sync with the destination cache. Final version should rely on the destinaion cache for acquiring the CN(s) address for use in Correspodent Registeration
    typedef std::vector<Ipv6Address> CnList;
    CnList cnList;
    CnList::iterator itCNList; // declaring an iterator over the cnList vector

    /** Subclasses for the different timers */
    class INET_API BuTransmitIfEntry : public TimerIfEntry {
      public:
        uint buSequenceNumber; // sequence number of the BU sent
        uint lifeTime; // lifetime of the BU sent
        // Time variable related to the time at which BU was sent
        simtime_t presentSentTimeBU; // stores the present time at which BU is/was sent
        bool homeRegistration; // indicates whether this goes to HA or CN;
    };

    class INET_API TestInitTransmitIfEntry : public TimerIfEntry {
      public:
        Ptr<MobilityHeader> testInitMsg; // either the HoTI or CoTI
    };

    class INET_API BrTransmitIfEntry : public TimerIfEntry {
      public:
        uint retries; // number of BRRs already sent
    };

    class INET_API BulExpiryIfEntry : public TimerIfEntry {
      public:
        Ipv6Address CoA, HoA; // the CoA and HoA of the MN that were used for this BUL entry
    };

    class INET_API BcExpiryIfEntry : public TimerIfEntry {
      public:
        Ipv6Address HoA; // HoA of the MN
    };

    class INET_API TokenExpiryIfEntry : public TimerIfEntry {
      public:
        Ipv6Address cnAddr; // CN whose token is expiring
        int tokenType; // KEY_XX indicates whether it is a care-of token, etc.
    };

  protected:
    /************************Miscellaneous Stuff***************************/
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    //================MIPv6 Related Functions=================================================
    /**
     * This is where all the mobility messages are sifted through and sent to appropriate functions
     * for processing.
     */
    void processMobilityMessage(Packet *inPacket);

    /**
     * This method finally creates the timer structure and schedules the message for sending.
     */
    void createBUTimer(const Ipv6Address& buDest, NetworkInterface *ie, const uint lifeTime,
            bool homeRegistration);

    /**
     * Similiar to the previous one, this method creates an BU timer with registration lifetime equal to 0.
     */
    void createDeregisterBUTimer(const Ipv6Address& buDest, NetworkInterface *ie);

    /*
     * This method creates and starts a timer for an advertising interface over which BUs will be sent until it gets acknowledged
     * by an appropriate BA. This routine also "intialises" the necessary variables in a struct BuTransmitIfEntry that is created to
     * keep these variables for access.
     */
    void createBUTimer(const Ipv6Address& buDest, NetworkInterface *ie);

    /**
     * This method is called when the timer created in createBUTimer() has fired.
     * The BU is created and the appropriate method for sending it called. The timer structure is updated and rescheduled.
     */
    void sendPeriodicBU(cMessage *msg);

    /**
     * Method for creating and sending a BU by a MN.
     */
    void createAndSendBUMessage(const Ipv6Address& dest, NetworkInterface *ie, const uint buSeq,
            const uint lifeTime, const int bindAuthData = 0);

    /**
     * Update the an entry of the BUL with the provided parameters.
     */
    void updateBUL(BindingUpdate *bu, const Ipv6Address& dest, const Ipv6Address& CoA,
            NetworkInterface *ie, const simtime_t sendTime);

    /**
     * This method takes an interface and a destination address and returns the appropriate IfEntry for an BU.
     * Is supposed to be used until the valid BA is received for the respective BU.
     */
    xMIPv6::BuTransmitIfEntry *fetchBUTransmitIfEntry(NetworkInterface *ie, const Ipv6Address& dest);

    /**
     * Append tags to the Mobility Messages (BU, BA etc) and send it out to the Ipv6 Module
     */
    void sendMobilityMessageToIPv6Module(Packet *msg, const Ipv6Address& destAddr,
            const Ipv6Address& srcAddr = Ipv6Address::UNSPECIFIED_ADDRESS, int interfaceId = -1,
            simtime_t sendTime = 0); // overloaded for use at CN - CB
//    void sendMobilityMessageToIPv6Module(cMessage *msg, const Ipv6Address& destAddr, simtime_t sendTime = 0); // overloaded for use at CN - CB

    /**
     * Process a BU - only applicable to HAs and CNs.
     */
    void processBUMessage(Packet *inPacket, const Ptr<const BindingUpdate>& bu);

    /**
     * Validate a BU - only applicable to HAs and CNs
     */
    bool validateBUMessage(Packet *inPacket, const Ptr<const BindingUpdate>& bu);

    /**
     * Similiar to validateBUMessage(). However, this one is used only by HA to verify deregistration BU.
     */
    bool validateBUderegisterMessage(Packet *inPacket, const Ptr<const BindingUpdate>& bu);

    /**
     * Constructs and send a BA to the Ipv6 module. Only applicable to HAs and CNs.
     */
    void createAndSendBAMessage(const Ipv6Address& src,
            const Ipv6Address& dest, int interfaceId, const BaStatus& baStatus, const uint baSeq,
            const int bindingAuthorizationData, const uint lifeTime, simtime_t sendTime = 0);

    /**
     * Processes the received BA and creates tunnels or mobility header paths if appropriate.
     */
    void processBAMessage(Packet *inPacket, const Ptr<const BindingAcknowledgement>& ba);

    /**
     * Validates a Binding Acknowledgement for a mobile node.
     */
    bool validateBAck(Packet *inPacket, const BindingAcknowledgement& ba);

    /**
     * Creates and sends Binding Error message.
     */
    void createAndSendBEMessage(const Ipv6Address& dest, const BeStatus& beStatus);

  public:
    /**
     * Initiates the Mobile IP protocol.
     * Method to be used when we have moved to a new access network and the new CoA is available for that interface.
     */
    void initiateMipv6Protocol(NetworkInterface *ie, const Ipv6Address& CoA);

    /**
     * This method destroys all tunnels associated to the previous CoA
     * and sends appropriate BU(s) to HA and CN(s).
     */
    void returningHome(const Ipv6Address& CoA, NetworkInterface *ie);

//
// Route Optimization related functions
//
    /**
     *  The following method is used for triggering RO to a CN.
     */
    virtual void triggerRouteOptimization(const Ipv6Address& destAddress,
            const Ipv6Address& HoA, NetworkInterface *ie);

  protected:
    /**
     * Creates HoTI and CoTI messages and sends them to the CN if timers are not already existing.
     * If home and care-of tokens are already available a BU is directly sent to the CN.
     */
    virtual void initReturnRoutability(const Ipv6Address& cnDest, NetworkInterface *ie);

    /**
     * Creates and schedules a timer for either a HoTI or a CoTI transmission.
     */
    void createTestInitTimer(const Ptr<MobilityHeader> testInit, const Ipv6Address& dest, NetworkInterface *ie, simtime_t sendTime = 0);

    /**
     * If a TestInit timer was fired, this method gets called. The message is sent and the Binding Update List accordingly updated.
     * Afterwards the transmission timer is rescheduled.
     */
    void sendTestInit(cMessage *msg);

    /**
     * Cancels the current existing timer and reschedules it with initial values.
     */
    /*void resetTestInitIfEntry(const Ipv6Address& dest, int interfaceID, int msgType);*/

    /**
     * Similiar to the other resetTestInitIfEntry() method, but this one searches for the appropriate
     * transmission structure first as the interfaceID is not known but needed as lookup key.
     */
//    void resetTestInitIfEntry(const Ipv6Address& dest, int msgType);

    /**
     * Reset the transmission structure for a BU and reschedule it for the provided time.
     */
    void resetBUIfEntry(const Ipv6Address& dest, int interfaceID, simtime_t retransmissionTime);

    /**
     * Creates and sends a HoTI message to the specified destination.
     */
    void createAndSendHoTIMessage(const Ipv6Address& cnDest, NetworkInterface *ie);

    /**
     * Creates and sends a CoTI message to the specified destination.
     */
    void createAndSendCoTIMessage(const Ipv6Address& cnDest, NetworkInterface *ie);

    /**
     * Create and send a HoT message.
     */
    void processHoTIMessage(Packet *inPacket, const Ptr<const HomeTestInit>& HoTI);

    /**
     * Create and send a CoT message.
     */
    void processCoTIMessage(Packet *inPacket, const Ptr<const CareOfTestInit>& CoTI);

    /**
     * First verifies a received HoT message and sends a BU to the CN if the care-of keygen token
     * is available as well. Retransmission of HoTI message is rescheduled.
     */
    void processHoTMessage(Packet *inPacket, const Ptr<const HomeTest>& HoT);

    /**
     * Verifies a HoT according to the RFC, Section 11.6.2
     */
    bool validateHoTMessage(Packet *inPacket, const HomeTest& HoT);

    /**
     * Like processHoTMessage(), but related to CoT.
     */
    void processCoTMessage(Packet *inPacket, const Ptr<const CareOfTest>& CoT);

    /**
     * Like validateHoTMessage(), but related to CoT.
     */
    bool validateCoTMessage(Packet *inPacket, const CareOfTest& CoT);

    /**
     * Send a BU depending on current status of:
     * * Registration or Deregistration phase
     * * Availability of tokens
     *
     * Return true or false depending on whether a BU has been sent or not.
     */
    bool checkForBUtoCN(BindingUpdateList::BindingUpdateListEntry& bulEntry, NetworkInterface *ie);

    /**
     * Creates a timer for sending a BU.
     */
    void sendBUtoCN(BindingUpdateList::BindingUpdateListEntry& bulEntry, NetworkInterface *ie);

    /**
     * Process the Type 2 Routing Header which belongs to the provided datagram.
     *
     * Swaps the addresses between the original destination address of the datagram and
     * the field in the routing header.
     */
    void processType2RH(Packet *packet, Ipv6RoutingHeader *rh);

    /**
     * Perform validity checks according to RFC 3775 - Section 6.4
     */
    bool validateType2RH(const Ipv6Header& ipv6Header, const Ipv6RoutingHeader& rh);

    /**
     * Process the Home Address Option which belongs to the provided datagram.
     *
     * Swaps the addresses between the original source address of the datagram and
     * the field in the option.
     */
    void processHoAOpt(Packet *packet, HomeAddressOption *hoaOpt);

//
// Binding Refresh Request related functions
//

    /**
     * Creates a timer for a Binding Refresh Request message that is going to be fired in scheduledTime seconds.
     */
    void createBRRTimer(const Ipv6Address& brDest, NetworkInterface *ie, const uint scheduledTime);

    /**
     * Handles a fired BRR message transmission structure.
     * Creates and sends and appropriate Binding Refresh Request.
     * Transmission structure is rescheduled afterwards.
     */
    void sendPeriodicBRR(cMessage *msg);

    /**
     * Creates a Binding Refresh Request and sends it to the Ipv6 module.
     */
    void createAndSendBRRMessage(const Ipv6Address& dest, NetworkInterface *ie);

    /**
     * Processes the Binding Refresh Message.
     */
    void processBRRMessage(Packet *inPacket, const Ptr<const BindingRefreshRequest>& brr);

  protected:
//
// Helper functions
//

    /**
     * Deletes the appropriate entry from the transmitIfList and cancels
     * the corresponding retransmission timer.
     */
    bool cancelTimerIfEntry(const Ipv6Address& dest, int interfaceID, int msgType);

    /**
     * Checks whether there exists an TransmitIfEntry for the specified values.
     * In case a new TODOTimerIfEntry is added, this method has to be appropriately extended in order
     * to cover the new data structure.
     * Returns true on success and false otherwise.
     */
    bool pendingTimerIfEntry(Ipv6Address& dest, int interfaceID, int msgType);

    /**
     * Returns a pointer to an TimerIfEntry object identified by the provided key,
     * which can be one of the possible polymorphic types. In case there does not yet
     * exist such an entry, a new one is created.
     * The type of the TimerIfEntry is specified with the provided timerType.
     */
    TimerIfEntry *getTimerIfEntry(Key& key, int timerType);

    /**
     * Searches for a transmitEntry with the given destination address which is of type
     * timerType.
     * Returns nullptr if no such entry exists.
     */
    TimerIfEntry *searchTimerIfEntry(Ipv6Address& dest, int timerType);

    /**
     * Removes timers of all types for the specified destination address and interfaceId.
     * Whenever a new mobility related timer is added, is MUST be added within this method
     * to ensure proper removal.
     */
    void removeTimerEntries(const Ipv6Address& dest, int interfaceId);

    /**
     * Cancel all timers (TransmitIf entities for HA and CNs) related to the provided interfaceId and CoA.
     * In addition the tunnels to the Home Agent and the CNs are destroyed as well.
       0     */
    void cancelEntries(int interfaceId, Ipv6Address& CoA);

    /**
     * Remove all entries from the interfaceCoAList.
     */
    void removeCoAEntries();

//
// Helper functions for BUL/BC expiry management
//
    /**
     * Creates or overwrites a timer for BUL expiry that fires at provided scheduledTime.
     */
    void createBULEntryExpiryTimer(BindingUpdateList::BindingUpdateListEntry *entry,
            NetworkInterface *ie, simtime_t scheduledTime);

    /**
     * Handles the situation of a BUL expiry. Either a BU is sent in advance for renewal or the BUL entry is removed.
     */
    void handleBULExpiry(cMessage *msg);

    /**
     * Creates or overwrites a timer for BC expiry that fires at provided scheduledTime.
     */
    void createBCEntryExpiryTimer(const Ipv6Address& HoA, NetworkInterface *ie, simtime_t scheduledTime);

    /**
     * Handles the expiry of a BC entry.
     * Entry is removed from BC and tunnels/routing paths are destroyed.
     */
    void handleBCExpiry(cMessage *msg);

//
// Helper functions for token expiry
//
    /**
     * Creates or overwrites a timer for home keygen token expiry that fires at provided scheduledTime.
     */
    void createHomeTokenEntryExpiryTimer(Ipv6Address& cnAddr, NetworkInterface *ie, simtime_t scheduledTime)
    {
        createTokenEntryExpiryTimer(cnAddr, ie, scheduledTime, KEY_HTOKEN_EXP);
    }

    /**
     * Creates or overwrites a timer for care-of keygen token expiry that fires at provided scheduledTime.
     */
    void createCareOfTokenEntryExpiryTimer(Ipv6Address& cnAddr, NetworkInterface *ie, simtime_t scheduledTime)
    {
        createTokenEntryExpiryTimer(cnAddr, ie, scheduledTime, KEY_CTOKEN_EXP);
    }

  private:
    /**
     * Creates or overwrites a timer for {home, care-of} keygen token expiry that fires at provided scheduledTime.
     * Parameter tokenType is provided as KEY_XTOKEN_EXP
     */
    void createTokenEntryExpiryTimer(Ipv6Address& cnAddr, NetworkInterface *ie, simtime_t scheduledTime, int tokenType);

    /**
     * Handles the event that indicates that a {care-of,home} keygen token has expired.
     */
    void handleTokenExpiry(cMessage *msg);
};

} // namespace inet

#endif

