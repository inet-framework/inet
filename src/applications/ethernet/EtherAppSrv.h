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

#ifndef __INET_ETHERAPPSRV_H
#define __INET_ETHERAPPSRV_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"
#include "NodeStatus.h"

#define MAX_REPLY_CHUNK_SIZE   1497


/**
 * Server-side process EtherAppCli.
 */
class INET_API EtherAppSrv : public InetSimpleModule, public ILifecycle
{
  protected:
    int localSAP;
    NodeStatus *nodeStatus;

    // statistics
    long packetsSent;
    long packetsReceived;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void startApp();
    virtual void stopApp();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual bool isNodeUp();
    void registerDSAP(int dsap);
    void sendPacket(cPacket *datapacket, const MACAddress& destAddr, int destSap);
};

#endif
