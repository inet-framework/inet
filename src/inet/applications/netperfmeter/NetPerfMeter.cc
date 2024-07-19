```cpp
// * --------------------------------------------------------------------------
// *
// *     //====//  //===== <===//===>  //====//
// *    //        //          //      //    //    SCTP Optimization Project
// *   //=====   //          //      //====//   ==============================
// *        //  //          //      //           University of Duisburg-Essen
// *  =====//  //=====     //      //
// *
// * --------------------------------------------------------------------------
//
// Copyright (C) 2009-2015 by Thomas Dreibholz <dreibh@iem.uni-due.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/netperfmeter/NetPerfMeter.h"

#include "inet/applications/netperfmeter/NetPerfMeter_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(NetPerfMeter);

//#define EV std::cout

// ###### Get pareto-distributed double random value ########################
// Parameters:
// location = the location parameter (also: scale parameter): x_m, x_min or m
// shape    = the shape parameter: alpha or k
//
// Based on rpareto from GNU R's VGAM package
// (http://cran.r-project.org/web/packages/VGAM/index.html):
// rpareto <- function (n, location, shape)
// {
//     ans <- location/runif(n)^(1/shape)
//     ans[location <= 0] <- NaN
//     ans[shape <= 0] <- NaN
//     ans
// }
//
// Some description:
// http://en.wikipedia.org/wiki/Pareto_distribution
//
// Mean: E(X) = shape*location / (shape - 1) for alpha > 1
// => location = E(X)*(shape - 1) / shape
//
static cNEDValue pareto(cComponent *context, cNEDValue argv[], int argc)
{
    const int rng = argc == 3 ? (int)argv[2] : 0;
    const double location = argv[0].doubleValueInUnit(argv[0].getUnit());
    const double shape = argv[1].doubleValueInUnit(argv[1].getUnit());

    const double r = RNGCONTEXT uniform(0.0, 1.0, rng);
    const double result = location / pow(r, 1.0 / shape);

//    printf("%1.6f  => %1.6f   (location=%1.6f shape=%1.6f)\n", r, result, location, shape);
    return cNEDValue(result, argv[0].getUnit());
}

Define_NED_Function(pareto, "quantity pareto(quantity location, quantity shape, long rng?)");

// ###### Constructor #######################################################
NetPerfMeter::NetPerfMeter()
    : ConnectTimer(nullptr), StartTimer(nullptr), ResetTimer(nullptr), StopTimer(nullptr),
      SocketSCTP(nullptr), IncomingSocketSCTP(nullptr), SocketTCP(nullptr), IncomingSocketTCP(nullptr), SocketUDP(nullptr)
{
    SendingAllowed = false;
    ConnectionID = 0;
    resetStatistics();
}

// ###### Destructor ########################################################
NetPerfMeter::~NetPerfMeter()
{
    cancelAndDelete(ConnectTimer);
    cancelAndDelete(StartTimer);
    cancelAndDelete(ResetTimer);
    cancelAndDelete(StopTimer);
    for (auto& elem : TransmitTimerVector) {
        cancelAndDelete(elem);
    }
    delete SocketUDP;
    delete SocketTCP;
    delete IncomingSocketTCP;
    delete SocketSCTP;
    delete IncomingSocketSCTP;
}

// ###### Parse vector of cDynamicExpression from string ####################
void NetPerfMeter::parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
                                         const char* string,
                                         const char* delimiters)
{
    expressionVector.clear();
    cStringTokenizer tokenizer(string, delimiters);
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        cDynamicExpression expression;
        expression.parse(token);
        expressionVector.push_back(expression);
    }
}

// ###### initialize() method ###############################################
void NetPerfMeter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // ====== Handle parameters ==============================================
        ActiveMode = par("activeMode");
        const char *protocolPar = par("protocol");
        if (strcmp(protocolPar, "TCP") == 0) {
            TransportProtocol = TCP;
        }
        else if (strcmp(protocolPar, "SCTP") == 0) {
            TransportProtocol = SCTP;
        }
        else if (strcmp(protocolPar, "UDP") == 0) {
            TransportProtocol = UDP;
        }
        else {
            throw cRuntimeError("Bad protocol setting!");
        }

        RequestedOutboundStreams = 1;
        MaxInboundStreams = 1;
        ActualOutboundStreams = 1;
        ActualInboundStreams = 1;
        LastStreamID = 1;
        MaxMsgSize = par("maxMsgSize");
        QueueSize = par("queueSize");
        DecoupleSaturatedStreams = par("decoupleSaturatedStreams");
        RequestedOutboundStreams = par("outboundStreams");
        if ((RequestedOutboundStreams < 1) || (RequestedOutboundStreams > 65535)) {
            throw cRuntimeError("Invalid number of outbound streams; use range from [1, 65535]");
        }
        MaxInboundStreams = par("maxInboundStreams");
        if ((MaxInboundStreams < 1) || (MaxInboundStreams > 65535)) {
            throw cRuntimeError("Invalid number of inbound streams; use range from [1, 65535]");
        }
        UnorderedMode = par("unordered");
        if ((UnorderedMode < 0.0) || (UnorderedMode > 1.0)) {
            throw cRuntimeError("Bad value for unordered probability; use range from [0.0, 1.0]");
        }
        UnreliableMode = par("unreliable");
        if ((UnreliableMode < 0.0) || (UnreliableMode > 1.0)) {
            throw cRuntimeError("Bad value for unreliable probability; use range from [0.0, 1.0]");
        }
        parseExpressionVector(FrameRateExpressionVector, par("frameRateString"), ";");
        parseExpressionVector(FrameSizeExpressionVector, par("frameSizeString"), ";");

        TraceIndex = ~0;
        if (strcmp(par("traceFile").stringValue(), "") != 0) {
            std::fstream traceFile(par("traceFile").stringValue());
            if (!traceFile.good()) {
                throw cRuntimeError("Unable to load trace file");
            }
            while (!traceFile.eof()) {
                TraceEntry traceEntry = {};

                char line[256];
                traceFile.getline(line, sizeof(line), '\n');
                if (sscanf(line, "%lf %u %u", &traceEntry.InterFrameDelay, &traceEntry.FrameSize, &traceEntry.StreamID) >= 2) {
//                    std::cout << "Frame: " << traceEntry.InterFrameDelay << "\t" << traceEntry.FrameSize << "\t" << traceEntry.StreamID << endl;
                    TraceVector.push_back(traceEntry);
                }
            }
        }
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // ====== Initialize and bind socket =====================================
        for (unsigned int i = 0; i < RequestedOutboundStreams; i++) {
            SenderStatistics *senderStatistics = new SenderStatistics;
            SenderStatisticsMap[i] = senderStatistics;
        }
        for (unsigned int i = 0; i < MaxInboundStreams; i++) {
            ReceiverStatistics *receiverStatistics = new ReceiverStatistics;
            ReceiverStatisticsMap[i] = receiverStatistics;
        }

        ConnectTime = par("connectTime");
        StartTime = par("startTime");
        ResetTime = par("resetTime");
        StopTime = par("stopTime");
        MaxOnOffCycles = par("maxOnOffCycles");

        HasFinished = false;
        OffTimer = nullptr;
        OnTimer = nullptr;
        OnOffCycleCounter = 0;

        EV << simTime() << ", " << getFullPath() << ": Initialize"
           << "\tConnectTime=" << ConnectTime
           << "\tStartTime=" << StartTime
           << "\tResetTime=" << ResetTime
           << "\tStopTime=" << StopTime
           << endl;

        if (!ActiveMode) {
            // Passive mode: create and bind socket immediately.
            // For active mode, the socket will be created just before connect().
            createAndBindSocket();
        }

        // ====== Schedule Connect Timer =========================================
        ConnectTimer = new cMessage("ConnectTimer", TIMER_CONNECT);
        scheduleAt(ConnectTime, ConnectTimer);
    }
}

// ###### finish() method ###################################################
void NetPerfMeter::finish()
{
    if (TransportProtocol == SCTP) {
    }
    else if (TransportProtocol == TCP) {
    }
    else if (TransportProtocol == UDP) {
    }

    for (auto& [key, value] : SenderStatisticsMap) {
        delete value;
    }
    SenderStatisticsMap.clear();

    for (auto& [key, value] : ReceiverStatisticsMap) {
        delete value;
    }
    ReceiverStatisticsMap.clear();
}

// ###### Show I/O status ###################################################
void NetPerfMeter::refreshDisplay() const
{
    unsigned long long totalSentBytes = 0;
    for (const auto& [key, value] : SenderStatisticsMap) {
        totalSentBytes += value->SentBytes;
    }

    unsigned long long totalReceivedBytes = 0;
    for (const auto& [key, value] : ReceiverStatisticsMap) {
        totalReceivedBytes += value->ReceivedBytes;
    }

    char status[64];
    snprintf(status, sizeof(status), "In: %llu, Out: %llu",
            totalReceivedBytes, totalSentBytes);
    getDisplayString().setTagArg("t", 0, status);
    // TODO also was setStatusString("Connecting"), setStatusString("Closed")
}

// ###### Handle timer ######################################################
void NetPerfMeter::handleTimer(cMessage *msg)
{
    // ====== Transmit timer =================================================
    if (auto transmitTimer = dynamic_cast<NetPerfMeterTransmitTimer *>(msg)) {
        TransmitTimerVector[transmitTimer->getStreamID()] = nullptr;
        if (!TraceVector.empty()) {
            sendDataOfTraceFile(QueueSize);
        }
        else {
            sendDataOfNonSaturatedStreams(QueueSize, transmitTimer->getStreamID());
        }
    }
    // ====== Off timer ======================================================
    else if (msg == OffTimer) {
        EV << simTime() << ", " << getFullPath() << ": Entering OFF mode" << endl;
        OffTimer = nullptr;
        stopSending();
    }
    // ====== On timer =======================================================
    else if (msg == OnTimer) {
        EV << simTime() << ", " << getFullPath() << ": Entering ON mode" << endl;
        OnTimer = nullptr;
        startSending();
    }
    // ====== Reset timer ====================================================
    else if (msg == ResetTimer) {
        EV << simTime() << ", " << getFullPath() << ": Reset" << endl;

        ResetTimer = nullptr;
        resetStatistics();

        ASSERT(StopTimer == nullptr);
        if (StopTime > 0.0) {
            StopTimer = new cMessage("StopTimer", TIMER_STOP);
            scheduleAfter(StopTime, StopTimer);
        }
    }
    // ====== Stop timer =====================================================
    else if (msg == StopTimer) {
        EV << simTime() << ", " << getFullPath() << ": STOP" << endl;

        StopTimer = nullptr;
        if (OffTimer) {
            cancelAndDelete(OffTimer);
            OffTimer = nullptr;
        }
        if (OnTimer) {
            cancelAndDelete(OnTimer);
            OnTimer = nullptr;
        }

        if (TransportProtocol == SCTP) {
            if (IncomingSocketSCTP != nullptr) {
                IncomingSocketSCTP->close();
            }
            else if (SocketSCTP != nullptr) {
                SocketSCTP->close();
            }
        }
        else if (TransportProtocol == TCP) {
            if (SocketTCP != nullptr) {
                SocketTCP->close();
            }
        }
        teardownConnection(true);
    }
    // ====== Start timer ====================================================
    else if (msg == StartTimer) {
        EV << simTime() << ", " << getFullPath() << ": Start" << endl;

        StartTimer = nullptr;
        startSending();
    }
    // ====== Connect timer ==================================================
    else if (msg == ConnectTimer) {
        EV << simTime() << ", " << getFullPath() << ": Connect" << endl;

        ConnectTimer = nullptr;
        establishConnection();
    }
}

// ###### Handle message ####################################################
void NetPerfMeter::handleMessage(cMessage *msg)
{
    // ====== Timer handling =================================================
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    // ====== SCTP ===========================================================
    else if (TransportProtocol == SCTP) {
        switch (msg->getKind()) {
            // ------ Data -----------------------------------------------------
            case SCTP_I_DATA:
                receiveMessage(msg);
                break;
            // ------ Data Arrival Indication ----------------------------------
            case SCTP_I_DATA_NOTIFICATION: {
                // Data has arrived -> request it from the SCTP module.
                auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
                auto dataIndication = tags.getTag<SctpCommandReq>();
                Packet *cmsg = new Packet("ReceiveRequest", SCTP_C_RECEIVE);
                auto cmd = cmsg->addTag<SctpSendReq>();
                cmd->setSocketId(dataIndication->getSocketId());
                cmd->setSid(dataIndication->getSid());
                cmsg->addTag<SocketReq>()->setSocketId(dataIndication->getSocketId());
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&inet::Protocol::sctp);
                send(cmsg, "socketOut");
                break;
            }
            // ------ Connection available -----------------------------------
            case SCTP_I_AVAILABLE: {
                EV_INFO << "SCTP_I_AVAILABLE arrived\n";
                Message *message = check_and_cast<Message *>(msg);
                int newSockId = message->getTag<SctpAvailableReq>()->getNewSocketId();
                EV_INFO << "new socket id = " << newSockId << endl;
                Request *cmsg = new Request("SCTP_C_ACCEPT_SOCKET_ID", SCTP_C_ACCEPT_SOCKET_ID);
                cmsg->addTag<SctpAvailableReq>()->setSocketId(newSockId);
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&inet::Protocol::sctp);
                cmsg->addTag<SocketReq>()->setSocketId(newSockId);
                EV_INFO << "Sending accept socket id request ..." << endl;
                send(cmsg, "socketOut");
                break;
            }
            // ------ Connection established -----------------------------------
            case SCTP_I_ESTABLISHED: {
                Message *message = check_and_cast<Message *>(msg);
                auto& tags = message->getTags();
                auto connectInfo = tags.getTag<SctpConnectReq>();
                ActualOutboundStreams = connectInfo->getOutboundStreams();
                if (ActualOutboundStreams > RequestedOutboundStreams) {
                    ActualOutboundStreams = RequestedOutboundStreams;
                }
                ActualInboundStreams = connectInfo->getInboundStreams();
                if (ActualInboundStreams > MaxInboundStreams) {
                    ActualInboundStreams = MaxInboundStreams;
                }
                LastStreamID = ActualOutboundStreams - 1;
                // NOTE: Start sending on stream 0!
                successfullyEstablishedConnection(msg, QueueSize);
                break;
            }
            // ------ Queue indication -----------------------------------------
            case SCTP_I_SENDQUEUE_ABATED: {
                Message *message = check_and_cast<Message *>(msg);
                auto& tags = message->getTags();
                const auto& sendQueueAbatedIndication = tags.getTag<SctpSendQueueAbatedReq>();
                ASSERT(sendQueueAbatedIndication != nullptr);
                // Queue is underfull again -> give it more data.
                SendingAllowed = true;
                if (TraceVector.empty()) {
                    sendDataOfSaturatedStreams(sendQueueAbatedIndication->getBytesAvailable(), sendQueueAbatedIndication);
                }
                break;
            }
            case SCTP_I_SENDQUEUE_FULL:
                SendingAllowed = false;
                break;
            // ------ Errors ---------------------------------------------------
            case SCTP_I_PEER_CLOSED:
            case SCTP_I_CLOSED:
            case SCTP_I_CONNECTION_REFUSED:
            case SCTP_I_CONNECTION_RESET:
            case SCTP_I_TIMED_OUT:
            case SCTP_I_ABORT:
            case SCTP_I_CONN_LOST:
            case SCTP_I_SHUTDOWN_RECEIVED:
                teardownConnection();
                break;
            default:
                break;
        }
    }
    // ====== TCP ============================================================
    else if (TransportProtocol == TCP) {
        switch (msg->getKind()) {
            // ------ Data -----------------------------------------------------
            case TCP_I_DATA:
            case TCP_I_URGENT_DATA:
                receiveMessage(msg);
                break;
            // ------ Connection available -----------------------------------
            case TCP_I_AVAILABLE: {
                EV_INFO << "TCP_I_AVAILABLE arrived\n";
                TcpAvailableInfo *availableInfo = check_and_cast<TcpAvailableInfo *>(msg->getControlInfo());
                int newSockId = availableInfo->getNewSocketId();
                EV_INFO << "new socket id = " << newSockId << endl;
                Request *cmsg = new Request("TCP_C_ACCEPT", TCP_C_ACCEPT);
                auto *acceptCmd = new TcpAcceptCommand();
                cmsg->setControlInfo(acceptCmd);
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&inet::Protocol::tcp);
                cmsg->addTag<SocketReq>()->setSocketId(newSockId);
                EV_INFO << "Sending accept socket id request ..." << endl;
                send(cmsg, "socketOut");
                break;
            }
            // ------ Connection established -----------------------------------
            case TCP_I_ESTABLISHED:
                successfullyEstablishedConnection(msg, 0);
                break;
            // ------ Queue indication -----------------------------------------
            case TCP_I_SEND_MSG: {
                const TcpCommand *tcpCommand = check_and_cast<TcpCommand *>(msg->getControlInfo());
                // Queue is underfull again -> give it more data.
                if (SocketTCP != nullptr) { // T.D. 16.11.2011: Ensure that there is still a TCP socket!
                    SendingAllowed = true;
                    if (TraceVector.empty()) {
                        sendDataOfSaturatedStreams(tcpCommand->getUserId(), nullptr);
                    }
                }
            }
            break;
            // ------ Errors ---------------------------------------------------
            case TCP_I_PEER_CLOSED:
            case TCP_I_CLOSED:
            case TCP_I_CONNECTION_REFUSED:
            case TCP_I_CONNECTION_RESET:
            case TCP_I_TIMED_OUT:
                teardownConnection();
                break;
        }
    }
    // ====== UDP =========================================================

