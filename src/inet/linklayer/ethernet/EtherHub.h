/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

#ifndef __INET_ETHERHUB_H
#define __INET_ETHERHUB_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"

namespace inet {

/**
 * Models a wiring hub. It simply broadcasts the received message
 * on all other ports.
 */
class INET_API EtherHub : public cSimpleModule, protected cListener
{
  public:
    struct PortInfo
    {
        std::set<int> forwardFromPorts;
        EthernetSignalBase *incomingSignal = nullptr;
        long incomingOrigId = -1;
        long outgoingOrigId = -1;
        simtime_t outgoingStartTime;
        bool outgoingCollision = false;
    };
  protected:
    std::vector<PortInfo> portInfos;

    int numPorts;    // sizeof(ethg)
    int inputGateBaseId;    // gate id of ethg$i[0]
    int outputGateBaseId;    // gate id of ethg$o[0]
    bool dataratesDiffer;
    double datarate = 0;

    // statistics
    long numMessages;    // number of messages handled

  public:
    virtual ~EtherHub();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void checkConnections(bool errorWhenAsymmetric);
    virtual void rxCutOnPort(int portIdx);
    virtual void cutActiveTxOnPort(int portIdx);
    virtual void copyIncomingsToPort(int outPort);
    virtual void cutSignalEnd(EthernetSignalBase* signal, simtime_t duration);
    virtual void forwardSignalFrom(int arrivalPort);
};

} // namespace inet

#endif // ifndef __INET_ETHERHUB_H

