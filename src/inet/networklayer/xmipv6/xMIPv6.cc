/**
 * Copyright (C) 2007
 * Faqir Zarrar Yousaf
 * Communication Networks Institute, University of Dortmund, Germany.
 * Christian Bauer
 * Institute of Communications and Navigation, German Aerospace Center (DLR)

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"
#include "inet/networklayer/xmipv6/BindingCache.h"
#include "inet/networklayer/xmipv6/BindingUpdateList.h"
#include "inet/networklayer/xmipv6/xMIPv6.h"

namespace inet {

#define MK_SEND_PERIODIC_BU         1
#define MK_SEND_PERIODIC_BR         2
#define MK_SEND_TEST_INIT           11
#define MK_BUL_EXPIRY               21
#define MK_BC_EXPIRY                22
#define MK_TOKEN_EXPIRY             23
#define BRR_TIMEOUT_THRESHOLD       5        // time in seconds before the expiry of a BU when a Binding Refresh Msg. will be sent
#define BRR_RETRIES                 4        // number of BRRs to be sent to MN
#define MAX_TOKEN_LIFETIME          500        //210  // maximum valid lifetime for the tokens used in RR
#define MAX_RR_BINDING_LIFETIME     4000        //420  // maximum valid lifetime of a binding for CNs
#define TEST_INIT_RETRANS_FACTOR    8        // HoTI and CoTI will be retransmitted every MAX_RR_BINDING_LIFETIME * TEST_INIT_RETRANS_FACTOR seconds

// sizes of mobility messages and headers in bytes
#define SIZE_MOBILITY_HEADER        6    // 6.1.1 mobility header = 48 bit
#define SIZE_BU                     6    // 6.1.7 BU message = 48 bit
#define SIZE_HOA_OPTION             20    // HoA option = 128+16 bit + 16 (dest. opts. header)
#define SIZE_BIND_AUTH_DATA         14    // 6.2.7 Binding Auth. Data = 112 bit
#define SIZE_BACK                   6    // 6.1.8 BAck message = 48 bit
#define SIZE_NONCE_INDICES          6    // 6.2.6 Nonce Indices = 48 bit (PadN = 16 bit) -> no padding required for nonce indices
#define SIZE_HOTI                   10    // 6.1.3 HoTI = 80 bit
#define SIZE_COTI                   10    // 6.1.4 CoTI = 80 bit
#define SIZE_HOT                    18    // 6.1.5 HoT = 144 bit
#define SIZE_COT                    18    // 6.1.6 CoT = 144 bit
#define SIZE_BE                     18    // 6.1.9 BE message = 144 bit
#define SIZE_BRR                    2    // 6.1.2 BRR reserved = 16 bit

Define_Module(xMIPv6);

/**
 * Destructur
 *
 * Ensures that the memory from the list with all TimerIfEntry's gets
 * properly released.
 */
xMIPv6::~xMIPv6()
{
    auto it = transmitIfList.begin();

    while (it != transmitIfList.end()) {
        Key key = it->first;

        // advance pointer to make sure it does not become invalid
        // after the cancel() call
        ++it;

        cancelTimerIfEntry(key.dest, key.interfaceID, key.type);
    }
}

void xMIPv6::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        EV_TRACE << "Initializing xMIPv6 module" << endl;

        // statistic collection
        /*statVectorBUtoHA.setName("BU to HA");
           statVectorBUtoCN.setName("BU to CN");
           statVectorBUtoMN.setName("BU to MN");
           statVectorBAtoMN.setName("BA to MN");
           statVectorBAfromHA.setName("BA from HA");
           statVectorBAfromCN.setName("BA from CN");
           statVectorHoTItoCN.setName("HoTI to CN");
           statVectorCoTItoCN.setName("CoTI to CN");
           statVectorHoTtoMN.setName("HoT to MN");
           statVectorCoTtoMN.setName("CoT to MN");
           statVectorHoTfromCN.setName("HoT from CN");
           statVectorCoTfromCN.setName("CoT from CN");*/

        tunneling = getModuleFromPar<Ipv6Tunneling>(par("ipv6TunnelingModule"), this);    // access to tunneling module

        // moved rt6 initialization to here, as we should
        // set the MIPv6 flag as soon as possible for use
        // with other modules.
        cModule *host = getContainingNode(this);
        rt6 = L3AddressResolver().getIpv6RoutingTableOf(host);
        rt6->setIsMobileNode(par("isMobileNode"));
        rt6->setMipv6Support(true);
        rt6->setIsHomeAgent(par("isHomeAgent"));
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        ipv6nd = getModuleFromPar<Ipv6NeighbourDiscovery>(par("ipv6NeighbourDiscoveryModule"), this);

        if (rt6->isMobileNode()) {
            bul = getModuleFromPar<BindingUpdateList>(par("bindingUpdateListModule"), this);
            bc = nullptr;
        }
        else {
            bc = getModuleFromPar<BindingCache>(par("bindingCacheModule"), this);
            bul = nullptr;
        }

        WATCH_VECTOR(cnList);
        WATCH_MAP(interfaceCoAList);
    }
}

void xMIPv6::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_DETAIL << "Self message received!\n";

        if (msg->getKind() == MK_SEND_PERIODIC_BU) {
            EV_DETAIL << "Periodic BU Timeout Message Received\n";
            sendPeriodicBU(msg);
        }
        else if (msg->getKind() == MK_SEND_PERIODIC_BR) {
            EV_DETAIL << "Periodic BRR Timeout Message Received\n";
            sendPeriodicBRR(msg);
        }
        else if (msg->getKind() == MK_SEND_TEST_INIT) {
            EV_DETAIL << "HoTI/CoTI Timeout Message Received\n";
            sendTestInit(msg);
        }
        else if (msg->getKind() == MK_BUL_EXPIRY) {
            EV_DETAIL << "BUL Expiry Timeout Message Received\n";
            handleBULExpiry(msg);
        }
        else if (msg->getKind() == MK_BC_EXPIRY) {
            EV_DETAIL << "BUL Expiry Timeout Message Received\n";
            handleBCExpiry(msg);
        }
        else if (msg->getKind() == MK_TOKEN_EXPIRY) {
            EV_DETAIL << "RR token expired" << endl;
            handleTokenExpiry(msg);
        }
        else
            throw cRuntimeError("Unrecognized Timer"); //stops sim w/ error msg.
    }
    // if its a MIPv6 related mobility message
    else {
        auto packet = check_and_cast<Packet *>(msg);
        if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::mobileipv6) {
            EV_INFO << " Received MIPv6 related message" << endl;
            processMobilityMessage(packet);
        }
        // CB on 29.08.07
        // normal datagram with an extension header
        else if (auto packet = dynamic_cast<Packet *>(msg)) {
            Ipv6ExtensionHeader *eh = (Ipv6ExtensionHeader *)packet->getContextPointer();
            if (auto rh = dynamic_cast<Ipv6RoutingHeader *>(eh))
                processType2RH(packet, rh);
            else if (auto hao = dynamic_cast<HomeAddressOption *>(eh))
                processHoAOpt(packet, hao);
            else
                throw cRuntimeError("Unknown Extension Header.");
        }
        else
            throw cRuntimeError("Unknown message type received.");
    }
}

void xMIPv6::processMobilityMessage(Packet *inPacket)
{
    const auto& mipv6Msg = inPacket->peekAtFront<MobilityHeader>();

    EV_INFO << "Processing of MIPv6 related mobility message" << endl;

    if (const auto& bu = dynamicPtrCast<const BindingUpdate>(mipv6Msg)) {
        EV_INFO << "Message recognised as BINDING UPDATE (BU)" << endl;
        //EV << "\n<<<<<<<<Giving Control to processBUMessage()>>>>>>>\n";
        processBUMessage(inPacket, bu);
    }
    else if (const auto& ba = dynamicPtrCast<const BindingAcknowledgement>(mipv6Msg)) {
        EV_INFO << "Message recognised as BINDING ACKNOWLEDGEMENT (BA)" << endl;
        //EV << "\n<<<<<<<<Giving Control to processBAMessage()>>>>>>>\n";
        processBAMessage(inPacket, ba);
    }
    // 28.08.07 - CB
    else if (const auto& hoti = dynamicPtrCast<const HomeTestInit>(mipv6Msg)) {
        EV_INFO << "Message recognised as HOME TEST INIT (HoTI)" << endl;
        processHoTIMessage(inPacket, hoti);
    }
    else if (const auto& coti = dynamicPtrCast<const CareOfTestInit>(mipv6Msg)) {
        EV_INFO << "Message recognised as CARE-OF TEST INIT (CoTI)" << endl;
        processCoTIMessage(inPacket, coti);
    }
    else if (const auto& ht = dynamicPtrCast<const HomeTest>(mipv6Msg)) {
        EV_INFO << "Message recognised as HOME TEST (HoT)" << endl;
        processHoTMessage(inPacket, ht);
    }
    else if (const auto& cot = dynamicPtrCast<const CareOfTest>(mipv6Msg)) {
        EV_INFO << "Message recognised as CARE-OF TEST (CoT)" << endl;
        processCoTMessage(inPacket, cot);
    }
    else if (const auto& brr = dynamicPtrCast<const BindingRefreshRequest>(mipv6Msg)) {
        EV_INFO << "Message recognised as Binding Refresh Request" << endl;
        processBRRMessage(inPacket, brr);
    }
    else {
        EV_WARN << "Unrecognised mobility message... Dropping" << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, inPacket, &details);
        delete inPacket;
    }
}

void xMIPv6::initiateMipv6Protocol(InterfaceEntry *ie, const Ipv6Address& CoA)
{
    Enter_Method_Silent();    // can be called by NeighborDiscovery module

    if (!(ie->isLoopback()) && rt6->isMobileNode()) {
        EV_INFO << "Initiating Mobile Ipv6 protocol..." << endl;

        // The MN is supposed to send a BU to the HA after forming a CoA
        Ipv6Address haDest = ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress();    // HA address for use in the BU for Home Registration

        createBUTimer(haDest, ie);

        // RO with CNs is triggered after receiving a valid BA from the HA
    }

    // a movement occured -> BUL entries for CNs not valid anymore
    Ipv6Address HoA = ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress();

    for (auto cn : cnList) {    // run an iterator through the CN map
        BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->fetch(cn);
        ASSERT(bulEntry != nullptr);
        //bul->resetBindingCacheEntry(*bulEntry, HoA);
        bul->removeBinding(cn);         //FIXME need revision: this function sometimes remove the entry from bul, and bul->resetCareOfToken(cn, HoA) creates assert
        // care-of token becomes invalid with new CoA
        bul->resetCareOfToken(cn, HoA);
        tunneling->destroyTunnelForExitAndTrigger(HoA, cn);
    }
}

/**
 * This method destroys the HA tunnel associated to the previous CoA
 * and sends an appropriate BU to the HA.
 */
void xMIPv6::returningHome(const Ipv6Address& CoA, InterfaceEntry *ie)
{
    Enter_Method_Silent();    // can be called by NeighborDiscovery module

    /*11.5.4
       A mobile node detects that it has returned to its home link through
       the movement detection algorithm in use (Section 11.5.1), when the
       mobile node detects that its home subnet prefix is again on-link.*/

    EV_INFO << "MIPv6 Returning home procedure..." << endl;

    // cancel timers

    // eventually we could have some unacknowledged BUs
    // we have to cancel these: first the one for the Home Agent...
    const Ipv6Address& HA = ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress();
    removeTimerEntries(HA, ie->getInterfaceId());
    // destroy tunnel to HA
    tunneling->destroyTunnel(CoA, HA);
    // unregister binding from HA..
    createDeregisterBUTimer(HA, ie);
    bul->setMobilityState(HA, BindingUpdateList::DEREGISTER);

    // ...and then the messages for CNs
    for (itCNList = cnList.begin(); itCNList != cnList.end(); itCNList++) {    // run an iterator through the CN map
        // we first cancel potential timers for the respective CN
        removeTimerEntries(*(itCNList), ie->getInterfaceId());

        // then we send the BU for deregistration
        if (bul->isValidBinding(*(itCNList))) {
            Ipv6Address cn = *(itCNList);
            //createDeregisterBUTimer(*(itCNList), ie);
            BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->lookup(cn);
            ASSERT(bulEntry != nullptr);
            bulEntry->state = BindingUpdateList::DEREGISTER;
            checkForBUtoCN(*bulEntry, ie);
        }

        // destroy the tunnel now
        // we have to as it is invalid (bound to old CoA that is not available anymore)
        tunneling->destroyTunnelForExitAndTrigger(ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress(), *(itCNList));
    }
}

void xMIPv6::createBUTimer(const Ipv6Address& buDest, InterfaceEntry *ie)
{
    // if we send a new BU we can delete any potential existing BUL expiry timer for this destination
    cancelTimerIfEntry(buDest, ie->getInterfaceId(), KEY_BUL_EXP);

    BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->fetch(buDest);
    ASSERT(bulEntry != nullptr);

    if (bulEntry->state != BindingUpdateList::DEREGISTER)
        bulEntry->state = BindingUpdateList::REGISTER;

    // update lifetime, 14.9.07
    //if (homeRegistration)
    if (buDest == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress())
        createBUTimer(buDest, ie, ie->getProtocolData<Ipv6InterfaceData>()->_getMaxHaBindingLifeTime(), true);
    else {
        if (bulEntry->state == BindingUpdateList::DEREGISTER)
            createDeregisterBUTimer(buDest, ie);
        else
            createBUTimer(buDest, ie, ie->getProtocolData<Ipv6InterfaceData>()->_getMaxRrBindingLifeTime(), false);
    }
}

void xMIPv6::createDeregisterBUTimer(const Ipv6Address& buDest, InterfaceEntry *ie)
{
    /*11.5.4
       The mobile node SHOULD then send a Binding Update to its home agent,
       to instruct its home agent to no longer intercept or tunnel packets
       for it. In this home registration, the mobile node MUST set the
       Acknowledge (A) and Home Registration (H) bits, set the Lifetime
       field to zero, and set the care-of address for the binding to the
       mobile node's own home address.  The mobile node MUST use its home
       address as the source address in the Binding Update.*/

    /*11.7.2
       If the Binding Update is sent to the correspondent node, requesting
       the deletion of any existing Binding Cache entry it has for the
       mobile node, the care-of address is set to the mobile node's home
       address and the Lifetime field set to zero.*/

    //createBUTimer(buDest, ie, 0, homeRegistration);
    createBUTimer(buDest, ie, 0, buDest == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress());
}

