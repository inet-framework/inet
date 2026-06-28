//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELNEIGHBOURTABLE_H
#define __INET_BABELNEIGHBOURTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/routing/babel/BabelDefs.h"
#include "inet/routing/babel/BabelInterfaceTable.h"

namespace inet {
namespace babel {

/**
 * A discovered Babel neighbour on one interface (RFC 6126, section 3.2). Holds
 * the Hello-reception history bitmap and the inbound/outbound link costs, and
 * the Hello/IHU timeout timers.
 */
class INET_API BabelNeighbour : public cObject
{
  protected:
    BabelInterface *interface = nullptr;
    L3Address address;
    uint16_t history;
    uint16_t cost;
    uint16_t txcost;
    uint16_t expHSeqno;
    uint16_t neighHelloInterval;
    uint16_t neighIhuInterval;
    BabelTimer *neighHelloTimer = nullptr;
    BabelTimer *neighIhuTimer = nullptr;

  public:
    BabelNeighbour(BabelInterface *iface, const L3Address& addr, BabelTimer *nht, BabelTimer *nit) :
        interface(iface), address(addr), history(0), cost(COST_INF), txcost(COST_INF),
        expHSeqno(0), neighHelloInterval(0), neighIhuInterval(0),
        neighHelloTimer(nht), neighIhuTimer(nit)
    {
        ASSERT(iface != nullptr);
        ASSERT(nht != nullptr);
        ASSERT(nit != nullptr);
        neighHelloTimer->setContextPointer(this);
        neighIhuTimer->setContextPointer(this);
    }

    BabelNeighbour() :
        history(0), cost(COST_INF), txcost(COST_INF),
        expHSeqno(0), neighHelloInterval(0), neighIhuInterval(0) {}

    virtual ~BabelNeighbour();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelNeighbour& bn) { return os << bn.str(); }

    BabelInterface *getInterface() const { return interface; }
    void setInterface(BabelInterface *iface) { interface = iface; }

    const L3Address& getAddress() const { return address; }
    void setAddress(const L3Address& addr) { address = addr; }

    uint16_t getHistory() const { return history; }
    void setHistory(uint16_t h) { history = h; }

    uint16_t getCost() const { return cost; }
    void setCost(uint16_t c) { cost = c; }

    uint16_t getTxcost() const { return txcost; }
    void setTxcost(uint16_t t) { txcost = t; }

    uint16_t getExpHSeqno() const { return expHSeqno; }
    void setExpHSeqno(uint16_t ehsn) { expHSeqno = ehsn; }

    uint16_t getNeighHelloInterval() const { return neighHelloInterval; }
    void setNeighHelloInterval(uint16_t nhi) { neighHelloInterval = nhi; }

    uint16_t getNeighIhuInterval() const { return neighIhuInterval; }
    void setNeighIhuInterval(uint16_t nii) { neighIhuInterval = nii; }

    void noteReceive() { history = (history >> 1) | 0x8000; }
    void noteLoss() { history = history >> 1; }

    bool recomputeCost();
    uint16_t computeRxcost() const;

    void resetNHTimer();
    void resetNITimer();
    void resetNHTimer(double delay);
    void resetNITimer(double delay);
    void deleteNHTimer();
    void deleteNITimer();
};

class INET_API BabelNeighbourTable
{
  protected:
    std::vector<BabelNeighbour *> neighbours;

  public:
    virtual ~BabelNeighbourTable();

    std::vector<BabelNeighbour *>& getNeighbours() { return neighbours; }
    int getNumOfNeighOnIface(BabelInterface *iface);

    BabelNeighbour *findNeighbour(BabelInterface *iface, const L3Address& addr);
    BabelNeighbour *addNeighbour(BabelNeighbour *neigh);
    void removeNeighbour(BabelNeighbour *neigh);
    void removeNeighboursOnIface(BabelInterface *iface);
};

} // namespace babel
} // namespace inet

#endif
