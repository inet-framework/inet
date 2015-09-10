//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#ifndef __INET_IPVXTRAFGEN_H
#define __INET_IPVXTRAFGEN_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/applications/generic/IPvXTrafSink.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

/**
 * IP traffic generator application. See NED for more info.
 */
class INET_API IPvXTrafGen : public cSimpleModule, public ILifecycle
{
  protected:
    enum Kinds { START = 100, NEXT };

    // parameters: see the NED files for more info
    simtime_t startTime;
    simtime_t stopTime;
    cPar *sendIntervalPar = nullptr;
    cPar *packetLengthPar = nullptr;
    int protocol = -1;
    std::vector<L3Address> destAddresses;
    int numPackets = 0;

    // state
    NodeStatus *nodeStatus = nullptr;
    cMessage *timer = nullptr;
    bool isOperational = false;

    // statistic
    int numSent = 0;
    int numReceived = 0;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:
    virtual void scheduleNextPacket(simtime_t previous);
    virtual void cancelNextPacket();
    virtual bool isNodeUp();
    virtual bool isEnabled();

    virtual L3Address chooseDestAddr();
    virtual void sendPacket();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void startApp();

    virtual void printPacket(cPacket *msg);
    virtual void processPacket(cPacket *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  public:
    IPvXTrafGen();
    virtual ~IPvXTrafGen();
};

} // namespace inet

#endif // ifndef __INET_IPVXTRAFGEN_H

