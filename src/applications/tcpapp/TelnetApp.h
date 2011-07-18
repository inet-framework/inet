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

#ifndef __INET_TELNETAPP_H
#define __INET_TELNETAPP_H

#include "TCPGenericCliAppBase.h"


/**
 * An example Telnet client application. The server app should be TCPGenericSrvApp.
 */
class INET_API TelnetApp : public TCPGenericCliAppBase
{
  protected:
    cMessage *timeoutMsg;
    int numLinesToType; // lines (commands) the user will type in this session
    int numCharsToType; // characters the user will type for current line (command)
    simtime_t stopTime;

  public:
    TelnetApp();
    virtual ~TelnetApp();

  protected:
    /** Redefined initialize(int stage). Number of stages used from TCPgenericCliAppBase. */
    virtual void initialize(int stage);

    /** Redefined. */
    virtual void handleTimer(cMessage *msg);

    /** Redefined. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /** Redefined. */
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);

    /** Redefined to start another session after a delay. */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Redefined to reconnect after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    /** schedules msg only if t < stopTime */
    virtual int checkedScheduleAt(simtime_t t, cMessage *msg);
};

#endif

