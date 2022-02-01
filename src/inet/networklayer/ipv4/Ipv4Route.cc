//
// Copyright (C) 2004-2006 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4/Ipv4Route.h"

#include <stdio.h>

#include <sstream>

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"

namespace inet {

Register_Class(Ipv4Route);
Register_Class(Ipv4MulticastRoute);

Ipv4Route::~Ipv4Route()
{
    delete protocolData;
}

const char *inet::Ipv4Route::getSourceTypeAbbreviation() const {
    switch (sourceType) {
        case IFACENETMASK:
            return "C";
        case MANUAL:
            return getDestination().isUnspecified() ? "S*" : "S";
        case ROUTER_ADVERTISEMENT:
            return "ra";
        case RIP:
            return "R";
        case OSPF:
            return "O";
        case BGP:
            return "B";
        case EIGRP:
            return getAdminDist() < Ipv4Route::dEIGRPExternal ? "D" : "D EX";
        case LISP:
            return "l";
        case BABEL:
            return "ba";
        case ODR:
            return "o";
        default:
            return "???";
    }
}

std::string Ipv4Route::str() const
{
    std::stringstream out;
    out << getSourceTypeAbbreviation();
    if (getDestination().isUnspecified())
        out << " 0.0.0.0";
    else
        out << " " << getDestination();
    out << "/";
    if (getNetmask().isUnspecified())
        out << "0";
    else
        out << getNetmask().getNetmaskLength();
    out << " gw:";
    if (gateway.isUnspecified())
        out << "*";
    else
        out << getGateway();
    if (rt && rt->isAdminDistEnabled())
        out << " AD:" << adminDist;
    out << " metric:" << metric;
    out << " if:";
    if (!interfacePtr)
        out << "*";
    else
        out << getInterfaceName();

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
    for (auto& elem : outInterfaces)
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
    for (auto& elem : outInterfaces) {
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
        for (auto& elem : outInterfaces)
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
    for (; it != outInterfaces.end(); ++it) {
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

bool Ipv4MulticastRoute::removeOutInterface(const NetworkInterface *ie)
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

