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

#ifndef __INET_TCPECHOAPP_H
#define __INET_TCPECHOAPP_H

#include "inet/common/INETDefs.h"

#include "inet/applications/tcpapp/TcpServerHostApp.h"
#include "inet/common/INETMath.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */
class INET_API TcpEchoApp : public TcpServerHostApp
{
  protected:
    simtime_t delay;
    double echoFactor = NaN;

    long bytesRcvd = 0;
    long bytesSent = 0;

  protected:
    virtual void sendDown(Packet *packet);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;
    virtual void refreshDisplay() const override;

  public:
    TcpEchoApp();
    ~TcpEchoApp();

    friend class TcpEchoAppThread;
};

class INET_API TcpEchoAppThread : public TcpServerThreadBase
{
  protected:
    TcpEchoApp *echoAppModule = nullptr;

  public:
    /**
     * Called when connection is established.
     */
    virtual void established() override;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(Packet *msg, bool urgent) override;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void timerExpired(cMessage *timer) override;

    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket) override { TcpServerThreadBase::init(hostmodule, socket); echoAppModule = check_and_cast<TcpEchoApp *>(hostmod); }
};

} // namespace inet

#endif // ifndef __INET_TCPECHOAPP_H