void xMIPv6::createBUTimer(const Ipv6Address& buDest, InterfaceEntry *ie, const uint lifeTime,
        bool homeRegistration)
{
    Enter_Method("createBUTimer()");
    EV_INFO << "Creating BU timer at sim time: " << simTime() << " seconds." << endl;
    cMessage *buTriggerMsg = new cMessage("sendPeriodicBU", MK_SEND_PERIODIC_BU);

    // check if there already exists a BUTimer entry for this key
    Key key(buDest, ie->getInterfaceId(), KEY_BU);
    // fetch a valid TimerIfEntry obect
    BuTransmitIfEntry *buIfEntry = check_and_cast<BuTransmitIfEntry *>(getTimerIfEntry(key, TRANSMIT_TYPE_BU));
    // TODO: Investigate problem witht he following line. : runtime error because of attempted message rescheduling
    //cancelAndDelete(buIfEntry->timer);

    buIfEntry->dest = buDest;
    buIfEntry->ifEntry = ie;
    buIfEntry->timer = buTriggerMsg;

    // retrieve sequence number from BUL
    // if no entry exists, the method will return 0
    buIfEntry->buSequenceNumber = bul->getSequenceNumber(buDest);    //the sequence number gets initialized and stored here

    buIfEntry->lifeTime = lifeTime;

    /*11.8
       If the mobile node is sending a Binding Update and does not have
       an existing binding at the home agent, it SHOULD use
       InitialBindackTimeoutFirstReg (see Section 13) as a value for the
       initial retransmission timer.*/
    if (!bul->isInBindingUpdateList(buDest))
        buIfEntry->ackTimeout = ie->getProtocolData<Ipv6InterfaceData>()->_getInitialBindAckTimeoutFirst(); //the backoff constant gets initialised here
    /*Otherwise, the mobile node should use the specified value of
       INITIAL_BINDACK_TIMEOUT for the initial retransmission timer.*/
    else
        buIfEntry->ackTimeout = ie->getProtocolData<Ipv6InterfaceData>()->_getInitialBindAckTimeout(); // if there's an entry in the BUL, use different value

    buIfEntry->homeRegistration = homeRegistration;

    buTriggerMsg->setContextPointer(buIfEntry);    // attaching the buIfEntry info corresponding to a particular address ith message

    // send BU now
    //scheduleAt(buIfEntry->initScheduledBUTime, buTriggerMsg); //Scheduling a message which will trigger a BU towards buIfEntry->dest
    scheduleAt(simTime(), buTriggerMsg);    //Scheduling a message which will trigger a BU towards buIfEntry->dest
}

void xMIPv6::sendPeriodicBU(cMessage *msg)
{
    EV_INFO << "Sending periodic BU message at time: " << simTime() << " seconds." << endl;
    BuTransmitIfEntry *buIfEntry = (BuTransmitIfEntry *)msg->getContextPointer();    //detaching the corresponding buIfEntry pointer
    //EV << "### lifetime of buIfEntry=" << buIfEntry->lifeTime << " and seq#= " << buIfEntry->buSequenceNumber << endl;
    InterfaceEntry *ie = buIfEntry->ifEntry;    //copy the ie info
    Ipv6Address& buDest = buIfEntry->dest;
    buIfEntry->presentSentTimeBU = simTime();    //records the present time at which BU is sent

    buIfEntry->nextScheduledTime = buIfEntry->presentSentTimeBU + buIfEntry->ackTimeout;
    /*11.8
       The retransmissions by the mobile node MUST use an exponential back-
       off process in which the timeout period is doubled upon each
       retransmission*/
    buIfEntry->ackTimeout = (buIfEntry->ackTimeout) * 2;

    /*Each Binding Update MUST have a Sequence Number greater than the
       Sequence Number value sent in the previous Binding Update to the same
       destination address (if any). The sequence numbers are compared
       modulo 2**16, as described in Section 9.5.1.*/
    buIfEntry->buSequenceNumber = (buIfEntry->buSequenceNumber + 1) % 65536;

    // Added by CB, 28.08.07
    if (!buIfEntry->homeRegistration) {    // this BU goes to a CN
        //Ipv6Address CoA = ie->ipv6()->globalAddress();
        Ipv6Address CoA = bul->getCoA(buDest);    // 24.9.07 - CB
        //TODO think of a good mechanism to obtain the appropriate/correct CoA
        // Problem 1: ie->ipv6()->globalAddress() retrieves the HoA
        // Problem 2: bul->getCoA(buDest) becomes a problem in case of Multihoming
        // Solution: globalAddress(TYPE_COA) ?
        int bindingAuthorizationData = bul->generateBAuthData(buDest, CoA);
        createAndSendBUMessage(buDest, ie, buIfEntry->buSequenceNumber, buIfEntry->lifeTime, bindingAuthorizationData);

        // statistic collection
        /*statVectorBUtoCN.record(1);*/
    }
    else {
        createAndSendBUMessage(buDest, ie, buIfEntry->buSequenceNumber, buIfEntry->lifeTime);

        // statistic collection
        /*statVectorBUtoHA.record(1);*/
    }

    /*if (buIfEntry->ackTimeout < ie->ipv6()->_maxBindAckTimeout())
       {
        //buIfEntry->presentBindAckTimeout = buIfEntry->nextBindAckTimeout; //reassign the timeout value
        //scheduleAt(buIfEntry->nextScheduledTime, msg);
       }
       else*/
    if (!(buIfEntry->ackTimeout < ie->getProtocolData<Ipv6InterfaceData>()->_getMaxBindAckTimeout())) {
        EV_DETAIL << "Crossed maximum BINDACK timeout...resetting to predefined maximum." << endl;    //buIfEntry->nextBindAckTimeout << " ++++++\n";
        //ev << "\n++++Present Sent Time: " << buIfEntry->presentSentTimeBU << " Present TimeOut: " << buIfEntry->ackTimeout << endl;
        //buIfEntry->nextScheduledTime = buIfEntry->presentSentTimeBU + buIfEntry->maxBindAckTimeout;
        buIfEntry->ackTimeout = ie->getProtocolData<Ipv6InterfaceData>()->_getMaxBindAckTimeout();
        //buIfEntry->nextScheduledTime = ie->ipv6()->_maxBindAckTimeout();
        //ev << "\n++++Next Sent Time: " << buIfEntry->nextScheduledTime << endl;//" Next TimeOut: " << buIfEntry->nextBindAckTimeout << endl;
        //scheduleAt(buIfEntry->nextScheduledTime, msg);
    }

    EV_DETAIL << "Present Sent Time: " << buIfEntry->presentSentTimeBU << ", Present TimeOut: " << buIfEntry->ackTimeout << endl;
    EV_DETAIL << "Next Sent Time: " << buIfEntry->nextScheduledTime << endl;    // << " Next TimeOut: " << buIfEntry->nextBindAckTimeout << endl;
    scheduleAt(buIfEntry->nextScheduledTime, msg);
}

void xMIPv6::createAndSendBUMessage(const Ipv6Address& dest, InterfaceEntry *ie, const uint buSeq, const uint lifeTime, const int bindAuthData)
{
    EV_INFO << "Creating and sending Binding Update" << endl;
    // TODO use the globalAddress(Ipv6InterfaceData::CoA) in the address selection somewhere above (caller)
    Ipv6Address CoA = ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA);    // source address of MN

    if (CoA.isUnspecified())
        CoA = ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress(); // in case a CoA is not availabile (e.g. returning home)

    auto packet = new Packet("Binding Update");
    const auto& bu = makeShared<BindingUpdate>();

    /*11.7.1
       To register a care-of address or to extend the lifetime of an
       existing registration, the mobile node sends a packet to its home
       agent containing a Binding Update, with the packet constructed as
       follows:*/
    /*11.7.2
       A Binding Update is created as follows:*/
    bu->setMobilityHeaderType(BINDING_UPDATE);

    /*11.7.1
       o  The value specified in the Lifetime field MUST be non-zero and
         SHOULD be less than or equal to the remaining valid lifetime of
         the home address and the care-of address specified for the
         binding.*/
    /*6.1.7
       Lifetime
       16-bit unsigned integer.  The number of time units remaining
       before the binding MUST be considered expired.  A value of zero
       indicates that the Binding Cache entry for the mobile node MUST be
       deleted.  (In this case the specified care-of address MUST also be
       set equal to the home address.)  One time unit is 4 seconds.
     */
    bu->setLifetime(lifeTime / 4);

    bu->setSequence(buSeq);

    /*11.7.1
       o  The Acknowledge (A) bit MUST be set in the Binding Update.*/
    bu->setAckFlag(true);

    /*o  The Home Registration (H) bit MUST be set in the Binding Update.*/
    // set flag depending on whether the BU goes to HA or not - CB
    bu->setHomeRegistrationFlag(dest == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress());

    /*11.7.1
       o  If the mobile node's link-local address has the same interface
         identifier as the home address for which it is supplying a new
         care-of address, then the mobile node SHOULD set the Link-Local
         Address Compatibility (L) bit.
       o  If the home address was generated using RFC 3041 [18], then the
         link local address is unlikely to have a compatible interface
         identifier.  In this case, the mobile node MUST clear the Link-
         Local Address Compatibility (L) bit.*/
    // The link identifiers are always the same in our simulations. As
    // long as this is not changing, we can stick to the value "true"
    bu->setLinkLocalAddressCompatibilityFlag(true);    // fine for now

    bu->setKeyManagementFlag(false);    // no IKE/IPsec available anyway

    /*11.7.1
       o  The packet MUST contain a Home Address destination option, giving
         the mobile node's home address for the binding.*/
    /*11.7.2
       o  The home address of the mobile node MUST be added to the packet in
         a Home Address destination option, unless the Source Address is
         the home address.*/
    Ipv6Address HoA = ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::HoA);
    ASSERT(!HoA.isUnspecified());

    // As every Ipv6 Datagram sending the BU has to have the Home Address Option, I have
    // made this field a part of BU message to ease my task of simulation...
    // this can be accessed from the InterfaceTable of the MN.
    bu->setHomeAddressMN(HoA);    //HoA of MN

    /*11.7.2
       o  The Mobility Header is constructed according to rules in Section
         6.1.7 and Section 5.2.6, including the Binding Authorization Data
         (calculated as defined in Section 6.2.7) and possibly the Nonce
         Indices mobility options.*/
    bu->setBindingAuthorizationData(bindAuthData);    // added for BU to CN

    int nonceIndicesSize = 0;

    if (!bu->getHomeRegistrationFlag())
        nonceIndicesSize = SIZE_NONCE_INDICES;

    // setting message size
    int bindAuthSize = 0;

    if (bindAuthData != UNDEFINED_BIND_AUTH_DATA)
        bindAuthSize = SIZE_BIND_AUTH_DATA; // (6.2.3 PadN = 16 bit) -> no padding required if nonces provided // TODO check whether nonces valid

    bu->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_BU + SIZE_HOA_OPTION + bindAuthSize + nonceIndicesSize));

    /*11.7.1
       When sending a Binding Update to its home agent, the mobile node MUST
       also create or update the corresponding Binding Update List entry, as
       specified in Section 11.7.2.*/
    updateBUL(bu.get(), dest, CoA, ie, simTime());

    packet->insertAtFront(bu);

    /*11.7.1
       o  The care-of address for the binding MUST be used as the Source
         Address in the packet's Ipv6 header, unless an Alternate Care-of
         Address mobility option is included in the Binding Update.  This
         option MUST be included in all home registrations, as the ESP
         protocol will not be able to protect care-of addresses in the Ipv6
         header.  (Mobile Ipv6 implementations that know they are using
         IPsec AH to protect a particular message might avoid this option.
         For brevity the usage of AH is not discussed in this document.)*/
    /*11.7.2
       o  The current care-of address of the mobile node MUST be sent either
         in the Source Address of the Ipv6 header, or in the Alternate
         Care-of Address mobility option.
       o  The Destination Address of the Ipv6 header MUST contain the
         address of the correspondent node.*/
    sendMobilityMessageToIPv6Module(packet, dest, CoA, ie->getInterfaceId());
    //sendMobilityMessageToIPv6Module(bu, dest);
}

void xMIPv6::updateBUL(BindingUpdate *bu, const Ipv6Address& dest, const Ipv6Address& CoA,
        InterfaceEntry *ie, const simtime_t sendTime)
{
    uint buLife = 4 * bu->getLifetime();    /* 6.1.7 One time unit is 4 seconds. */
    uint buSeq = bu->getSequence();

    Ipv6Address HoA = bu->getHomeAddressMN();

    // to point to the struct where i am globally recording the startisitcs for sent time and next sent time for the BU:
    BuTransmitIfEntry *buIfEntry = fetchBUTransmitIfEntry(ie, dest);

    if (buIfEntry == nullptr) {
        EV_WARN << "No scheduled BU entry available!\n";
        return;
    }

    //simtime_t sentTime = buIfEntry->presentSentTimeBU;
    //simtime_t nextSentTime = buIfEntry->nextScheduledTime;

    //ASSERT(bul);
    bul->addOrUpdateBUL(dest, HoA, CoA, buLife, buSeq, sendTime);    //, nextSentTime); //updates the binding Update List
    //EV << "#### Updated BUL with lifetime=" << buLife << "and sentTime=" << sentTime << endl;
}

xMIPv6::BuTransmitIfEntry *xMIPv6::fetchBUTransmitIfEntry(InterfaceEntry *ie, const Ipv6Address& dest)
{
    // TODO use STL search algorithm
    for (auto & elem : transmitIfList) {
        if (BuTransmitIfEntry *buIfEntry = dynamic_cast<BuTransmitIfEntry *>(elem.second)) {
            if (buIfEntry->ifEntry->getInterfaceId() == ie->getInterfaceId() && buIfEntry->dest == dest)
                return buIfEntry;
        }
    }
    return nullptr;
}

void xMIPv6::sendMobilityMessageToIPv6Module(Packet *msg, const Ipv6Address& destAddr,
        const Ipv6Address& srcAddr, int interfaceId, simtime_t sendTime)    // overloaded for use at CN - CB
{
    EV_INFO << "Appending ControlInfo to mobility message\n";
    delete msg->removeTagIfPresent<DispatchProtocolReq>();
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mobileipv6);
    if (interfaceId != -1)
        msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    msg->addTagIfAbsent<L3AddressReq>()->setSrcAddress(srcAddr);
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddr);
    msg->addTagIfAbsent<HopLimitReq>()->setHopLimit(255);
    EV_INFO << "ControlInfo appended successfully. Sending mobility message to Ipv6 module\n";

    EV_DETAIL << "controlInfo: DestAddr=" << destAddr
              << " SrcAddr=" << srcAddr
              << " InterfaceId=" << interfaceId << endl;

    // TODO solve the HA DAD problem in a different way
    // (delay currently specified via the sendTime parameter)
    if (sendTime > 0)
        sendDelayed(msg, sendTime, "toIPv6");
    else
        send(msg, "toIPv6");
}

