//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "SCTPClient.h"

#include "AddressResolver.h"
#include "ModuleAccess.h"
#include "NodeStatus.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1
#define MSGKIND_ABORT    2
#define MSGKIND_PRIMARY  3
#define MSGKIND_RESET    4
#define MSGKIND_STOP     5


Define_Module(SCTPClient);

simsignal_t SCTPClient::sentPkSignal = registerSignal("sentPk");
simsignal_t SCTPClient::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t SCTPClient::echoedPkSignal = registerSignal("echoedPk");


SCTPClient::SCTPClient()
{
    timeMsg = NULL;
    stopTimer = NULL;
    primaryChangeTimer = NULL;
}

SCTPClient::~SCTPClient()
{
    cancelAndDelete(timeMsg);
    cancelAndDelete(stopTimer);
    cancelAndDelete(primaryChangeTimer);
}

void SCTPClient::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    sctpEV3 << "initialize SCTP Client stage "<< stage << endl;
    if (stage == INITSTAGE_LOCAL)
    {
        numSessions = numBroken = packetsSent = packetsRcvd = bytesSent = echoedBytesSent = bytesRcvd = 0;

        echo = par("echo").boolValue();
        ordered = par("ordered").boolValue();
        finishEndsSimulation = (bool)par("finishEndsSimulation");

        numRequestsToSend = 0;
        numPacketsToReceive = 0;
        queueSize = par("queueSize");
        WATCH(numRequestsToSend);
        recordScalar("ums", (uint32) par("requestLength"));

        timeMsg = new cMessage("CliAppTimer");
        timeMsg->setKind(MSGKIND_CONNECT);
        scheduleAt((simtime_t)par("startTime"), timeMsg);

        sendAllowed = true;
        bufferSize = 0;
        timer = false;
        stopTimer = NULL;
        primaryChangeTimer = NULL;

        WATCH(numSessions);
        WATCH(numBroken);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(bytesRcvd);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // parameters
        const char *addressesString = par("localAddress");
        AddressVector addresses = AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int32 port = par("localPort");

        if (addresses.size() == 0)
            socket.bind(port);
        else
            socket.bindx(addresses, port);

        socket.setCallbackObject(this);
        socket.setOutputGate(gate("sctpOut"));
        setStatusString("waiting");

        simtime_t stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO)
        {
            stopTimer = new cMessage("StopTimer");
            stopTimer->setKind(MSGKIND_STOP);
            scheduleAt((simtime_t)par("stopTime"), stopTimer);
            timer = true;
        }

        simtime_t primaryTime = par("primaryTime");
        if (primaryTime != SIMTIME_ZERO)
        {
            primaryChangeTimer = new cMessage("PrimaryTime");
            primaryChangeTimer->setKind(MSGKIND_PRIMARY);
            scheduleAt(primaryTime, primaryChangeTimer);
        }
    }
}

void SCTPClient::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleTimer(msg);
    }
    else
    {
        socket.processMessage(PK(msg));
    }
}

void SCTPClient::connect()
{
    const char *connectAddress = par("connectAddress");
    int32 connectPort = par("connectPort");
    inStreams = par("inboundStreams");
    outStreams = par("outboundStreams");
    socket.setInboundStreams(inStreams);
    socket.setOutboundStreams(outStreams);
    EV << "issuing OPEN command\n";
    setStatusString("connecting");
    EV << "connect to address " << connectAddress << "\n";
    bool streamReset = par("streamReset");
    socket.connect(AddressResolver().resolve(connectAddress, 1), connectPort, streamReset, (int32)par("prMethod"), (uint32)par("numRequestsPerSession"));

    if (!streamReset)
        streamReset = false;
    else if (streamReset == true)
    {
        cMessage* cmsg = new cMessage("StreamReset");
        cmsg->setKind(MSGKIND_RESET);
        sctpEV3 << "StreamReset Timer scheduled at " << simulation.getSimTime() << "\n";
        scheduleAt(simulation.getSimTime()+(double)par("streamRequestTime"), cmsg);
    }

    for (uint16 i = 0; i < outStreams; i++)
    {
        streamRequestLengthMap[i] = par("requestLength");
        streamRequestRatioMap[i] = 1;
        streamRequestRatioSendMap[i] = 1;
    }

    uint32 streamNum = 0;
    cStringTokenizer ratioTokenizer(par("streamRequestRatio").stringValue());
    while (ratioTokenizer.hasMoreTokens())
    {
        const char *token = ratioTokenizer.nextToken();
        streamRequestRatioMap[streamNum] = (uint32) atoi(token);
        streamRequestRatioSendMap[streamNum] = (uint32) atoi(token);

        streamNum++;
    }

    numSessions++;
}

