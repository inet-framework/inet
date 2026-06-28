//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELINTERFACETABLE_H
#define __INET_BABELINTERFACETABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/routing/babel/BabelCost.h"
#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

/**
 * Per-interface Babel state. The protocol runs on a set of the host's network
 * interfaces; this holds the timers, sequence number, intervals and address
 * families of one of them. Unlike the ANSA original, the UDP sockets live in
 * the Babel module (one shared socket), not here.
 */
class INET_API BabelInterface : public cObject
{
  protected:
    NetworkInterface *interface = nullptr; ///< the network interface
    int afsend; ///< address family used for sending (AF::IPvX / IPv4 / IPv6 / NONE)
    int afdist; ///< address family of distributed prefixes
    bool splitHorizon;
    bool wired;
    uint16_t helloSeqno;
    uint16_t helloInterval; ///< centiseconds
    uint16_t updateInterval; ///< centiseconds
    BabelTimer *helloTimer = nullptr;
    BabelTimer *updateTimer = nullptr;
    bool enabled;
    uint16_t nominalrxcost;
    std::vector<netPrefix<L3Address>> directlyconn;
    IBabelCostComputation *ccm = nullptr; ///< link-cost strategy (not owned)

  public:
    BabelInterface() :
        afsend(AF::IPv6), afdist(AF::IPv6), splitHorizon(false), wired(false),
        helloSeqno(0), helloInterval(defval::HELLO_INTERVAL_CS),
        updateInterval(defval::UPDATE_INTERVAL_MULT * defval::HELLO_INTERVAL_CS),
        enabled(false), nominalrxcost(defval::NOM_RXCOST_WIRED) {}

    BabelInterface(NetworkInterface *iface, uint16_t hellSeq) :
        interface(iface), afsend(AF::IPv6), afdist(AF::IPv6), splitHorizon(false), wired(false),
        helloSeqno(hellSeq), helloInterval(defval::HELLO_INTERVAL_CS),
        updateInterval(defval::UPDATE_INTERVAL_MULT * defval::HELLO_INTERVAL_CS),
        enabled(false), nominalrxcost(defval::NOM_RXCOST_WIRED)
    {
        ASSERT(iface != nullptr);
    }

    virtual ~BabelInterface();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelInterface& i) { return os << i.str(); }

    int getInterfaceId() const { return interface ? interface->getInterfaceId() : -1; }
    const char *getIfaceName() const { return interface ? interface->getInterfaceName() : "-"; }

    NetworkInterface *getInterface() const { return interface; }
    void setInterface(NetworkInterface *i) { interface = i; }

    int getAfSend() const { return afsend; }
    void setAfSend(int afs) { afsend = afs; }

    int getAfDist() const { return afdist; }
    void setAfDist(int afd) { afdist = afd; }

    bool getSplitHorizon() const { return splitHorizon; }
    void setSplitHorizon(bool sh) { splitHorizon = sh; }

    bool getWired() const { return wired; }
    void setWired(bool w) { wired = w; }

    uint16_t getHSeqno() const { return helloSeqno; }
    void setHSeqno(uint16_t s) { helloSeqno = s; }
    uint16_t getIncHSeqno() { return ++helloSeqno; }

    uint16_t getNominalRxcost() const { return nominalrxcost; }
    void setNominalRxcost(uint16_t nrx) { nominalrxcost = nrx; }

    uint16_t getHInterval() const { return helloInterval; }
    void setHInterval(uint16_t i) { helloInterval = i; }

    uint16_t getUInterval() const { return updateInterval; }
    void setUInterval(uint16_t i) { updateInterval = i; }

    BabelTimer *getHTimer() const { return helloTimer; }
    void setHTimer(BabelTimer *t) { helloTimer = t; }

    BabelTimer *getUTimer() const { return updateTimer; }
    void setUTimer(BabelTimer *t) { updateTimer = t; }

    bool getEnabled() const { return enabled; }
    void setEnabled(bool e) { enabled = e; }

    void addDirectlyConn(const netPrefix<L3Address>& pre);
    const std::vector<netPrefix<L3Address>>& getDirectlyConn() const { return directlyconn; }

    IBabelCostComputation *getCostComputationModule() const { return ccm; }
    void setCostComputationModule(IBabelCostComputation *cm) { ccm = cm; }

    void resetHTimer();
    void resetHTimer(double delay);
    void resetUTimer();
    void resetUTimer(double delay);
    void deleteHTimer();
    void deleteUTimer();
};

class INET_API BabelInterfaceTable
{
  protected:
    std::vector<BabelInterface *> interfaces;

  public:
    virtual ~BabelInterfaceTable();

    std::vector<BabelInterface *>& getInterfaces() { return interfaces; }
    BabelInterface *findInterfaceById(int ifaceId);
    BabelInterface *addInterface(BabelInterface *iface);
    void removeInterface(BabelInterface *iface);
    void removeInterface(int ifaceId);
};

} // namespace babel
} // namespace inet

#endif