/*
   void xMIPv6::sendMobilityMessageToIPv6Module(cMessage *msg, const Ipv6Address& destAddr)
   {
    EV << "\n<<======THIS IS THE (SMALL) ROUTINE FOR APPENDING CONTROL INFO TO MOBILITY MESSAGES =====>>\n";

    Ipv6ControlInfo *controlInfo = new Ipv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6EXT_MOB); //specifies the next header value = 135 for the Mobility Header
    controlInfo->setDestAddr(destAddr);
    controlInfo->setHopLimit(255);
    controlInfo->setInterfaceId(-1);
    msg->setControlInfo(controlInfo);

    EV << "\n<<======CONTROL INFO APPENDED SUCCESSFULLY; SENDING MOBILITY MESSAGE TO Ipv6 MODULE =====>>\n";

    EV << "controlInfo: DestAddr=" << controlInfo->destAddr()
       << "SrcAddr=" << controlInfo->srcAddr()
       << "InterfaceId=" << controlInfo->interfaceId() << endl;

    // TODO solve the HA DAD problem in a different way
    if (BindingAcknowledgement *ba = dynamic_cast<BindingAcknowledgement*>(msg) && rt6->isHomeAgent())
    {
        if (ba->getStatus() < 128 && (ba->getLifetime() != 0))
        {
            EV << "Message is positive BA with status: " << ba->getStatus() << ", sending it with a PSEUDO DAD Delay of 60 sec" << endl;
            sendDelayed(msg, 1, "toIPv6");
            return;
        }
    }

    send(msg,"toIPv6");
   }
 */

void xMIPv6::processBUMessage(Packet *inPacket, const Ptr<const BindingUpdate>& bu)
{
    EV_INFO << "Entered BU processing method" << endl;

    /*if ((!rt6->isMobileNode() && !rt6->isRouter() && !rt6->isHomeAgent()) ||
         (rt6->isRouter() && rt6->isHomeAgent() && !rt6->isMobileNode())
       ) // if node is either HA or CN, only then a BA is sent otherwise not.
     */
    // BA is not sent when node is a mobile node and not a HA at the same time
    if (rt6->isMobileNode() && !rt6->isHomeAgent()) {    // rewrote condition to make it more clear - CB
        EV_WARN << "Wrong Node: not HA or CN" << endl;

        if (hasGUI())
            bubble("Wrong Node: not HA or CN");

        delete inPacket;
        return;
    }

    BaStatus status;
    bool validBUMessage = validateBUMessage(inPacket, bu);

    if (validBUMessage) {
        auto ifTag = inPacket->getTag<InterfaceInd>();
        auto addrTag = inPacket->getTag<L3AddressInd>();
        const Ipv6Address& HoA = bu->getHomeAddressMN();
        Ipv6Address CoA = addrTag->getSrcAddress().toIpv6();
        Ipv6Address destAddress = addrTag->getDestAddress().toIpv6();
        uint buLifetime = bu->getLifetime() * 4;    /* 6.1.7 One time unit is 4 seconds. */
        uint buSequence = bu->getSequence();
        bool homeRegistration = bu->getHomeRegistrationFlag();

        // handling for binding de-registration

        /*9.5.1
           If the Lifetime specified in the Binding Update is zero or the
           specified care-of address matches the home address for the
           binding, then this is a request to delete the cached binding for
           the home address. [...] If the Home Registration (H) bit is set
           in the Binding Update, the Binding Update is processed according
           to the procedure specified in Section 10.3.2; otherwise, it is
           processed according to the procedure specified in Section 9.5.3.*/
        if ((buLifetime == 0) || (CoA == HoA)) {
            // Check home registration flag -> not much difference in Section 10.3.2 and 9.5.3 for our purpose

            // de-register binding
            if (rt6->isHomeAgent() && !validateBUderegisterMessage(inPacket, bu)) {    // HAs have to validate the BU
                /* If the receiving node has no entry marked as a home registration
                   in its Binding Cache for this mobile node, then this node MUST
                   reject the Binding Update and SHOULD return a Binding
                   Acknowledgement to the mobile node, in which the Status field is
                   set to 133 (not home agent for this mobile node).*/
                status = NOT_HA_FOR_THIS_MN;    //enum defined in MobilityHeader.msg file
                uint baSeqNumber = bu->getSequence();    //the sequence number from Rxed BU is copied into BA.
                createAndSendBAMessage(destAddress, CoA, ifTag->getInterfaceId(), status,
                        bu->getBindingAuthorizationData(), baSeqNumber, buLifetime);    // swapped src and dest, update lifeTime
                EV_ERROR << "Not HA for this MN. Responding with appropirate BA...\n";
                delete inPacket;
                return;
            }
            /*9.5.3
               Any existing binding for the given home address MUST be deleted.  A
                 Binding Cache entry for the home address MUST NOT be created in
                 response to receiving the Binding Update.*/
            /*10.3.2
               If the home agent does not reject the Binding Update as described
                 above, then it MUST delete any existing entry in its Binding Cache
                 for this mobile node.*/
            bc->deleteEntry(HoA);

            /*In addition, the home agent MUST stop intercepting packets on the
               mobile node's home link that are addressed to the mobile node*/
            // of course this is also true for CNs
            tunneling->destroyTunnelFromTrigger(HoA);

            // kill BC expiry timer
            cancelTimerIfEntry(HoA, ifTag->getInterfaceId(), KEY_BC_EXP);

            /*10.3.2
               Then, the home agent MUST return a Binding Acknowledgement to the mobile node */
            /*9.5.4
               o  If the Acknowledge (A) bit set is set in the Binding Update, a
                Binding Acknowledgement MUST be sent.*/
            if (rt6->isHomeAgent() || bu->getAckFlag()) {
                /*constructed as follows:
                   o  The Status field MUST be set to a value 0, indicating success.*/
                status = BINDING_UPDATE_ACCEPTED;    //enum defined in MobilityHeader.msg file
                /*o  The Sequence Number field MUST be copied from the Sequence Number
                    given in the Binding Update.*/
                uint baSeqNumber = bu->getSequence();
                /*o  The Lifetime field MUST be set to zero.*/
                uint lifeTime = 0;

                /*//The following was omitted://
                   o  The Key Management Mobility Capability (K) bit is set or cleared
                    and actions based on its value are performed as described in the
                    previous section.  The mobile node's home address is used as its
                    new care-of address for the purposes of moving the key management
                    connection to a new endpoint.
                   o  The Binding Refresh Advice mobility option MUST be omitted.*/
                createAndSendBAMessage(destAddress, CoA, ifTag->getInterfaceId(), status, baSeqNumber,
                        bu->getBindingAuthorizationData(), lifeTime);    // swapped src and dest
            }

            if (!rt6->isHomeAgent()) {    // this is a CN
                // cancel existing Binding Refresh Request timer
                // (if there exists one)
                int interfaceID = ifTag->getInterfaceId();
                cancelTimerIfEntry(HoA, interfaceID, KEY_BR);
            }

            EV_INFO << "Deregistered binding\n";
            bubble("Deregistered binding!");
        }
        else {
            // binding lifetime is nonzero
            /*9.5.1
               If the Lifetime specified in the Binding Update is nonzero and the
               specified care-of address is not equal to the home address for the
               binding, then this is a request to cache a binding for the home
               address. If the Home Registration (H) bit is set in the Binding
               Update, the Binding Update is processed according to the procedure
               specified in Section 10.3.1; otherwise, it is processed according
               to the procedure specified in Section 9.5.2.*/
            if (homeRegistration) {
                /* 10.3.1
                   o  If the node implements only correspondent node functionality, or
                      has not been configured to act as a home agent, then the node MUST
                      reject the Binding Update.  The node MUST also return a Binding
                      Acknowledgement to the mobile node, in which the Status field is
                      set to 131 (home registration not supported).*/
                if (!rt6->isHomeAgent()) {
                    status = HOME_REGISTRATION_NOT_SUPPORTED;    //enum defined in MobilityHeader.msg file
                    uint baSeqNumber = bu->getSequence();
                    uint lifeTime = 0;
                    createAndSendBAMessage(destAddress, CoA, ifTag->getInterfaceId(), status, baSeqNumber,
                            bu->getBindingAuthorizationData(), lifeTime);

                    delete inPacket;
                    return;
                }
                else if (!rt6->isOnLinkAddress(HoA)) {
                    /*Else, if the home address for the binding (the Home Address field
                       in the packet's Home Address option) is not an on-link Ipv6
                       address with respect to the home agent's current Prefix List, then
                       the home agent MUST reject the Binding Update and SHOULD return a
                       Binding Acknowledgement to the mobile node, in which the Status
                       field is set to 132 (not home subnet).*/
                    status = NOT_HOME_SUBNET;    //enum defined in MobilityHeader.msg file
                    uint baSeqNumber = bu->getSequence();
                    uint lifeTime = 0;
                    createAndSendBAMessage(destAddress, CoA, ifTag->getInterfaceId(), status, baSeqNumber,
                            bu->getBindingAuthorizationData(), lifeTime);

                    delete inPacket;
                    return;
                }
            }

            bool existingBinding = bc->isInBindingCache(HoA);
            bc->addOrUpdateBC(HoA, CoA, buLifetime, buSequence, homeRegistration);
            // for both HA and CN we create a BCE expiry timer
            createBCEntryExpiryTimer(HoA, ift->getInterfaceById(ifTag->getInterfaceId()), simTime() + buLifetime);

            /*10.3.1
               Regardless of the setting of the Acknowledge (A) bit in the Binding
               Update, the home agent MUST return a Binding Acknowledgement to the
               mobile node*/
            /*9.5.4
               If the Acknowledge (A) bit set is set in the Binding Update, a
                Binding Acknowledgement MUST be sent.  Otherwise, the treatment
                depends on the below rule.*/
            if (bu->getAckFlag() || rt6->isHomeAgent()) {
                /*10.3.1
                   The Status field MUST be set to a value indicating success.  The
                    value 1 (accepted but prefix discovery necessary) MUST be used if
                    the subnet prefix of the specified home address is deprecated, or
                    becomes deprecated during the lifetime of the binding, or becomes
                    invalid at the end of the lifetime.  The value 0 MUST be used
                    otherwise.  For the purposes of comparing the binding and prefix
                    lifetimes, the prefix lifetimes are first converted into units of
                    four seconds by ignoring the two least significant bits.*/
                status = BINDING_UPDATE_ACCEPTED;    //enum defined in MobilityHeader.msg file
                /*The Sequence Number field MUST be copied from the Sequence Number
                    given in the Binding Update.*/
                uint baSeqNumber = bu->getSequence();
                /*The Lifetime field MUST be set to the remaining lifetime for the
                   binding as set by the home agent in its home registration Binding
                   Cache entry for the mobile node, as described above.*/
                uint lifeTime = bc->getLifetime(HoA);

                /* 10.3.1
                   Unless this home agent already has a binding for the given home
                   address, the home agent MUST perform Duplicate Address Detection [13]
                   on the mobile node's home link before returning the Binding
                   Acknowledgement.*/
                simtime_t sendTime;
                if (rt6->isHomeAgent())
                    // HA has to do DAD in case this is a new binding for this HoA
                    sendTime = existingBinding ? 0 : 1;
                else
                    sendTime = 0;

                createAndSendBAMessage(destAddress, CoA, ifTag->getInterfaceId(), status, baSeqNumber,
                        //bu->getBindingAuthorizationData(), 15, sendTime); // swapped src and dest
                        bu->getBindingAuthorizationData(), lifeTime, sendTime);    // swapped src and dest, corrected lifetime value

                /*If this Duplicate Address Detection fails for the given
                   home address or an associated link local address, then the home agent
                   MUST reject the complete Binding Update and MUST return a Binding
                   Acknowledgement to the mobile node, in which the Status field is set
                   to 134 (Duplicate Address Detection failed).*/
                // TODO
            }
            else {    // condition: ! bu->getAckFlag()
                EV_INFO << "BU Validated as OK: ACK FLAG NOT SET" << endl;
                bubble("!!!BU VALID --- ACK FLAG = False !!!");
            }

            if (rt6->isHomeAgent()) {    // establish tunnel to MN
                Ipv6Address& HA = destAddress;

                // we first destroy the already existing tunnel if
                // there exists one
                tunneling->destroyTunnelForEntryAndTrigger(HA, HoA);

                tunneling->createTunnel(Ipv6Tunneling::NORMAL, HA, CoA, HoA);
                //bubble("Established tunnel to mobile node.");
            }
            else {
                      // we first destroy the already existing RH2 path if
                      // there exists one
                Ipv6Address& CNAddress = destAddress;
                tunneling->destroyTunnelForEntryAndTrigger(CNAddress, HoA);

                // establish RH2 pseudo-tunnel at correspondent node
                tunneling->createTunnel(Ipv6Tunneling::T2RH, CNAddress, CoA, HoA);

                // cancel existing Binding Refresh Request timer
                // (if there exists one)
                int interfaceID = ifTag->getInterfaceId();
                cancelTimerIfEntry(HoA, interfaceID, KEY_BR);

                // and then we initialize a (new) BRR timer that gets
                // fired as soon as the BU lifetime is closing to 0.
                // Then we send Binding Refresh Requests once again
                // until we receive a valid BU.
                // FOX uncommented BRR below
                //InterfaceEntry *ie = ift->interfaceAt(interfaceID);
                //createBRRTimer(HoA, ie, buLifetime - BRR_TIMEOUT_THRESHOLD);
                //createBRRTimer(HoA, ie, BRR_TIMEOUT_THRESHOLD);
            }
        }
    }
    else {
        EV_WARN << "BU Validation Failed: Dropping message" << endl;
        bubble("BU Validation Failed: Dropping Packet");
    }

    delete inPacket;
}

