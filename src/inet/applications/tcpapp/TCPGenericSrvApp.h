//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPGENERICSRVAPP_H
#define __INET_TCPGENERICSRVAPP_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * Generic server application. It serves requests coming in GenericAppMsg
 * request messages. Clients are usually subclassed from TCPAppBase.
 *
 * @see GenericAppMsg, TCPAppBase
 */
class INET_API TCPGenericSrvApp : public cSimpleModule, public ILifecycle
{
  protected:
    simtime_t delay;
    simtime_t maxMsgDelay;

    long msgsRcvd;
    long msgsSent;
    long bytesRcvd;
    long bytesSent;

    // statistics:
    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;

  protected:
    virtual void sendBack(cMessage *msg);
    virtual void sendOrSchedule(cMessage *msg, simtime_t delay);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }
};

} // namespace inet

#endif // ifndef __INET_TCPGENERICSRVAPP_H