void SCTPClient::close()
{
    setStatusString("closing");
    socket.close();
}


void SCTPClient::setStatusString(const char *s)
{
    if (ev.isGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void SCTPClient::socketEstablished(int32, void *, uint64 buffer )
{
    int32 count = 0;
    EV << "SCTPClient: connected\n";
    setStatusString("connected");
    bufferSize = buffer;
    // determine number of requests in this session
    numRequestsToSend = (long) par("numRequestsPerSession");
    numPacketsToReceive = (long) par("numPacketsToReceive");

    if (numRequestsToSend < 1)
        numRequestsToSend = 0;

    sctpEV3 << "SCTPClient:numRequestsToSend=" << numRequestsToSend << "\n";

    // perform first request (next one will be sent when reply arrives)
    if ((numRequestsToSend > 0 && !timer) || timer)
    {
        if ((simtime_t)par("thinkTime") > 0)
        {
            if (sendAllowed)
            {
                sendRequest();

                if (!timer)
                    numRequestsToSend--;
            }

            timeMsg->setKind(MSGKIND_SEND);
            scheduleAt(simulation.getSimTime()+(simtime_t)par("thinkTime"), timeMsg);
        }
        else
        {
            if (queueSize>0)
            {
                while (((!timer && numRequestsToSend > 0) || timer) && count++ < queueSize*2 && sendAllowed)
                {
                    if (count == queueSize*2)
                        sendRequest();
                    else
                        sendRequest(false);

                    if (!timer)
                    {
                        if (--numRequestsToSend == 0)
                            sendAllowed = false;
                    }
                }

                if (((!timer && numRequestsToSend>0) || timer) && sendAllowed)
                    sendQueueRequest();
            }
            else
            {
                while ((((!timer && numRequestsToSend>0) || timer) && sendAllowed && bufferSize>0) ||
                    (((!timer && numRequestsToSend>0) || timer) && sendAllowed && buffer==0))
                {
                    if (!timer && numRequestsToSend==1)
                        sendRequest(true);
                    else
                        sendRequest(false);

                    if (!timer && (--numRequestsToSend == 0))
                        sendAllowed = false;
                }
            }
        }

        if ((!timer && numPacketsToReceive == 0) && (simtime_t)par("waitToClose")>0)
        {
            timeMsg->setKind(MSGKIND_ABORT);
            scheduleAt(simulation.getSimTime()+(simtime_t)par("waitToClose"), timeMsg);
        }

        if ((!timer && numRequestsToSend == 0) && (simtime_t)par("waitToClose")==0)
        {
            sctpEV3 << "socketEstablished:no more packets to send, call shutdown\n";
            socket.shutdown();

            if (timeMsg->isScheduled())
                cancelEvent(timeMsg);

            if (finishEndsSimulation)
            {
                endSimulation();
            }
        }
    }
}

void SCTPClient::sendQueueRequest()
{
    cPacket* cmsg = new cPacket("Queue");
    SCTPInfo* qinfo = new SCTPInfo();
    qinfo->setText(queueSize);
    cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
    qinfo->setAssocId(socket.getConnectionId());
    cmsg->setControlInfo(qinfo);
    sctpEV3 << "Sending queue request ..." << endl;
    socket.sendRequest(cmsg);
}

void SCTPClient::sendRequestArrived()
{
    int32 count = 0;

    sctpEV3 << "sendRequestArrived numRequestsToSend=" << numRequestsToSend << "\n";

    while (((!timer && numRequestsToSend > 0) || timer) && count++ < queueSize && sendAllowed)
    {
        sendRequest(count == queueSize);

        if (!timer)
            numRequestsToSend--;

        if ((!timer && numRequestsToSend == 0))
        {
            sctpEV3 << "no more packets to send, call shutdown\n";
            socket.shutdown();

            if (timeMsg->isScheduled())
                cancelEvent(timeMsg);

            if (finishEndsSimulation)
                endSimulation();
        }
    }
}

void SCTPClient::socketDataArrived(int32, void *, cPacket *msg, bool)
{
    packetsRcvd++;

    sctpEV3 << "Client received packet Nr " << packetsRcvd << " from SCTP\n";
    SCTPCommand* ind = check_and_cast<SCTPCommand*>(msg->removeControlInfo());
    emit(rcvdPkSignal, msg);
    bytesRcvd += msg->getByteLength();

    if (echo)
    {
        // FIXME why do it: msg->dup(); delete msg;
        SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage*>(msg->dup());
        delete msg;
        cPacket* cmsg = new cPacket("SVData");
        echoedBytesSent += smsg->getByteLength();
        emit(echoedPkSignal, smsg);
        cmsg->encapsulate(smsg);
        cmsg->setKind(ind->getSendUnordered() ? SCTP_C_SEND_UNORDERED : SCTP_C_SEND_ORDERED);
        packetsSent++;
        socket.send(cmsg, 1);
    }

    if (par("numPacketsToReceive").longValue() > 0)
    {
        numPacketsToReceive--;
        if (numPacketsToReceive == 0)
            close();
    }

    delete ind;
}

void SCTPClient::sendRequest(bool last)
{
    uint32 i, sendBytes;

    sendBytes = par("requestLength");

    // find next stream
    uint16 nextStream = 0;
    for (uint16 i = 0; i < outStreams; i++)
    {
        if (streamRequestRatioSendMap[i] > streamRequestRatioSendMap[nextStream])
            nextStream = i;
    }

    // no stream left, reset map
    if (nextStream == 0 && streamRequestRatioSendMap[nextStream] == 0)
    {
        for (uint16 i = 0; i < outStreams; i++)
        {
            streamRequestRatioSendMap[i] = streamRequestRatioMap[i];
            if (streamRequestRatioSendMap[i] > streamRequestRatioSendMap[nextStream])
                nextStream = i;
        }
    }

    if (nextStream == 0 && streamRequestRatioSendMap[nextStream] == 0)
    {
        opp_error("Invalid setting of streamRequestRatio: only 0 weightings");
    }

    sendBytes = streamRequestLengthMap[nextStream];
    streamRequestRatioSendMap[nextStream]--;


    if (sendBytes < 1)
        sendBytes = 1;

    cPacket* cmsg = new cPacket("AppData");
    SCTPSimpleMessage* msg = new SCTPSimpleMessage("data");

    msg->setDataArraySize(sendBytes);

    for (i=0; i < sendBytes; i++)
        msg->setData(i, 'a');

    msg->setDataLen(sendBytes);
    msg->setEncaps(false);
    msg->setByteLength(sendBytes);
    msg->setCreationTime(simulation.getSimTime());
    cmsg->encapsulate(msg);
    cmsg->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);

    // send SCTPMessage with SCTPSimpleMessage enclosed
    sctpEV3 << "Sending request ..." << endl;
    bufferSize -= sendBytes;

    if (bufferSize < 0)
        last = true;

    emit(sentPkSignal, msg);
    socket.send(cmsg, par("prMethod"), par("prValue"), last);
    bytesSent += sendBytes;
}

void SCTPClient::handleTimer(cMessage *msg)
{
    switch (msg->getKind())
    {
        case MSGKIND_CONNECT:
            EV << "starting session call connect\n";
            connect();
            break;

        case MSGKIND_SEND:
            if (((!timer && numRequestsToSend>0) || timer))
            {
                if (sendAllowed)
                {
                    sendRequest();
                    if (!timer)
                        numRequestsToSend--;
                }
                if ((simtime_t)par("thinkTime") > 0)
                    scheduleAt(simulation.getSimTime()+(simtime_t)par("thinkTime"), timeMsg);

                if ((!timer && numRequestsToSend == 0) && (simtime_t)par("waitToClose")==0)
                {
                    socket.shutdown();
                    if (timeMsg->isScheduled())
                        cancelEvent(timeMsg);

                    if (finishEndsSimulation)
                    {
                        endSimulation();
                    }
                }
            }
            else if ((!timer && numRequestsToSend == 0) && (simtime_t)par("waitToClose")==0)
            {
                socket.shutdown();
                if (timeMsg->isScheduled())
                    cancelEvent(timeMsg);

                if (finishEndsSimulation)
                {
                    endSimulation();
                }
            }
            break;

        case MSGKIND_ABORT:
            close();
            break;

        case MSGKIND_PRIMARY:
            setPrimaryPath((const char*)par("newPrimary"));
            break;

        case MSGKIND_RESET:
            sctpEV3 << "StreamReset Timer expired at Client at " << simulation.getSimTime() << "...send notification\n";
            sendStreamResetNotification();
            delete msg;
            break;

        case MSGKIND_STOP:
            numRequestsToSend = 0;
            sendAllowed = false;
            socket.abort();
            socket.close();

            if (timeMsg->isScheduled())
                cancelEvent(timeMsg);

            socket.close();

            if (finishEndsSimulation)
            {
                endSimulation();
            }

            break;

        default:
            EV << "MsgKind =" << msg->getKind() << " unknown\n";
            break;
    }
}

void SCTPClient::socketDataNotificationArrived(int32 connId, void *ptr, cPacket *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cPacket* cmsg = new cPacket("CMSG-DataArr");
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    socket.sendNotification(cmsg);
}

void SCTPClient::shutdownReceivedArrived(int32 connId)
{
    if (numRequestsToSend == 0)
    {
        cPacket* cmsg = new cPacket("Request");
        SCTPInfo* qinfo = new SCTPInfo();
        cmsg->setKind(SCTP_C_NO_OUTSTANDING);
        qinfo->setAssocId(connId);
        cmsg->setControlInfo(qinfo);
        socket.sendNotification(cmsg);
    }
}

void SCTPClient::socketPeerClosed(int32, void *)
{
    // close the connection (if not already closed)
    if (socket.getState() == SCTPSocket::PEER_CLOSED)
    {
        EV << "remote SCTP closed, closing here as well\n";
        close();
    }
}

void SCTPClient::socketClosed(int32, void *)
{
    // *redefine* to start another session etc.
    EV << "connection closed\n";
    setStatusString("closed");

    if (primaryChangeTimer)
    {
        cancelEvent(primaryChangeTimer);
        delete primaryChangeTimer;
        primaryChangeTimer = NULL;
    }
}

void SCTPClient::socketFailure(int32, void *, int32 code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");
    numBroken++;
    // reconnect after a delay
    timeMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simulation.getSimTime()+(simtime_t)par("reconnectInterval"), timeMsg);
}

