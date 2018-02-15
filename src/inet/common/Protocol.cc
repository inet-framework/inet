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

Protocol::Protocol(const char *name, const char *descriptiveName) :
    id(nextId++),
    name(name),
    descriptiveName(descriptiveName)
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

const Protocol Protocol::aodv("aodv", "AODV");
const Protocol Protocol::arp("arp", "ARP");
const Protocol Protocol::bgp("bgp", "BGP");
const Protocol Protocol::dsdv2("dsdv2", "DSDV 2");
const Protocol Protocol::dsr("dsr", "DSR");
const Protocol Protocol::dymo("dymo", "DYMO");
const Protocol Protocol::egp("egp", "EGP");
const Protocol Protocol::ethernet("ethernet", "Ethernet");
const Protocol Protocol::echo("echo", "Echo");
const Protocol Protocol::gnp("gnp", "GNP"); // INET specific Generic Network Protocol
const Protocol Protocol::gpsr("gpsr", "GPSR");
const Protocol Protocol::icmpv4("icmpv4", "ICMP v4");
const Protocol Protocol::icmpv6("icmpv6", "ICMP v6");
const Protocol Protocol::ieee80211("ieee80211", "IEEE 802.11");
const Protocol Protocol::ieee8022("ieee8022", "IEEE 802.2");
const Protocol Protocol::igmp("igmp", "IGMP");
const Protocol Protocol::igp("igp", "IGP");
const Protocol Protocol::ipv4("ipv4", "IP v4");
const Protocol Protocol::ipv6("ipv6", "IP v6");
const Protocol Protocol::mobileipv6("mobileipv6", "Mobile IP v6");
const Protocol Protocol::manet("manet", "MANET");
const Protocol Protocol::mpls("mpls", "MPLS");
const Protocol Protocol::ospf("ospf", "OSPF");
const Protocol Protocol::pim("pim", "PIM");
const Protocol Protocol::ppp("ppp", "PPP");
const Protocol Protocol::rsvp("rsvp", "RSVP");
const Protocol Protocol::sctp("sctp", "SCTP");
const Protocol Protocol::stp("stp", "STP");
const Protocol Protocol::tcp("tcp", "TCP");
const Protocol Protocol::udp("udp", "UDP");
const Protocol Protocol::xtp("xtp", "XTP");

} // namespace inet

