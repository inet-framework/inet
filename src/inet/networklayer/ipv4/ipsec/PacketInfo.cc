//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/networklayer/ipv4/ipsec/PacketInfo.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {
namespace ipsec {

std::string PacketInfo::str() const
{
    std::stringstream out;

    out << "Protocol: " << nextProtocol;
    switch (nextProtocol) {
        case IP_PROT_TCP:
        case IP_PROT_UDP:
            out << " Local: " << localAddress.str() << ":" << localPort << ";";
            out << " Remote: " << remoteAddress.str() << ":" << remotePort << ";";
            break;

        default:
            out << " Local: " << localAddress.str() << ";";
            out << " Remote: " << remoteAddress.str() << ";";
    }
    return out.str();
}

}  // namespace ipsec
}  // namespace inet

