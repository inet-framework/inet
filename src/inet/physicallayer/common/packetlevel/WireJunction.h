/*
 * Copyright (C) 2020 Opensim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INET_WIREJUNCTION_H
#define __INET_WIREJUNCTION_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

/**
 * Models a wiring hub. It simply broadcasts the received message
 * (which can be a transmission update, requiring special attention)
 * on all other ports.
 *
 * Note that no special attention is made to appropriately model
 * frame truncations that occur when the link breaks, or comes
 * back up while a transmission is underway. This is mainly so
 * because being a generic model, WireJunction doesn't know how
 * to produce sliced (truncated) versions of frames. If you need
 * to precisely model what happens when the link state changes,
 * you cannot use this class.
 */
class INET_API WireJunction : public cSimpleModule, protected cListener
{
  protected:
    struct TxInfo {
        long incomingTxId = -1;
        long outgoingPort = -1;
        long outgoingTxId = -1;
        simtime_t finishTime;
    };

  protected:
    std::vector<TxInfo> txList;
    int numPorts;    // sizeof(port)
    int inputGateBaseId;  // gate id of port$i[0]
    int outputGateBaseId; // gate id of port$o[0]

    // statistics
    long numMessages;    // number of messages handled

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void setChannelModes();
    virtual void setGateModes();
    virtual void addTxInfo(long incomingTxId, int port, long outgoingTxId, simtime_t finishTime);
    virtual void updateTxInfo(TxInfo *txInfo, simtime_t finishTime) {txInfo->finishTime = finishTime;}
    virtual TxInfo *findTxInfo(long incomingTxId, int port);
};

} //namespace physicallayer
} // namespace inet

#endif

