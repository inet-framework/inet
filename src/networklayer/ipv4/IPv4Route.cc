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

#include "IPv4Route.h"
#include "IPv4InterfaceData.h"

#include "InterfaceEntry.h"
#include "IRoutingTable.h"


std::string IPv4Route::info() const
{
    std::stringstream out;

    out << "dest:"; if (dest.isUnspecified()) out << "*  "; else out << dest << "  ";
    out << "gw:"; if (gateway.isUnspecified()) out << "*  "; else out << gateway << "  ";
    out << "mask:"; if (netmask.isUnspecified()) out << "*  "; else out << netmask << "  ";
    out << "metric:" << metric << " ";
    out << "if:"; if (!interfacePtr) out << "*  "; else out << interfacePtr->getName() << "(" << interfacePtr->ipv4Data()->getIPAddress() << ")  ";
    out << (gateway.isUnspecified() ? "DIRECT" : "REMOTE");


    switch (source)
    {
        case MANUAL:       out << " MANUAL"; break;
        case IFACENETMASK: out << " IFACENETMASK"; break;
        case RIP:          out << " RIP"; break;
        case OSPF:         out << " OSPF"; break;
        case BGP:          out << " BGP"; break;
        case ZEBRA:        out << " ZEBRA"; break;
        case MANET:        out << " MANET"; break;
        default:           out << " ???"; break;
    }

    return out.str();
}

std::string IPv4Route::detailedInfo() const
{
    return std::string();
}

bool IPv4Route::equals(const IPv4Route& route) const
{
    return rt == route.rt && dest == route.dest && netmask == route.netmask && gateway == route.gateway &&
           interfacePtr == route.interfacePtr && source == route.source && metric == route.metric;
}

const char *IPv4Route::getInterfaceName() const
{
    return interfacePtr ? interfacePtr->getName() : "";
}

void IPv4Route::changed(int fieldCode)
{
    if (rt)
        rt->routeChanged(this, fieldCode);
}

IPv4MulticastRoute::~IPv4MulticastRoute()
{
    for (ChildInterfaceVector::iterator it = children.begin(); it != children.end(); ++it)
        delete *it;
    children.clear();
}

std::string IPv4MulticastRoute::info() const
{
    std::stringstream out;

    out << "origin:"; if (origin.isUnspecified()) out << "*  "; else out << origin << "  ";
    out << "mask:"; if (originNetmask.isUnspecified()) out << "*  "; else out << originNetmask << "  ";
    out << "group:"; if (group.isUnspecified()) out << "*  "; else out << group << "  ";
    out << "metric:" << metric << " ";
    out << "parent:"; if (!parent) out << "*  "; else out << parent->getName() << "  ";
    out << "children:";
    for (unsigned int i = 0; i < children.size(); ++i)
    {
        if (i > 0)
            out << ",";
        out << children[i]->getInterface()->getName();
    }

    switch (source)
    {
        case MANUAL:       out << " MANUAL"; break;
        case DVMRP:        out << " DVRMP"; break;
        case PIM_SM:       out << " PIM-SM"; break;
        default:           out << " ???"; break;
    }

    return out.str();
}

std::string IPv4MulticastRoute::detailedInfo() const
{
    return info();
}



bool IPv4MulticastRoute::addChild(InterfaceEntry *ie, bool isLeaf)
{
    ChildInterfaceVector::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        if ((*it)->getInterface() == ie)
            break;
    }

    if (it != children.end())
    {
        if ((*it)->isLeaf() != isLeaf)
        {
            delete *it;
            *it = new ChildInterface(ie, isLeaf);
            changed(F_CHILDREN);
            return true;
        }
        else
            return false;
    }
    else
    {
        children.push_back(new ChildInterface(ie, isLeaf));
        changed(F_CHILDREN);
        return true;
    }
}

bool IPv4MulticastRoute::removeChild(InterfaceEntry *ie)
{
    for (ChildInterfaceVector::iterator it = children.begin(); it != children.end(); ++it)
    {
        if ((*it)->getInterface() == ie)
        {
            delete *it;
            children.erase(it);
            changed(F_CHILDREN);
            return true;
        }
    }
    return false;
}

void IPv4MulticastRoute::changed(int fieldCode)
{
    if (rt)
        rt->multicastRouteChanged(this, fieldCode);
}
