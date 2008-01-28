//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    _netmask = allOnes;

    _metric = 0;

    // TBD add default multicast groups!
}

std::string IPv4InterfaceData::info() const
{
    std::stringstream out;
    out << "IP:{inet_addr:" << inetAddress() << "/" << netmask().netmaskLength();
    if (!multicastGroups().empty())
    {
        out << " mcastgrps:";
        for (unsigned int j=0; j<multicastGroups().size(); j++)
            if (!multicastGroups()[j].isUnspecified())
                out << (j>0?",":"") << multicastGroups()[j];
    }
    out << "}";
    return out.str();
}

std::string IPv4InterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "inet addr:" << inetAddress() << "\tMask: " << netmask() << "\n";

    out << "Metric: " << metric() << "\n";

    out << "Groups:";
    for (unsigned int j=0; j<multicastGroups().size(); j++)
        if (!multicastGroups()[j].isUnspecified())
            out << "  " << multicastGroups()[j];
    out << "\n";
    return out.str();
}


