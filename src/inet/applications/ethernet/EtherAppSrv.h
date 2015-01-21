//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ETHERAPPSRV_H
#define __INET_ETHERAPPSRV_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

#define MAX_REPLY_CHUNK_SIZE    1497

/**
 * Server-side process EtherAppCli.
 */
class INET_API EtherAppSrv : public cSimpleModule, public ILifecycle
{
  protected:
    int localSAP = 0;
    NodeStatus *nodeStatus = nullptr;

    // statistics
    long packetsSent = 0;
    long packetsReceived = 0;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void startApp();
    virtual void stopApp();
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    virtual bool isNodeUp();
    void registerDSAP(int dsap);
    void sendPacket(cPacket *datapacket, const MACAddress& destAddr, int destSap);
};

} // namespace inet

#endif // ifndef __INET_ETHERAPPSRV_H

