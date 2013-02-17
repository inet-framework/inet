//
// Copyright (C) 2009 Kyeong Soo (Joseph) Kim
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

#include "HttpClientApp.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1


Define_Module(HttpClientApp);

HttpClientApp::HttpClientApp()
{
    timeoutMsg = NULL;
}

HttpClientApp::~HttpClientApp()
{
    cancelAndDelete(timeoutMsg);
}

void HttpClientApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    sessionDelaySignal = registerSignal("sessionDelay");
    sessionTransferRateSignal = registerSignal("sessionTransferRate");

    numEmbeddedObjects = 0;
    numSessionsFinished = 0;
    sumSessionDelays = 0.0;
    sumSessionSizes = 0.0;
    sumSessionTransferRates = 0.0;

    earlySend = false;  // TBD make it parameter
    htmlObjectRcvd = false;
    warmupFinished = false;
    WATCH(numEmbeddedObjects);
    WATCH(earlySend);

    timeoutMsg = new cMessage("timer");
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt((simtime_t)par("startTime"), timeoutMsg);
}

void HttpClientApp::sendRequest()
{
     EV << "sending request for an embedded object, " << numEmbeddedObjects-1 << " more to go\n";

     long requestLength = par("requestLength");
     long replyLength = par("embeddedObjectLength"); // this request is for an embedded object within an HTML object
     if (requestLength<1) requestLength=1;
     if (replyLength<1) replyLength=1;

     sendPacket(requestLength, replyLength);
}

void HttpClientApp::sendHtmlRequest()
{
     EV << "sending HTML request\n";

     long requestLength = par("requestLength");
     long replyLength = par("htmlObjectLength");	// this request is for an HTML object
     if (requestLength<1) requestLength=1;
     if (replyLength<1) replyLength=1;

     sendPacket(requestLength, replyLength);

     htmlObjectRcvd = false;
}

void HttpClientApp::connect()
{
	// Initialise per session.
	// Note that session delays will include connection (i.e., socket) set up time as well.
	// To exclude connection set up time, initialise this variable in "socketEstablished()".
	sessionStart = simTime();
	bytesRcvdAtSessionStart = bytesRcvd;

	TCPGenericCliAppBase::connect();
}

void HttpClientApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind())
    {
        case MSGKIND_CONNECT:
            EV << "starting a new HTTP session\n";
            connect(); // active OPEN

            // significance of earlySend: if true, data will be sent already
            // in the ACK of SYN, otherwise only in a separate packet (but still
            // immediately)
            if (earlySend)
                sendHtmlRequest(); // note that the first request is for an HTML object!
            break;

        case MSGKIND_SEND:
           sendRequest();
           numEmbeddedObjects--;
           // no scheduleAt(): next request will be sent when reply to this one
           // arrives (see socketDataArrived())
           break;
    }
}

void HttpClientApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    // Determine the number of embedded objects in an HTML object,
    // for which requests are to be sent in this session.
    numEmbeddedObjects = (long) par("numEmbeddedObjects");
    if (numEmbeddedObjects<0) numEmbeddedObjects=0;

    // Send the first request if not already done (next one will be sent when reply arrives)
    // Note that the first request in HTTP is special in that it is for an HTML object;
    // following requests are for embedded objects.
    // So we use a dedicated function for this first request (i.e., "sendHtmlRequest").
    if (!earlySend)
        sendHtmlRequest();

    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod())
        {
            warmupFinished = true;
            numSessions = 1;
            numBroken = 0;
        }
	}
}

void HttpClientApp::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    if (htmlObjectRcvd)
    {   // this is the 2nd (or later) response to embedded object.
        if (numEmbeddedObjects > 0)
        {
            EV << "reply for embedded object arrived\n";
            timeoutMsg->setKind(MSGKIND_SEND);
            scheduleAt(simTime()+(simtime_t)par("thinkTime"), timeoutMsg);
        }
        else
        {
            EV << "reply to the last request arrived, closing session\n";
            close();
        }
    }
    else
    {   // this is the response to HTML object (i.e., 1st response from the server).
        EV << "reply for HTML object arrived\n";
        if (numEmbeddedObjects > 0)
        {
            timeoutMsg->setKind(MSGKIND_SEND);
            scheduleAt(simTime()+(simtime_t)par("parsingTime"), timeoutMsg);
            htmlObjectRcvd = true;
        }
        else
        {
            EV << "no embedded object, closing session\n";
            close();
        }
    }
}

void HttpClientApp::socketClosed(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketClosed(connId, ptr);

    if (warmupFinished == true)
	{
        bytesRcvdAtSessionEnd = bytesRcvd;
        double sessionDelay = SIMTIME_DBL(simTime() - sessionStart);
        long sessionSize = bytesRcvdAtSessionEnd - bytesRcvdAtSessionStart;

		// update session statistics
		numSessionsFinished++;
		sumSessionSizes += sessionSize;	///< counting the size of sessions only after the warm-up period
		sumSessionDelays += sessionDelay;
		sumSessionTransferRates += sessionSize / sessionDelay;

        // emit statistics signals
        emit(sessionDelaySignal, sessionDelay);
        emit(sessionTransferRateSignal, sessionSize / sessionDelay);
	}

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("idleInterval"), timeoutMsg);
}

void HttpClientApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("reconnectInterval"), timeoutMsg);
}

void HttpClientApp::finish()
{
	TCPGenericCliAppBase::finish();

    // record session statistics
    if (numSessionsFinished > 0)
    {
        double avgSessionDelay = sumSessionDelays/double(numSessionsFinished);
        double avgSessionThroughput = sumSessionSizes/sumSessionDelays;
        double meanSessionTransferRate = sumSessionTransferRates/numSessionsFinished;

        recordScalar("number of finished sessions", numSessionsFinished);
        recordScalar("number of broken sessions", numBroken);
        recordScalar("average session delay [s]", avgSessionDelay);
        recordScalar("average session throughput [B/s]", avgSessionThroughput);
        recordScalar("mean session transfer rate [B/s]", meanSessionTransferRate);

        EV << getFullPath() << ": closed " << numSessionsFinished << " sessions\n";
        EV << getFullPath() << ": experienced " << avgSessionDelay << " [s] average session delay\n";
        EV << getFullPath() << ": experienced " << avgSessionThroughput << " [B/s] average session throughput\n";
        EV << getFullPath() << ": experienced " << meanSessionTransferRate << " [B/s] mean session transfer rate\n";
    }
}
