//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

//
// Implementation of RFC 2473
// Generic Packet Tunneling in Ipv6
//
// This module implements the basic tunneling functionality.
// Packet flow is modeled as in the RFC: each two input and output
// gates are used to exchange packets with the Ipv6 core.
//
// Not implemented:
//  - 4.1.1
//  - 5.1
//  - 7.
//
// TODO
//  - 8.: Tunnel Error Reporting and Processing

#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"

#include <algorithm>
#include <functional>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Mipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

#include "inet/networklayer/xmipv6/MobilityHeader_m.h" // for HA Option header
#include "inet/networklayer/xmipv6/xMIPv6.h"

namespace inet {

Define_Module(Ipv6Tunneling);

Ipv6Tunneling::Ipv6Tunneling()
{
}

void Ipv6Tunneling::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        vIfIndexTop = INT_MAX; // virtual interface number set to maximum int value
        noOfNonSplitTunnels = 0; // current number of non-split tunnels on this host

        WATCH(vIfIndexTop);
        WATCH(noOfNonSplitTunnels);
        WATCH(tunnels);
    }
}

void Ipv6Tunneling::handleMessageWhenUp(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);

    if (packet->getArrivalGate()->isName("upperLayerIn")) {
        // decapsulate
        decapsulateDatagram(packet);
    }
    else if (packet->getArrivalGate()->isName("linkLayerIn")) {
        // encapsulate
        encapsulateDatagram(packet);
    }
    else
        throw cRuntimeError("Ipv6Tunneling: Unknown gate: %s!", packet->getArrivalGate()->getFullName());
}

Ipv6Tunneling::Tunnel::Tunnel(const Ipv6Address& _entry, const Ipv6Address& _exit, const Ipv6Address& _destTrigger)
    : entry(_entry), exit(_exit), tunnelMTU(IPv6_MIN_MTU - 40), tunnelType(SPLIT), destTrigger(_destTrigger)
{
}

int Ipv6Tunneling::createTunnel(TunnelType tunnelType,
        const Ipv6Address& entry, const Ipv6Address& exit, const Ipv6Address& destTrigger)
{
    ASSERT(entry != Ipv6Address::UNSPECIFIED_ADDRESS);
    ASSERT(exit != Ipv6Address::UNSPECIFIED_ADDRESS);

    // Test for entry and exit point node pointing to same node i.e. localDeliver
    // addresses to prevent loopback encapsulation 4.1.2
    if ((tunnelType == NORMAL || tunnelType == SPLIT || tunnelType == NON_SPLIT) // check does not work for T2RH or HoA_Opt. Why?
        && rt->isLocalAddress(entry) && rt->isLocalAddress(exit))
    {
        EV_INFO << "Cannot create tunnel with local endpoints (prevents loopback tunneling)" << endl;
        return 0;
    }

    int search = findTunnel(entry, exit, destTrigger);
    if (search != 0) {
        EV_INFO << "Tunnel with entry = " << entry << ", exit = " << exit << " and trigger = "
                << destTrigger << " already exists!" << endl;
        return search;
    }

    if (vIfIndexTop <= (ift->getBiggestInterfaceId()))
        throw cRuntimeError("Error: Not more than %d tunnels supported!", INT_MAX - ift->getBiggestInterfaceId());

    if ((destTrigger == Ipv6Address::UNSPECIFIED_ADDRESS) && (noOfNonSplitTunnels == 1))
        throw cRuntimeError("Error: Not more than 1 non-split tunnel supported!");

    // 6.1-6.2
    ASSERT(entry.isUnicast());

    // "Real" tunnels (NORMAL/SPLIT/NON_SPLIT) are backed by a dynamically created
    // Ipv6TunnelInterface: the IPv6 layer routes to it like any interface and the
    // interface performs the encapsulation. The pseudo "tunnels" (T2RH, HA_OPT)
    // still use a virtual interface index (vIfIndexTop) and the legacy path.
    bool useNetworkInterface = (tunnelType == NORMAL || tunnelType == SPLIT || tunnelType == NON_SPLIT);
    NetworkInterface *networkInterface = useNetworkInterface ? createTunnelNetworkInterface(entry, exit) : nullptr;
    int key = useNetworkInterface ? networkInterface->getInterfaceId() : vIfIndexTop;

    tunnels[key] = Tunnel(entry, exit, destTrigger);
    tunnels[key].networkInterface = networkInterface;

    if (useNetworkInterface) {
        if (destTrigger == Ipv6Address::UNSPECIFIED_ADDRESS) {
            // this is a "full" tunnel over which everything gets routed
            tunnels[key].tunnelType = NON_SPLIT;
            noOfNonSplitTunnels++;
        }

        // default values: 5.
        // 6.4
        tunnels[key].trafficClass = 0;

        // 6.5
        tunnels[key].flowLabel = 0;

        // 6.3
        // The tunnel hop limit default value for hosts is the Ipv6 Neighbor
        // Discovery advertised hop limit [ND-Spec].
        if (rt->isRouter())
            tunnels[key].hopLimit = IPv6__INET_DEFAULT_ROUTER_HOPLIMIT;
        else
            tunnels[key].hopLimit = 255;

        // 6.7
        // TODO perform path MTU on link (interface resolved via exit address)
        tunnels[key].tunnelMTU = IPv6_MIN_MTU - 40;

        EV_INFO << "Tunneling: Created tunnel with entry=" << entry << ", exit=" << exit
                << " and trigger=" << destTrigger << endl;
    }
    else if (tunnelType == T2RH || tunnelType == HA_OPT) {
        tunnels[key].tunnelType = tunnelType;

        if (tunnelType == T2RH)
            EV_INFO << "Tunneling: Created RH2 path with entry=" << entry << ", exit=" << exit
                    << " and trigger=" << destTrigger << endl;
        else
            EV_INFO << "Tunneling: Created HA option path with entry=" << entry << ", exit=" << exit
                    << " and trigger=" << destTrigger << endl;
    }
    else {
        throw cRuntimeError("tunnel type %d not supported for createTunnel()", tunnelType);
    }

    if (hasGUI())
        bubble("Created Tunnel");

    // real tunnels are keyed by their interface id; pseudo tunnels consume a vIfIndex
    return useNetworkInterface ? key : vIfIndexTop--;
}

