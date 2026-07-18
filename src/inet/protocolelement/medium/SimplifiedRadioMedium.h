//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLIFIEDRADIOMEDIUM_H
#define __INET_SIMPLIFIEDRADIOMEDIUM_H

#include <set>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/SimpleModule.h"

namespace inet {

/**
 * A minimal shared wireless medium, modelled on WireJunction: a frame transmitted on one radio is
 * broadcast (as a Signal) to every OTHER radio that is in range, after the connecting channel's
 * propagation delay. Two deliberate simplifications on top of the plain broadcast:
 *
 *  - `ranges`: an adjacency spec limiting which radios hear each other (e.g. a line), so multi-hop
 *    forwarding is physically necessary. Empty = fully connected.
 *  - `packetLossProbability`: each in-range delivery is independently dropped with this probability,
 *    which (via the missing acknowledgement) drives retransmission + backoff. Reproducible under a
 *    fixed RNG seed.
 *
 * It does NOT model SNR/capture or collisions (concurrent transmissions are simply both delivered);
 * that is the "simplified" in the name.
 */
class INET_API SimplifiedRadioMedium : public SimpleModule, protected cListener
{
  protected:
    struct TxInfo {
        long incomingTxId = -1;
        long outgoingPort = -1;
        long outgoingTxId = -1;
        simtime_t finishTime;
    };

    std::vector<TxInfo> txList;
    int numRadios = 0;
    int inputGateBaseId = -1;
    int outputGateBaseId = -1;
    double packetLossProbability = 0;
    std::vector<std::set<int>> inRange; // inRange[i] = radios that hear radio i (symmetric)

    long numMessages = 0;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void parseRanges(const char *rangesString);
    virtual bool areInRange(int i, int j) const { return inRange[i].find(j) != inRange[i].end(); }

    virtual void setChannelModes();
    virtual void setGateModes();
    virtual void addTxInfo(long incomingTxId, int port, long outgoingTxId, simtime_t finishTime);
    virtual void updateTxInfo(TxInfo *txInfo, simtime_t finishTime) { txInfo->finishTime = finishTime; }
    virtual TxInfo *findTxInfo(long incomingTxId, int port);
};

} // namespace inet

#endif
