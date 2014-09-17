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

#ifndef __INET_TCPAPPBASE_H
#define __INET_TCPAPPBASE_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

namespace inet {

/**
 * Base class for clients app for TCP-based request-reply protocols or apps.
 * Handles a single session (and TCP connection) at a time.
 *
 * It needs the following NED parameters: localAddress, localPort, connectAddress, connectPort.
 */
class INET_API TCPAppBase : public cSimpleModule, public TCPSocket::CallbackInterface
{
  protected:
    TCPSocket socket;

    // statistics
    int numSessions;
    int numBroken;
    int packetsSent;
    int packetsRcvd;
    int bytesSent;
    int bytesRcvd;

    //statistics:
    static simsignal_t connectSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    /* Utility functions */
    virtual void connect();
    virtual void close();
    virtual void sendPacket(cPacket *pkt);
    virtual void setStatusString(const char *s);

    /* TCPSocket::CallbackInterface callback methods */
    virtual void handleTimer(cMessage *msg) = 0;
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) { delete status; }
};

} // namespace inet

#endif // ifndef __INET_TCPAPPBASE_H

