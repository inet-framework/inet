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

#include "inet/networklayer/generic/GenericRoute.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"

namespace inet {

Register_Class(GenericRoute);

std::string GenericRoute::info() const
{
    std::stringstream out;

    out << "dest:";
    if (destination.isUnspecified())
        out << "*  ";
    else
        out << destination << "  ";
    out << "gw:";
    if (nextHop.isUnspecified())
        out << "*  ";
    else
        out << nextHop << "  ";
    out << "metric:" << metric << " ";
    out << "if:";
    if (!interface)
        out << "*";
    else
        out << interface->getName();
    if (interface && interface->getGenericNetworkProtocolData())
        out << "(" << interface->getGenericNetworkProtocolData()->getAddress() << ")";
    out << "  ";
    out << (nextHop.isUnspecified() ? "DIRECT" : "REMOTE");
    out << " " << IRoute::sourceTypeName(sourceType);

    return out.str();
}

std::string GenericRoute::detailedInfo() const
{
    return "";    //TODO
}

bool GenericRoute::equals(const IRoute& route) const
{
    return false;    //TODO
}

void GenericRoute::changed(int fieldCode)
{
    if (owner)
        owner->routeChanged(this, fieldCode);
}

IRoutingTable *GenericRoute::getRoutingTableAsGeneric() const
{
    return owner;
}

//---

#if 0    /*FIXME TODO!!!! */

std::string GenericMulticastRoute::info() const
{
    return "";    //TODO
}

std::string GenericMulticastRoute::detailedInfo() const
{
    return "";    //TODO
}

bool GenericMulticastRoute::addChild(InterfaceEntry *ie, bool isLeaf)
{
    //TODO:
    children.push_back(Child());
    Child& child = children.back();
    child.ie = ie;
    child.isLeaf = isLeaf;
}

bool GenericMulticastRoute::removeChild(InterfaceEntry *ie)
{
    //TODO:
}

#endif /*0*/

} // namespace inet