bool xMIPv6::validateBUMessage(Packet *packet, const Ptr<const BindingUpdate>& bu)
{
    auto ifTag = packet->getTag<InterfaceInd>();
    // Performs BU Validation according to RFC3775 Sec 9.5.1

    EV_INFO << "\n<<<<<<<<<ROUTINE WHERE BU GETS VALIDATED>>>>>>>>>>>>>>><<\n";

    Ipv6Address src = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address homeAddress = bu->getHomeAddressMN();    //confirm whether it is getHomeAddressMN() or simply homeAddress()
    uint seqNumber = bu->getSequence();    //The seq Number of the recieved BU
    uint bcSeqNumber = bc->readBCSequenceNumber(homeAddress);    //The seq Number of the last recieved BU in the Binding cache

    // restructured the following and removed "delete bu"
    if (!(src.isGlobal() && src.isUnicast())) {
        EV_WARN << "BU Validation Failed: SrcAdress is not unicast Global !" << endl;
        EV_WARN << "Dropping unvalidated BU message" << endl;
        bubble("!! BU Validation Failed !!");
        return false;    //result = false;
    }
    if (!(homeAddress.isGlobal() && homeAddress.isUnicast())) {
        EV_WARN << "BU Validation Failed: Home Adress of MN is not unicast Global !" << endl;
        bubble("!! BU Validation Failed !!");
        EV_WARN << "Dropping unvalidated BU message" << endl;
        return false;    //result = false;
    }
    /*9.5.1
       This Sequence Number comparison MUST be performed modulo 2**16, i.e.,
       the number is a free running counter represented modulo 65536.  A
       Sequence Number in a received Binding Update is considered less than
       or equal to the last received number if its value lies in the range
       of the last received number and the preceding 32768 values, inclusive.*/
    else if (((bcSeqNumber % 65536) > seqNumber) || ((32768 + bcSeqNumber) % 65536 < seqNumber)) {
        EV_WARN << "BU Validation Failed: Received Seq#: " << seqNumber << " is LESS THAN in BC: "
                << bcSeqNumber << endl;
        bubble("!! BU Validation Failed !!");
        EV_WARN << "Dropping unvalidated BU message" << endl;

        /*9.5.1
           If the mobile node sends a sequence number which is not greater than
             the sequence number from the last valid Binding Update for this home
           address, then the receiving node MUST send back a Binding
           Acknowledgement with status code 135, and the last accepted sequence
           number in the Sequence Number field of the Binding Acknowledgement.*/
        Ipv6Address destAddress = packet->getTag<L3AddressInd>()->getDestAddress().toIpv6();
        createAndSendBAMessage(destAddress, homeAddress, ifTag->getInterfaceId(), REGISTRATION_TYPE_CHANGE_DISALLOWED,
                bu->getBindingAuthorizationData(), bcSeqNumber, 0);

        return false;
    }

    // this is the CN validation part
    if (!bu->getHomeRegistrationFlag()) {
        if (bc->getHomeRegistration(homeAddress) != bu->getHomeRegistrationFlag()) {
            /*9.5.1
               If a binding already exists for the given home address and the home
                 registration flag has a different value than the Home Registration
                 (H) bit in the Binding Update, then the receiving node MUST send back
                 a Binding Acknowledgement with status code 139 (registration type
                 change disallowed).  The home registration flag stored in the Binding
                 Cache entry MUST NOT be changed.*/
            EV_WARN << "BU Validation Failed: home registration flag in binding cache has different value than in the binding update" << endl;
            createAndSendBAMessage(src, homeAddress, ifTag->getInterfaceId(), REGISTRATION_TYPE_CHANGE_DISALLOWED,
                    bu->getBindingAuthorizationData(), bcSeqNumber, 0);

            return false;
        }

        int homeToken = bc->generateHomeToken(homeAddress, 0);    // TODO nonce
        int careOfToken = bc->generateCareOfToken(src, 0);    // TODO nonce
        int bindAuthData = bc->generateKey(homeToken, careOfToken, src);

        if (bu->getLifetime() == 0) {    // deregistration BU
            EV_DETAIL << "homeToken=" << homeToken << ", careOfToken=" << careOfToken
                      << " , bindAuthData=" << bindAuthData
                      << ", BU data=" << bu->getBindingAuthorizationData() << endl;

            if ((bu->getBindingAuthorizationData() != bindAuthData)
                && (bu->getBindingAuthorizationData() != homeToken))    // this is quick and dirty -> TODO
            {
                EV_WARN << "BU Validation Failed: Binding Authorization Data invalid!" << endl;
                return false;
            }
        }
        else {    // registration BU
            if (bu->getBindingAuthorizationData() != bindAuthData) {
                EV_WARN << "BU Validation Failed: Binding Authorization Data invalid!" << endl;
                return false;
            }
        }
    }

    // If all the above tests are passed the Received BU is valid
    EV_INFO << "BU validation passed" << endl;

    if (hasGUI())
        bubble("BU Validated");

    return true;    //result;
}

bool xMIPv6::validateBUderegisterMessage(Packet *inPacket, const Ptr<const BindingUpdate>& bu)
{
    /*To begin processing the Binding Update, the home agent MUST perform
       the following test:

         o  If the receiving node has no entry marked as a home registration
       in its Binding Cache for this mobile node, then this node MUST
       reject the Binding Update and SHOULD return a Binding
       Acknowledgement to the mobile node, in which the Status field is
       set to 133 (not home agent for this mobile node).*/
    return bc->isInBindingCache(bu->getHomeAddressMN())
           && bc->getHomeRegistration(bu->getHomeAddressMN());
}

void xMIPv6::createAndSendBAMessage(const Ipv6Address& src, const Ipv6Address& dest,
        int interfaceId, const BaStatus& baStatus, const uint baSeq,
        const int bindingAuthorizationData, const uint lifeTime, const simtime_t sendTime)
{
    EV_TRACE << "Entered createAndSendBAMessage() method" << endl;

    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);    // To find the interface on which the BU was received

    // swapping src and destination for the ack packet
    //Ipv6Address source = ie->ipv6()->linkLocalAddress();
    //Ipv6Address destination = src;
    //Ipv6Address src = ie ->ipv6()->linkLocalAddress();

    // uncommented the code above - we can use the swapped original src and
    // dest from the ctrlInfo as it is provided in the first two arguments

    auto packet = new Packet("Binding Acknowledgement");
    const auto& ba = makeShared<BindingAcknowledgement>();
    ba->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT);
    ba->setStatus(baStatus);
    ba->setSequenceNumber(baSeq);    //this sequence number will correspond to the ACKed BU

    // we are providing lifetime as a parameter
    ba->setLifetime(lifeTime / 4);    /* 6.1.8 ...in time units of 4 seconds... */

    /*9.5.4
       If the Status field in the Binding Acknowledgement contains the value
       136 (expired home nonce index), 137 (expired care-of nonce index), or
       138 (expired nonces) then the message MUST NOT include the Binding
       Authorization Data mobility option.  Otherwise, the Binding
       Authorization Data mobility option MUST be included, and MUST meet
       the specific authentication requirements for Binding Acknowledgements
       as defined in Section 5.2.*/
    if (ba->getStatus() != EXPIRED_HOME_NONCE_INDEX &&
        ba->getStatus() != EXPIRED_CARE_OF_NONCE_INDEX &&
        ba->getStatus() != EXPIRED_NONCES)
    {
        ba->setBindingAuthorizationData(bindingAuthorizationData);
    }

    // setting message size
    int bindAuthSize = 0;

    if (bindingAuthorizationData != UNDEFINED_BIND_AUTH_DATA)
        bindAuthSize = SIZE_BIND_AUTH_DATA + 2; // Binding Auth. Data + 6.2.3 PadN = 16 bit

    ba->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_BACK + bindAuthSize));
    packet->insertAtFront(ba);

    /*The rules for selecting the Destination IP address (and, if required,
       routing header construction) for the Binding Acknowledgement to the
       mobile node are the same as in the previous section.  When the Status
       field in the Binding Acknowledgement is greater than or equal to 128
       and the Source Address of the Binding Update is on the home link, the
       home agent MUST send it to the mobile node's link layer address
       (retrieved either from the Binding Update or through Neighbor
       Solicitation).*/
    // TODO

    sendMobilityMessageToIPv6Module(packet, dest, src, ie->getInterfaceId(), sendTime);

    // statistic collection
    /*if (rt6->isHomeAgent())
        statVectorBAtoMN.record(1);
       else
        statVectorBAtoMN.record(2);*/
}

void xMIPv6::processBAMessage(Packet *inPacket, const Ptr<const BindingAcknowledgement>& ba)
{
    EV_TRACE << "\n<<<<<<<<<This is where BA gets processed>>>>>>>>>\n";
    //bool retransmitBU = false;
    Ipv6Address baSource = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    auto ifTag = inPacket->getTag<InterfaceInd>();
    InterfaceEntry *ie = ift->getInterfaceById(ifTag->getInterfaceId());    //the interface on which the BAck was received

    if (rt6->isMobileNode()) {
        if (!validateBAck(inPacket, *ba)) {
            EV_INFO << "Discarding invalid BAck...\n";
            delete inPacket;

            // statistic collection
            /*if (baSource == ie->ipv6()->getHomeAgentAddress())
                statVectorBAfromHA.record(3);
               else
                statVectorBAfromCN.record(3);*/

            return;
        }

        /*11.7.3
           When a mobile node receives a packet carrying a valid Binding
           Acknowledgement, the mobile node MUST examine the Status field as
           follows:
           o  If the Status field indicates that the Binding Update was accepted
             (the Status field is less than 128), then the mobile node MUST
             update the corresponding entry in its Binding Update List to
             indicate that the Binding Update has been acknowledged; the mobile
             node MUST then stop retransmitting the Binding Update.*/
        if (ba->getStatus() < 128) {
            EV_INFO << "Binding was accepted." << endl;

            // As the BU is obviously valid, we can remove the transmission timer for BU
            cancelTimerIfEntry(baSource, ie->getInterfaceId(), KEY_BU);

            if (ba->getLifetime() == 0) {    // BAck to deregistration BU
                if (baSource == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress()) {
                    /*11.5.4
                       After receiving the Binding Acknowledgement for its Binding Update to
                         its home agent, the mobile node MUST multicast onto the home link (to
                         the all-nodes multicast address) a Neighbor Advertisement [12], to
                         advertise the mobile node's own link-layer address for its own home
                         address.*/
                    ipv6nd->sendUnsolicitedNa(ie);

                    // statistic collection
                    /*statVectorBAfromHA.record(2);*/
                }
                /*else
                    statVectorBAfromCN.record(2);*/

                // delete the entry from the BUL
                bul->removeBinding(baSource);
                // remove all timers related to this BA address
                removeTimerEntries(baSource, ifTag->getInterfaceId());
            }
            else {
                // binding with lifeTime >0 was accepted

                // moved the code below from sendBU()
                // retrieve the appropriate BUL entry
                BindingUpdateList::BindingUpdateListEntry *entry = bul->lookup(baSource);
                ASSERT(entry != nullptr);

                // establish tunnel, but only if we have not already acked the BU before
                if (entry->BAck == false && entry->destAddress == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress()) {    // BA from HA
                    removeCoAEntries();    // TODO would be better if this is done somewhere else or in a comletely different way
                    interfaceCoAList[ie->getInterfaceId()] = entry->careOfAddress;

                    tunneling->createTunnel(Ipv6Tunneling::NORMAL, entry->careOfAddress, entry->destAddress);
                    //bubble("Established tunnel to home agent.");

                    /**11.5.1
                         After updating its home registration, the mobile
                           node then updates associated mobility bindings in correspondent nodes
                           that it is performing route optimization with as specified in Section
                           11.7.2.*/
                    // initiate RR for the CNs
                    for (itCNList = cnList.begin(); itCNList != cnList.end(); itCNList++) {    // run an iterator through the CN map
                        Ipv6Address& cnDest = *(itCNList);

                        //entry->state = BindingUpdateList::RR;
                        //if (!bul->isValidBinding(cnDest)) // to initiate HoTI/CoTI resending
                        triggerRouteOptimization(cnDest, ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress(), ie);
                    }

                    // statistic collection
                    /*statVectorBAfromHA.record(1);*/
                }
                else if (entry->BAck == false) {    // BA from CN
                    tunneling->destroyTunnelForExitAndTrigger(entry->homeAddress, baSource);
                    tunneling->createTunnel(Ipv6Tunneling::HA_OPT, entry->careOfAddress, entry->homeAddress, baSource);
                    //tunneling->createPseudoTunnel(CoA, bu->getHomeAddressMN(), dest, TUNNEL_HA_OPT);
                    //bubble("Established Type 2 Routing Header path to CN.");

                    // statistic collection
                    /*statVectorBAfromCN.record(1);*/

                    // fire event to MIH subscribers
                    emit(mipv6RoCompletedSignal, (cObject *)nullptr);
                }

                // set BAck flag in BUL
                entry->BAck = true;

                // set mobility state in BUL
                entry->state = BindingUpdateList::REGISTERED;

                /*11.7.3
                   In addition, if the value specified in the Lifetime field in the
                    Binding Acknowledgement is less than the Lifetime value sent in
                    the Binding Update being acknowledged, the mobile node MUST
                    subtract the difference between these two Lifetime values from the
                    remaining lifetime for the binding as maintained in the
                    corresponding Binding Update List entry (with a minimum value for
                    the Binding Update List entry lifetime of 0).  That is, if the
                    Lifetime value sent in the Binding Update was L_update, the
                    Lifetime value received in the Binding Acknowledgement was L_ack,
                    and the current remaining lifetime of the Binding Update List
                    entry is L_remain, then the new value for the remaining lifetime
                    of the Binding Update List entry should be
                      max((L_remain - (L_update - L_ack)), 0)
                    where max(X, Y) is the maximum of X and Y.*/
                int l_ack = ba->getLifetime() * 4;    /* 6.1.7 One time unit is 4 seconds. */
                int l_update = entry->bindingLifetime;
                int l_remain = entry->bindingLifetime - (SIMTIME_DBL(simTime() - entry->sentTime));
                int x = l_remain - (l_update - l_ack);
                entry->bindingLifetime = x > 0 ? x : 0;
                entry->bindingExpiry = simTime() + entry->bindingLifetime;
                // we schedule the timer that manages the BUL entry expiration
                // TODO currently we schedule the expiry message some seconds (PRE_BINDING_EXPIRY)
                //         before the actual expiration. Can be improved.
                simtime_t scheduledTime = entry->bindingExpiry - PRE_BINDING_EXPIRY;
                scheduledTime = scheduledTime > 0 ? scheduledTime : 0;

                /*EV << "l_ack=" << l_ack << ", l_update=" << l_update << ", l_remain=" << l_remain << ", x=" << x << endl;
                   EV << "entry->bindingLifetime=" << entry->bindingLifetime << " and entry->bindingExpiry=" << entry->bindingExpiry << endl;*/
                EV_DETAIL << "Scheduling BULEntryExpiryTimer for " << scheduledTime << endl;
                createBULEntryExpiryTimer(entry, ie, scheduledTime);

                // 11.06.08 - CB: rescheduling changed. Handled by BUL entry expiry.
                /*// reschedule the message if the BA is from a CN and not from the HA
                   retransmitBU = (entry->destAddress != ie->ipv6()->getHomeAgentAddress());*/

                /*if (baSource == ie->ipv6()->getHomeAgentAddress())
                   {
                    // initiate RR for CNs moved to above
                   }*/
            }
        }
        /*o  If the Status field indicates that the Binding Update was rejected
             (the Status field is greater than or equal to 128), then the
             mobile node can take steps to correct the cause of the error and
             retransmit the Binding Update (with a new Sequence Number value),
             subject to the rate limiting restriction specified in Section
             11.8.  If this is not done or it fails, then the mobile node
             SHOULD record in its Binding Update List that future Binding
             Updates SHOULD NOT be sent to this destination.*/
        else {
            EV_INFO << "Binding was rejected.\n";

            /*11.7.1
               If the home agent rejects the value, it sends back a
               Binding Acknowledgement with a status code 135, and the last accepted
               sequence number in the Sequence Number field of the Binding
               Acknowledgement.  The mobile node MUST store this information and use
               the next Sequence Number value for the next Binding Update it sends.*/

            // retransmission is performed anyway as timers are not deleted
            // TODO store DO_NOT_SEND_BU in BUL
        }
    }

    // update 11.6.08 - CB: rescheduling of BU is now handled by BUL entry expiry
    // for now, we delete the BU transmission timer irrelevant of the status of the BA
    // TODO catch BA failure case
    /*
       int interfaceID = ctrlInfo->interfaceId();
       if (retransmitBU)
        resetBUIfEntry(baSource, interfaceID, retransmissionTime); // add bindingLifetime
       else
        cancelTransmitIfEntry(baSource, interfaceID, KEY_BU);
     */

    delete inPacket;
}

