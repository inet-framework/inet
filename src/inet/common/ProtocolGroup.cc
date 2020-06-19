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

#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/common/IpProtocolId_m.h"

namespace inet {

ProtocolGroup::ProtocolGroup(const char *name, std::map<int, const Protocol *> protocolNumberToProtocol) :
    name(name),
    protocolNumberToProtocol(protocolNumberToProtocol)
{
    for (auto it : protocolNumberToProtocol)
        protocolToProtocolNumber[it.second] = it.first;
}

const Protocol *ProtocolGroup::findProtocol(int protocolNumber) const
{
    auto it = protocolNumberToProtocol.find(protocolNumber);
    return it != protocolNumberToProtocol.end() ? it->second : nullptr;
}

const Protocol *ProtocolGroup::getProtocol(int protocolNumber) const
{
    auto protocol = findProtocol(protocolNumber);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: number = %d", protocolNumber);
}

int ProtocolGroup::findProtocolNumber(const Protocol *protocol) const
{
    auto it = protocolToProtocolNumber.find(protocol);
    return it != protocolToProtocolNumber.end() ? it->second : -1;
}

int ProtocolGroup::getProtocolNumber(const Protocol *protocol) const
{
    auto protocolNumber = findProtocolNumber(protocol);
    if (protocolNumber != -1)
        return protocolNumber;
    else
        throw cRuntimeError("Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
}

void ProtocolGroup::addProtocol(int protocolId, const Protocol *protocol)
{
    protocolNumberToProtocol[protocolId] = protocol;
    protocolToProtocolNumber[protocol] = protocolId;
}


//FIXME use constants instead of numbers

// excerpt from http://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
ProtocolGroup ProtocolGroup::ethertype("ethertype", {
    { 0x0800, &Protocol::ipv4 },
    { 0x0806, &Protocol::arp },
    { 0x2000, &Protocol::cdp },              // TODO remove it, it's a CISCO code for LLC, ANSAINET project use it currently
    { 0x22EA, &Protocol::srp},
    { 0x22F0, &Protocol::tsn},
    { 0x22F3, &Protocol::trill},
    { 0x22F4, &Protocol::l2isis},
    { 0x8100, &Protocol::ieee8021QCtag },
    { 0x86DD, &Protocol::ipv6 },
    { 0x36FC, &Protocol::flooding },         // ETHERTYPE_INET_FLOODING, not in any standards
    { 0x86FD, &Protocol::probabilistic },         // ETHERTYPE_INET_PROBABILISTIC, not in any standards
    { 0x86FE, &Protocol::wiseRoute },         // ETHERTYPE_INET_WISE, not in any standards
    { 0x86FF, &Protocol::nextHopForwarding },         // ETHERTYPE_INET_NEXTHOP
    { 0x8808, &Protocol::ethernetFlowCtrl },
    { 0x8847, &Protocol::mpls },
    { 0x88A8, &Protocol::ieee8021QStag },
    { 0x88CC, &Protocol::lldp },
    { 0x891d, &Protocol::tteth },
});

// excerpt from http://www.iana.org/assignments/ppp-numbers/ppp-numbers.xhtml
ProtocolGroup ProtocolGroup::pppprotocol("pppprotocol", {
    { 0x0021, &Protocol::ipv4 },
    { 0x0057, &Protocol::ipv6 },
    { 0x0281, &Protocol::mpls },        // MPLS unicast
    { 0x39FC, &Protocol::flooding },         // INET_FLOODING, not in any standards
    { 0x39FD, &Protocol::probabilistic },         // INET_PROBABILISTIC, not in any standards
    { 0x39FE, &Protocol::wiseRoute },         // INET_WISE, not in any standards
    { 0x39FF, &Protocol::nextHopForwarding },         // INET_NEXT_HOP_FORWARDING, not in any standards
});

// excerpt from http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
ProtocolGroup ProtocolGroup::ipprotocol("ipprotocol", {
    { 1, &Protocol::icmpv4 },
    { 2, &Protocol::igmp },
    { 4, &Protocol::ipv4 },
    { 6, &Protocol::tcp },
    { 8, &Protocol::egp },
    { 9, &Protocol::igp },
    { 17, &Protocol::udp },
    { 36, &Protocol::xtp },
    { 41, &Protocol::ipv6 },
    { 46, &Protocol::rsvpTe },
    { 48, &Protocol::dsr },
    { 58, &Protocol::icmpv6 },
    { 89, &Protocol::ospf },
    { 103, &Protocol::pim },
    { 132, &Protocol::sctp },
    { 135, &Protocol::mobileipv6 },
    { 138, &Protocol::manet },

    { 249, &Protocol::linkStateRouting },    // INET specific: Link State Routing Protocol
    { 250, &Protocol::flooding },    // INET specific: Probabilistic Network Protocol
    { 251, &Protocol::probabilistic },    // INET specific: Probabilistic Network Protocol
    { 252, &Protocol::wiseRoute },    // INET specific: Probabilistic Network Protocol
    { 253, &Protocol::nextHopForwarding },    // INET specific: Next Hop Forwarding
    { 254, &Protocol::echo },    // INET specific: Echo Protocol
});

ProtocolGroup ProtocolGroup::snapOui("snapOui", {
    //TODO do not add {0, .... }, it is a  special value: the protocolId contains the ethertype value
    // { 0x00000C, &Protocol::ciscoSnap } //TODO
});

ProtocolGroup ProtocolGroup::ieee8022protocol("ieee8022protocol", {
    { 0x4242, &Protocol::stp },
    { 0xFE00, &Protocol::isis },
    { 0xAAAA, &Protocol::ieee8022snap },
});

ProtocolGroup ProtocolGroup::udpprotocol("udpprotocol", {
    { 554, &Protocol::rtsp },
    { 6696, &Protocol::babel },
});

ProtocolGroup ProtocolGroup::tcpprotocol("tcpprotocol", {
    { 21, &Protocol::ftp },
    { 22, &Protocol::ssh },
    { 23, &Protocol::telnet },
    { 80, &Protocol::http },
    { 554, &Protocol::rtsp },
});

} // namespace inet

