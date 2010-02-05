//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


//  Author: Andras Varga, 2004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <algorithm>
#include <sstream>

#include "IPv4InterfaceData.h"


IPv4InterfaceData::IPv4InterfaceData()
{
    static const IPAddress allOnes("255.255.255.255");
    netmask = allOnes;

    metric = 0;

    // TBD add default multicast groups!
}

std::string IPv4InterfaceData::info() const
{
    std::stringstream out;
    out << "IP:{inet_addr:" << getIPAddress() << "/" << getNetmask().getNetmaskLength();
    if (!getMulticastGroups().empty())
    {
        out << " mcastgrps:";
        for (unsigned int j=0; j<getMulticastGroups().size(); j++)
            if (!getMulticastGroups()[j].isUnspecified())
                out << (j>0?",":"") << getMulticastGroups()[j];
    }
    out << "}";
    return out.str();
}

std::string IPv4InterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "inet addr:" << getIPAddress() << "\tMask: " << getNetmask() << "\n";

    out << "Metric: " << getMetric() << "\n";

    out << "Groups:";
    for (unsigned int j=0; j<getMulticastGroups().size(); j++)
        if (!getMulticastGroups()[j].isUnspecified())
            out << "  " << getMulticastGroups()[j];
    out << "\n";
    return out.str();
}

bool IPv4InterfaceData::isMemberOfMulticastGroup(const IPAddress& multicastAddress) const
{
    int n = getMulticastGroups().size();
    for (int i=0; i<n; i++)
        if (multicastAddress.equals(getMulticastGroups()[i]))
            return true;
    return false;
}

