//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_TCPGENERICCLIAPPBASE_H
#define __INET_TCPGENERICCLIAPPBASE_H

#include <omnetpp.h>
#include "TCPSocket.h"


/**
 * Base class for clients app for TCP-based request-reply protocols or apps.
 * Handles a single session (and TCP connection) at a time.
 *
 * It needs the following NED parameters: address, port, connectAddress, connectPort.
 *
 * Generally used together with GenericAppMsg and TCPGenericSrvApp.
 */
class INET_API TCPGenericCliAppBase : public cSimpleModule, public TCPSocket::CallbackInterface
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

  protected:
    /**
     * Initialization. Should be redefined to perform or schedule a connect().
     */
    virtual void initialize();

    /**
     * For self-messages it invokes handleTimer(); messages arriving from TCP
     * will get dispatched to the socketXXX() functions.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Records basic statistics: numSessions, packetsSent, packetsRcvd,
     * bytesSent, bytesRcvd. Redefine to record different or more statistics
     * at the end of the simulation.
     */
    virtual void finish();

    /** @name Utility functions */
    //@{
    /** Issues an active OPEN to the address/port given as module parameters */
    virtual void connect();

    /** Issues CLOSE command */
    virtual void close();

    /** Sends a GenericAppMsg of the given length */
    virtual void sendPacket(int numBytes, int expectedReplyBytes, bool serverClose=false);

    /** When running under GUI, it displays the given string next to the icon */
    virtual void setStatusString(const char *s);
    //@}

    /** Invoked from handleMessage(). Should be redefined to handle self-messages. */
    virtual void handleTimer(cMessage *msg) = 0;

    /** @name TCPSocket::CallbackInterface callback methods */
    //@{
    /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /**
     * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
     * Beware: this funcion deletes the incoming message, which might not be what you want.
     */
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);

    /** Since remote TCP closed, invokes close(). Redefine if you want to do something else. */
    virtual void socketPeerClosed(int connId, void *yourPtr);

    /** Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection. */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    /** Redefine to handle incoming TCPStatusInfo. */
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    //@}
};

#endif


