//
// Copyright (C) 2012 Univerdidad de Malaga.
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/WirelessGetNeig.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet{

WirelessGetNeig::WirelessGetNeig() {
    cTopology topo("topo");
    topo.extractByProperty("networkNode");
    cModule *mod = dynamic_cast<cModule*>(getOwner());
    for (mod = dynamic_cast<cModule*>(getOwner())->getParentModule(); mod != 0;
            mod = mod->getParentModule()) {
        cProperties *properties = mod->getProperties();
        if (properties && properties->getAsBool("networkNode"))
            break;
    }
    listNodes.clear();
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *destNode = topo.getNode(i);
        IMobility *mod;
        cModule *host = destNode->getModule();
        mod = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        if (mod == nullptr)
            throw cRuntimeError("node or mobility module not found");
        nodeInfo info;
        info.mob = mod;
        info.itable = L3AddressResolver().findInterfaceTableOf(destNode->getModule());
        auto addr = L3AddressResolver().getAddressFrom(info.itable);
        listNodes[addr] = info;

        for (int i = 0; i < info.itable->getNumInterfaces(); i++) {
            InterfaceEntry * entry = info.itable->getInterface(i);
            if (entry->isLoopback())
                continue;
            listNodesMac[L3Address(entry->getMacAddress())] = info;
        }
    }
}

WirelessGetNeig::~WirelessGetNeig() {
    // TODO Auto-generated destructor stub
    listNodes.clear();
    listNodesMac.clear();
}

void WirelessGetNeig::getNeighbours(const L3Address &node,
        std::vector<L3Address>&list, const double &distance) {

    if (node.getType() != L3Address::IPv4 && node.getType() != L3Address::MAC)
        throw cRuntimeError("Address type not supported");

    list.clear();
    if (node.getType() == L3Address::IPv4) {
        auto it = listNodes.find(node);
        if (it == listNodes.end())
            throw cRuntimeError("node not found");
        Coord pos = it->second.mob->getCurrentPosition();
        for (it = listNodes.begin(); it != listNodes.end(); ++it) {
            if (it->first == node)
                continue;
            if (pos.distance(it->second.mob->getCurrentPosition()) < distance) {
                list.push_back(it->first);
            }
        }

    }
    else {
        auto it = listNodesMac.find(node);
        if (it == listNodesMac.end())
            throw cRuntimeError("node not found");

        Coord pos = it->second.mob->getCurrentPosition();
        for (it = listNodesMac.begin(); it != listNodesMac.end(); ++it) {
            if (it->first == node)
                continue;
            if (pos.distance(it->second.mob->getCurrentPosition()) < distance) {
                list.push_back(it->first);
            }
        }
    }
}

void WirelessGetNeig::getNeighbours(const L3Address &node, std::vector<L3Address>&list, const double &distance, std::vector<Coord> & coord) {

    if (node.getType() != L3Address::IPv4 && node.getType() != L3Address::MAC)
        throw cRuntimeError("Address type not supported");

    list.clear();

    if (node.getType() == L3Address::IPv4 ) {
        auto it = listNodes.find(node);
        if (it == listNodes.end())
            throw cRuntimeError("node not found");
        Coord pos = it->second.mob->getCurrentPosition();
        for (it = listNodes.begin(); it != listNodes.end(); ++it) {
            if (it->first == node)
                continue;
            if (pos.distance(it->second.mob->getCurrentPosition()) < distance) {
                list.push_back(it->first);
                coord.push_back(pos);
            }
        }
    }
    else {
        auto it = listNodesMac.find(node);
        if (it == listNodesMac.end())
            throw cRuntimeError("node not found");

        Coord pos = it->second.mob->getCurrentPosition();
        for (it = listNodesMac.begin(); it != listNodesMac.end(); ++it) {
            if (it->first == node)
                continue;
            if (pos.distance(it->second.mob->getCurrentPosition()) < distance) {
                list.push_back(it->first);
                coord.push_back(pos);
            }
        }
    }

}

EulerAngles WirelessGetNeig::getDirection(const L3Address &node, const L3Address &dest, double &distance) {

    if (node.getType() != L3Address::IPv4 && node.getType() != L3Address::MAC)
        throw cRuntimeError("Address type not supported");
    Coord pos,pos2;

    if (node.getType() == L3Address::IPv4 ) {
        auto it = listNodes.find(node);
        if (it == listNodes.end())
            throw cRuntimeError("node not found");
        auto it2 = listNodes.find(dest);
        if (it2 == listNodes.end())
            throw cRuntimeError("node not found");
        pos = it->second.mob->getCurrentPosition();
        pos2 = it2->second.mob->getCurrentPosition();
        distance = pos.distance(pos2);
    }
    else {
        auto it = listNodesMac.find(node);
        if (it == listNodesMac.end())
            throw cRuntimeError("node not found");

        auto it2 = listNodesMac.find(dest);
        if (it2 == listNodesMac.end())
            throw cRuntimeError("node not found");

        pos = it->second.mob->getCurrentPosition();
        pos2 = it2->second.mob->getCurrentPosition();
        distance = pos.distance(pos2);
    }
    return EulerAngles(rad(pos.angle(pos2)),rad(0),rad(0));
}

}
