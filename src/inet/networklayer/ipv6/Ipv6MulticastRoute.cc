//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6MulticastRoute.h"

#include <algorithm>
#include <sstream>

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

Register_Abstract_Class(Ipv6MulticastRoute);

Ipv6MulticastRoute::Ipv6MulticastRoute(const Ipv6MulticastRoute& other) :
    rt(nullptr),
    origin(other.origin),
    prefixLength(other.prefixLength),
    group(other.group),
    inInterface(other.inInterface ? new IMulticastRoute::InInterface(*other.inInterface) : nullptr),
    sourceType(other.sourceType),
    source(other.source),
    metric(other.metric)
{
    for (const auto& elem : other.outInterfaces)
        outInterfaces.push_back(new IMulticastRoute::OutInterface(*elem));
}

Ipv6MulticastRoute::~Ipv6MulticastRoute()
{
    delete inInterface;
    for (auto& elem : outInterfaces)
        delete elem;
    outInterfaces.clear();
}

void Ipv6MulticastRoute::changed(int fieldCode)
{
    if (rt)
        rt->multicastRouteChanged(this, fieldCode);
}

IRoutingTable *Ipv6MulticastRoute::getRoutingTableAsGeneric() const
{
    return getRoutingTable();
}

bool Ipv6MulticastRoute::matches(const Ipv6Address& origin, const Ipv6Address& group) const
{
    return (this->group.isUnspecified() || this->group == group) &&
           origin.matches(this->origin, prefixLength);
}

std::string Ipv6MulticastRoute::str() const
{
    std::stringstream out;

    out << "origin:";
    if (origin.isUnspecified())
        out << "*  ";
    else
        out << origin << "/" << prefixLength << "  ";
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
        out << elem->getInterface()->getInterfaceName();
        first = false;
    }

    out << " " << IMulticastRoute::sourceTypeName(sourceType);

    return out.str();
}

std::string Ipv6MulticastRoute::detailedInfo() const
{
    return str();
}

void Ipv6MulticastRoute::setInInterface(InInterface *_inInterface)
{
    if (inInterface != _inInterface) {
        delete inInterface;
        inInterface = _inInterface;
        changed(F_IN);
    }
}

void Ipv6MulticastRoute::clearOutInterfaces()
{
    if (!outInterfaces.empty()) {
        for (auto& elem : outInterfaces)
            delete elem;
        outInterfaces.clear();
        changed(F_OUT);
    }
}

void Ipv6MulticastRoute::addOutInterface(OutInterface *outInterface)
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
    std::sort(outInterfaces.begin(), outInterfaces.end(), [] (const OutInterface *i1, const OutInterface *i2) {
        return strcmp(i1->getInterface()->getInterfaceName(), i2->getInterface()->getInterfaceName()) <= 0;
    });
    changed(F_OUT);
}

bool Ipv6MulticastRoute::removeOutInterface(const NetworkInterface *ie)
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

void Ipv6MulticastRoute::removeOutInterface(unsigned int i)
{
    OutInterface *outInterface = outInterfaces.at(i);
    delete outInterface;
    outInterfaces.erase(outInterfaces.begin() + i);
    changed(F_OUT);
}

bool Ipv6MulticastRoute::hasOutInterface(const NetworkInterface *networkInterface) const
{
    for (auto& elem : outInterfaces)
        if (elem->getInterface() == networkInterface)
            return true;
    return false;
}

} // namespace inet