bool xMIPv6::validateBAck(Packet *packet, const BindingAcknowledgement& ba)
{
    /*11.7.3
       Upon receiving a packet carrying a Binding Acknowledgement, a mobile
       node MUST validate the packet according to the following tests:

       o  The packet meets the authentication requirements for Binding
         Acknowledgements defined in Section 6.1.8 and Section 5.  That is,
         if the Binding Update was sent to the home agent, underlying IPsec
         protection is used.  If the Binding Update was sent to the
         correspondent node, the Binding Authorization Data mobility option
         MUST be present and have a valid value.*/
    Ipv6Address cnAddress = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    auto ifTag = packet->getTag<InterfaceInd>();
    InterfaceEntry *ie = ift->getInterfaceById(ifTag->getInterfaceId());

    if (cnAddress != ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress()) {    // BAck comes from a CN and not from the HA
        if (ba.getBindingAuthorizationData() == UNDEFINED_BIND_AUTH_DATA) {
            EV_WARN << "BA Validation Failed: Binding Authorization Data invalid !" << endl;
            return false;
        }
    }

    /*o  The Sequence Number field matches the Sequence Number sent by the
         mobile node to this destination address in an outstanding Binding
         Update.*/
    if (bul->getSequenceNumber(cnAddress) != ba.getSequenceNumber()) {
        EV_WARN << "BA Validation Failed: Sequence number from BA does not match the one from the BUL!!\n";
        return false;
    }

    return true;
}

/**
 * Alain Tigyo, 21.03.2008
 * The following code is used for triggering RO to a CN.
 */
void xMIPv6::triggerRouteOptimization(const Ipv6Address& destAddress, const Ipv6Address& HoA, InterfaceEntry *ie)
{
    if (bul->getMobilityState(destAddress) == BindingUpdateList::NONE)
        bul->setMobilityState(destAddress, BindingUpdateList::RR);

    int vIndex = tunneling->getVIfIndexForDest(destAddress, Ipv6Tunneling::MOBILITY);

    if (vIndex > ift->getNumInterfaces()) {
        EV_INFO << "Route Optimization for: " << destAddress << " already triggered";

        // we have to check whether our current CoA is different from the one saved in the BUL
        // (this would mean we have moved to a new access network on this interface)
        if ((bul->getCoA(destAddress) != ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA))
            || bul->getMobilityState(destAddress) == BindingUpdateList::NONE)
        {
            // we have a new CoA compared to BUL entry -> redo RO
            initReturnRoutability(destAddress, ie);
            EV_INFO << " ...moved to new network. Initializing RO.";
        }

        EV_INFO << endl;
    }
    else {
        if ((!bul->isInBindingUpdateList(destAddress, HoA))
            || (bul->getMobilityState(destAddress) == BindingUpdateList::RR))
        {
            EV_INFO << "Initialise Route Optimization for: " << destAddress << "\n";
            initReturnRoutability(destAddress, ie);

            auto CRiterator = find(cnList.begin(), cnList.end(), destAddress);
            if (CRiterator == cnList.end())
                cnList.push_back(destAddress);
        }
    }
}

void xMIPv6::initReturnRoutability(const Ipv6Address& cnDest, InterfaceEntry *ie)
{
    EV_TRACE << "Initiating Return Routability...\n";
    Enter_Method("initReturnRoutability()");

    bool sendHoTI = true, sendCoTI = true;

    /*11.6.1
       A mobile node that initiates a return routability procedure MUST send
       (in parallel) a Home Test Init message and a Care-of Test Init
       messages. However, if the mobile node has recently received (see
       Section 5.2.7) one or both home or care-of keygen tokens, and
       associated nonce indices for the desired addresses, it MAY reuse
       them.  Therefore, the return routability procedure may in some cases
       be completed with only one message pair.  It may even be completed
       without any messages at all, if the mobile node has a recent home
       keygen token and has previously visited the same care-of address so
       that it also has a recent care-of keygen token.*/
    BindingUpdateList::BindingUpdateListEntry *cnEntry = bul->fetch(cnDest);
    ASSERT(cnEntry);
    cnEntry->state = BindingUpdateList::RR;

    /*A Home Test Init message MUST be created as described in Section
       6.1.3.*/
    //if (cnEntry != nullptr)
    //{
    // if there exists an entry in the BUL, check whether we already
    // have valid home and care-of tokens

    // check whether the last received home token is still valid
    //if ((cnEntry->tokenH != UNDEFINED_TOKEN) && (cnEntry->sentHoTI + ie->ipv6()->_maxTokenLifeTime() > simTime()))
    if (bul->isHomeTokenAvailable(cnDest, ie)) {
        EV_INFO << "Valid home token available - sending HoTI later.\n";
        sendHoTI = false;
    }
    //else
    //    delete HoTI;

    /*if ((cnEntry->tokenC != UNDEFINED_TOKEN) && (cnEntry->sentCoTI + ie->ipv6()->_maxTokenLifeTime() > simTime()))*/
    if (bul->isCareOfTokenAvailable(cnDest, ie)) {
        EV_INFO << "Valid care-of token available - sending CoTI later.\n" << endl;

        // we already have a care-of token
        sendCoTI = false;
    }
    //else
    //    delete CoTI;

    if (!sendHoTI && !sendCoTI) {    // cnEntry can not be nullptr as a consequence of the other two flag's values
        // we already had a valid home and care-of token
        // -> no need for sending HoTI/CoTI; we can
        // immediately continue with sending a BU
        cnEntry->state = BindingUpdateList::RR_COMPLETE;

        sendBUtoCN(*cnEntry, ie);
    }
    //}

    if (sendHoTI && !bul->recentlySentHOTI(cnDest, ie)) {
        // no entry for this CN available: create Home Test Init message to be sent via HA
        createAndSendHoTIMessage(cnDest, ie);
    }

    if (sendCoTI && !bul->recentlySentCOTI(cnDest, ie)) {
        /*A Care-of Test Init message MUST be created as described in Section
           6.1.4.*/
        // Care-of Test Init Message to CN
        createAndSendCoTIMessage(cnDest, ie);
    }
}

void xMIPv6::createTestInitTimer(const Ptr<MobilityHeader> testInit, const Ipv6Address& dest, InterfaceEntry *ie, simtime_t sendTime)
{
    EV_DETAIL << "\n++++++++++TEST INIT TIMER CREATED AT SIM TIME: " << simTime()
              << " seconds+++++++++++++++++ \n";

    cMessage *testInitTriggerMsg = new cMessage("sendTestInit", MK_SEND_TEST_INIT);

    /*11.8
       When the mobile node sends a Mobile Prefix Solicitation, Home Test
       Init, Care-of Test Init or Binding Update for which it expects a
       response, the mobile node has to determine a value for the initial
       retransmission timer:*/
    // check if there already exists a testInitTimer entry for this key
    int msgType;

    if (dynamicPtrCast<HomeTestInit>(testInit))
        msgType = KEY_HI;
    else
        msgType = KEY_CI;

    // TODO refactor the code below, as it is also used in createBUTimer
    Key key(dest, ie->getInterfaceId(), msgType);
    // fetch a valid TimerIfEntry object
    TestInitTransmitIfEntry *tiIfEntry = check_and_cast<TestInitTransmitIfEntry *>(getTimerIfEntry(key, TRANSMIT_TYPE_TI));
    //delete tiIfEntry->testInitMsg;
    cancelAndDelete(tiIfEntry->timer);

    tiIfEntry->timer = testInitTriggerMsg;

    tiIfEntry->dest = dest;
    tiIfEntry->ifEntry = ie;
    tiIfEntry->testInitMsg = testInit;
    /*o  Otherwise, the mobile node should use the specified value of
           INITIAL_BINDACK_TIMEOUT for the initial retransmission timer.*/
    tiIfEntry->ackTimeout = ie->getProtocolData<Ipv6InterfaceData>()->_getInitialBindAckTimeout();
    tiIfEntry->nextScheduledTime = simTime();    // we send the HoTI/CoTI now

    testInitTriggerMsg->setContextPointer(tiIfEntry);    // attach the Test Init If Entry to this message

    // scheduling a message which will trigger the Test Init for sendTime seconds
    // if not called with a parameter for sendTime, the message will be scheduled for NOW
    scheduleAt(simTime() + sendTime, testInitTriggerMsg);
}

void xMIPv6::sendTestInit(cMessage *msg)
{
    // FIXME the following line is unsafe, rewrite it
    TestInitTransmitIfEntry *tiIfEntry = (TestInitTransmitIfEntry *)msg->getContextPointer();    //check_and_cast<TestInitTransmitIfEntry*>((TestInitTransmitIfEntry*) msg->contextPointer());
    InterfaceEntry *ie = tiIfEntry->ifEntry;

    /*11.6.1
       When sending a Home Test Init or Care-of Test Init message
       the mobile node MUST record in its Binding Update List the following
       fields from the messages:
       o  The IP address of the node to which the message was sent.
       o  The home address of the mobile node.  This value will appear in
          the Source Address field of the Home Test Init message.  When
          sending the Care-of Test Init message, this address does not
          appear in the message, but represents the home address for which
          the binding is desired.
       o  The time at which each of these messages was sent.
       o  The cookies used in the messages.
       Note that a single Care-of Test Init message may be sufficient even
       when there are multiple home addresses.  In this case the mobile node
       MAY record the same information in multiple Binding Update List
       entries.*/

    // retrieve the cookie from the Test Init message
    if (auto homeTestInit = dynamicPtrCast<HomeTestInit>(tiIfEntry->testInitMsg)) {
        // moved the following two lines to here
        Ipv6Address HoA = ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::HoA);
        ASSERT(!HoA.isUnspecified());

        auto outPacket = new Packet("HoTI");

        // update cache
        bul->addOrUpdateBUL(tiIfEntry->dest, HoA, simTime(), homeTestInit->getHomeInitCookie(), true);
        // mark the current home token as invalid
        bul->resetHomeToken(tiIfEntry->dest, HoA);
        // and send message
        outPacket->insertAtFront(homeTestInit);
        sendMobilityMessageToIPv6Module(outPacket, tiIfEntry->dest, HoA);

        // statistic collection
        /*statVectorHoTItoCN.record(1);*/
    }
    else {
        // must be of type CareOfTestInit
        auto careOfTestInit = CHK(dynamicPtrCast<CareOfTestInit>(tiIfEntry->testInitMsg));
              // moved the following two lines to here
        Ipv6Address CoA = ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA);
        ASSERT(!CoA.isUnspecified());

        auto outPacket = new Packet("CoTI");

        // update cache
        bul->addOrUpdateBUL(tiIfEntry->dest, CoA, simTime(), careOfTestInit->getCareOfInitCookie(), false);
        // mark the current care-of token as invalid
        bul->resetCareOfToken(tiIfEntry->dest, CoA);
        // and send message
        outPacket->insertAtFront(careOfTestInit);
        sendMobilityMessageToIPv6Module(outPacket, tiIfEntry->dest, CoA, ie->getInterfaceId());

        // statistic collection
        /*statVectorCoTItoCN.record(1);*/
    }

    /*11.8
       If the mobile node fails to receive a valid matching response within
         the selected initial retransmission interval, the mobile node SHOULD
        retransmit the message until a response is received.*/
    tiIfEntry->nextScheduledTime = tiIfEntry->ackTimeout + simTime();

    /*The retransmissions by the mobile node MUST use an exponential back-
         off process in which the timeout period is doubled upon each
         retransmission, until either the node receives a response or the
         timeout period reaches the value MAX_BINDACK_TIMEOUT. The mobile
         node MAY continue to send these messages at this slower rate
         indefinitely.*/
    tiIfEntry->ackTimeout = tiIfEntry->ackTimeout * 2;
    if (!(tiIfEntry->ackTimeout < ie->getProtocolData<Ipv6InterfaceData>()->_getMaxBindAckTimeout()))
        tiIfEntry->ackTimeout = ie->getProtocolData<Ipv6InterfaceData>()->_getMaxBindAckTimeout();

    msg->setContextPointer(tiIfEntry);
    scheduleAt(tiIfEntry->nextScheduledTime, msg);

    EV_INFO << "Scheduled next HoTI/CoTI for time=" << tiIfEntry->nextScheduledTime
            << " with timeout=" << tiIfEntry->ackTimeout << " for dest="
            << tiIfEntry->dest << endl;

    //delete tiIfEntry;
    //delete msg;
}

/*void xMIPv6::resetTestInitIfEntry(const Ipv6Address& dest, int interfaceID, int msgType)
   {
    ASSERT(msgType == KEY_HI || msgType == KEY_CI);

    Key key(dest, interfaceID, msgType);
    auto pos = transmitIfList.find(key);

    if (pos == transmitIfList.end())
    {
        //EV << "+++ No corresponding TimerIfEntry found! +++\n";
        return;
    }

    TimerIfEntry* entry = (pos->second);
    ASSERT(entry);

    // first we cancel the current retransmission timer
    cancelEvent(entry->timer);
    // then we reset the timeout value to the initial value
    entry->ackTimeout = entry->ifEntry->ipv6()->_initialBindAckTimeout();
    // and then we reschedule again for the time when the token expires
    entry->nextScheduledTime = simTime() + MAX_TOKEN_LIFETIME * TEST_INIT_RETRANS_FACTOR;
    scheduleAt(entry->nextScheduledTime, entry->timer);

    EV << "Updated TestTransmitIfEntry and corresponding timer.\n";

    // TODO check for token expiry in BUL
   }*/