int Ipv6Tunneling::findTunnel(const Ipv6Address& src, const Ipv6Address& dest,
        const Ipv6Address& destTrigger) const
{
    Tunnel t0(src, dest, destTrigger);
    for (const auto& tun: tunnels) {
        if (tun.second == t0)
            return tun.first;
    }
    return 0;
}

bool Ipv6Tunneling::destroyTunnel(const Ipv6Address& src, const Ipv6Address& dest,
        const Ipv6Address& destTrigger)
{
    EV_INFO << "Destroy tunnel entry =" << src << ", exit = " << dest
            << ", destTrigger = " << destTrigger << "\n";

    // search for tunnel with given entry and exit point as well as trigger
    int vIfIndex = findTunnel(src, dest, destTrigger);

    if (vIfIndex == 0) {
        EV_WARN << "Tunnel not found\n";
        return false;
    }

    // if we delete a non-split tunnel, then we can
    // also decrement the appropriate counter
    if (tunnels[vIfIndex].tunnelType == NON_SPLIT)
        noOfNonSplitTunnels--;

    // tear down the backing virtual interface for real tunnels
    if (tunnels[vIfIndex].networkInterface)
        deleteTunnelNetworkInterface(tunnels[vIfIndex].networkInterface);

    // TODO store vIfIndex for later reuse when creating a new tunnel
    tunnels.erase(vIfIndex);

    // reset the index if we do not have a single tunnel anymore
    resetVIfIndex();

    if (hasGUI())
        bubble("Destroyed Tunnel");

    return true;
}

void Ipv6Tunneling::destroyTunnel(const Ipv6Address& entry, const Ipv6Address& exit)
{
    for (auto it = tunnels.begin(); it != tunnels.end();) {
        if (it->second.entry == entry && it->second.exit == exit) {
            destroyTunnel(it->second.entry, it->second.exit, it->second.destTrigger);
            break;
        }
        else
            ++it;
    }

    // reset the index if we do not have a single tunnel anymore
    resetVIfIndex();
}

void Ipv6Tunneling::destroyTunnelForExitAndTrigger(const Ipv6Address& exit, const Ipv6Address& trigger)
{
    for (auto it = tunnels.begin(); it != tunnels.end();) {
        if (it->second.exit == exit && it->second.destTrigger == trigger) {
            destroyTunnel(it->second.entry, it->second.exit, it->second.destTrigger);
            break;
        }
        else
            ++it;
    }

    // reset the index if we do not have a single tunnel anymore
    resetVIfIndex();
}