void SCTPClient::socketStatusArrived(int32 assocId, void *yourPtr, SCTPStatusInfo *status)
{
    struct pathStatus ps;
    SCTPPathStatus::iterator i = sctpPathStatus.find(status->getPathId());

    if (i != sctpPathStatus.end())
    {
        ps = i->second;
        ps.active = status->getActive();
    }
    else
    {
        ps.active = status->getActive();
        ps.pid = status->getPathId();
        ps.primaryPath = false;
        sctpPathStatus[ps.pid] = ps;
    }
}

void SCTPClient::setPrimaryPath(const char* str)
{

    cPacket* cmsg = new cPacket("CMSG-SetPrimary");
    SCTPPathInfo *pinfo = new SCTPPathInfo();

    if (strcmp(str, "") != 0)
    {
        pinfo->setRemoteAddress(Address(str));
    }
    else
    {
        str = (const char*)par("newPrimary");
        if (strcmp(str, "") != 0)
            pinfo->setRemoteAddress(Address(str));
        else
        {
            str = (const char*)par("connectAddress");
            pinfo->setRemoteAddress(Address(str));
        }
    }

    pinfo->setAssocId(socket.getConnectionId());
    cmsg->setKind(SCTP_C_PRIMARY);
    cmsg->setControlInfo(pinfo);
    socket.sendNotification(cmsg);
}

