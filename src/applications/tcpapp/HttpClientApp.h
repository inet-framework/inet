//
// Copyright (C) 2009 Kyeong Soo (Joseph) Kim
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_HTTPCLIENTAPP_H
#define __INET_HTTPCLIENTAPP_H

#include <omnetpp.h>
#include "TCPGenericCliAppBase.h"
//#include "TCPBasicClientApp.h"


/**
 * An example request-reply based HTTP client application.
 */
class INET_API HttpClientApp : public TCPGenericCliAppBase
{
  protected:
    simsignal_t sessionDelaySignal;
    simsignal_t sessionTransferRateSignal;
    
    cMessage *timeoutMsg;
    bool earlySend;	///< if true, don't wait with sendRequest() until established()
    bool htmlObjectRcvd;	///< if true, response to HTML object (i.e., 1st request) has been received
    bool warmupFinished;	///< if true, start statistics gathering
    int numEmbeddedObjects;	///< number of embedded objects per HTML object
    int numSessionsFinished;    ///< number of sessions finished (not just started)
    int bytesRcvdAtSessionStart;    ///< number of bytes received (so far) at the start of a session
									///< this is used to calculate session size and session transfer rate
									///< (i.e., session size to session delay ratio)
    int bytesRcvdAtSessionEnd;  ///< number of bytes received (so far) at the end of a session
                                ///< this is used to calculate average session throughput
    simtime_t sessionStart; ///< start time of a session
    double sumSessionDelays;    ///< sum of session delays
    double sumSessionSizes; ///< sum of session sizes in bytes
    double sumSessionTransferRates; ///< sum of session transfer rates in bytes/second

    /** Utility: sends a request to the server */
    virtual void sendRequest();

    /** Utility: sends an HTML request (1st of a session) to the server */
    virtual void sendHtmlRequest();

  public:
    HttpClientApp();
    virtual ~HttpClientApp();

  protected:
    /** Redefined to schedule a connect(). */
    virtual void initialize();

    /** Redefined. */
    virtual void finish();

    /** Redefined. */
    virtual void connect();

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
};

#endif
