//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/arp/ipv4/Arp.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

simsignal_t Arp::arpRequestSentSignal = registerSignal("arpRequestSent");
simsignal_t Arp::arpReplySentSignal = registerSignal("arpReplySent");

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
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        retryTimeout = par("retryTimeout");
        retryCount = par("retryCount");
        cacheTimeout = par("cacheTimeout");
        proxyArpInterfaces = par("proxyArpInterfaces").stdstringValue();

        proxyArpInterfacesMatcher.setPattern(proxyArpInterfaces.c_str(), false, true, false);

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
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        registerService(Protocol::arp, gate("netwIn"), gate("netwOut"));
        registerProtocol(Protocol::arp, gate("ifOut"), gate("ifIn"));
    }
}

void Arp::finish()
{
}

Arp::~Arp()
{
    for (auto& elem : arpCache)
        delete elem.second;
}

void Arp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        requestTimedOut(msg);
    }
    else {
        Packet *packet = check_and_cast<Packet *>(msg);
        processArpPacket(packet);
    }
}

void Arp::handleStartOperation(LifecycleOperation *operation)
{
    ASSERT(arpCache.empty());
}

void Arp::handleStopOperation(LifecycleOperation *operation)
{
    flush();
}

void Arp::handleCrashOperation(LifecycleOperation *operation)
{
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

void Arp::refreshDisplay() const
{
    OperationalBase::refreshDisplay();

    std::stringstream os;

    os << "size:" << arpCache.size() << " sent:" << numRequestsSent << "\n"
       << "repl:" << numRepliesSent << " fail:" << numFailedResolutions;

    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void Arp::initiateArpResolution(Ipv4Address nextHopAddr, ArpCacheEntry *entry)
{
    entry->pending = true;
    entry->numRetries = 0;
    entry->lastUpdate = SIMTIME_ZERO;
    entry->macAddress = MacAddress::UNSPECIFIED_ADDRESS;
    entry->ipv4Address = nextHopAddr;
    sendArpRequest(entry->ie, nextHopAddr);

    // start timer
    ASSERT(entry->timer == nullptr);
    entry->timer = new cMessage("ARP timeout");
    entry->timer->setContextPointer(entry);
    scheduleAfter(retryTimeout, entry->timer);

    numResolutions++;
    Notification signal(nextHopAddr, MacAddress::UNSPECIFIED_ADDRESS, entry->ie);
    emit(arpResolutionInitiatedSignal, &signal);
}

void Arp::sendArpRequest(const NetworkInterface *ie, Ipv4Address ipAddress)
{
    // find our own IPv4 address and MAC address on the given interface
    MacAddress myMACAddress = ie->getMacAddress();
    Ipv4Address myIPAddress = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

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

    auto macAddrReq = packet->addTag<MacAddressReq>();
    macAddrReq->setSrcAddress(myMACAddress);
    macAddrReq->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    packet->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    if (ie->getProtocol() != nullptr)
        packet->addTag<DispatchProtocolReq>()->setProtocol(ie->getProtocol());
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
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
        Ipv4Address nextHopAddr = entry->ipv4Address;
        EV_INFO << "ARP request for " << nextHopAddr << " timed out, resending\n";
        sendArpRequest(entry->ie, nextHopAddr);
        scheduleAfter(retryTimeout, selfmsg);
        return;
    }

    delete selfmsg;

    // max retry count reached: ARP failure.
    // throw out entry from cache
    EV << "ARP timeout, max retry count " << retryCount << " for " << entry->ipv4Address << " reached.\n";
    Notification signal(entry->ipv4Address, MacAddress::UNSPECIFIED_ADDRESS, entry->ie);
    emit(arpResolutionFailedSignal, &signal);
    arpCache.erase(entry->ipv4Address);
    delete entry;
    numFailedResolutions++;
}

bool Arp::addressRecognized(Ipv4Address destAddr, NetworkInterface *ie)
{
    if (rt->isLocalAddress(destAddr))
        return true;
    else {
        // if proxy ARP is enables in interface ie
        if (proxyArpInterfacesMatcher.matches(ie->getInterfaceName())) {
            // if we can route this packet, and the output port is
            // different from this one, then say yes
            NetworkInterface *rtie = rt->getInterfaceForDestAddr(destAddr);
            return rtie != nullptr && rtie != ie;
        }
        else
            return false;
    }
}

void Arp::dumpArpPacket(const ArpPacket *arp)
{
    EV_DETAIL << (arp->getOpcode() == ARP_REQUEST ? "ARP_REQ" : arp->getOpcode() == ARP_REPLY ? "ARP_REPLY" : "unknown type")
              << "  src=" << arp->getSrcIpAddress() << " / " << arp->getSrcMacAddress()
              << "  dest=" << arp->getDestIpAddress() << " / " << arp->getDestMacAddress() << "\n";
}

void Arp::processArpPacket(Packet *packet)
{
    EV_INFO << "Received " << packet << " from network protocol.\n";
    const auto& arp = packet->peekAtFront<ArpPacket>();
    dumpArpPacket(arp.get());

    // extract input port
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());

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

    MacAddress srcMacAddress = arp->getSrcMacAddress();
    Ipv4Address srcIpAddress = arp->getSrcIpAddress();

    if (srcMacAddress.isUnspecified())
        throw cRuntimeError("wrong ARP packet: source MAC address is empty");
    if (srcIpAddress.isUnspecified())
        throw cRuntimeError("wrong ARP packet: source IPv4 address is empty");

    bool mergeFlag = false;
    // "If ... sender protocol address is already in my translation table"
    auto it = arpCache.find(srcIpAddress);
    if (it != arpCache.end()) {
        // "update the sender hardware address field"
        ArpCacheEntry *entry = it->second;
        updateArpCache(entry, srcMacAddress);
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
                arpCache.insert(arpCache.begin(), std::make_pair(srcIpAddress, entry));
                entry->ipv4Address = srcIpAddress;
                entry->ie = ie;

                entry->pending = false;
                entry->timer = nullptr;
                entry->numRetries = 0;
            }
            updateArpCache(entry, srcMacAddress);
        }

        // "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
        switch (arp->getOpcode()) {
            case ARP_REQUEST: {
                EV_DETAIL << "Packet was ARP REQUEST, sending REPLY\n";
                MacAddress myMACAddress = resolveMacAddressForArpReply(ie, arp.get());
                if (myMACAddress.isUnspecified()) {
                    delete packet;
                    return;
                }

                Ipv4Address myIPAddress = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

                // "Swap hardware and protocol fields", etc.
                const auto& arpReply = makeShared<ArpPacket>();
                Ipv4Address origDestAddress = arp->getDestIpAddress();
                arpReply->setDestIpAddress(srcIpAddress);
                arpReply->setDestMacAddress(srcMacAddress);
                arpReply->setSrcIpAddress(origDestAddress);
                arpReply->setSrcMacAddress(myMACAddress);
                arpReply->setOpcode(ARP_REPLY);
                Packet *outPk = new Packet("arpREPLY");
                outPk->insertAtFront(arpReply);
                auto macAddressReq = outPk->addTag<MacAddressReq>();
                macAddressReq->setSrcAddress(myMACAddress);
                macAddressReq->setDestAddress(srcMacAddress);
                outPk->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
                if (ie->getProtocol() != nullptr)
                    outPk->addTag<DispatchProtocolReq>()->setProtocol(ie->getProtocol());
                else
                    outPk->removeTagIfPresent<DispatchProtocolReq>();
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

MacAddress Arp::resolveMacAddressForArpReply(const NetworkInterface *ie, const ArpPacket *arp)
{
    return ie->getMacAddress();
}

void Arp::updateArpCache(ArpCacheEntry *entry, const MacAddress& macAddress)
{
    EV_DETAIL << "Updating ARP cache entry: " << entry->ipv4Address << " <--> " << macAddress << "\n";

    // update entry
    if (entry->pending) {
        entry->pending = false;
        cancelAndDelete(entry->timer);
        entry->timer = nullptr;
        entry->numRetries = 0;
    }
    entry->macAddress = macAddress;
    entry->lastUpdate = simTime();
    Notification signal(entry->ipv4Address, macAddress, entry->ie);
    emit(arpResolutionCompletedSignal, &signal);
}

MacAddress Arp::resolveL3Address(const L3Address& address, const NetworkInterface *ie)
{
    Enter_Method("resolveMACAddress(%s,%s)", address.str().c_str(), ie->getInterfaceName());

    Ipv4Address addr = address.toIpv4();
    auto it = arpCache.find(addr);
    if (it == arpCache.end()) {
        // no cache entry: launch ARP request
        ArpCacheEntry *entry = new ArpCacheEntry();
        entry->owner = this;
        arpCache.insert(arpCache.begin(), std::make_pair(addr, entry));
        entry->ipv4Address = addr;
        entry->ie = ie;

        EV << "Starting ARP resolution for " << addr << "\n";
        initiateArpResolution(addr, entry);
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
        entry->ie = ie; // routing table may have changed
        initiateArpResolution(addr, entry);
    }
    return MacAddress::UNSPECIFIED_ADDRESS;
}

L3Address Arp::getL3AddressFor(const MacAddress& macAddr) const
{
    Enter_Method("getL3AddressFor");

    if (macAddr.isUnspecified())
        return Ipv4Address::UNSPECIFIED_ADDRESS;

    simtime_t now = simTime();
    for (const auto& elem : arpCache)
        if (elem.second->macAddress == macAddr && elem.second->lastUpdate + cacheTimeout >= now)
            return elem.first;

    return Ipv4Address::UNSPECIFIED_ADDRESS;
}

// Also known as ARP Announcement
void Arp::sendArpGratuitous(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address ipAddr, ArpOpcode opCode)
{
    Enter_Method("sendArpGratuitous");

    // both must be set
    ASSERT(!srcAddr.isUnspecified());
    ASSERT(!ipAddr.isUnspecified());

    // fill out everything in ARP Request packet except dest MAC address
    Packet *packet = new Packet("arpGrt");
    const auto& arp = makeShared<ArpPacket>();
    arp->setOpcode(opCode);
    arp->setSrcMacAddress(srcAddr);
    arp->setSrcIpAddress(ipAddr);
    arp->setDestIpAddress(ipAddr);
    arp->setDestMacAddress(MacAddress::BROADCAST_ADDRESS);
    packet->insertAtFront(arp);

    auto macAddressReq = packet->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(srcAddr);
    macAddressReq->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    packet->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    if (ie->getProtocol() != nullptr)
        packet->addTag<DispatchProtocolReq>()->setProtocol(ie->getProtocol());
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::arp);

    ArpCacheEntry *entry = new ArpCacheEntry();
    arpCache.insert(arpCache.begin(), std::make_pair(ipAddr, entry));
    entry->ipv4Address = ipAddr;
    entry->ie = ie;

    entry->pending = false;
    entry->timer = nullptr;
    entry->numRetries = 0;

    entry->lastUpdate = simTime();
    // updateARPCache(entry, srcAddr); //FIXME

    // send out
    send(packet, "ifOut");
}

// A client should send out 'ARP Probe' to probe the newly received IPv4 address.
// Refer to RFC 5227, IPv4 Address Conflict Detection
void Arp::sendArpProbe(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address probedAddr)
{
    Enter_Method("sendArpProbe");

    // both must be set
    ASSERT(!srcAddr.isUnspecified());
    ASSERT(!probedAddr.isUnspecified());

    Packet *packet = new Packet("arpProbe");
    const auto& arp = makeShared<ArpPacket>();
    arp->setOpcode(ARP_REQUEST);
    arp->setSrcMacAddress(srcAddr);
    arp->setSrcIpAddress(Ipv4Address::UNSPECIFIED_ADDRESS);
    arp->setDestIpAddress(probedAddr);
    arp->setDestMacAddress(MacAddress::UNSPECIFIED_ADDRESS);
    packet->insertAtFront(arp);

    auto macAddressReq = packet->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(srcAddr);
    macAddressReq->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    packet->addTag<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    if (ie->getProtocol() != nullptr)
        packet->addTag<DispatchProtocolReq>()->setProtocol(ie->getProtocol());
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::arp);

    // send out
    send(packet, "ifOut");
}

} // namespace inet

