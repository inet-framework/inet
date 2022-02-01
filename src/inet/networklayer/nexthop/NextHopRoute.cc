//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/nexthop/NextHopRoute.h"

#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#include "inet/networklayer/nexthop/NextHopRoutingTable.h"

namespace inet {

Register_Class(NextHopRoute);

std::string NextHopRoute::str() const
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
        out << interface->getInterfaceName();
    if (interface)
        if (auto nextHopData = interface->findProtocolData<NextHopInterfaceData>())
            out << "(" << nextHopData->getAddress() << ")";
    out << "  ";
    out << (nextHop.isUnspecified() ? "DIRECT" : "REMOTE");
    out << " " << IRoute::sourceTypeName(sourceType);
    if (protocolData)
        out << " " << protocolData->str();

    return out.str();
}

std::string NextHopRoute::detailedInfo() const
{
    return ""; // TODO
}

bool NextHopRoute::equals(const IRoute& route) const
{
    return false; // TODO
}

void NextHopRoute::changed(int fieldCode)
{
    if (owner)
        owner->routeChanged(this, fieldCode);
}

IRoutingTable *NextHopRoute::getRoutingTableAsGeneric() const
{
    return owner;
}

// ---

#if 0 /*FIXME TODO!!!! */

std::string NextHopMulticastRoute::str() const
{
    return ""; // TODO
}

std::string NextHopMulticastRoute::detailedInfo() const
{
    return ""; // TODO
}

bool NextHopMulticastRoute::addChild(NetworkInterface *ie, bool isLeaf)
{
    // TODO
    children.push_back(Child());
    Child& child = children.back();
    child.ie = ie;
    child.isLeaf = isLeaf;
}

bool NextHopMulticastRoute::removeChild(NetworkInterface *ie)
{
    // TODO
}

#endif /*0*/

} // namespace inet

