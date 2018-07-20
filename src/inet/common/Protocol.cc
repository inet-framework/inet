//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/Protocol.h"

namespace inet {

int Protocol::nextId = 0;
std::map<int, const Protocol *> Protocol::idToProtocol;
std::map<std::string, const Protocol *> Protocol::nameToProtocol;

Protocol::Protocol(const char *name, const char *descriptiveName, Layer layer) :
    id(nextId++),
    name(name),
    descriptiveName(descriptiveName),
    layer(layer)
{
    idToProtocol[id] = this;
    nameToProtocol[name] = this;
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
    auto it = idToProtocol.find(id);
    return it != idToProtocol.end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(int id)
{
    const Protocol *protocol = findProtocol(id);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: id = %d" , id);
}

const Protocol *Protocol::findProtocol(const char *name)
{
    auto it = nameToProtocol.find(name);
    return it != nameToProtocol.end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(const char *name)
{
    const Protocol *protocol = findProtocol(name);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: name = %s" , name);
}

// Standard protocol identifiers
const Protocol Protocol::aodv("aodv", "AODV");
const Protocol Protocol::arp("arp", "ARP", Protocol::NetworkLayer);
const Protocol Protocol::bgp("bgp", "BGP");
const Protocol Protocol::bmac("bmac", "B-MAC", Protocol::LinkLayer);
const Protocol Protocol::dsdv2("dsdv2", "DSDV 2");
const Protocol Protocol::dsr("dsr", "DSR");
const Protocol Protocol::dymo("dymo", "DYMO");
const Protocol Protocol::egp("egp", "EGP");
const Protocol Protocol::ethernetMac("ethernetmac", "Ethernet MAC", Protocol::LinkLayer);
const Protocol Protocol::ethernetPhy("ethernetphy", "Ethernet PHY", Protocol::PhysicalLayer);
const Protocol Protocol::gpsr("gpsr", "GPSR");
const Protocol Protocol::icmpv4("icmpv4", "ICMPv4", Protocol::NetworkLayer);
const Protocol Protocol::icmpv6("icmpv6", "ICMPv6", Protocol::NetworkLayer);
const Protocol Protocol::ieee80211Mac("ieee80211mac", "IEEE 802.11 MAC", Protocol::LinkLayer);
const Protocol Protocol::ieee80211Mgmt("ieee80211mgmt", "IEEE 802.11 MGMT", Protocol::LinkLayer);
const Protocol Protocol::ieee80211Phy("ieee80211phy", "IEEE 802.11 PHY", Protocol::PhysicalLayer);
const Protocol Protocol::ieee802154("ieee802154", "IEEE 802.15.4");
const Protocol Protocol::ieee8022("ieee8022", "IEEE 802.2", Protocol::LinkLayer);
const Protocol Protocol::igmp("igmp", "IGMP", Protocol::NetworkLayer);
const Protocol Protocol::igp("igp", "IGP");
const Protocol Protocol::ipv4("ipv4", "IPv4", Protocol::NetworkLayer);
const Protocol Protocol::ipv6("ipv6", "IPv6", Protocol::NetworkLayer);
const Protocol Protocol::lmac("lmac", "L-MAC");
const Protocol Protocol::manet("manet", "MANET");
const Protocol Protocol::mobileipv6("mobileipv6", "Mobile IPv6");
const Protocol Protocol::mpls("mpls", "MPLS");
const Protocol Protocol::ospf("ospf", "OSPF");
const Protocol Protocol::pim("pim", "PIM");
const Protocol Protocol::ppp("ppp", "PPP", Protocol::LinkLayer);
const Protocol Protocol::rip("rip", "RIP");
const Protocol Protocol::rsvpTe("rsvpte", "RSVP-TE");
const Protocol Protocol::sctp("sctp", "SCTP", Protocol::TransportLayer);
const Protocol Protocol::stp("stp", "STP");
const Protocol Protocol::tcp("tcp", "TCP", Protocol::TransportLayer);
const Protocol Protocol::udp("udp", "UDP", Protocol::TransportLayer);
const Protocol Protocol::xmac("xmac", "X-MAC");
const Protocol Protocol::xtp("xtp", "XTP");

// INET specific conceptual protocol identifiers
const Protocol Protocol::ackingMac("ackingmac", "Acking MAC");
const Protocol Protocol::apskPhy("apskphy", "APSK PHY", Protocol::PhysicalLayer);
const Protocol Protocol::csmaCaMac("csmacamac", "CSMA/CA MAC");
const Protocol Protocol::echo("echo", "Echo"); // Echo protocol (ping request/reply)
const Protocol Protocol::flooding("flooding", "Flooding", Protocol::NetworkLayer);
const Protocol Protocol::nextHopForwarding("nexthopforwarding", "Next Hop Forwarding"); // Next Hop Forwarding
const Protocol Protocol::linkStateRouting("linkstaterouting", "LinkStateRouting");
const Protocol Protocol::probabilistic("probabilistic", "Probabilistic", Protocol::NetworkLayer); // Probabilistic Network Protocol
const Protocol Protocol::shortcutMac("shortcutmac", "Shortcut MAC");
const Protocol Protocol::shortcutPhy("shortcutphy", "Shortcut PHY", Protocol::PhysicalLayer);
const Protocol Protocol::unitDisk("unitdisk", "UnitDisk");
const Protocol Protocol::wiseRoute("wiseroute", "WiseRoute"); // WiseRoute Network Protocol

} // namespace inet

