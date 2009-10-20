//
// Copyright (C) 2004-2009 Andras Varga; Kyeong Soo (Joseph) Kim
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

#include "HttpClientApp.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1


Define_Module(HttpClientApp);

void HttpClientApp::initialize()
{
    TCPBasicClientApp::initialize();

    numSessionsFinished = 0;
    sumSessionDelays = 0.0;
    sumSessionTransferRates = 0.0;
}

void HttpClientApp::connect()
{
	TCPBasicClientApp::connect();

	// Initialise per session.
	// Note that session delays will include connection (i.e., socket) set up time as well.
	// To exclude connection set up time, initialise this variable in "socketEstablished()".
	sessionStart = simTime();
	bytesRcvdAtSessionStart = bytesRcvd;
}

void HttpClientApp::socketClosed(int connId, void *ptr)
{
    TCPBasicClientApp::socketClosed(connId, ptr);

    // update session statistics
    numSessionsFinished++;
    double sessionDelay = SIMTIME_DBL(simTime() - sessionStart);
    int sessionSize = bytesRcvd - bytesRcvdAtSessionStart;
    sumSessionDelays += sessionDelay;
    sumSessionTransferRates += sessionSize/sessionDelay;
}

void HttpClientApp::finish()
{
	TCPBasicClientApp::finish();

    // record session statistics
    if (numSessionsFinished > 0) {
        double avgSessionDelay = sumSessionDelays/double(numSessionsFinished);
        double avgSessionThroughput = bytesRcvd/sumSessionDelays;
        double meanSessionTransferRate = sumSessionTransferRates/numSessionsFinished;

        recordScalar("number of finished sessions", numSessionsFinished);
        recordScalar("average session delay [s]", avgSessionDelay);
        recordScalar("average session throughput [B/s]", avgSessionThroughput);
        recordScalar("mean session transfer rate [B/s]", meanSessionTransferRate);

        EV << getFullPath() << ": closed " << numSessionsFinished << " sessions\n";
        EV << getFullPath() << ": experienced " << avgSessionDelay << " [s] average session delay\n";
        EV << getFullPath() << ": experienced " << avgSessionThroughput << " [B/s] average session throughput\n";
        EV << getFullPath() << ": experienced " << meanSessionTransferRate << " [B/s] mean session transfer rate\n";
    }
}
