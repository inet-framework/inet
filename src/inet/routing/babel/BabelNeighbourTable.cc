//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelNeighbourTable.h"

#include <bitset>
#include <sstream>

namespace inet {
namespace babel {

BabelNeighbour::~BabelNeighbour()
{
    deleteNHTimer();
    deleteNITimer();
}

std::string BabelNeighbour::str() const
{
    std::stringstream out;
    out << address;
    out << " on " << (interface ? interface->getIfaceName() : "-");
    out << " H:" << static_cast<std::bitset<16>>(history);
    out << " cost:" << cost;
    out << " txc:" << txcost;
    out << " rxc:" << computeRxcost();
    out << " eHsn:" << expHSeqno;
    out << " Hint:" << neighHelloInterval;
    out << " Iint:" << neighIhuInterval;
    return out.str();
}

bool BabelNeighbour::recomputeCost()
{
    uint16_t newcost = COST_INF;

    if (!interface->getInterface()->isUp())
        newcost = COST_INF;
    else if (txcost == COST_INF)
        newcost = COST_INF; // txcost is infinity -> cost must be infinity
    else if (interface->getCostComputationModule())
        newcost = interface->getCostComputationModule()->computeCost(history, interface->getNominalRxcost(), txcost);

    if (cost != newcost) {
        cost = newcost;
        return true;
    }
    return false;
}

uint16_t BabelNeighbour::computeRxcost() const
{
    if (interface->getCostComputationModule())
        return interface->getCostComputationModule()->computeRxcost(history, interface->getNominalRxcost());
    return COST_INF;
}

void BabelNeighbour::resetNHTimer() { resetTimer(neighHelloTimer, CStoS(neighHelloInterval)); }
void BabelNeighbour::resetNITimer() { resetTimer(neighIhuTimer, CStoS(neighIhuInterval)); }
void BabelNeighbour::resetNHTimer(double delay) { resetTimer(neighHelloTimer, delay); }
void BabelNeighbour::resetNITimer(double delay) { resetTimer(neighIhuTimer, delay); }
void BabelNeighbour::deleteNHTimer() { deleteTimer(&neighHelloTimer); }
void BabelNeighbour::deleteNITimer() { deleteTimer(&neighIhuTimer); }

BabelNeighbourTable::~BabelNeighbourTable()
{
    for (auto neigh : neighbours)
        delete neigh;
    neighbours.clear();
}

int BabelNeighbourTable::getNumOfNeighOnIface(BabelInterface *iface)
{
    int numofneigh = 0;
    for (auto neigh : neighbours)
        if (neigh->getInterface() == iface)
            ++numofneigh;
    return numofneigh;
}

BabelNeighbour *BabelNeighbourTable::findNeighbour(BabelInterface *iface, const L3Address& addr)
{
    for (auto neigh : neighbours)
        if (neigh->getAddress() == addr && neigh->getInterface() == iface)
            return neigh;
    return nullptr;
}

BabelNeighbour *BabelNeighbourTable::addNeighbour(BabelNeighbour *neigh)
{
    ASSERT(neigh != nullptr);
    BabelNeighbour *intable = findNeighbour(neigh->getInterface(), neigh->getAddress());
    if (intable != nullptr)
        return intable;
    neighbours.push_back(neigh);
    return neigh;
}

void BabelNeighbourTable::removeNeighbour(BabelNeighbour *neigh)
{
    for (auto it = neighbours.begin(); it != neighbours.end(); ++it) {
        if (*it == neigh) {
            delete *it;
            neighbours.erase(it);
            return;
        }
    }
}

void BabelNeighbourTable::removeNeighboursOnIface(BabelInterface *iface)
{
    for (auto it = neighbours.begin(); it != neighbours.end();) {
        if ((*it)->getInterface() == iface) {
            delete *it;
            it = neighbours.erase(it);
        }
        else
            ++it;
    }
}

} // namespace babel
} // namespace inet
