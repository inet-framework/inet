/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
 * Copyright (C) 2014 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/arp/ipv4/Arp.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

simsignal_t Arp::arpRequestSentSignal = registerSignal("arpRequestSent");
simsignal_t Arp::arpReplySentSignal = registerSignal("arpReplySent");

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

static std::ostream& operator<<(std::ostream& out, const Arp::ArpCacheEntry& e)
{
    if (e.pending)
        out << "pending (" << e.numRetries << " retries)";
    else
        out << "MAC:" << e.macAddress << "  age:" << floor(simTime() - e.lastUpdate) << "s";
    return out;
}

Define_Module(Arp);

Arp::Arp()
{
}

void Arp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        retryTimeout = par("retryTimeout");
        retryCount = par("retryCount");
        cacheTimeout = par("cacheTimeout");
        respondToProxyARP = par("respondToProxyARP");

        // init statistics
        numRequestsSent = numRepliesSent = 0;
        numResolutions = numFailedResolutions = 0;
        WATCH(numRequestsSent);
        WATCH(numRepliesSent);
        WATCH(numResolutions);
        WATCH(numFailedResolutions);

        WATCH_PTRMAP(arpCache);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // TODO: registerProtocol
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {    // IP addresses should be available
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        isUp = isNodeUp();
        registerService(Protocol::arp, gate("netwIn"), gate("ifIn"));
        registerProtocol(Protocol::arp, gate("ifOut"), gate("netwOut"));
    }
}

void Arp::finish()
{
}

Arp::~Arp()
{
    for (auto & elem : arpCache)
        delete elem.second;
}

void Arp::handleMessage(cMessage *msg)
{
    if (!isUp) {
        handleMessageWhenDown(msg);
        return;
    }

    if (msg->isSelfMessage()) {
        requestTimedOut(msg);
    }
    else {
        Packet *packet = check_and_cast<Packet *>(msg);
        processARPPacket(packet);
    }
}

void Arp::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
    EV_WARN << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool Arp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_NETWORK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_NETWORK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }
    return true;
}

void Arp::start()
{
    ASSERT(arpCache.empty());
    isUp = true;
}

void Arp::stop()
{
    isUp = false;
    flush();
}

void Arp::flush()
{
    while (!arpCache.empty()) {
        auto i = arpCache.begin();
        ArpCacheEntry *entry = i->second;
        cancelAndDelete(entry->timer);
        entry->timer = nullptr;
        delete entry;
        arpCache.erase(i);
    }
}

bool Arp::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void Arp::refreshDisplay() const
{
    std::stringstream os;

    os << "size:" << arpCache.size() << " sent:" << numRequestsSent << "\n"
       << "repl:" << numRepliesSent << " fail:" << numFailedResolutions;

    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void Arp::initiateARPResolution(ArpCacheEntry *entry)
{
    Ipv4Address nextHopAddr = entry->myIter->first;
    entry->pending = true;
    entry->numRetries = 0;
    entry->lastUpdate = SIMTIME_ZERO;
    entry->macAddress = MacAddress::UNSPECIFIED_ADDRESS;
    sendARPRequest(entry->ie, nextHopAddr);

    // start timer
    cMessage *msg = entry->timer = new cMessage("ARP timeout");
    msg->setContextPointer(entry);
    scheduleAt(simTime() + retryTimeout, msg);

    numResolutions++;
    Notification signal(nextHopAddr, MacAddress::UNSPECIFIED_ADDRESS, entry->ie);
    emit(arpResolutionInitiatedSignal, &signal);
}

void Arp::sendARPRequest(const InterfaceEntry *ie, Ipv4Address ipAddress)
{
    // find our own IPv4 address and MAC address on the given interface
    MacAddress myMACAddress = ie->getMacAddress();
    Ipv4Address myIPAddress = ie->ipv4Data()->getIPAddress();

    // both must be set
    ASSERT(!myMACAddress.isUnspecified());
    ASSERT(!myIPAddress.isUnspecified());

    // fill out everything in ARP Request packet except dest MAC address
    Packet *packet = new Packet("arpREQ");
    const auto& arp = makeShared<ArpPacket>();
    arp->setOpcode(ARP_REQUEST);
    arp->setSrcMacAddress(myMACAddress);
    arp->setSrcIpAddress(myIPAddress);
    arp->setDestIpAddress(ipAddress);
    packet->insertAtFront(arp);

    packet->addTag<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    packet->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::arp);
    // send out
    EV_INFO << "Sending " << packet << " to network protocol.\n";
    emit(arpRequestSentSignal, packet);
    send(packet, "ifOut");
    numRequestsSent++;
}