void Ipv6Tunneling::destroyTunnelForEntryAndTrigger(const Ipv6Address& entry, const Ipv6Address& trigger)
{
    for (auto it = tunnels.begin(); it != tunnels.end();) {
        if (it->second.entry == entry && it->second.destTrigger == trigger) {
            destroyTunnel(it->second.entry, it->second.exit, it->second.destTrigger);
            break;
        }
        else
            ++it;
    }

    // reset the index if we do not have a single tunnel anymore
    resetVIfIndex();
}

void Ipv6Tunneling::destroyTunnels(const Ipv6Address& entry)
{
    for (auto it = tunnels.begin(); it != tunnels.end();) {
        if (it->second.entry == entry) {
            auto oldIt = it;
            ++it;

            destroyTunnel(oldIt->second.entry, oldIt->second.exit, oldIt->second.destTrigger);
        }
        else
            ++it;
    }

    // reset the index if we do not have a single tunnel anymore
    resetVIfIndex();
}

void Ipv6Tunneling::destroyTunnelFromTrigger(const Ipv6Address& trigger)
{
    for (auto& elem : tunnels) {
        if (elem.second.destTrigger == trigger) {
            destroyTunnel(elem.second.entry, elem.second.exit, elem.second.destTrigger);

            // reset the index if we do not have a single tunnel anymore
            resetVIfIndex();

            return; // there can not be more than one tunnel for a trigger
        }
    }
}

int Ipv6Tunneling::getVIfIndexForDest(const Ipv6Address& destAddress)
{
//    Enter_Method("Looking up Tunnels (%s)", destAddress.str().c_str());
    EV_INFO << "Looking up tunnels...";

    // first we look for tunnels with destAddress as trigger
    int vIfIndex = lookupTunnels(destAddress);

    if (vIfIndex == -1) {
        // then the only chance left for finding a suitable tunnel
        // is to find a non-split tunnel
        vIfIndex = doPrefixMatch(destAddress);
    }

    EV_DETAIL << "found vIf=" << vIfIndex << endl;

    return vIfIndex;
}

int Ipv6Tunneling::getVIfIndexForDest(const Ipv6Address& destAddress, TunnelType tunnelType)
{
    int outInterfaceId = -1;

    for (auto& elem : tunnels) {
        if (tunnelType == NORMAL || tunnelType == NON_SPLIT || tunnelType == SPLIT) {
            // we search here for tunnels which have a destination trigger and
            // check whether the trigger is equal to the destination
            // only "normal" tunnels, both split and non-split, are possible entry points
            if ((elem.second.tunnelType == NON_SPLIT) ||
                (elem.second.tunnelType == SPLIT && elem.second.destTrigger == destAddress))
            {
                outInterfaceId = elem.first;
                break;
            }
        }
        else if (tunnelType == MOBILITY || tunnelType == HA_OPT || tunnelType == T2RH) {
            if (elem.second.tunnelType != NON_SPLIT &&
                elem.second.tunnelType != SPLIT &&
                elem.second.destTrigger == destAddress)
            {
                outInterfaceId = elem.first;
                break;
            }
        }
    }

    return outInterfaceId;
}

