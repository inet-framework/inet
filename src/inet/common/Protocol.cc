//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/Protocol.h"

namespace inet {

std::map<int, const Protocol *>& Protocol::getIdToProtocol()
{
    static std::map<int, const Protocol *> idToProtocol;
    return idToProtocol;
}

std::map<std::string, const Protocol *>& Protocol::getNameToProtocol()
{
    static std::map<std::string, const Protocol *> nameToProtocol;
    return nameToProtocol;
}

Protocol::Protocol(const char *name, const char *descriptiveName, Layer layer) :
    id(getNextId()),
    name(name),
    descriptiveName(descriptiveName),
    layer(layer)
{
    getIdToProtocol()[id] = this;
    getNameToProtocol()[name] = this;
    if (strchr(name, ' ') != nullptr)
        throw cRuntimeError("Space is not allowed in protocol name");
}

std::string Protocol::str() const
{
    std::ostringstream os;
    os << getName() << "(" << id << ")";
    return os.str();
}

const Protocol *Protocol::findProtocol(int id)
{
    auto it = getIdToProtocol().find(id);
    return it != getIdToProtocol().end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(int id)
{
    const Protocol *protocol = findProtocol(id);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: id = %d", id);
}

const Protocol *Protocol::findProtocol(const char *name)
{
    auto it = getNameToProtocol().find(name);
    return it != getNameToProtocol().end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(const char *name)
{
    const Protocol *protocol = findProtocol(name);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: name = %s", name);
}

uint64_t Protocol::getNextId()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedCounterName("inet::Protocol::nextId");
    static uint64_t staticCounter;
    if (cSimulation::getActiveSimulation() == nullptr) {
        // for initializing static const Protocol instances (Protocol::aodv, etc.)
        return staticCounter++;
    }
    else {
        // for Protocol instances created during simulation
        uint64_t& nextId = getSimulationOrSharedDataManager()->getSharedCounter(handle, staticCounter);
        return nextId++;
    }
}

// Standard protocol identifiers
const Protocol Protocol::aodv("aodv", "AODV");
const Protocol Protocol::arp("arp", "ARP", Protocol::NetworkLayer);
const Protocol Protocol::babel("babel", "BABEL");
const Protocol Protocol::bgp("bgp", "BGP");
const Protocol Protocol::bmac("bmac", "B-MAC", Protocol::LinkLayer);
const Protocol Protocol::cdp("cdp", "CDP", Protocol::NetworkLayer);
const Protocol Protocol::clns("clns", "CLNS", Protocol::NetworkLayer);
const Protocol Protocol::dsdv2("dsdv2", "DSDV 2");
const Protocol Protocol::dsr("dsr", "DSR");
const Protocol Protocol::dymo("dymo", "DYMO");
const Protocol Protocol::egp("egp", "EGP");
const Protocol Protocol::ethernetFlowCtrl("ethernetflowctrl", "Ethernet Flow Control", Protocol::LinkLayer);
const Protocol Protocol::ethernetMac("ethernetmac", "Ethernet MAC", Protocol::LinkLayer);
const Protocol Protocol::ethernetPhy("ethernetphy", "Ethernet PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ftp("ftp", "FTP");
const Protocol Protocol::gpsr("gpsr", "GPSR");
const Protocol Protocol::http("http", "HTTP");
const Protocol Protocol::icmpv4("icmpv4", "ICMPv4", Protocol::NetworkLayer);
const Protocol Protocol::icmpv6("icmpv6", "ICMPv6", Protocol::NetworkLayer);
const Protocol Protocol::ieee80211DsssPhy("ieee80211dsssphy", "IEEE 802.11 DSSS PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211ErpOfdmPhy("ieee80211erpofdmphy", "IEEE 802.11 ERP OFDM PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211FhssPhy("ieee80211fhssphy", "IEEE 802.11 FHSS PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211HrDsssPhy("ieee80211hrdsssphy", "IEEE 802.11 HR/DSSS PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211HtPhy("ieee80211htphy", "IEEE 802.11 HT PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211IrPhy("ieee80211irphy", "IEEE 802.11 IR PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211Mac("ieee80211mac", "IEEE 802.11 MAC", Protocol::LinkLayer);
const Protocol Protocol::ieee80211Mgmt("ieee80211mgmt", "IEEE 802.11 MGMT", Protocol::LinkLayer);
const Protocol Protocol::ieee80211OfdmPhy("ieee80211ofdmphy", "IEEE 802.11 OFDM PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee80211VhtPhy("ieee80211vhtphy", "IEEE 802.11 VHT PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee802154("ieee802154", "IEEE 802.15.4");
const Protocol Protocol::ieee8021ae("ieee8021ae", "IEEE 802.1AE");
const Protocol Protocol::ieee8021qCTag("ieee8021qctag", "IEEE 802.1Q C-TAG");
const Protocol Protocol::ieee8021qSTag("ieee8021qstag", "IEEE 802.1Q S-TAG");
const Protocol Protocol::ieee8022llc("ieee8022llc", "IEEE 802.2 LLC", Protocol::LinkLayer);
const Protocol Protocol::ieee8022snap("ieee8022snap", "IEEE 802.2 SNAP", Protocol::LinkLayer);
const Protocol Protocol::ieee802epd("ieee802epd", "IEEE 802.11 EPD", Protocol::LinkLayer);
const Protocol Protocol::igmp("igmp", "IGMP", Protocol::NetworkLayer);
const Protocol Protocol::igp("igp", "IGP");
const Protocol Protocol::ipv4("ipv4", "IPv4", Protocol::NetworkLayer);
const Protocol Protocol::ipv6("ipv6", "IPv6", Protocol::NetworkLayer);
const Protocol Protocol::isis("isis", "IS-IS");
const Protocol Protocol::l2isis("l2isis", "L2 IS-IS");
const Protocol Protocol::lldp("lldp", "LLDP", Protocol::NetworkLayer);
const Protocol Protocol::lmac("lmac", "L-MAC");
const Protocol Protocol::manet("manet", "MANET");
const Protocol Protocol::mobileipv6("mobileipv6", "Mobile IPv6");
const Protocol Protocol::mpls("mpls", "MPLS");
const Protocol Protocol::ospf("ospf", "OSPF");
const Protocol Protocol::eigrp("eigrp", "EIGRP");
const Protocol Protocol::pim("pim", "PIM");
const Protocol Protocol::ppp("ppp", "PPP", Protocol::LinkLayer);
const Protocol Protocol::rip("rip", "RIP");
const Protocol Protocol::rsvpTe("rsvpte", "RSVP-TE");
const Protocol Protocol::rtsp("rtsp", "RTSP");
const Protocol Protocol::sctp("sctp", "SCTP", Protocol::TransportLayer);
const Protocol Protocol::srp("srp", "SRP");
const Protocol Protocol::ssh("ssh", "SSH");
const Protocol Protocol::stp("stp", "STP");
const Protocol Protocol::tcp("tcp", "TCP", Protocol::TransportLayer);
const Protocol Protocol::telnet("telnet", "TELNET");
const Protocol Protocol::trill("trill", "TRILL");
const Protocol Protocol::tsn("tsn", "TSN");
const Protocol Protocol::tteth("tteth", "TT-ETH");
const Protocol Protocol::udp("udp", "UDP", Protocol::TransportLayer);
const Protocol Protocol::xmac("xmac", "X-MAC");
const Protocol Protocol::xtp("xtp", "XTP");

// INET specific conceptual protocol identifiers
const Protocol Protocol::ackingMac("ackingmac", "Acking MAC");
const Protocol Protocol::apskPhy("apskphy", "APSK PHY", Protocol::PhysicalLayer);
const Protocol Protocol::csmaCaMac("csmacamac", "CSMA/CA MAC");
const Protocol Protocol::echo("echo", "Echo"); // Echo protocol (ping request/reply)
const Protocol Protocol::flooding("flooding", "Flooding", Protocol::NetworkLayer);
const Protocol Protocol::linkStateRouting("linkstaterouting", "LinkStateRouting");
const Protocol Protocol::nextHopForwarding("nexthopforwarding", "Next Hop Forwarding"); // Next Hop Forwarding
const Protocol Protocol::probabilistic("probabilistic", "Probabilistic", Protocol::NetworkLayer); // Probabilistic Network Protocol
const Protocol Protocol::shortcutMac("shortcutmac", "Shortcut MAC");
const Protocol Protocol::shortcutPhy("shortcutphy", "Shortcut PHY", Protocol::PhysicalLayer);
const Protocol Protocol::unitDisk("unitdisk", "UnitDisk");
const Protocol Protocol::unknown("unknown", "Unknown");
const Protocol Protocol::wiseRoute("wiseroute", "WiseRoute"); // WiseRoute Network Protocol

const Protocol Protocol::gptp("gptp", "gPTP");
const Protocol Protocol::ieee8021rTag("ieee8021rtag", "IEEE 802.1R TAG"); // TODO: move to its place in the alphanumeric ordering, causes fingerprints to change because some hypothetical protocols serialize the protocol ID into the packet
} // namespace inet

