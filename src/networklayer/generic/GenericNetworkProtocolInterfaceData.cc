//
// Copyright (C) 2012 Opensim Ltd.
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

#include <algorithm>
#include <sstream>

#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"

namespace inet {

std::string GenericNetworkProtocolInterfaceData::info() const
{
    std::stringstream out;
//TODO
//    out << "IPv4:{inet_addr:" << getIPAddress() << "/" << getNetmask().getNetmaskLength();
//    out << "}";
    return out.str();
}

std::string GenericNetworkProtocolInterfaceData::detailedInfo() const
{
//TODO
    std::stringstream out;
//    out << "inet addr:" << getIPAddress() << "\tMask: " << getNetmask() << "\n";
//    out << "Metric: " << getMetric() << "\n";
    return out.str();
}

} // namespace inet

