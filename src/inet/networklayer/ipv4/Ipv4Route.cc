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

#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

Register_Class(Ipv4Route);
Register_Class(Ipv4MulticastRoute);

Ipv4Route::~Ipv4Route()
{
    delete protocolData;
}

std::string Ipv4Route::str() const
{
    std::stringstream out;

    out << "dest:";
    if (dest.isUnspecified())
        out << "*  ";
    else
        out << dest << "  ";
    out << "gw:";
    if (gateway.isUnspecified())
        out << "*  ";
    else
        out << gateway << "  ";
    out << "mask:";
    if (netmask.isUnspecified())
        out << "*  ";
    else
        out << netmask << "  ";
    out << "metric:" << metric << " ";
    out << "if:";
    if (!interfacePtr)
        out << "*";
    else
        out << interfacePtr->getInterfaceName();
    if (interfacePtr && interfacePtr->ipv4Data())
        out << "(" << interfacePtr->ipv4Data()->getIPAddress() << ")";
    out << "  ";
    out << (gateway.isUnspecified() ? "DIRECT" : "REMOTE");
    out << " " << IRoute::sourceTypeName(sourceType);
    if (protocolData)
        out << " " << protocolData->str();
    return out.str();
}

std::string Ipv4Route::detailedInfo() const
{
    return std::string();
}

bool Ipv4Route::equals(const Ipv4Route& route) const
{
    return rt == route.rt && dest == route.dest && netmask == route.netmask && gateway == route.gateway &&
           interfacePtr == route.interfacePtr && sourceType == route.sourceType && metric == route.metric;
}

const char *Ipv4Route::getInterfaceName() const
{
    return interfacePtr ? interfacePtr->getInterfaceName() : "";
}

void Ipv4Route::changed(int fieldCode)
{
    if (rt)
        rt->routeChanged(this, fieldCode);
}

IRoutingTable *Ipv4Route::getRoutingTableAsGeneric() const
{
    return getRoutingTable();
}

Ipv4MulticastRoute::~Ipv4MulticastRoute()
{
    delete inInterface;
    for (auto & elem : outInterfaces)
        delete elem;
    outInterfaces.clear();
}

std::string Ipv4MulticastRoute::str() const
{
    std::stringstream out;

    out << "origin:";
    if (origin.isUnspecified())
        out << "*  ";
    else
        out << origin << "  ";
    out << "mask:";
    if (originNetmask.isUnspecified())
        out << "*  ";
    else
        out << originNetmask << "  ";
    out << "group:";
    if (group.isUnspecified())
        out << "*  ";
    else
        out << group << "  ";
    out << "metric:" << metric << " ";
    out << "in:";
    if (!inInterface)
        out << "*  ";
    else
        out << inInterface->getInterface()->getInterfaceName() << "  ";
    out << "out:";
    bool first = true;
    for (auto & elem : outInterfaces) {
        if (!first)
            out << ",";
        if (elem->isEnabled()) {
            out << elem->getInterface()->getInterfaceName();
            first = false;
        }
    }

    out << " " << IMulticastRoute::sourceTypeName(sourceType);

    return out.str();
}

std::string Ipv4MulticastRoute::detailedInfo() const
{
    return str();
}

void Ipv4MulticastRoute::setInInterface(InInterface *_inInterface)
{
    if (inInterface != _inInterface) {
        delete inInterface;
        inInterface = _inInterface;
        changed(F_IN);
    }
}

void Ipv4MulticastRoute::clearOutInterfaces()
{
    if (!outInterfaces.empty()) {
        for (auto & elem : outInterfaces)
            delete elem;
        outInterfaces.clear();
        changed(F_OUT);
    }
}

IRoutingTable *Ipv4MulticastRoute::getRoutingTableAsGeneric() const
{
    return getRoutingTable();
}

void Ipv4MulticastRoute::addOutInterface(OutInterface *outInterface)
{
    ASSERT(outInterface);

    auto it = outInterfaces.begin();
    for ( ; it != outInterfaces.end(); ++it) {
        if ((*it)->getInterface() == outInterface->getInterface()) {
            delete *it;
            *it = outInterface;
            changed(F_OUT);
            return;
        }
    }

    outInterfaces.push_back(outInterface);
    changed(F_OUT);
}

bool Ipv4MulticastRoute::removeOutInterface(const InterfaceEntry *ie)
{
    for (auto it = outInterfaces.begin(); it != outInterfaces.end(); ++it) {
        if ((*it)->getInterface() == ie) {
            delete *it;
            outInterfaces.erase(it);
            changed(F_OUT);
            return true;
        }
    }
    return false;
}

void Ipv4MulticastRoute::removeOutInterface(unsigned int i)
{
    OutInterface *outInterface = outInterfaces.at(i);
    delete outInterface;
    outInterfaces.erase(outInterfaces.begin() + i);
    changed(F_OUT);
}

void Ipv4MulticastRoute::changed(int fieldCode)
{
    if (rt)
        rt->multicastRouteChanged(this, fieldCode);
}

} // namespace inet

