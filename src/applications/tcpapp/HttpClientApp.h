//
// Copyright (C) 2004-2009 Andras Varga; Kyeong Soo (Joseph) Kim
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

#include "TCPBasicClientApp.h"


/**
 * An example request-reply based HTTP client application.
 */
class INET_API HttpClientApp : public TCPBasicClientApp
{
  protected:
      int numSessionsFinished;      // number of sessions finished (not just started)
	  int bytesRcvdAtSessionStart;	// number of bytes received (so far) at the start of a session
									// this is used to calculate session size and session transfer rate
									// (i.e., session size to session delay ratio)
	  simtime_t sessionStart;	// start time of a session
	  double sumSessionDelays;	// sum of session delays
	  double sumSessionTransferRates;	// sum of session transfer rates in bytes/second

    /** Utility: sends a HTML request to the server */
    virtual void sendHtmlRequest();

  protected:
    /** Redefined to schedule a connect(). */
    virtual void initialize();

    /** Redefined. */
    virtual void finish();

    /** Redefined. */
    virtual void connect();

    /** Redefined. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /** Redefined to start another session after a delay. */
    virtual void socketClosed(int connId, void *yourPtr);
};

#endif