void Ipv6Tunneling::encapsulateDatagram(Packet *packet)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    int vIfIndex = -1;

    if (ipv6Header->getProtocolId() == IP_PROT_IPv6EXT_MOB) {
        // only look at non-split tunnel
        // (HoTI is only sent over HA tunnel)
        vIfIndex = doPrefixMatch(ipv6Header->getDestAddress());
    }

    if ((ipv6Header->getProtocolId() != IP_PROT_IPv6EXT_MOB) || (vIfIndex == -1)) {
        // look up all tunnels for dgram's destination
        vIfIndex = getVIfIndexForDest(ipv6Header->getDestAddress());
//        EV << "looked up again!" << endl;
    }

    if (vIfIndex == -1)
        throw cRuntimeError("encapsulateDatagram(): tunnel not existent");

    // TODO copy information from old ctrlInfo into new one (Traffic Class, Flow label, etc.)
    delete packet->removeControlInfo();

    if ((tunnels[vIfIndex].tunnelType == T2RH) || (tunnels[vIfIndex].tunnelType == HA_OPT)) {
        // pseudo-tunnel for Type 2 Routing Header
        // or Home Address Option

        Ipv6Address src = ipv6Header->getSrcAddress();
        Ipv6Address dest = ipv6Header->getDestAddress();
        Ipv6Address rh2; // dest

        if (src.isUnspecified()) {
            // if we do not have a valid source address, we'll have to ask
            // the routing table for the correct interface.
            int interfaceId;
            (void)(rt->lookupDestCache(dest, interfaceId));
        }

        if (tunnels[vIfIndex].tunnelType == T2RH) {
            // this is the CN -> MN path
            src = tunnels[vIfIndex].entry; // CN address
            dest = tunnels[vIfIndex].exit; // CoA
            rh2 = tunnels[vIfIndex].destTrigger; // HoA
        }
        else {
            // path MN -> CN
            src = tunnels[vIfIndex].entry; // CoA
            dest = tunnels[vIfIndex].destTrigger; // CN address
            rh2 = tunnels[vIfIndex].exit; // HoA
        }

        // get rid of the encapsulation of the Ipv6 module
        packet->popAtFront<Ipv6Header>();
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(ipv6Header->getProtocol());

        if (tunnels[vIfIndex].tunnelType == T2RH) {
            // construct Type 2 Routing Header (RFC 3775 - 6.4.1)
            Ipv6RoutingHeader *t2RH = new Ipv6RoutingHeader();
            t2RH->setRoutingType(2);
            t2RH->setSegmentsLeft(1);
            t2RH->setAddressArraySize(1);
            t2RH->setChunkLength(B(8 + 1 * 16));
            // old src becomes address of T2RH
            t2RH->setAddress(0, rh2);

            // append T2RH to routing headers
            packet->addTagIfAbsent<Ipv6ExtHeaderReq>()->appendExtensionHeader(t2RH);

            EV_INFO << "Added Type 2 Routing Header." << endl;
        }
        else { // HA_OPT
            auto *destOptsHdr = new Ipv6DestinationOptionsHeader();
            auto *haOpt = new HomeAddressOption();
            haOpt->setHomeAddress(rh2);
            destOptsHdr->getTlvOptionsForUpdate().appendTlvOption(haOpt);

            // append Destination Options header containing HA option
            packet->addTagIfAbsent<Ipv6ExtHeaderReq>()->appendExtensionHeader(destOptsHdr);

            EV_INFO << "Added Home Address Option header." << endl;
        }

        auto addresses = packet->addTagIfAbsent<L3AddressReq>();
        // new src is tunnel entry (either CoA or CN)
        addresses->setSrcAddress(src);
        // copy old dest addr
        addresses->setDestAddress(dest);

        send(packet, "upperLayerOut");
    }
    else {
        // normal tunnel - just modify controlInfo and send
        // datagram back to Ipv6 module for encapsulation

        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
        auto addresses = packet->addTagIfAbsent<L3AddressReq>();
        addresses->setSrcAddress(tunnels[vIfIndex].entry);
        addresses->setDestAddress(tunnels[vIfIndex].exit);

        send(packet, "upperLayerOut");
    }
}

void Ipv6Tunneling::decapsulateDatagram(Packet *packet)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // decapsulation is performed in Ipv6 module
    Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();

    // RFC 3775, 10.4.5: HA must verify tunnel source matches a known tunnel exit
    if (rt->isHomeAgent() && !isTunnelExit(srcAddr)) {
        EV_INFO << "Dropping packet: source address of tunnel IP header different from tunnel exit points!" << endl;
        delete packet;
        return;
    }

    // FIX: we leave the interface Id to it's previous value to make sure
    // that later processing knowns from which interface the datagram came from
    // (important if several interfaces are available)
//    controlInfo->setInterfaceId(-1);

    send(packet, "linkLayerOut");

    // trigger Route Optimization if we are a MN receiving tunneled data from HA
    if (rt->isMobileNode()) {
        NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
        if ((srcAddr == ie->getProtocolData<Mipv6InterfaceData>()->getHomeAgentAddress())
            && (ipv6Header->getProtocolId() != IP_PROT_IPv6EXT_MOB))
        {
            EV_INFO << "Checking Route Optimization for: " << ipv6Header->getSrcAddress() << endl;
            xMIPv6 *mipv6 = findModuleFromPar<xMIPv6>(par("xmipv6Module"), this);
            if (mipv6)
                mipv6->triggerRouteOptimization(ipv6Header->getSrcAddress(), ie->getProtocolData<Mipv6InterfaceData>()->getMNHomeAddress(), ie);
        }
    }
}

