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


#include "TCPBasicClientApp.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1


Define_Module(TCPBasicClientApp);

TCPBasicClientApp::TCPBasicClientApp()
{
    timeoutMsg = NULL;
}

TCPBasicClientApp::~TCPBasicClientApp()
{
    cancelAndDelete(timeoutMsg);
}

void TCPBasicClientApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    timeoutMsg = new cMessage("timer");

    numRequestsToSend = 0;
    earlySend = false;  // TBD make it parameter
    WATCH(numRequestsToSend);
    WATCH(earlySend);

    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt((simtime_t)par("startTime"), timeoutMsg);
}

void TCPBasicClientApp::sendRequest()
{
     EV << "sending request, " << numRequestsToSend-1 << " more to go\n";

     long requestLength = par("requestLength");
     long replyLength = par("replyLength");
     if (requestLength<1) requestLength=1;
     if (replyLength<1) replyLength=1;

     sendPacket(requestLength, replyLength);
}

void TCPBasicClientApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind())
    {
        case MSGKIND_CONNECT:
            EV << "starting session\n";
            connect(); // active OPEN

            // significance of earlySend: if true, data will be sent already
            // in the ACK of SYN, otherwise only in a separate packet (but still
            // immediately)
            if (earlySend)
                sendRequest();
            break;

        case MSGKIND_SEND:
           sendRequest();
           numRequestsToSend--;
           // no scheduleAt(): next request will be sent when reply to this one
           // arrives (see socketDataArrived())
           break;
    }
}

void TCPBasicClientApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    // determine number of requests in this session
    numRequestsToSend = (long) par("numRequestsPerSession");
    if (numRequestsToSend<1) numRequestsToSend=1;

    // perform first request if not already done (next one will be sent when reply arrives)
    if (!earlySend)
        sendRequest();
    numRequestsToSend--;
}

void TCPBasicClientApp::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    if (numRequestsToSend>0)
    {
        EV << "reply arrived\n";
        timeoutMsg->setKind(MSGKIND_SEND);
        scheduleAt(simTime()+(simtime_t)par("thinkTime"), timeoutMsg);
    }
    else
    {
        EV << "reply to last request arrived, closing session\n";
        close();
    }
}

void TCPBasicClientApp::socketClosed(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketClosed(connId, ptr);

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("idleInterval"), timeoutMsg);
}

void TCPBasicClientApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("reconnectInterval"), timeoutMsg);
}