void SCTPClient::sendStreamResetNotification()
{
    uint32 type;

    type = (uint32)par("streamResetType");
    if (type >= 6 && type <= 9)
    {
        cPacket* cmsg = new cPacket("CMSG-SR");
        SCTPResetInfo *rinfo = new SCTPResetInfo();
        rinfo->setAssocId(socket.getConnectionId());
        rinfo->setRemoteAddr(socket.getRemoteAddr());
        type = (uint32)par("streamResetType");
        rinfo->setRequestType((uint16)type);
        cmsg->setKind(SCTP_C_STREAM_RESET);
        cmsg->setControlInfo(rinfo);
        socket.sendNotification(cmsg);
    }
}


void SCTPClient::msgAbandonedArrived(int32 assocId)
{
    chunksAbandoned++;
}


void SCTPClient::sendqueueFullArrived(int32 assocId)
{
    sendAllowed = false;
}

void SCTPClient::sendqueueAbatedArrived(int32 assocId, uint64 buffer)
{
    bufferSize = buffer;
    sendAllowed = true;

    while ((((!timer && numRequestsToSend>0) || timer) && sendAllowed && bufferSize>0) ||
            (((!timer && numRequestsToSend>0) || timer) && sendAllowed && buffer==0))
    {
        if (!timer && numRequestsToSend==1)
            sendRequest(true);
        else
            sendRequest(false);

        if (!timer && (--numRequestsToSend == 0))
            sendAllowed = false;
    }

    if ((!timer && numRequestsToSend == 0) && (simtime_t) par("waitToClose") == 0)
    {
        sctpEV3 << "socketEstablished:no more packets to send, call shutdown\n";
        socket.shutdown();

        if (timeMsg->isScheduled())
            cancelEvent(timeMsg);

        if (finishEndsSimulation)
        {
            endSimulation();
        }
    }
}

void SCTPClient::addressAddedArrived(int32 assocId, Address remoteAddr)
{
}

void SCTPClient::finish()
{
    EV << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
    sctpEV3 << "Client finished\n";
}