int Ipv6Tunneling::lookupTunnels(const Ipv6Address& dest)
{
    int outInterfaceId = -1;

    // we search here for tunnels which have a destination trigger and
    // check whether the trigger is equal to the destination
    // only split tunnels or mobility paths are possible entry points
    for (auto& elem : tunnels) {
        if ((elem.second.tunnelType != NON_SPLIT) && (elem.second.destTrigger == dest)) {
            outInterfaceId = elem.first;
            break;
        }
    }

    return outInterfaceId;
}

int Ipv6Tunneling::doPrefixMatch(const Ipv6Address& dest)
{
    int outInterfaceId = -1;

    // we'll just stop at the first match, because it is assumed that not
    // more than a single non-split tunnel is possible
    for (auto& elem : tunnels) {
        if (elem.second.tunnelType == NON_SPLIT) {
            outInterfaceId = elem.first;
            break;
        }
    }

    return outInterfaceId;
}

bool Ipv6Tunneling::isTunnelExit(const Ipv6Address& exit)
{
    for (auto& elem : tunnels) {
        // mobility "tunnels" are not relevant for decapsulation
        // 17.10.07 - same for Home Address Option
        if (elem.second.tunnelType != T2RH && elem.second.tunnelType != HA_OPT
            && elem.second.exit == exit)
        {
            return true;
        }
    }

    return false;
}

NetworkInterface *Ipv6Tunneling::createTunnelNetworkInterface(const Ipv6Address& source, const Ipv6Address& destination)
{
    cModule *node = getContainingNode(this);
    cModuleType *moduleType = cModuleType::get("inet.networklayer.ipv6tunneling.Ipv6TunnelInterface");
    std::string name = std::string("ip6tun") + std::to_string(tunnelInterfaceCounter++);
    cModule *module = moduleType->create(name.c_str(), node);
    module->par("interfaceTableModule") = check_and_cast<cModule *>(ift.get())->getFullPath().c_str();
    module->par("source") = source.str().c_str();
    module->par("destination") = destination.str().c_str();
    module->finalizeParameters();
    module->buildInside();

    // wire it into the node's link-layer dispatcher exactly as LinkLayerNodeBase
    // wires its static tun[] slot: tun.upperLayerOut --> li.in++, li.out++ --> tun.upperLayerIn
    cModule *li = node->getSubmodule("li");
    cGate *liOut = li->getOrCreateFirstUnconnectedGate("out", 0, false, true);
    cGate *liIn = li->getOrCreateFirstUnconnectedGate("in", 0, false, true);
    liOut->connectTo(module->gate("upperLayerIn"));
    module->gate("upperLayerOut")->connectTo(liIn);

    module->callInitialize();
    return check_and_cast<NetworkInterface *>(module);
}

void Ipv6Tunneling::deleteTunnelNetworkInterface(NetworkInterface *networkInterface)
{
    // InterfaceTable::deleteInterface() removes the interface from the table and
    // deletes the module (which disconnects its gates), so nothing else is needed.
    ift->deleteInterface(networkInterface);
}

/*
   TunnelType Ipv6Tunneling::getTunnelType(const int vIfIndex)
   {
    auto i = tunnels.find(vIfIndex);

    if (i == tunnels.end())
        return 0;
    else
        return i->second.tunnelType;
   }
 */

std::ostream& operator<<(std::ostream& os, const Ipv6Tunneling::Tunnel& tun)
{
    os << "tunnel entry = " << tun.entry << ", exit = " << tun.exit << ", tunnelMTU = "
       << tun.tunnelMTU << ", dest = " << tun.destTrigger << ", type: ";

    switch (tun.tunnelType) {
        case Ipv6Tunneling::SPLIT:
            os << "split tunnel";
            break;

        case Ipv6Tunneling::NON_SPLIT:
            os << "non-split tunnel";
            break;

        case Ipv6Tunneling::T2RH:
            os << "T2RH path";
            break;

        case Ipv6Tunneling::HA_OPT:
            os << "Home Address Option path";
            break;

        default:
            throw cRuntimeError("Not a valid type for an existing tunnel!");
            break;
    }

    os << endl;

    return os;
}

// lifecycle management

void Ipv6Tunneling::handleStopOperation(LifecycleOperation *operation)
{
    tunnels.clear();
    vIfIndexTop = INT_MAX;
    noOfNonSplitTunnels = 0;
}

void Ipv6Tunneling::handleCrashOperation(LifecycleOperation *operation)
{
    tunnels.clear();
    vIfIndexTop = INT_MAX;
    noOfNonSplitTunnels = 0;
}

} // namespace inet

