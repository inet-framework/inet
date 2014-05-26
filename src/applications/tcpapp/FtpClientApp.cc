//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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

#include "FtpClientApp.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1


Define_Module(FtpClientApp);

FtpClientApp::FtpClientApp()
{
    timeoutMsg = NULL;
}

FtpClientApp::~FtpClientApp()
{
    cancelAndDelete(timeoutMsg);
}

void FtpClientApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    sessionDelaySignal = registerSignal("sessionDelay");
    sessionTransferRateSignal = registerSignal("sessionTransferRate");

    numSessionsFinished = 0;
    sumSessionDelays = 0.0;
    sumSessionSizes = 0.0;
    sumSessionTransferRates = 0.0;

    earlySend = false;  // TBD make it parameter
    warmupFinished = false;
    WATCH(earlySend);

    timeoutMsg = new cMessage("timer");
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt((simtime_t)par("startTime"), timeoutMsg);
}

void FtpClientApp::sendRequest()
{
     EV << "sending FTP request\n";

     long requestLength = par("requestLength");
     long replyLength = par("fileSize");
     if (requestLength<1) requestLength=1;
     if (replyLength<1) replyLength=1;

     sendPacket(requestLength, replyLength);
}

void FtpClientApp::connect()
{
	// Initialise per session.
	// Note that session delays will include connection (i.e., socket) set up time as well.
	// To exclude connection set up time, initialise this variable in "socketEstablished()".
	sessionStart = simTime();
	bytesRcvdAtSessionStart = bytesRcvd;

	TCPGenericCliAppBase::connect();
}

void FtpClientApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind())
    {
        case MSGKIND_CONNECT:
            EV << "starting a new FTP session\n";
            connect(); // active OPEN

            // significance of earlySend: if true, data will be sent already
            // in the ACK of SYN, otherwise only in a separate packet (but still
            // immediately)
            if (earlySend)
                sendRequest();
            break;

        default:
            error("Received an unknown message kind");
            break;
    }
}

void FtpClientApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    // Send the request for a file to download if not already done (next one will be sent when reply arrives)
    if (!earlySend)
        sendRequest();

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

void FtpClientApp::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    EV << "reply to the request (i.e., file) arrived, closing session\n";
    close();
}

void FtpClientApp::socketClosed(int connId, void *ptr)
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

void FtpClientApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("reconnectInterval"), timeoutMsg);
}

void FtpClientApp::finish()
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