/*void xMIPv6::resetTestInitIfEntry(const Ipv6Address& dest, int msgType)
   {
    ASSERT(msgType == KEY_HI || msgType == KEY_CI);

    auto pos = transmitIfList.begin();
    for ( ; pos != transmitIfList.end(); ++pos)
    {
        if (pos->first.type == msgType && pos->first.dest == dest)
            break;
    }

    if (pos == transmitIfList.end())
        return;

    // TODO refactor: call resetTestInitIfEntry(const Ipv6Address& dest, int interfaceID, int msgType)
    TimerIfEntry* entry = (pos->second);
    ASSERT(entry);

    // first we cancel the current retransmission timer
    cancelEvent(entry->timer);
    // then we reset the timeout value to the initial value
    entry->ackTimeout = entry->ifEntry->ipv6()->_initialBindAckTimeout();
    // and then we reschedule again for the time when the token expires
    entry->nextScheduledTime = simTime() + MAX_TOKEN_LIFETIME * TEST_INIT_RETRANS_FACTOR;
    scheduleAt(entry->nextScheduledTime, entry->timer);

    EV << "Updated TestTransmitIfEntry and corresponding timer.\n";

    // TODO check for token expiry in BUL
   }*/

void xMIPv6::resetBUIfEntry(const Ipv6Address& dest, int interfaceID, simtime_t retransmissionTime)
{
    /*ASSERT(msgType == KEY_BU);
       Key key(dest, interfaceID, msgType);*/

    Key key(dest, interfaceID, KEY_BU);

    auto pos = transmitIfList.find(key);

    if (pos == transmitIfList.end()) {
        //EV << "### No corresponding TimerIfEntry found! ###\n";
        return;
    }

    TimerIfEntry *entry = (pos->second);
    ASSERT(entry);

    // first we cancel the current retransmission timer
    cancelEvent(entry->timer);
    // then we reset the timeout value to the initial value
    entry->ackTimeout = entry->ifEntry->getProtocolData<Ipv6InterfaceData>()->_getInitialBindAckTimeout();
    // and then we reschedule again for BU expiry time
    // (with correct offset for scheduling)
    entry->nextScheduledTime = retransmissionTime;

    scheduleAt(entry->nextScheduledTime, entry->timer);

    EV_INFO << "Updated BuTransmitIfEntry and corresponding timer.\n";
}

void xMIPv6::createAndSendHoTIMessage(const Ipv6Address& cnDest, InterfaceEntry *ie)
{
    const auto& HoTI = makeShared<HomeTestInit>();
    HoTI->setMobilityHeaderType(HOME_TEST_INIT);
    HoTI->setHomeInitCookie(HO_COOKIE);
    // setting message size
    HoTI->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_HOTI));

    createTestInitTimer(HoTI, cnDest, ie);
}

void xMIPv6::createAndSendCoTIMessage(const Ipv6Address& cnDest, InterfaceEntry *ie)
{
    const auto& CoTI = makeShared<CareOfTestInit>();
    CoTI->setMobilityHeaderType(CARE_OF_TEST_INIT);
    CoTI->setCareOfInitCookie(CO_COOKIE);
    // setting message size
    CoTI->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_COTI));

    createTestInitTimer(CoTI, cnDest, ie);
}

void xMIPv6::processHoTIMessage(Packet *inPacket, const Ptr<const HomeTestInit>& homeTestInit)
{
    // 9.4.1 & 9.4.3
    Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address HoA = inPacket->getTag<L3AddressInd>()->getDestAddress().toIpv6();

    auto outPacket = new Packet("HoT");
    const auto& homeTest = makeShared<HomeTest>();
    homeTest->setMobilityHeaderType(HOME_TEST);
    homeTest->setHomeInitCookie(homeTestInit->getHomeInitCookie());
    homeTest->setHomeKeyGenToken(bc->generateHomeToken(HoA, 0));    // TODO nonce
    // setting message size
    homeTest->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_HOT));
    outPacket->insertAtFront(homeTest);

    EV_INFO << "\n<<======HoT MESSAGE FORMED; APPENDING CONTROL INFO=====>>\n";
    sendMobilityMessageToIPv6Module(outPacket, srcAddr, HoA);

    // statistic collection
    /*statVectorHoTtoMN.record(1);*/

    delete inPacket;
}

void xMIPv6::processCoTIMessage(Packet *inPacket, const Ptr<const CareOfTestInit>& coti)
{
    // 9.4.2 & 9.4.4
    Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address coa = inPacket->getTag<L3AddressInd>()->getDestAddress().toIpv6();

    auto outPacket = new Packet("CoT");
    const auto& cot = makeShared<CareOfTest>();
    cot->setMobilityHeaderType(CARE_OF_TEST);
    cot->setCareOfInitCookie(coti->getCareOfInitCookie());
    cot->setCareOfKeyGenToken(bc->generateCareOfToken(coa, 0));    // TODO nonce
    // setting message size
    cot->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_COT));
    outPacket->insertAtFront(cot);

    EV_INFO << "\n<<======CoT MESSAGE FORMED; APPENDING CONTROL INFO=====>>\n";
    sendMobilityMessageToIPv6Module(outPacket, srcAddr, coa);

    // statistic collection
    /*statVectorCoTtoMN.record(1);*/

    delete inPacket;
}

void xMIPv6::processHoTMessage(Packet *inPacket, const Ptr<const HomeTest>& homeTest)
{
    if (!validateHoTMessage(inPacket, *homeTest)) {
        EV_WARN << "HoT validation not passed: dropping message" << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, inPacket, &details);
    }
    else {
        EV_WARN << "HoT validation passed: updating BUL" << endl;
        Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
        //Ipv6Address& destAddr = ctrlInfo->destAddr();
        int interfaceID = inPacket->getTag<InterfaceInd>()->getInterfaceId();
        InterfaceEntry *ie = ift->getInterfaceById(interfaceID);

        BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->lookup(srcAddr);
        ASSERT(bulEntry != nullptr);

        // cancel the existing HoTI retransmission timer
        cancelTimerIfEntry(srcAddr, interfaceID, KEY_HI);

        // set the home keygen token
        bulEntry->tokenH = homeTest->getHomeKeyGenToken();

        // set token expiry appropriately
        createHomeTokenEntryExpiryTimer(srcAddr, ift->getInterfaceById(interfaceID),
                simTime() + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime());

        // if we have the care-of token as well, we can send the BU to the CN now
        // but only if we have not already sent or are about to send a BU
        // call method that potentially sends a BU now
        checkForBUtoCN(*bulEntry, ie);

        // gather statistics
        /*statVectorHoTfromCN.record(1);*/
    }

    delete inPacket;
}

bool xMIPv6::validateHoTMessage(Packet *inPacket, const HomeTest& homeTest)
{
    // RFC - 11.6.2
    Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address destAddr = inPacket->getTag<L3AddressInd>()->getDestAddress().toIpv6();
    //int interfaceID = ctrlInfo->interfaceId();

    /* The Source Address of the packet belongs to a correspondent node
       for which the mobile node has a Binding Update List entry with a
       state indicating that return routability procedure is in progress.
       Note that there may be multiple such entries. */
    BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->lookup(srcAddr);
    if (bulEntry == nullptr) {
        EV_WARN << "Invalid HoT: No entry in BUL" << endl;
        return false;    // no entry in BUL
    }

    /* The Binding Update List indicates that no home keygen token has
       been received yet. */
    // TODO reactivate this code as soon as token expiry is available in the BUL
    /*if (bulEntry->tokenH != UNDEFINED_TOKEN)
       {
        EV << "Invalid HoT: Home keygen token already exists." << endl;
        return false; // 0 is expected to indicate "undefined"
       }*/

    /* The Destination Address of the packet has the home address of the
       mobile node, and the packet has been received in a tunnel from the
       home agent. */
    // TODO check whether it is necessary to check HoA exlusively for the
    // interface the message was received on.
    if (!rt6->isHomeAddress(destAddr)) {
        EV_WARN << "Invalid HoT: Destination Address not HoA." << endl;
        return false;    // TODO check whether packet was received from HA tunnel
    }

    /* The Home Init Cookie field in the message matches the value stored
       in the Binding Update List. */
    if (bulEntry->cookieHoTI != (int)homeTest.getHomeInitCookie()) {
        EV_WARN << "Invalid HoT: Cookie value different from the stored one." << endl;
        return false;
    }

    // if we have come that far, the HoT is valid
    return true;
}

void xMIPv6::processCoTMessage(Packet * inPacket, const Ptr<const CareOfTest>& CoT)
{
    if (!validateCoTMessage(inPacket, *CoT)) {
        EV_WARN << "CoT validation not passed: dropping message" << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, inPacket, &details);
    }
    else {
        EV_INFO << "CoT validation passed: updating BUL" << endl;

        Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
        //Ipv6Address& destAddr = ctrlInfo->destAddr();
        auto ifTag = inPacket->getTag<InterfaceInd>();
        int interfaceID = ifTag->getInterfaceId();

        InterfaceEntry *ie = ift->getInterfaceById(interfaceID);

        BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->lookup(srcAddr);
        ASSERT(bulEntry != nullptr);

        // cancel the existing CoTI retransmission timer
        cancelTimerIfEntry(srcAddr, interfaceID, KEY_CI);

        // set the care-of keygen token
        bulEntry->tokenC = CoT->getCareOfKeyGenToken();

        // set token expiry appropriately
        createCareOfTokenEntryExpiryTimer(srcAddr, ift->getInterfaceById(interfaceID),
                simTime() + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime());

        // if we have the home token as well, we can send the BU to the CN now
        // but only if we have already sent or are about to send a BU

        // call method that potentially sends BU now
        checkForBUtoCN(*bulEntry, ie);

        // reset the CoTI timer for the time the token expires
        //resetTestInitIfEntry(srcAddr, interfaceID, KEY_CI);

        /*// delete the CoT retransmission timer
           cancelTimerIfEntry(srcAddr, interfaceID, KEY_CI);*/

        // gather statistics
        /*statVectorCoTfromCN.record(1);*/
    }

    delete inPacket;
}

bool xMIPv6::validateCoTMessage(Packet *inPacket, const CareOfTest& CoT)
{
    // RFC - 11.6.2
    Ipv6Address srcAddr = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address destAddr = inPacket->getTag<L3AddressInd>()->getDestAddress().toIpv6();
    auto ifTag = inPacket->getTag<InterfaceInd>();
    int interfaceID = ifTag->getInterfaceId();

    /* The Source Address of the packet belongs to a correspondent node
       for which the mobile node has a Binding Update List entry with a
       state indicating that return routability procedure is in progress.
       Note that there may be multiple such entries. */
    BindingUpdateList::BindingUpdateListEntry *bulEntry = bul->lookup(srcAddr);
    if (bulEntry == nullptr) {
        EV_WARN << "Invalid CoT: No entry in BUL" << endl;
        return false;    // no entry in BUL
    }

    if (bulEntry->sentHoTI == 0 && bulEntry->sentCoTI == 0) {
        EV_WARN << "Invalid CoT: No RR procedure initialized for this CN." << endl;
        return false;    // no RR procedure started for this entry
    }

    /* The Binding Update List indicates that no care-of keygen token has
       been received yet.. */
    // TODO reactive this code as soon as token expiry is available in the BUL
    /*if (bulEntry->tokenC != UNDEFINED_TOKEN)
       {
        EV << "Invalid CoT: Already received a care-of keygen token." << endl;
        return false; // 0 is expected to indicate "undefined"
       }*/

    /* The Destination Address of the packet is the current care-of
       address of the mobile node. */
    InterfaceEntry *ie = ift->getInterfaceById(interfaceID);
    if (destAddr != ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA)) {
        EV_WARN << "Invalid CoT: CoA not valid anymore." << endl;
        return false;
    }

    /* The Care-of Init Cookie field in the message matches the value
       stored in the Binding Update List. */
    if (bulEntry->cookieCoTI != (int)CoT.getCareOfInitCookie()) {
        EV_WARN << "Invalid CoT: Cookie value different from the stored one." << endl;
        return false;
    }

    // if we have come that far, the CoT is valid
    return true;
}

bool xMIPv6::checkForBUtoCN(BindingUpdateList::BindingUpdateListEntry& bulEntry, InterfaceEntry *ie)
{
    EV_INFO << "Checking whether a new BU has to be sent to CN." << endl;

    if (bulEntry.state == BindingUpdateList::DEREGISTER) {
        // we are supposed to send a deregistration BU

        if (!bul->isValidBinding(bulEntry.destAddress)) {
            // no valid binding existing; nothing to do
            // TODO cleanup operations?
            return false;
        }

        // we need a valid home keygen token for deregistration
        if (bul->isHomeTokenAvailable(bulEntry.destAddress, ie)) {
            sendBUtoCN(bulEntry, ie);
            return true;
        }
        else {
            // no token available
            // send HoTI
            createAndSendHoTIMessage(bulEntry.destAddress, ie);
            return false;
        }
    }
    else {    // case REGISTER, REGISTERED and anything else
              // for a registration BU we need both valid care-of and home keygen tokens
        if (!bul->isCareOfTokenAvailable(bulEntry.destAddress, ie)) {
            if (bul->recentlySentCOTI(bulEntry.destAddress, ie))
                return false;

            createAndSendCoTIMessage(bulEntry.destAddress, ie);
            return false;
        }

        if (!bul->isHomeTokenAvailable(bulEntry.destAddress, ie)) {
            if (bul->recentlySentHOTI(bulEntry.destAddress, ie))
                return false;

            createAndSendHoTIMessage(bulEntry.destAddress, ie);
            return false;
        }

        bulEntry.state = BindingUpdateList::RR_COMPLETE;

        // tokens are available: now we also need the case that the binding is about to expire or we have no valid binding at all
        if ((bul->isBindingAboutToExpire(bulEntry.destAddress)
             || !bul->isValidBinding(bulEntry.destAddress))
            && bulEntry.state != BindingUpdateList::REGISTER
            && bulEntry.state != BindingUpdateList::NONE)
        {
            sendBUtoCN(bulEntry, ie);
            return true;
        }
        else
            return false;
    }
}

