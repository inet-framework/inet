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

#ifndef __INET_MIPV6_H
#define __INET_MIPV6_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"

#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv6/IIpv6ExtensionHeaderHandler.h"
#include "inet/networklayer/mipv6/BindingUpdateList.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h" // for HAOpt & RH2

namespace inet {

// Foreign declarations:
class BindingCache;
class BindingUpdate;
class NetworkInterface;
class Ipv6Header;
class Ipv6NeighbourDiscovery;
class Ipv6RoutingTable;

// Keys for timer list (=message type)
enum TimerKey {
    KEY_BU               = 0, // Binding Update
    KEY_HI               = 1, // HoTI
    KEY_CI               = 2, // CoTI
    KEY_BR               = 3, // Binding Refresh Request
    KEY_BUL_EXP          = 4, // BUL entry expiry
    KEY_BC_EXP           = 5, // BC entry expiry
    KEY_HTOKEN_EXP       = 6, // home token expiry
    KEY_CTOKEN_EXP       = 7, // care-of token expiry
};

// Timer-if-entry type codes
enum TimerIfEntryType {
    TRANSMIT_TYPE_BU     = 51, // BuTransmitIfEntry
    TRANSMIT_TYPE_TI     = 52, // TestInitTransmitIfEntry
    EXPIRY_TYPE_BUL      = 61, // BulExpiryIfEntry
    EXPIRY_TYPE_BC       = 62, // BcExpiryIfEntry
    EXPIRY_TYPE_TOKEN    = 63, // {Home, CareOf}TokenExpiryIfEntry
};

/**
 * Implements RFC 3775 Mobility Support in Ipv6.
 */
class INET_API Mipv6 : public OperationalBase, public IIpv6ExtensionHeaderHandler, public IIpv6TlvOptionHandler, public NetfilterBase::HookBase
{
  public:
    virtual ~Mipv6();

  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    opp_component_ptr<Ipv6RoutingTable> rt6;
    ModuleRefByPar<BindingUpdateList> bul;
    ModuleRefByPar<BindingCache> bc;
    ModuleRefByPar<Ipv6NeighbourDiscovery> ipv6nd;

    //
    // IP tunnel management (RFC 2473), moved here from the former Ipv6Tunneling
    // module. A "tunnel" is a dynamically created Ipv6TunnelInterface (built by
    // Ipv6RoutingTable::createTunnelNetworkInterface) that performs IPv6-in-IPv6
    // encapsulation; the IPv6 layer routes to it like any interface. Tunnels are
    // indexed by their interface id. Mipv6 steers home-address traffic onto them
    // via requestTunnelOutputInterface().
    //
    enum TunnelType {
        INVALID = 0,
        SPLIT,
        NON_SPLIT,
        NORMAL, // either split or non-split
    };

    struct Tunnel {
        Tunnel(const Ipv6Address& entry = Ipv6Address::UNSPECIFIED_ADDRESS,
                const Ipv6Address& exit = Ipv6Address::UNSPECIFIED_ADDRESS,
                const Ipv6Address& destTrigger = Ipv6Address::UNSPECIFIED_ADDRESS)
            : entry(entry), exit(exit), tunnelType(SPLIT), destTrigger(destTrigger) {}

        bool operator==(const Tunnel& rhs) const
        {
            return entry == rhs.entry && exit == rhs.exit && destTrigger == rhs.destTrigger;
        }

        Ipv6Address entry; // entry point of the tunnel
        Ipv6Address exit; // exit point of the tunnel
        TunnelType tunnelType = INVALID; // split or non-split (defaults to SPLIT, see ctor)
        // if set, only packets to this destination are routed over the tunnel
        // (split tunnel); if unspecified, (nearly) everything is (non-split tunnel)
        Ipv6Address destTrigger;
        // the backing virtual interface that performs the encapsulation
        NetworkInterface *networkInterface = nullptr;
    };

    typedef std::map<int, struct Tunnel> Tunnels;
    Tunnels tunnels; // indexed by the tunnel interface id
    int noOfNonSplitTunnels = 0; // number of non-split tunnels on this host

