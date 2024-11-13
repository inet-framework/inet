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
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

#ifdef INET_WITH_xMIPv6
#include "inet/networklayer/xmipv6/MobilityHeader_m.h" // for HA Option header
#include "inet/networklayer/xmipv6/xMIPv6.h"
#endif // ifdef INET_WITH_xMIPv6

namespace inet {

Define_Module(Ipv6Tunneling);

Ipv6Tunneling::Ipv6Tunneling()
{
}

void Ipv6Tunneling::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        vIfIndexTop = INT_MAX; // virtual interface number set to maximum int value
        noOfNonSplitTunnels = 0; // current number of non-split tunnels on this host

        WATCH_MAP(tunnels);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void Ipv6Tunneling::handleMessage(cMessage *msg)
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
    tunnels[vIfIndexTop] = Tunnel(entry, exit, destTrigger);

    if (tunnelType == NORMAL || tunnelType == SPLIT || tunnelType == NON_SPLIT) {
        if (destTrigger == Ipv6Address::UNSPECIFIED_ADDRESS) {
            // this is a "full" tunnel over which everything gets routed
            tunnels[vIfIndexTop].tunnelType = NON_SPLIT;
            noOfNonSplitTunnels++;
        }

        // default values: 5.
        // 6.4
        tunnels[vIfIndexTop].trafficClass = 0;

        // 6.5
        tunnels[vIfIndexTop].flowLabel = 0;

        // 6.3
        // The tunnel hop limit default value for hosts is the Ipv6 Neighbor
        // Discovery advertised hop limit [ND-Spec].
        if (rt->isRouter())
            tunnels[vIfIndexTop].hopLimit = IPv6__INET_DEFAULT_ROUTER_HOPLIMIT;
        else
            tunnels[vIfIndexTop].hopLimit = 255;

        // 6.7
        // TODO perform path MTU on link (interface resolved via exit address)
        tunnels[vIfIndexTop].tunnelMTU = IPv6_MIN_MTU - 40;

        EV_INFO << "Tunneling: Created tunnel with entry=" << entry << ", exit=" << exit
                << " and trigger=" << destTrigger << endl;
    }
    else if (tunnelType == T2RH || tunnelType == HA_OPT) {
        tunnels[vIfIndexTop].tunnelType = tunnelType;

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

    return vIfIndexTop--; // decrement vIfIndex for use with next createTunnel() call
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

#ifdef INET_WITH_xMIPv6
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

        if (tunnels[vIfIndex].tunnelType == T2RH) { // update 15.01.08 - CB
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
            /*For a type 2 routing header, the Hdr Ext Len MUST be 2.  The Segments
               Left value describes the number of route segments remaining; i.e.,
               number of explicitly listed intermediate nodes still to be visited
               before reaching the final destination.  Segments Left MUST be 1.*/
            Ipv6RoutingHeader *t2RH = new Ipv6RoutingHeader();
            t2RH->setRoutingType(2);
            t2RH->setSegmentsLeft(1);
            t2RH->setAddressArraySize(1);
            t2RH->setByteLength(B(8 + 1 * 16));
            // old src becomes address of T2RH
            t2RH->setAddress(0, rh2);

            // append T2RH to routing headers
            packet->addTagIfAbsent<Ipv6ExtHeaderReq>()->appendExtensionHeader(t2RH);

            EV_INFO << "Added Type 2 Routing Header." << endl;
        }
        else { // HA_OPT
               /*The Home Address option is carried by the Destination Option
                  extension header (Next Header value = 60).  It is used in a packet
                  sent by a mobile node while away from home, to inform the recipient
                  of the mobile node's home address.*/
            HomeAddressOption *haOpt = new HomeAddressOption();
            haOpt->setHomeAddress(rh2);

            // append HA option to routing headers
            packet->addTagIfAbsent<Ipv6ExtHeaderReq>()->appendExtensionHeader(haOpt);

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
#endif // INET_WITH_xMIPv6
    // normal tunnel - just modify controlInfo and send
    // datagram back to Ipv6 module for encapsulation

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
    auto addresses = packet->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(tunnels[vIfIndex].entry);
    addresses->setDestAddress(tunnels[vIfIndex].exit);

    send(packet, "upperLayerOut");
#ifdef INET_WITH_xMIPv6
}

#endif // ifdef INET_WITH_xMIPv6
}

void Ipv6Tunneling::decapsulateDatagram(Packet *packet)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // decapsulation is performed in Ipv6 module
    Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();

#ifdef INET_WITH_xMIPv6
    // we only decapsulate packets for which we have a tunnel
    // where the exit point is equal to the packets source
    // 11.9.07 - CB
    if (rt->isHomeAgent() && !isTunnelExit(srcAddr)) {
        /*RFC 3775, 10.4.5
           Otherwise, when a home agent decapsulates a tunneled packet from
           the mobile node, the home agent MUST verify that the Source
           Address in the tunnel IP header is the mobile node's primary
           care-of address.  Otherwise, any node in the Internet could send
           traffic through the home agent and escape ingress filtering
           limitations.  This simple check forces the attacker to know the
           current location of the real mobile node and be able to defeat
           ingress filtering. This check is not necessary if the reverse-
           tunneled packet is protected by ESP in tunnel mode.*/
        EV_INFO << "Dropping packet: source address of tunnel IP header different from tunnel exit points!" << endl;
        delete packet;
        return;
    }
#endif // ifdef INET_WITH_xMIPv6

    // FIX: we leave the interface Id to it's previous value to make sure
    // that later processing knowns from which interface the datagram came from
    // (important if several interfaces are available)
//    controlInfo->setInterfaceId(-1);

    send(packet, "linkLayerOut");

#ifdef INET_WITH_xMIPv6
    // Alain Tigyo, 21.03.2008
    // The following code is used for triggering RO to a CN
    NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    if (rt->isMobileNode() && (srcAddr == ie->getProtocolData<Ipv6InterfaceData>()->getHomeAgentAddress())
        && (ipv6Header->getProtocolId() != IP_PROT_IPv6EXT_MOB))
    {
        EV_INFO << "Checking Route Optimization for: " << ipv6Header->getSrcAddress() << endl;
        xMIPv6 *mipv6 = findModuleFromPar<xMIPv6>(par("xmipv6Module"), this);
        if (mipv6)
            mipv6->triggerRouteOptimization(ipv6Header->getSrcAddress(), ie->getProtocolData<Ipv6InterfaceData>()->getMNHomeAddress(), ie);
    }
#endif // ifdef INET_WITH_xMIPv6
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

} // namespace inet