void xMIPv6::sendBUtoCN(BindingUpdateList::BindingUpdateListEntry& bulEntry, InterfaceEntry *ie)
{
    /*11.7.2
       Upon successfully completing the return routability procedure, and
       after receiving a successful Binding Acknowledgement from the Home
       Agent, a Binding Update MAY be sent to the correspondent node.*/
    EV_INFO << "Sending BU to CN" << endl;

    /*
       // The createBU method requires information on the interface. This is not available in the
       // bul entry though. Hence we just fetch a token expiry timer to retrieve this information.
       TimerIfEntry* transmitEntry = searchTimerIfEntry(bulEntry.destAddress,     KEY_HTOKEN_EXP); // we could also use a care-of keygen token of course...
       ASSERT(transmitEntry!=nullptr);
       InterfaceEntry* ie = transmitEntry->ifEntry;*/

    if (bulEntry.state != BindingUpdateList::DEREGISTER)
        bulEntry.state = BindingUpdateList::REGISTER;

    createBUTimer(bulEntry.destAddress, ie);
    //createBUTimer(bulEntry.destAddress, ie, false);
}

void xMIPv6::processType2RH(Packet *packet, Ipv6RoutingHeader *rh)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    //EV << "Processing RH2..." << endl;

    if (!validateType2RH(*ipv6Header.get(), *rh)) {
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    bool validRH2 = false;
    const Ipv6Address& HoA = rh->getAddress(0);

    /*11.3.3
       A node receiving a packet addressed to itself (i.e., one of the
       node's addresses is in the Ipv6 destination field) follows the next
       header chain of headers and processes them.  When it encounters a
       type 2 routing header during this processing, it performs the
       following checks.  If any of these checks fail, the node MUST
       silently discard the packet.*/

    /*The length field in the routing header is exactly 2.*/
    // omitted

    /*The segments left field in the routing header is 1 on the wire.*/
    if (rh->getSegmentsLeft() != 1) {
        EV_WARN << "Invalid RH2 - segments left field must be 1. Dropping packet." << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    // probably datagram from CN to MN

    /*The Home Address field in the routing header is one of the node's
       home addresses, if the segments left field was 1.  Thus, in
       particular the address field is required to be a unicast routable
       address.*/
    if (rt6->isHomeAddress(HoA)) {
        /*Once the above checks have been performed, the node swaps the Ipv6
           destination field with the Home Address field in the routing header,
           decrements segments left by one from the value it had on the wire,
           and resubmits the packet to IP for processing the next header.*/
        ipv6Header = nullptr;
        packet->trimFront();
        auto newIpv6Header = packet->removeAtFront<Ipv6Header>();
        newIpv6Header->setDestAddress(HoA);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        validRH2 = true;
    }
    else {
        /*If any of these checks fail, the node MUST
           silently discard the packet.*/
        //delete datagram;
        EV_WARN << "Invalid RH2 - not a HoA. Dropping packet." << endl;
    }

    packet->setContextPointer(nullptr);

    if (validRH2) {
        EV_INFO << "Valid RH2 - copied address from RH2 to datagram" << endl;
        send(packet, "toIPv6");
    }
    else {
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

bool xMIPv6::validateType2RH(const Ipv6Header& ipv6Header, const Ipv6RoutingHeader& rh)
{
    // cf. RFC 3775 - 6.4

    if (rh.getAddressArraySize() != 1) {
        EV_WARN << "Invalid RH2 header: no HoA provided!" << endl;
        return false;
    }

    Ipv6Address CoA = ipv6Header.getSrcAddress();
    Ipv6Address HoA = rh.getAddress(0);

    /* The IP address contained in the routing header, since it is the mobile
       node's home address, MUST be a unicast routable address. */
    if (!HoA.isUnicast()) {
        EV_WARN << "Invalid RH2 header: address in RH2 is not a unicast routable address!" << endl;
        return false;
    }

    /* Furthermore, if the scope of the home address is smaller than the
       scope of the care-of address, the mobile node MUST discard the packet */
    if (HoA.getScope() < CoA.getScope()) {
        EV_WARN << "Invalid RH2 header: scope of Address in RH2 is smaller than the source address of the datagram!" << endl;
        return false;
    }

    return true;
}

void xMIPv6::processHoAOpt(Packet *packet, HomeAddressOption *hoaOpt)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();

    // datagram from MN to CN
    bool validHoAOpt = false;
    const Ipv6Address& HoA = hoaOpt->getHomeAddress();
    const Ipv6Address& CoA = ipv6Header->getSrcAddress();

    /*9.3.1
       Packets containing a
       Home Address option MUST be dropped if there is no corresponding
       Binding Cache entry.  A corresponding Binding Cache entry MUST have
       the same home address as appears in the Home Address destination
       option, and the currently registered care-of address MUST be equal to
       the source address of the packet.*/
    ASSERT(bc != nullptr);

    if (bc->isInBindingCache(HoA, CoA)) {
        ipv6Header = nullptr;
        packet->trimFront();
        auto newIpv6Header = packet->removeAtFront<Ipv6Header>();
        newIpv6Header->setSrcAddress(HoA);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, newIpv6Header);
        validHoAOpt = true;
    }
    else {
        EV_WARN << "Invalid RH2 destination - no entry in binding cache. Dropping packet." << endl;

        /*If the packet is dropped due the above tests, the correspondent node
           MUST send the Binding Error message as described in Section 9.3.3.
           The Status field in this message should be set to 1 (unknown binding
           for Home Address destination option).*/
        // previous code was erroneous, as destination from packet is probably
        // not a valid address of the interfaces of this node
        BeStatus status = UNKNOWN_BINDING_FOR_HOME_ADDRESS_DEST_OPTION;
        createAndSendBEMessage(CoA, status);
    }

    packet->setContextPointer(nullptr);
    delete hoaOpt;

    if (validHoAOpt) {
        EV_INFO << "Valid HoA Option - copied address from HoA Option to datagram" << endl;
        send(packet, "toIPv6");
    }
    else {
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void xMIPv6::createAndSendBEMessage(const Ipv6Address& dest, const BeStatus& beStatus)
{
    EV_TRACE << "\n<<<<<<<<< Entered createAndSendBEMessage() Function>>>>>>>\n";

    auto packet = new Packet("BError");
    const auto& be = makeShared<BindingError>();
    be->setMobilityHeaderType(BINDING_ERROR);
    be->setStatus(beStatus);

    // setting message size
    be->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_BE));
    packet->insertAtFront(be);

    sendMobilityMessageToIPv6Module(packet, dest);
}

bool xMIPv6::cancelTimerIfEntry(const Ipv6Address& dest, int interfaceID, int msgType)
{
    Key key(dest, interfaceID, msgType);
    auto pos = transmitIfList.find(key);

    if (pos == transmitIfList.end()) {
        //EV << "### No corresponding TimerIfEntry found! ###\n";
        return false;
    }

    TimerIfEntry *entry = (pos->second);

    cancelAndDelete(entry->timer);    // cancels the retransmission timer
    entry->timer = nullptr;

    transmitIfList.erase(key);    // remove entry from list

    delete entry;    // free memory

    EV_DEBUG << "Deleted TimerIfEntry and corresponding timer.\n";

    return true;
}

bool xMIPv6::pendingTimerIfEntry(Ipv6Address& dest, int interfaceID, int msgType)
{
    Key key(dest, interfaceID, msgType);
    auto pos = transmitIfList.find(key);

    // return true if there is an entry
    // and false otherwise
    return pos != transmitIfList.end();
}

xMIPv6::TimerIfEntry *xMIPv6::getTimerIfEntry(Key& key, int timerType)
{
    TimerIfEntry *ifEntry;
    auto pos = transmitIfList.find(key);

    if (pos != transmitIfList.end()) {
        // there already exists an unACKed retransmission timer for that entry
        // -> overwrite the old with the new one
        if (TestInitTransmitIfEntry *testInitIfEntry = dynamic_cast<TestInitTransmitIfEntry *>(pos->second)) {
            cancelAndDelete(testInitIfEntry->timer);    // delete the corresponding timer
            testInitIfEntry->testInitMsg = nullptr;

            ifEntry = testInitIfEntry;
        }
        else if (BuTransmitIfEntry *buIfEntry = dynamic_cast<BuTransmitIfEntry *>(pos->second)) {
            cancelAndDelete(buIfEntry->timer);    // delete the corresponding timer

            ifEntry = buIfEntry;
        }
        else if (BulExpiryIfEntry *bulExpIfEntry = dynamic_cast<BulExpiryIfEntry *>(pos->second)) {
            cancelAndDelete(bulExpIfEntry->timer);    // delete the corresponding timer

            ifEntry = bulExpIfEntry;
        }
        else if (BcExpiryIfEntry *bcExpIfEntry = dynamic_cast<BcExpiryIfEntry *>(pos->second)) {
            cancelAndDelete(bcExpIfEntry->timer);    // delete the corresponding timer

            ifEntry = bcExpIfEntry;
        }
        else if (TokenExpiryIfEntry *tokenExpIfEntry = dynamic_cast<TokenExpiryIfEntry *>(pos->second)) {
            cancelAndDelete(tokenExpIfEntry->timer);    // delete the corresponding timer
            tokenExpIfEntry->timer = nullptr;

            ifEntry = tokenExpIfEntry;
        }
        else
            throw cRuntimeError("Expected a subclass of TimerIfEntry!");

        ifEntry->timer = nullptr;
    }
    else {
        // we do not yet have an entry -> create a new one
        switch (timerType) {
            case TRANSMIT_TYPE_BU:
                ifEntry = new BuTransmitIfEntry();
                break;

            case TRANSMIT_TYPE_TI:
                ifEntry = new TestInitTransmitIfEntry();
                break;

            case EXPIRY_TYPE_BUL:
                ifEntry = new BulExpiryIfEntry();
                break;

            case EXPIRY_TYPE_BC:
                ifEntry = new BcExpiryIfEntry();
                break;

            case EXPIRY_TYPE_TOKEN:
                ifEntry = new TokenExpiryIfEntry();
                break;

            default:
                throw cRuntimeError("Expected a valid TimerIfEntry type!");
                break;
        }

        ifEntry->timer = nullptr;
        ifEntry->ifEntry = nullptr;

        transmitIfList.insert(std::make_pair(key, ifEntry));
    }

    return ifEntry;
}

xMIPv6::TimerIfEntry *xMIPv6::searchTimerIfEntry(Ipv6Address& dest, int timerType)
{
    for (auto & elem : transmitIfList) {
        if (elem.first.type == timerType && elem.first.dest == dest)
            return elem.second;
    }

    return nullptr;
}

void xMIPv6::removeTimerEntries(const Ipv6Address& dest, int interfaceId)
{
    if (rt6->isMobileNode()) {
        // HoTI
        cancelTimerIfEntry(dest, interfaceId, KEY_HI);
        // CoTI
        cancelTimerIfEntry(dest, interfaceId, KEY_CI);
        // BU
        cancelTimerIfEntry(dest, interfaceId, KEY_BU);
        // BRR
        cancelTimerIfEntry(dest, interfaceId, KEY_BR);
        // BUL expiry
        cancelTimerIfEntry(dest, interfaceId, KEY_BUL_EXP);
        // BC expiry
        //cancelTimerIfEntry(dest, interfaceId, KEY_BC_EXP);
        // token expiry
        cancelTimerIfEntry(dest, interfaceId, KEY_HTOKEN_EXP);
        cancelTimerIfEntry(dest, interfaceId, KEY_CTOKEN_EXP);
    }
    else if (rt6->isHomeAgent()) {
        cancelTimerIfEntry(dest, interfaceId, KEY_BC_EXP);
    }
    else {    // CN
        cancelTimerIfEntry(dest, interfaceId, KEY_BC_EXP);
    }
}

void xMIPv6::cancelEntries(int interfaceId, Ipv6Address& CoA)
{
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);

    // we have to cancel all existing timers

    // ...first for the HA
    Ipv6Address HA = ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress();
    //Ipv6Address HoA = ie->ipv6()->getMNHomeAddress();

    cancelTimerIfEntry(HA, interfaceId, KEY_BU);
    tunneling->destroyTunnel(CoA, HA);

    // ...and then for the CNs
    for (auto it = transmitIfList.begin(); it != transmitIfList.end(); ) {
        if ((*it).first.interfaceID == interfaceId) {
            auto oldIt = it++;

            // destroy tunnel (if we have a BU entry here)
            if ((*oldIt).first.type == KEY_BU)
                tunneling->destroyTunnelForEntryAndTrigger(CoA, (*oldIt).first.dest);

            // then cancel the pending event
            cancelTimerIfEntry((*oldIt).first.dest, (*oldIt).first.interfaceID, (*oldIt).first.type);
        }
        else
            ++it;
    }
}

void xMIPv6::removeCoAEntries()
{
    for (auto & elem : interfaceCoAList) {
        //if (it->first == ie->interfaceId())
        EV_DEBUG << "Cancelling timers for ifID=" << elem.first << " and CoA=" << elem.second << endl;
        cancelEntries(elem.first, elem.second);
    }

    // delete all entries after cleanup
    interfaceCoAList.clear();
}

void xMIPv6::createBRRTimer(const Ipv6Address& brDest, InterfaceEntry *ie, const uint scheduledTime)
{
    /*9.5.5
       If the sender knows that the Binding Cache entry is still in active
       use, it MAY send a Binding Refresh Request message to the mobile node
       in an attempt to avoid this overhead and latency due to deleting and
       recreating the Binding Cache entry.  This message is always sent to
       the home address of the mobile node.*/

    Enter_Method("createBRRTimer()");

    // check if there already exists a BRTimer entry for this key
    Key key(brDest, ie->getInterfaceId(), KEY_BR);
    BrTransmitIfEntry *brIfEntry;

    auto pos = transmitIfList.find(key);
    if (pos != transmitIfList.end()) {
        // there already exists an unACKed retransmission timer for that entry
        // -> overwrite the old with the new one
        if ((brIfEntry = dynamic_cast<BrTransmitIfEntry *>(pos->second))) {
            cancelAndDelete(brIfEntry->timer);    // delete the corresponding timer
        }
        else
            throw cRuntimeError("Expected BrTransmitIfEntry* !");
    }
    else {
        // we do not yet have an entry -> create a new one
        brIfEntry = new BrTransmitIfEntry();
        transmitIfList.insert(std::make_pair(key, brIfEntry));
    }

    brIfEntry->dest = brDest;
    brIfEntry->ifEntry = ie;
    brIfEntry->retries = 0;

    cMessage *brTriggerMsg = new cMessage("sendPeriodicBRR", MK_SEND_PERIODIC_BR);
    brIfEntry->timer = brTriggerMsg;
    brTriggerMsg->setContextPointer(brIfEntry);    // attaching the brIfEntry info corresponding to a particular address ith message

    // Scheduling a message which will trigger a BRR towards brIfEntry->dest
    scheduleAt(simTime() + scheduledTime, brTriggerMsg);
    EV_DETAIL << "\n++++++++++BRR TIMER CREATED FOR SIM TIME: " << simTime() + scheduledTime
              << " seconds+++++++++++++++++ \n";
}