void Arp::requestTimedOut(cMessage *selfmsg)
{
    ArpCacheEntry *entry = (ArpCacheEntry *)selfmsg->getContextPointer();
    entry->numRetries++;
    if (entry->numRetries < retryCount) {
        // retry
        Ipv4Address nextHopAddr = entry->myIter->first;
        EV_INFO << "ARP request for " << nextHopAddr << " timed out, resending\n";
        sendARPRequest(entry->ie, nextHopAddr);
        scheduleAt(simTime() + retryTimeout, selfmsg);
        return;
    }

    delete selfmsg;

    // max retry count reached: ARP failure.
    // throw out entry from cache
    EV << "ARP timeout, max retry count " << retryCount << " for " << entry->myIter->first << " reached.\n";
    Notification signal(entry->myIter->first, MacAddress::UNSPECIFIED_ADDRESS, entry->ie);
    emit(arpResolutionFailedSignal, &signal);
    arpCache.erase(entry->myIter);
    delete entry;
    numFailedResolutions++;
}

bool Arp::addressRecognized(Ipv4Address destAddr, InterfaceEntry *ie)
{
    if (rt->isLocalAddress(destAddr)) {
        return true;
    }
    else if (respondToProxyARP) {
        // respond to Proxy ARP request: if we can route this packet (and the
        // output port is different from this one), say yes
        InterfaceEntry *rtie = rt->getInterfaceForDestAddr(destAddr);
        return rtie != nullptr && rtie != ie;
    }
    else {
        return false;
    }
}

void Arp::dumpARPPacket(const ArpPacket *arp)
{
    EV_DETAIL << (arp->getOpcode() == ARP_REQUEST ? "ARP_REQ" : arp->getOpcode() == ARP_REPLY ? "ARP_REPLY" : "unknown type")
              << "  src=" << arp->getSrcIpAddress() << " / " << arp->getSrcMacAddress()
              << "  dest=" << arp->getDestIpAddress() << " / " << arp->getDestMacAddress() << "\n";
}

void Arp::processARPPacket(Packet *packet)
{
    EV_INFO << "Received " << packet << " from network protocol.\n";
    const auto& arp = packet->peekAtFront<ArpPacket>();
    dumpARPPacket(arp.get());

    // extract input port
    InterfaceEntry *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());

    //
    // Recipe a'la RFC 826:
    //
    // ?Do I have the hardware type in ar$hrd?
    // Yes: (almost definitely)
    //   [optionally check the hardware length ar$hln]
    //   ?Do I speak the protocol in ar$pro?
    //   Yes:
    //     [optionally check the protocol length ar$pln]
    //     Merge_flag := false
    //     If the pair <protocol type, sender protocol address> is
    //         already in my translation table, update the sender
    //         hardware address field of the entry with the new
    //         information in the packet and set Merge_flag to true.
    //     ?Am I the target protocol address?
    //     Yes:
    //       If Merge_flag is false, add the triplet <protocol type,
    //           sender protocol address, sender hardware address> to
    //           the translation table.
    //       ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
    //       Yes:
    //         Swap hardware and protocol fields, putting the local
    //             hardware and protocol addresses in the sender fields.
    //         Set the ar$op field to ares_op$REPLY
    //         Send the packet to the (new) target hardware address on
    //             the same hardware on which the request was received.
    //

    MacAddress srcMACAddress = arp->getSrcMacAddress();
    Ipv4Address srcIPAddress = arp->getSrcIpAddress();

    if (srcMACAddress.isUnspecified())
        throw cRuntimeError("wrong ARP packet: source MAC address is empty");
    if (srcIPAddress.isUnspecified())
        throw cRuntimeError("wrong ARP packet: source IPv4 address is empty");

    bool mergeFlag = false;
    // "If ... sender protocol address is already in my translation table"
    auto it = arpCache.find(srcIPAddress);
    if (it != arpCache.end()) {
        // "update the sender hardware address field"
        ArpCacheEntry *entry = it->second;
        updateARPCache(entry, srcMACAddress);
        mergeFlag = true;
    }

    // "?Am I the target protocol address?"
    // if Proxy ARP is enabled, we also have to reply if we're a router to the dest IPv4 address
    if (addressRecognized(arp->getDestIpAddress(), ie)) {
        // "If Merge_flag is false, add the triplet protocol type, sender
        // protocol address, sender hardware address to the translation table"
        if (!mergeFlag) {
            ArpCacheEntry *entry;
            if (it != arpCache.end()) {
                entry = it->second;
            }
            else {
                entry = new ArpCacheEntry();
                auto where = arpCache.insert(arpCache.begin(), std::make_pair(srcIPAddress, entry));
                entry->myIter = where;
                entry->ie = ie;

                entry->pending = false;
                entry->timer = nullptr;
                entry->numRetries = 0;
            }
            updateARPCache(entry, srcMACAddress);
        }

        // "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
        switch (arp->getOpcode()) {
            case ARP_REQUEST: {
                EV_DETAIL << "Packet was ARP REQUEST, sending REPLY\n";

                // find our own IPv4 address and MAC address on the given interface
                MacAddress myMACAddress = ie->getMacAddress();
                Ipv4Address myIPAddress = ie->ipv4Data()->getIPAddress();

                // "Swap hardware and protocol fields", etc.
                const auto& arpReply = makeShared<ArpPacket>();
                Ipv4Address origDestAddress = arp->getDestIpAddress();
                arpReply->setDestIpAddress(srcIPAddress);
                arpReply->setDestMacAddress(srcMACAddress);
                arpReply->setSrcIpAddress(origDestAddress);
                arpReply->setSrcMacAddress(myMACAddress);
                arpReply->setOpcode(ARP_REPLY);
                Packet *outPk = new Packet("arpREPLY");
                outPk->insertAtFront(arpReply);
                outPk->addTag<MacAddressReq>()->setDestAddress(srcMACAddress);
                outPk->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
                outPk->addTag<PacketProtocolTag>()->setProtocol(&Protocol::arp);

                // send out
                EV_INFO << "Sending " << outPk << " to network protocol.\n";
                emit(arpReplySentSignal, outPk);
                send(outPk, "ifOut");
                numRepliesSent++;
                break;
            }

            case ARP_REPLY: {
                EV_DETAIL << "Discarding packet\n";
                break;
            }

            case ARP_RARP_REQUEST:
                throw cRuntimeError("RARP request received: RARP is not supported");

            case ARP_RARP_REPLY:
                throw cRuntimeError("RARP reply received: RARP is not supported");

            default:
                throw cRuntimeError("Unsupported opcode %d in received ARP packet", arp->getOpcode());
        }
    }
    else {
        // address not recognized
        EV_INFO << "IPv4 address " << arp->getDestIpAddress() << " not recognized, dropping ARP packet\n";
    }
    delete packet;
}

