//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELTOPOLOGYTABLE_H
#define __INET_BABELTOPOLOGYTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/routing/babel/BabelDefs.h"
#include "inet/routing/babel/BabelNeighbourTable.h"

namespace inet {
namespace babel {

/**
 * One known route in the topology table (RFC 6126, section 3.2.5): a prefix as
 * advertised by a given originator and learned from a given neighbour (or, when
 * neighbour is null, a locally-originated route). The reported distance plus the
 * neighbour's cost yield the route metric.
 */
class INET_API BabelRoute : public cObject
{
  protected:
    netPrefix<L3Address> prefix;
    rid originator;
    BabelNeighbour *neighbour = nullptr; ///< null for a locally-originated route
    routeDistance rdistance;
    L3Address nexthop;
    bool selected = false;
    uint16_t updateInterval = 0;
    BabelTimer *expiryTimer = nullptr;
    BabelTimer *befExpiryTimer = nullptr;
    IRoute *rtentry = nullptr; ///< the installed routing-table entry, if any (not owned)

  public:
    BabelRoute() {}
    BabelRoute(const netPrefix<L3Address>& pre, BabelNeighbour *neigh, const rid& orig,
            const routeDistance& dist, const L3Address& nh, uint16_t ui, BabelTimer *et, BabelTimer *bet) :
        prefix(pre), originator(orig), neighbour(neigh), rdistance(dist), nexthop(nh),
        updateInterval(ui), expiryTimer(et), befExpiryTimer(bet)
    {
        if (expiryTimer != nullptr)
            expiryTimer->setContextPointer(this);
        if (befExpiryTimer != nullptr)
            befExpiryTimer->setContextPointer(this);
    }

    virtual ~BabelRoute();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelRoute& br) { return os << br.str(); }

    const netPrefix<L3Address>& getPrefix() const { return prefix; }
    void setPrefix(const netPrefix<L3Address>& p) { prefix = p; }

    const rid& getOriginator() const { return originator; }
    void setOriginator(const rid& o) { originator = o; }

    BabelNeighbour *getNeighbour() const { return neighbour; }
    void setNeighbour(BabelNeighbour *n) { neighbour = n; }

    routeDistance& getRDistance() { return rdistance; }
    const routeDistance& getRDistance() const { return rdistance; }
    void setRDistance(const routeDistance& rd) { rdistance = rd; }

    const L3Address& getNextHop() const { return nexthop; }
    void setNextHop(const L3Address& nh) { nexthop = nh; }

    bool getSelected() const { return selected; }
    void setSelected(bool s) { selected = s; }

    uint16_t getUpdateInterval() const { return updateInterval; }
    void setUpdateInterval(uint16_t ui) { updateInterval = ui; }

    BabelTimer *getETimer() const { return expiryTimer; }
    void setETimer(BabelTimer *et) { expiryTimer = et; }

    BabelTimer *getBETimer() const { return befExpiryTimer; }
    void setBETimer(BabelTimer *bet) { befExpiryTimer = bet; }

    IRoute *getRTEntry() const { return rtentry; }
    void setRTEntry(IRoute *rte) { rtentry = rte; }

    uint16_t metric() const;

    void resetETimer();
    void resetETimer(double delay);
    void resetBETimer();
    void resetBETimer(double delay);
    void deleteETimer();
    void deleteBETimer();
};

class INET_API BabelTopologyTable
{
  protected:
    std::vector<BabelRoute *> routes;

  public:
    virtual ~BabelTopologyTable();
    std::vector<BabelRoute *>& getRoutes() { return routes; }

    BabelRoute *findRoute(const netPrefix<L3Address>& p, BabelNeighbour *n, const rid& orig);
    BabelRoute *findRoute(const netPrefix<L3Address>& p, BabelNeighbour *n);
    BabelRoute *findSelectedRoute(const netPrefix<L3Address>& p);
    bool containShorterCovRoute(const netPrefix<L3Address>& p);
    BabelRoute *findRouteNotNH(const netPrefix<L3Address>& p, const L3Address& nh);
    BabelRoute *addRoute(BabelRoute *route);
    bool retractRoutesOnIface(BabelInterface *iface);
    bool removeRoute(BabelRoute *route);
    void removeRoutes();
};

} // namespace babel
} // namespace inet

#endif