    /**
     * Create a tunnel (entry -> exit). If destTrigger is set, only traffic to that
     * destination is routed over it (split tunnel); otherwise everything is
     * (non-split tunnel). Returns the tunnel interface id (0 if not created).
     */
    int createTunnel(TunnelType tunnelType, const Ipv6Address& entry, const Ipv6Address& exit,
            const Ipv6Address& destTrigger = Ipv6Address::UNSPECIFIED_ADDRESS);
    bool destroyTunnel(const Ipv6Address& src, const Ipv6Address& dest, const Ipv6Address& destTrigger);
    void destroyTunnels(const Ipv6Address& entry);
    void destroyTunnel(const Ipv6Address& entry, const Ipv6Address& exit);
    void destroyTunnelForExitAndTrigger(const Ipv6Address& exit, const Ipv6Address& trigger);
    void destroyTunnelForEntryAndTrigger(const Ipv6Address& entry, const Ipv6Address& trigger);
    void destroyTunnelFromTrigger(const Ipv6Address& trigger);
    /**
     * Returns the tunnel interface id whose destination trigger matches destAddress
     * (split tunnels first, then the non-split catch-all), or -1 if none.
     */
    int getVIfIndexForDest(const Ipv6Address& destAddress);
    int getVIfIndexForDest(const Ipv6Address& destAddress, TunnelType tunnelType);
    // true if there exists a tunnel with the given exit point
    bool isTunnelExit(const Ipv6Address& exit);
    // returns the interface id of the matching tunnel, or 0 if none
    int findTunnel(const Ipv6Address& src, const Ipv6Address& dest, const Ipv6Address& destTrigger) const;
    int lookupTunnels(const Ipv6Address& dest);
    int doPrefixMatch(const Ipv6Address& dest);

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

    /** whether this mobile node is currently at home; toggled at the home/away
     *  transitions (returningHome() / initiateMipv6Protocol()). Used only for the
     *  display string -- interfaceCoAList is not a reliable "at home" indicator
     *  because it is not cleared on returning home. */
    bool atHome = true;

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
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

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
    Mipv6::BuTransmitIfEntry *fetchBUTransmitIfEntry(NetworkInterface *ie, const Ipv6Address& dest);

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
    bool processType2RH(Packet *packet, Ipv6RoutingHeader *rh);

    // IIpv6ExtensionHeaderHandler
    virtual bool processExtensionHeader(Packet *packet, const Ipv6ExtensionHeader *eh) override;
    // IIpv6TlvOptionHandler
    virtual bool processTlvOption(Packet *packet, const Ipv6ExtensionHeader *eh, const TlvOptionBase *option) override;

    // INetfilter::IHook -- Mipv6 inserts the Type 2 Routing Header (CN->MN) and
    // the Home Address Option (MN->CN) on locally-originated, route-optimized
    // traffic via the datagramLocalOutHook (instead of the old T2RH/HA_OPT
    // pseudo-tunnels in Ipv6Tunneling).
    virtual Result datagramPreRoutingHook(Packet *datagram) override;
    virtual Result datagramForwardHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *datagram) override;

    /**
     * If the datagram is home-address-destined traffic at a home agent or
     * home-address-sourced traffic at a mobile node, request the corresponding tunnel
     * interface as its output interface (called from the local-out and pre-routing
     * hooks). Replaces the tunnel lookup that used to be in the Ipv6 module.
     */
    void requestTunnelOutputInterface(Packet *datagram);

    /**
     * A route-optimization extension-header insertion that the local-out hook
     * applies to outgoing traffic, replacing a T2RH/HA_OPT pseudo-tunnel.
     */
    struct RouteOptimization {
        enum Type { TYPE2_ROUTING_HEADER, HOME_ADDRESS_OPTION };
        Type type = TYPE2_ROUTING_HEADER;
        Ipv6Address entry; // T2RH: CN address; HA_OPT: care-of address
        Ipv6Address exit;  // T2RH: care-of address; HA_OPT: home address
    };

    // route-optimization insertions, keyed by the destination that triggers them
    // (T2RH: the MN home address; HA_OPT: the correspondent node address)
    std::map<Ipv6Address, RouteOptimization> routeOptimizations;

    void addRouteOptimization(RouteOptimization::Type type, const Ipv6Address& entry, const Ipv6Address& exit, const Ipv6Address& trigger);
    void removeRouteOptimizationForTrigger(const Ipv6Address& trigger);
    void removeRouteOptimizationForExitAndTrigger(const Ipv6Address& exit, const Ipv6Address& trigger);

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
    bool processHoAOpt(Packet *packet, HomeAddressOption *hoaOpt);

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

    /**
     * Human-readable mobility status of this (mobile) node, for the display string
     * (WATCH_EXPR "mobilityStatus"): "at home", "away (via home agent)" or
     * "route-optimized (N CN[s])".
     */
    std::string getMobilityStatusInfo() const;

    /**
     * The node's current care-of address as a string, or "(home)" when it has none,
     * for the display string (WATCH_EXPR "careOfAddress").
     */
    std::string getCareOfAddressInfo() const;

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