void Arp::updateARPCache(ArpCacheEntry *entry, const MacAddress& macAddress)
{
    EV_DETAIL << "Updating ARP cache entry: " << entry->myIter->first << " <--> " << macAddress << "\n";

    // update entry
    if (entry->pending) {
        entry->pending = false;
        delete cancelEvent(entry->timer);
        entry->timer = nullptr;
        entry->numRetries = 0;
    }
    entry->macAddress = macAddress;
    entry->lastUpdate = simTime();
    Notification signal(entry->myIter->first, macAddress, entry->ie);
    emit(arpResolutionCompletedSignal, &signal);
}

MacAddress Arp::resolveL3Address(const L3Address& address, const InterfaceEntry *ie)
{
    Enter_Method("resolveMACAddress(%s,%s)", address.str().c_str(), ie->getInterfaceName());

    Ipv4Address addr = address.toIpv4();
    ArpCache::const_iterator it = arpCache.find(addr);
    if (it == arpCache.end()) {
        // no cache entry: launch ARP request
        ArpCacheEntry *entry = new ArpCacheEntry();
        entry->owner = this;
        auto where = arpCache.insert(arpCache.begin(), std::make_pair(addr, entry));
        entry->myIter = where;    // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        entry->ie = ie;

        EV << "Starting ARP resolution for " << addr << "\n";
        initiateARPResolution(entry);
        return MacAddress::UNSPECIFIED_ADDRESS;
    }
    else if (it->second->pending) {
        // an ARP request is already pending for this address
        EV << "ARP resolution for " << addr << " is already pending\n";
        return MacAddress::UNSPECIFIED_ADDRESS;
    }
    else if (it->second->lastUpdate + cacheTimeout >= simTime()) {
        return it->second->macAddress;
    }
    else {
        EV << "ARP cache entry for " << addr << " expired, starting new ARP resolution\n";
        ArpCacheEntry *entry = it->second;
        entry->ie = ie;    // routing table may have changed
        initiateARPResolution(entry);
    }
    return MacAddress::UNSPECIFIED_ADDRESS;
}

L3Address Arp::getL3AddressFor(const MacAddress& macAddr) const
{
    Enter_Method_Silent();

    if (macAddr.isUnspecified())
        return Ipv4Address::UNSPECIFIED_ADDRESS;

    simtime_t now = simTime();
    for (const auto & elem : arpCache)
        if (elem.second->macAddress == macAddr && elem.second->lastUpdate + cacheTimeout >= now)
            return elem.first;


    return Ipv4Address::UNSPECIFIED_ADDRESS;
}

} // namespace inet

