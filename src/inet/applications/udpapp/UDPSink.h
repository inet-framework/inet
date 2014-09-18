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

#ifndef __INET_UDPSINK_H
#define __INET_UDPSINK_H

#include "inet/common/INETDefs.h"

#include "inet/applications/common/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

/**
 * Consumes and prints packets received from the UDP module. See NED for more info.
 */
class INET_API UDPSink : public ApplicationBase
{
  protected:
    enum SelfMsgKinds { START = 1, STOP };

    UDPSocket socket;
    int localPort;
    L3Address multicastGroup;
    simtime_t startTime;
    simtime_t stopTime;
    cMessage *selfMsg;

    int numReceived;
    static simsignal_t rcvdPkSignal;

  public:
    UDPSink();
    virtual ~UDPSink();

  protected:
    virtual void processPacket(cPacket *msg);
    virtual void setSocketOptions();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void finish();

    virtual void processStart();
    virtual void processStop();

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
};

} // namespace inet

#endif // ifndef __INET_UDPSINK_H

