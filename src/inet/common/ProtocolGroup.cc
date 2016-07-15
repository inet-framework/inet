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


//FIXME use constants instead of numbers

// excerpt from http://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
const ProtocolGroup ProtocolGroup::ethertype("ethertype", {
    { 0x0800, &Protocol::ipv4 },
    { 0x0806, &Protocol::arp},
    { 0x86DD, &Protocol::ipv6 },
    { 0x86FF, &Protocol::gnp },         // ETHERTYPE_INET_GENERIC
    { 0x8847, &Protocol::mpls },
});

// excerpt from http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
const ProtocolGroup ProtocolGroup::ipprotocol("ipprotocol", {
    { 1, &Protocol::icmpv4 },
    { 2, &Protocol::igmp },
    { 4, &Protocol::ipv4 },
    { 6, &Protocol::tcp },
    { 8, &Protocol::egp },
    { 9, &Protocol::igp },
    { 17, &Protocol::udp },
    { 36, &Protocol::xtp },
    { 41, &Protocol::ipv6 },
    { 46, &Protocol::rsvp },
    { 48, &Protocol::dsr },
    { 58, &Protocol::icmpv6 },
    { 89, &Protocol::ospf },
    { 103, &Protocol::pim },
    { 132, &Protocol::sctp},
    { 138, &Protocol::manet },

    { 253, &Protocol::gnp },    // INET specific: Generic Network Protocol
});

} // namespace inet