void xMIPv6::sendPeriodicBRR(cMessage *msg)
{
    EV_INFO << "\n<<====== Periodic BRR MESSAGE at time: " << simTime() << " seconds =====>>\n";
    BrTransmitIfEntry *brIfEntry = (BrTransmitIfEntry *)msg->getContextPointer();    //detaching the corresponding brIfEntry pointer
    InterfaceEntry *ie = brIfEntry->ifEntry;
    Ipv6Address& brDest = brIfEntry->dest;

    /*9.5.5
       The correspondent node MAY retransmit Binding Refresh Request
       messages as long as the rate limitation is applied.  The
       correspondent node MUST stop retransmitting when it receives a
       Binding Update.*/
    if (brIfEntry->retries < BRR_RETRIES) {    // we'll not retransmit infinite times
        brIfEntry->retries++;

        createAndSendBRRMessage(brDest, ie);

        // retransmit the Binding Refresh Message
        scheduleAt(simTime() + BRR_TIMEOUT_THRESHOLD, msg);
    }
    else {
        // we've tried often enough - remove the timer
        cancelTimerIfEntry(brDest, ie->getInterfaceId(), KEY_BR);
    }
}

void xMIPv6::createAndSendBRRMessage(const Ipv6Address& dest, InterfaceEntry *ie)
{
    EV_TRACE << "\n<<======THIS IS THE ROUTINE FOR CREATING AND SENDING BRR MESSAGE =====>>\n";
    auto outPacket = new Packet("Binding Refresh Request");
    const auto& brr = makeShared<BindingRefreshRequest>();

    /*6.1.2
       The Binding Refresh Request message uses the MH Type value 0.  When
       this value is indicated in the MH Type field, the format of the
       Message Data field in the Mobility Header is as follows:*/
    brr->setMobilityHeaderType(BINDING_REFRESH_REQUEST);

    brr->setChunkLength(B(SIZE_MOBILITY_HEADER + SIZE_BRR));
    outPacket->insertAtFront(brr);

    EV_INFO << "\n<<======BRR MESSAGE FORMED; APPENDING CONTROL INFO=====>>\n";
    Ipv6Address CoA = ie->getProtocolData<Ipv6InterfaceData>()->getGlobalAddress(Ipv6InterfaceData::CoA);
    ASSERT(!CoA.isUnspecified());
    sendMobilityMessageToIPv6Module(outPacket, dest, CoA, ie->getInterfaceId());
}

void xMIPv6::processBRRMessage(Packet *inPacket, const Ptr<const BindingRefreshRequest>& brr)
{
    /*11.7.4
       When a mobile node receives a packet containing a Binding Refresh
       Request message, the mobile node has a Binding Update List entry for
       the source of the Binding Refresh Request, and the mobile node wants
       to retain its binding cache entry at the correspondent node, then the
       mobile node should start a return routability procedure.*/
    Ipv6Address cnAddress = inPacket->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    Ipv6Address HoA = inPacket->getTag<L3AddressInd>()->getDestAddress().toIpv6();

    if (!bul->isInBindingUpdateList(cnAddress, HoA)) {
        EV_WARN << "BRR not accepted - no binding for this CN. Dropping message." << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, inPacket, &details);
    }
    else {
        auto ifTag = inPacket->getTag<InterfaceInd>();
        InterfaceEntry *ie = ift->getInterfaceById(ifTag->getInterfaceId());
        initReturnRoutability(cnAddress, ie);
    }

    delete inPacket;
}

void xMIPv6::createBULEntryExpiryTimer(BindingUpdateList::BindingUpdateListEntry *entry, InterfaceEntry *ie, simtime_t scheduledTime)
{
    //Enter_Method("createBULEntryExpiryTimer()");
    //EV << "Creating BUL entry expiry timer for sim time: " << entry->bindingExpiry << " seconds." << endl;
    cMessage *bulExpiryMsg = new cMessage("BULEntryExpiry", MK_BUL_EXPIRY);

    // we are able to associate the BUL entry later on based on HoA, CoA and destination (=HA address)
    Ipv6Address& HoA = entry->homeAddress;
    Ipv6Address& CoA = entry->careOfAddress;
    Ipv6Address& HA = entry->destAddress;

    Key key(HA, ie->getInterfaceId(), KEY_BUL_EXP);
    // fetch a valid TimerIfEntry obect
    BulExpiryIfEntry *bulExpIfEntry = check_and_cast<BulExpiryIfEntry *>(getTimerIfEntry(key, EXPIRY_TYPE_BUL));

    bulExpIfEntry->dest = HA;
    bulExpIfEntry->HoA = HoA;
    bulExpIfEntry->CoA = CoA;
    bulExpIfEntry->ifEntry = ie;
    bulExpIfEntry->timer = bulExpiryMsg;

    bulExpiryMsg->setContextPointer(bulExpIfEntry);    // information in the bulExpIfEntry is required for handler when message fires

    /*BulExpiryIfEntry* bulExpIfEntry = createBULEntryExpiryTimer(key, HA, HoA, CoA, ie);*/

    scheduleAt(scheduledTime, bulExpiryMsg);
    EV_INFO << "Scheduled BUL expiry (" << entry->bindingExpiry << "s) for time " << scheduledTime << "s" << endl;
    // WAS SCHEDULED FOR EXPIRY, NOT 2 SECONDS BEFORE!?!?!?
}

/*BulExpiryIfEntry* xMIPv6::createBULEntryExpiryTimer(Key& key, IPv6Adress& dest, IPv6Adress& HoA, IPv6Adress& CoA, InterfaceEntry* ie, cMessage* bulExpiryMsg)
   {
    BulExpiryIfEntry* bulExpIfEntry = (BulExpiryIfEntry*) getTimerIfEntry(key, EXPIRY_TYPE_BUL);

    bulExpIfEntry->dest = HA;
    bulExpIfEntry->HoA = HoA;
    bulExpIfEntry->CoA = CoA;
    bulExpIfEntry->ifEntry = ie;
    bulExpIfEntry->timer = bulExpiryMsg;

    bulExpiryMsg->setContextPointer(bulExpIfEntry); // information in the bulExpIfEntry is required for handler when message fires

    return bulExpIfEntry;
   }*/

void xMIPv6::handleBULExpiry(cMessage *msg)
{
    /*11.7.1
       Also, if the mobile node wants the services of the home agent beyond
       the current registration period, the mobile node should send a new
       Binding Update to it well before the expiration of this period, even
       if it is not changing its primary care-of address.*/
    BulExpiryIfEntry *bulExpIfEntry = (BulExpiryIfEntry *)msg->getContextPointer();    //detaching the corresponding bulExpIfEntry pointer
    ASSERT(bulExpIfEntry != nullptr);

    // we fetch the BUL entry that belongs to this expiration timer
    BindingUpdateList::BindingUpdateListEntry *entry = bul->lookup(bulExpIfEntry->dest);
    ASSERT(entry != nullptr);

    // A timer usually can appear for two times:
    // 1. Entry is shortly before expiration -> send new BU
    // 2. Entry expired -> remove
    if (simTime() < entry->bindingExpiry) {
        EV_INFO << "BUL entry about to expire - creating new BU timer..." << endl;
        // we have to store the pointer to the InterfaceIdentifier as the BUL expiry timer
        // will be canceled/deleted by createBUTimer(...)
        InterfaceEntry *ie = bulExpIfEntry->ifEntry;

        // send new BU
        // we immediately create a new BU transmission timer for BU to HA
        // but we only trigger BU creation for transmission to CN
        if (bulExpIfEntry->dest == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress())
            createBUTimer(bulExpIfEntry->dest, bulExpIfEntry->ifEntry);
        else {
            BindingUpdateList::BindingUpdateListEntry *entry = bul->lookup(bulExpIfEntry->dest);
            checkForBUtoCN(*entry, ie);
        }

        // we reschedule the expiration timer for expiry time
        EV_DETAIL << "Resetting expiry timer... " << endl;
        createBULEntryExpiryTimer(entry, ie, entry->bindingExpiry);

        // ...and that's it.
        // createBUTimer will take care of cancelling/overwriting the current BU timer.
        // As soon as we receive a valid BA to this BU the BUL expiry timer will be reset.
    }
    else {
        EV_INFO << "BUL entry has expired - removing entry and associated structures..." << endl;

        // TODO group everything from below in a purgeMobilityState() method

        entry->state = BindingUpdateList::NONE;
        // remove binding
        bul->removeBinding(bulExpIfEntry->dest);

        // remove all timers
        int interfaceID = bulExpIfEntry->ifEntry->getInterfaceId();
        removeTimerEntries(bulExpIfEntry->dest, interfaceID);

        // destroy tunnel
        tunneling->destroyTunnel(bulExpIfEntry->CoA, bulExpIfEntry->dest);

        // and remove entry from list
        cancelTimerIfEntry(bulExpIfEntry->dest, interfaceID, KEY_BUL_EXP);

        // deletion of the message already takes place in the cancelTimerIfEntry(.., KEY_BUL_EXP);
        //delete msg;
    }
}

void xMIPv6::createBCEntryExpiryTimer(const Ipv6Address& HoA, InterfaceEntry *ie, simtime_t scheduledTime)
{
    cMessage *bcExpiryMsg = new cMessage("BCEntryExpiry", MK_BC_EXPIRY);

    Key key(HoA, ie->getInterfaceId(), KEY_BC_EXP);
    // fetch a valid TimerIfEntry obect
    BcExpiryIfEntry *bcExpIfEntry = check_and_cast<BcExpiryIfEntry *>(getTimerIfEntry(key, EXPIRY_TYPE_BC));

    bcExpIfEntry->dest = HoA;
    bcExpIfEntry->HoA = HoA;
    bcExpIfEntry->ifEntry = ie;
    bcExpIfEntry->timer = bcExpiryMsg;

    bcExpiryMsg->setContextPointer(bcExpIfEntry);    // information in the bulExpIfEntry is required for handler when message fires

    scheduleAt(scheduledTime, bcExpiryMsg);
    EV_INFO << "Scheduled BC expiry for time " << scheduledTime << "s" << endl;
}

void xMIPv6::handleBCExpiry(cMessage *msg)
{
    /*10.3.1
       The home agent MAY further decrease the specified lifetime for the
       binding, for example based on a local policy.  The resulting
       lifetime is stored by the home agent in the Binding Cache entry,
       and this Binding Cache entry MUST be deleted by the home agent
       after the expiration of this lifetime.*/
    /*9.5.2
       Any Binding Cache entry MUST be deleted after the expiration of its lifetime.*/
    EV_INFO << "BC entry has expired - removing entry and associated structures..." << endl;

    BcExpiryIfEntry *bcExpIfEntry = (BcExpiryIfEntry *)msg->getContextPointer();    //detaching the corresponding bulExpIfEntry pointer
    ASSERT(bcExpIfEntry != nullptr);

    // remove binding from BC
    bc->deleteEntry(bcExpIfEntry->HoA);

    // and remove the tunnel
    tunneling->destroyTunnelFromTrigger(bcExpIfEntry->HoA);

    // and remove entry from list
    cancelTimerIfEntry(bcExpIfEntry->dest, bcExpIfEntry->ifEntry->getInterfaceId(), KEY_BC_EXP);
    // deletion of the message already takes place in the cancelTimerIfEntry(.., KEY_BC_EXP);

    // TODO
    // in the future we might send a Binding Refresh Request shortly before the expiration of the BCE
}

void xMIPv6::createTokenEntryExpiryTimer(Ipv6Address& cnAddr, InterfaceEntry *ie,
        simtime_t scheduledTime, int tokenType)
{
    cMessage *tokenExpiryMsg = new cMessage("TokenEntryExpiry", MK_TOKEN_EXPIRY);

    Key key(cnAddr, ie->getInterfaceId(), tokenType);
    // fetch a valid TimerIfEntry obect
    TokenExpiryIfEntry *tokenExpIfEntry = check_and_cast<TokenExpiryIfEntry *>(getTimerIfEntry(key, EXPIRY_TYPE_TOKEN));

    tokenExpIfEntry->cnAddr = cnAddr;
    tokenExpIfEntry->ifEntry = ie;
    tokenExpIfEntry->timer = tokenExpiryMsg;

    tokenExpIfEntry->tokenType = tokenType;

    tokenExpiryMsg->setContextPointer(tokenExpIfEntry);

    scheduleAt(scheduledTime, tokenExpiryMsg);
    EV_INFO << "Scheduled token expiry for time " << scheduledTime << "s" << endl;
}

void xMIPv6::handleTokenExpiry(cMessage *msg)
{
    TokenExpiryIfEntry *tokenExpIfEntry = (TokenExpiryIfEntry *)msg->getContextPointer();    //detaching the corresponding tokenExpIfEntry pointer
    ASSERT(tokenExpIfEntry != nullptr);

    if (tokenExpIfEntry->tokenType == KEY_CTOKEN_EXP) {
        EV_INFO << "Care-of keygen token for CN=" << tokenExpIfEntry->cnAddr << " expired";
        bul->resetCareOfToken(tokenExpIfEntry->cnAddr, tokenExpIfEntry->ifEntry->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress());
    }
    else if (tokenExpIfEntry->tokenType == KEY_HTOKEN_EXP) {
        EV_INFO << "Home keygen token for CN=" << tokenExpIfEntry->cnAddr << " expired";
        bul->resetHomeToken(tokenExpIfEntry->cnAddr, tokenExpIfEntry->ifEntry->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress());
    }
    else
        throw cRuntimeError("Unkown value for tokenType!");

    EV_INFO << "...removed token from BUL." << endl;

    if (bul->getMobilityState(tokenExpIfEntry->cnAddr) == BindingUpdateList::RR_COMPLETE)
        bul->setMobilityState(tokenExpIfEntry->cnAddr, BindingUpdateList::RR);

    cancelTimerIfEntry(tokenExpIfEntry->cnAddr, tokenExpIfEntry->ifEntry->getInterfaceId(), tokenExpIfEntry->tokenType);

    // and now send a Test Init message to retrieve a new token
    if (tokenExpIfEntry->tokenType == KEY_CTOKEN_EXP)
        createAndSendCoTIMessage(tokenExpIfEntry->cnAddr, tokenExpIfEntry->ifEntry);
    else
        createAndSendHoTIMessage(tokenExpIfEntry->cnAddr, tokenExpIfEntry->ifEntry);

    //delete msg;
}

} // namespace inet

