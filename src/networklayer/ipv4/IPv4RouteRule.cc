//
// Copyright (C) 2004-2006 Andras Varga
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

#include <stdio.h>
#include <sstream>
#include "IPv4RouteRule.h"
#include "InterfaceEntry.h"

void IPv4RouteRule::setRoule(Rule rule)
{
    if (DROP!=rule && NONE!=rule)
        opp_error("Rule not supported yet");
    this->rule = rule;
}

IPv4RouteRule::IPv4RouteRule()
{
    interfacePtr = NULL;
    rule = NONE;
    sPort=dPort=-1;
    srcAddress=IPv4Address::UNSPECIFIED_ADDRESS;
    srcNetmask=IPv4Address::UNSPECIFIED_ADDRESS;
    destAddress=IPv4Address::UNSPECIFIED_ADDRESS;
    destNetmask=IPv4Address::UNSPECIFIED_ADDRESS;
    protocol=IP_PROT_NONE;

}

IPv4RouteRule::~IPv4RouteRule()
{
}

std::string IPv4RouteRule::info() const
{
    std::stringstream out;
    out << "srcAddr:"; if (srcAddress.isUnspecified()) out << "*  "; else out << srcAddress << "  ";
    out << "srcMask:"; if (srcNetmask.isUnspecified()) out << "*  "; else out << srcNetmask << "  ";
    out << "srcPort:" << sPort << " ";
    out << "destAddr:"; if (destAddress.isUnspecified()) out << "*  "; else out << destAddress << "  ";
    out << "destMask:"; if (destNetmask.isUnspecified()) out << "*  "; else out << destNetmask << "  ";
    out << "destPort:" << dPort << " ";
    out << "if:"; if (!interfacePtr) out << "*  "; else out << interfacePtr->getName() << "  ";

    switch (rule)
    {
        case DROP:   out << " DROP"; break;
        case ACCEPT: out << " ACCEPT"; break;
        case NAT:    out << " NAT"; break;
        case NONE:   out << " NONE"; break;
        default:     out << " ???"; break;
    }

    return out.str();
}

std::string IPv4RouteRule::detailedInfo() const
{
    return std::string();
}

const char *IPv4RouteRule::getInterfaceName() const
{
    return interfacePtr ? interfacePtr->getName() : "";
}

IPv4RouteRule::Nat::~Nat()
{
    while (!natAddress.empty())
    {
       delete natAddress.begin()->second;
       natAddress.erase(natAddress.begin());
    }
}

