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

#ifndef __REQUESTRESPONSEAPP_H_
#define __REQUESTRESPONSEAPP_H_

#include <omnetpp.h>
#include "TCPGenericCliAppBase.h"


/**
 * An example request-reply based client application.
 */
class RequestResponseApp : public TCPGenericCliAppBase
{
  protected:
    cMessage *timeoutMsg;
    int numRequestsToSend; // requests to send in this session

  public:
    Module_Class_Members(RequestResponseApp, TCPGenericCliAppBase, 0);

    /** Redefined to schedule a connect(). */
    virtual void initialize();

    /** Redefined. */
    virtual void handleTimer(cMessage *msg);

    /** Redefined. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /** Redefined. */
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);

    /** Redefined to start another session after a delay. */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Redefined to reconnect after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);
};

#endif


