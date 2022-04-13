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
{
    SendingAllowed = false;
    ConnectionID = 0;
    resetStatistics();
}

// ###### Destructor ########################################################
NetPerfMeter::~NetPerfMeter()
{
    cancelAndDelete(ConnectTimer);
    ConnectTimer = nullptr;
    cancelAndDelete(StartTimer);
    StartTimer = nullptr;
    cancelAndDelete(ResetTimer);
    ResetTimer = nullptr;
    cancelAndDelete(StopTimer);
    StopTimer = nullptr;
    for (auto& elem : TransmitTimerVector) {
        cancelAndDelete(elem);
        elem = nullptr;
    }
    if (SocketUDP != nullptr) {
        delete SocketUDP;
        SocketUDP = nullptr;
    }
    if (SocketTCP != nullptr) {
        delete SocketTCP;
        SocketTCP = nullptr;
    }
    if (IncomingSocketTCP != nullptr) {
        delete IncomingSocketTCP;
        IncomingSocketTCP = nullptr;
    }
    if (SocketSCTP != nullptr) {
        delete SocketSCTP;
        SocketSCTP = nullptr;
    }
    if (IncomingSocketSCTP != nullptr) {
        delete IncomingSocketSCTP;
        IncomingSocketSCTP = nullptr;
    }
}

// ###### Parse vector of cDynamicExpression from string ####################
void NetPerfMeter::parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
                                         const char*                      string,
                                         const char*                      delimiters)
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
                TraceEntry traceEntry;
                traceEntry.InterFrameDelay = 0;
                traceEntry.FrameSize = 0;
                traceEntry.StreamID = 0;

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
        SocketSCTP = IncomingSocketSCTP = nullptr;
        SocketTCP = IncomingSocketTCP = nullptr;
        SocketUDP = nullptr;

        for (unsigned int i = 0; i < RequestedOutboundStreams; i++) {
            SenderStatistics *senderStatistics = new SenderStatistics;
            SenderStatisticsMap.insert(std::pair<unsigned int, SenderStatistics *>(i, senderStatistics));
        }
        for (unsigned int i = 0; i < MaxInboundStreams; i++) {
            ReceiverStatistics *receiverStatistics = new ReceiverStatistics;
            ReceiverStatisticsMap.insert(std::pair<unsigned int, ReceiverStatistics *>(i, receiverStatistics));
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

        if (ActiveMode == false) {
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

    std::map<unsigned int, SenderStatistics *>::iterator senderStatisticsIterator = SenderStatisticsMap.begin();
    while (senderStatisticsIterator != SenderStatisticsMap.end()) {
        delete senderStatisticsIterator->second;
        SenderStatisticsMap.erase(senderStatisticsIterator);
        senderStatisticsIterator = SenderStatisticsMap.begin();
    }
    std::map<unsigned int, ReceiverStatistics *>::iterator receiverStatisticsIterator = ReceiverStatisticsMap.begin();
    while (receiverStatisticsIterator != ReceiverStatisticsMap.end()) {
        delete receiverStatisticsIterator->second;
        ReceiverStatisticsMap.erase(receiverStatisticsIterator);
        receiverStatisticsIterator = ReceiverStatisticsMap.begin();
    }
}

// ###### Show I/O status ###################################################
void NetPerfMeter::refreshDisplay() const
{
    unsigned long long totalSentBytes = 0;
    for (std::map<unsigned int, SenderStatistics *>::const_iterator iterator = SenderStatisticsMap.begin();
         iterator != SenderStatisticsMap.end(); iterator++)
    {
        const SenderStatistics *senderStatistics = iterator->second;
        totalSentBytes += senderStatistics->SentBytes;
    }

    unsigned long long totalReceivedBytes = 0;
    for (std::map<unsigned int, ReceiverStatistics *>::const_iterator iterator = ReceiverStatisticsMap.begin();
         iterator != ReceiverStatisticsMap.end(); iterator++)
    {
        const ReceiverStatistics *receiverStatistics = iterator->second;
        totalReceivedBytes += receiverStatistics->ReceivedBytes;
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
    const NetPerfMeterTransmitTimer *transmitTimer = dynamic_cast<NetPerfMeterTransmitTimer *>(msg);
    if (transmitTimer) {
        TransmitTimerVector[transmitTimer->getStreamID()] = nullptr;
        if (TraceVector.size() > 0) {
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

        assert(StopTimer == nullptr);
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
//                SctpInfoReq* command = new SctpInfoReq("SendCommand");
//                SctpInfoReq *command = new SctpInfoReq();
//                command->setSocketId(dataIndication->getSocketId());
//                command->setSid(dataIndication->getSid());
//                command->setNumMsgs(dataIndication->getNumMsgs());
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
                assert(sendQueueAbatedIndication != nullptr);
                // Queue is underfull again -> give it more data.
                SendingAllowed = true;
                if (TraceVector.size() == 0) {
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
//                printf("SCTP: kind=%d\n", msg->getKind());
                break;
        }
    }
    // ====== TCP ============================================================
    else if (TransportProtocol == TCP) {
        short kind = msg->getKind();
        switch (kind) {
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
                    if (TraceVector.size() == 0) {
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
    // ====== UDP ============================================================
    else if (TransportProtocol == UDP) {
        switch (msg->getKind()) {
            // ------ Data -----------------------------------------------------
            case UDP_I_DATA:
                receiveMessage(msg);
                break;
            // ------ Error ----------------------------------------------------
            case UDP_I_ERROR:
                teardownConnection();
                break;
        }
    }

    delete msg;
}

// ###### Establish connection ##############################################
void NetPerfMeter::establishConnection()
{
    const char *remoteAddress = par("remoteAddress");
    const int remotePort = par("remotePort");

    // ====== Establish connection ===========================================
    if (ActiveMode == true) {
        createAndBindSocket();

        const char *primaryPath = par("primaryPath");
        PrimaryPath = (primaryPath[0] != 0x00) ? L3AddressResolver().resolve(primaryPath) : L3Address();

        if (TransportProtocol == SCTP) {
            AddressVector remoteAddressList;
            remoteAddressList.push_back(L3AddressResolver().resolve(remoteAddress));
            SocketSCTP->connectx(remoteAddressList, remotePort);
        }
        else if (TransportProtocol == TCP) {
            SocketTCP->renewSocket();
            SocketTCP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
        }
        else if (TransportProtocol == UDP) {
            SocketUDP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
            // Just start sending, since UDP is connection-less
            successfullyEstablishedConnection(nullptr, 0);
        }
        ConnectionEstablishmentTime = simTime();
    }
    else {
        // ------ Handle UDP on passive side ----------------
        if (TransportProtocol == UDP) {
//            SocketUDP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
            successfullyEstablishedConnection(nullptr, 0);
        }
    }
    EV << simTime() << ", " << getFullPath() << ": Sending allowed" << endl;
    SendingAllowed = true;
}

// ###### Connection has been established ###################################
void NetPerfMeter::successfullyEstablishedConnection(cMessage *msg,
                                                     const unsigned int queueSize)
{
    if (HasFinished) {
        EV << "Already finished -> no new connection!" << endl;
        SctpSocket newSocket(msg);
        newSocket.abort();
        return;
    }

    // ====== Update queue size ==============================================
    if (queueSize != 0) {
        QueueSize = queueSize;
        EV << simTime() << ", " << getFullPath() << ": Got queue size " << QueueSize << " from transport protocol" << endl;
    }

    // ====== Get connection ID ==============================================
    if (TransportProtocol == TCP) {
        if (ActiveMode == false) {
            assert(SocketTCP != nullptr);
            if (IncomingSocketTCP != nullptr) {
                delete IncomingSocketTCP;
            }
            IncomingSocketTCP = new TcpSocket(msg);
            IncomingSocketTCP->setOutputGate(gate("socketOut"));
        }

        ConnectionID = check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId();
        sendTCPQueueRequest(QueueSize); // Limit the send queue as given.
    }
    else if (TransportProtocol == SCTP) {
        if (ActiveMode == false) {
            assert(SocketSCTP != nullptr);
            if (IncomingSocketSCTP != nullptr) {
                delete IncomingSocketSCTP;
            }
            IncomingSocketSCTP = new SctpSocket(msg);
            IncomingSocketSCTP->setOutputGate(gate("socketOut"));
        }

        ConnectionID = check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId();
        sendSCTPQueueRequest(QueueSize); // Limit the send queue as given.
    }

    // ====== Initialize TransmitTimerVector =================================
    TransmitTimerVector.resize(ActualOutboundStreams);
    for (unsigned int i = 0; i < ActualOutboundStreams; i++) {
        TransmitTimerVector[i] = nullptr;
    }

    // ====== Schedule Start Timer to begin transmission =====================
    if (OnOffCycleCounter == 0) {
        assert(StartTimer == nullptr);
        StartTimer = new cMessage("StartTimer", TIMER_START);
        TransmissionStartTime = ConnectTime + StartTime;
        if (TransmissionStartTime < simTime()) {
            throw cRuntimeError("Connection establishment has been too late. Check startTime parameter!");
        }
        scheduleAt(TransmissionStartTime, StartTimer);

        // ====== Schedule Reset Timer to reset statistics ====================
        assert(ResetTimer == nullptr);
        ResetTimer = new cMessage("ResetTimer", TIMER_RESET);
        StatisticsResetTime = ConnectTime + ResetTime;
        scheduleAt(StatisticsResetTime, ResetTimer);
    }
    else {
        // ====== Restart transmission immediately ============================
        StartTimer = new cMessage("StartTimer");
        scheduleAfter(SIMTIME_ZERO, StartTimer);
    }
}

// ###### Start sending #####################################################
void NetPerfMeter::startSending()
{
    if (TraceVector.size() > 0) {
        sendDataOfTraceFile(QueueSize);
    }
    else {
        for (unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
            sendDataOfNonSaturatedStreams(QueueSize, streamID);
        }
        sendDataOfSaturatedStreams(QueueSize, nullptr);
    }

    // ------ On/Off handling ------------------------------------------------
    const simtime_t onTime = par("onTime");
    if (onTime.dbl() > 0.0) {
        OffTimer = new cMessage("OffTimer", TIMER_OFF);
        scheduleAfter(onTime, OffTimer);
    }
}

// ###### Stop sending ######################################################
void NetPerfMeter::stopSending()
{
    // ------ Stop all transmission timers ----------------------------------
    for (auto& elem : TransmitTimerVector) {
        cancelAndDelete(elem);
        elem = nullptr;
    }
    OnOffCycleCounter++;

    // ------ Schedule On timer ----------------------------------------------
    const simtime_t offDuration = par("offTime");
    if ((offDuration.dbl() > 0.0)
        && ((MaxOnOffCycles < 0) || (OnOffCycleCounter <= (unsigned int)MaxOnOffCycles)))
    {
        OnTimer = new cMessage("OnTimer", TIMER_ON);
        scheduleAfter(offDuration, OnTimer);
    }
}

// ###### Create and bind socket ############################################
void NetPerfMeter::createAndBindSocket()
{
    const char *localAddress = par("localAddress");
    const int localPort = par("localPort");
    if ((ActiveMode == false) && (localPort == 0)) {
        throw cRuntimeError("No local port number given in active mode!");
    }
    L3Address localAddr;
    if (*localAddress)
        localAddr = L3AddressResolver().resolve(localAddress);

    if (TransportProtocol == SCTP) {
        assert(SocketSCTP == nullptr);
        SocketSCTP = new SctpSocket;
        SocketSCTP->setInboundStreams(MaxInboundStreams);
        SocketSCTP->setOutboundStreams(RequestedOutboundStreams);
        SocketSCTP->setOutputGate(gate("socketOut"));
        SocketSCTP->bind(localPort);
        if (ActiveMode == false) {
            SocketSCTP->listen(true);
        }
    }
    else if (TransportProtocol == TCP) {
        assert(SocketTCP == nullptr);
        SocketTCP = new TcpSocket;
        SocketTCP->setOutputGate(gate("socketOut"));
        SocketTCP->bind(localAddr, localPort);
        if (ActiveMode == false) {
            SocketTCP->listen();
        }
    }
    else if (TransportProtocol == UDP) {
        assert(SocketUDP == nullptr);
        SocketUDP = new UdpSocket;
        SocketUDP->setOutputGate(gate("socketOut"));
        SocketUDP->bind(localAddr, localPort);
    }
}

// ###### Connection teardown ###############################################
void NetPerfMeter::teardownConnection(const bool stopTimeReached)
{
    for (auto& elem : TransmitTimerVector) {
        cancelAndDelete(elem);
        elem = nullptr;
    }

    if (ActiveMode == false) {
        if (TransportProtocol == SCTP) {
            if (IncomingSocketSCTP != nullptr) {
                delete IncomingSocketSCTP;
                IncomingSocketSCTP = nullptr;
            }
        }
        else if (TransportProtocol == TCP) {
            if (IncomingSocketTCP != nullptr) {
                delete IncomingSocketTCP;
                IncomingSocketTCP = nullptr;
            }
        }
    }
    if ((stopTimeReached) || (ActiveMode == true)) {
        if (TransportProtocol == SCTP) {
            if (SocketSCTP != nullptr) {
                SocketSCTP->close();
                delete SocketSCTP;
                SocketSCTP = nullptr;
            }
        }
        else if (TransportProtocol == TCP) {
            if (SocketTCP != nullptr) {
                SocketTCP->abort();
                delete SocketTCP;
                SocketTCP = nullptr;
            }
        }
        else if (TransportProtocol == UDP) {
            if (SocketUDP != nullptr) {
                delete SocketUDP;
                SocketUDP = nullptr;
            }
        }
        SendingAllowed = false;
        ConnectionID = 0;
    }

    if (stopTimeReached) {
        writeStatistics();
        HasFinished = true;
    }
}

// ###### Reset statistics ##################################################
void NetPerfMeter::resetStatistics()
{
    StatisticsStartTime = simTime();
    for (auto& elem : SenderStatisticsMap) {
        SenderStatistics *senderStatistics = elem.second;
        senderStatistics->reset();
    }
    for (auto& elem : ReceiverStatisticsMap) {
        ReceiverStatistics *receiverStatistics = elem.second;
        receiverStatistics->reset();
    }
}

// ###### Write scalar statistics ###########################################
void NetPerfMeter::writeStatistics()
{
    const simtime_t statisticsStopTime = simTime();
    const double duration = statisticsStopTime.dbl() - StatisticsStartTime.dbl();

    recordScalar("Total Measurement Duration", duration);
    recordScalar("On-Off Cycles", OnOffCycleCounter);

    // ====== Per-Stream Statistics ==========================================
    unsigned long long totalSentBytes = 0;
    unsigned long long totalSentMessages = 0;
    for (std::map<unsigned int, SenderStatistics *>::const_iterator iterator =
             SenderStatisticsMap.begin(); iterator != SenderStatisticsMap.end();
         iterator++)
    {
        const unsigned int streamID = iterator->first;
        if (streamID >= ActualOutboundStreams) {
            break;
        }
        const SenderStatistics *senderStatistics = iterator->second;
        totalSentBytes += senderStatistics->SentBytes;
        totalSentMessages += senderStatistics->SentMessages;

        const double transmissionBitRate = (duration > 0.0) ? (8 * senderStatistics->SentBytes / duration) : 0.0;
        const double transmissionByteRate = (duration > 0.0) ? (senderStatistics->SentBytes / duration) : 0.0;
        const double transmissionMessageRate = (duration > 0.0) ? (senderStatistics->SentMessages / duration) : 0.0;
        recordScalar(format("Transmission Bit Rate Stream #%u", streamID).c_str(), transmissionBitRate);
        recordScalar(format("Transmission Byte Rate Stream #%u", streamID).c_str(), transmissionByteRate);
        recordScalar(format("Transmission Message Rate Stream #%u", streamID).c_str(), transmissionMessageRate);
        recordScalar(format("Sent Bytes Stream #%u", streamID).c_str(), senderStatistics->SentBytes);
        recordScalar(format("Sent Messages Stream #%u", streamID).c_str(), senderStatistics->SentMessages);
    }

    unsigned long long totalReceivedBytes = 0;
    unsigned long long totalReceivedMessages = 0;
    for (auto& elem : ReceiverStatisticsMap) {
        const unsigned int streamID = elem.first;
        if (streamID >= ActualInboundStreams) {
            break;
        }
        ReceiverStatistics *receiverStatistics = elem.second;
        totalReceivedBytes += receiverStatistics->ReceivedBytes;
        totalReceivedMessages += receiverStatistics->ReceivedMessages;

        // NOTE: When sending "as much as possible", the transmission counters are
        //       set to 0 on reset. If the queue is long enough, there may be no
        //       need to fill it again => counters will be 0 here.
        const double receptionBitRate = (duration > 0.0) ? (8 * receiverStatistics->ReceivedBytes / duration) : 0.0;
        const double receptionByteRate = (duration > 0.0) ? (receiverStatistics->ReceivedBytes / duration) : 0.0;
        const double receptionMessageRate = (duration > 0.0) ? (receiverStatistics->ReceivedMessages / duration) : 0.0;
        recordScalar(format("Reception Bit Rate Stream #%u", streamID).c_str(), receptionBitRate);
        recordScalar(format("Reception Byte Rate Stream #%u", streamID).c_str(), receptionByteRate);
        recordScalar(format("Reception Message Rate Stream #%u", streamID).c_str(), receptionMessageRate);
        recordScalar(format("Received Bytes Stream #%u", streamID).c_str(), receiverStatistics->ReceivedBytes);
        recordScalar(format("Received Messages Stream #%u", streamID).c_str(), receiverStatistics->ReceivedMessages);
        receiverStatistics->ReceivedDelayHistogram.recordAs(format("Received Message Delay Stream #%u", streamID).c_str(), "s");
    }

    // ====== Total Statistics ===============================================
    // NOTE: When sending "as much as possible", the transmission counters are
    //       set to 0 on reset. If the queue is long enough, there may be no
    //       need to fill it again => counters will be 0 here.
    const double totalReceptionBitRate = (duration > 0.0) ? (8 * totalReceivedBytes / duration) : 0.0;
    const double totalReceptionByteRate = (duration > 0.0) ? (totalReceivedBytes / duration) : 0.0;
    const double totalReceptionMessageRate = (duration > 0.0) ? (totalReceivedMessages / duration) : 0.0;
    const double totalTransmissionBitRate = (duration > 0.0) ? (8 * totalSentBytes / duration) : 0.0;
    const double totalTransmissionByteRate = (duration > 0.0) ? (totalSentBytes / duration) : 0.0;
    const double totalTransmissionMessageRate = (duration > 0.0) ? (totalSentMessages / duration) : 0.0;

    // NOTE: The byte rate is redundant, but having bits and bytes
    //       makes manual reading of the results easier.
    recordScalar("Total Transmission Bit Rate", totalTransmissionBitRate);
    recordScalar("Total Transmission Byte Rate", totalTransmissionByteRate);
    recordScalar("Total Transmission Message Rate", totalTransmissionMessageRate);
    recordScalar("Total Sent Bytes", totalSentBytes);
    recordScalar("Total Sent Messages", totalSentMessages);

    recordScalar("Total Reception Bit Rate", totalReceptionBitRate);
    recordScalar("Total Reception Byte Rate", totalReceptionByteRate);
    recordScalar("Total Reception Message Rate", totalReceptionMessageRate);
    recordScalar("Total Received Bytes", totalReceivedBytes);
    recordScalar("Total Received Messages", totalReceivedMessages);

    resetStatistics(); // Make sure that it is not mistakenly used later
}

// ###### Transmit frame of given size via given stream #####################
unsigned long NetPerfMeter::transmitFrame(const unsigned int frameSize,
                                          const unsigned int streamID)
{
    EV << simTime() << ", " << getFullPath() << ": Transmit frame of size "
       << frameSize << " on stream #" << streamID << endl;
    assert(OnTimer == nullptr);

    // ====== TCP ============================================================
    unsigned long newlyQueuedBytes = 0;
    if (TransportProtocol == TCP) {
        // TCP is stream-oriented: just pass the amount of frame data.
        auto cmsg = new Packet("NetPerfMeterDataMessage");
        auto dataMessage = makeShared<BytesChunk>();
        std::vector<uint8_t> vec;
        vec.resize(frameSize);
        for (uint32_t i = 0; i < frameSize; i++)
            vec[i] = ((i & 1) ? 'D' : 'T');
        dataMessage->setBytes(vec);
        dataMessage->addTag<CreationTimeTag>()->setCreationTime(simTime());
        cmsg->insertAtBack(dataMessage);

        if (IncomingSocketTCP) {
            IncomingSocketTCP->send(cmsg);
        }
        else {
            SocketTCP->send(cmsg);
        }

        newlyQueuedBytes += frameSize;
        SenderStatistics *senderStatistics = getSenderStatistics(0);
        senderStatistics->SentBytes += frameSize;
        senderStatistics->SentMessages++;
    }
    // ====== Message-Oriented Protocols =====================================
    else {
        unsigned int bytesToSend = frameSize;
        do {
            const unsigned int msgSize = (bytesToSend > MaxMsgSize) ? MaxMsgSize : bytesToSend;

            if (false) {
            }
            // ====== SCTP =====================================================
#ifdef INET_WITH_SCTP
            else if (TransportProtocol == SCTP) {
                const bool sendUnordered = (UnorderedMode > 0.0) ? (uniform(0.0, 1.0) < UnorderedMode) : false;
                const bool sendUnreliable = (UnreliableMode > 0.0) ? (uniform(0.0, 1.0) < UnreliableMode) : false;

                auto cmsg = new Packet("NetPerfMeterDataMessage");
                auto dataMessage = makeShared<BytesChunk>();
                std::vector<uint8_t> vec;
                vec.resize(msgSize);
                for (uint32_t i = 0; i < msgSize; i++)
                    vec[i] = ((i & 1) ? 'D' : 'T');
                dataMessage->setBytes(vec);
                dataMessage->addTag<CreationTimeTag>()->setCreationTime(simTime());
                cmsg->insertAtBack(dataMessage);

                cmsg->addTag<SocketReq>()->setSocketId(ConnectionID);
                auto command = cmsg->addTag<SctpSendReq>();
                command->setSocketId(ConnectionID);
                command->setSid(streamID);
                command->setSendUnordered((sendUnordered == true) ? COMPLETE_MESG_UNORDERED : COMPLETE_MESG_ORDERED);
                command->setLast(true);
                command->setPrimary(PrimaryPath.isUnspecified());
                command->setRemoteAddr(PrimaryPath);
                command->setPrValue(1);
                command->setPrMethod((sendUnreliable == true) ? 2 : 0); // PR-SCTP policy: RTX

                cmsg->setKind(sendUnordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&inet::Protocol::sctp);
                send(cmsg, "socketOut");

                SenderStatistics *senderStatistics = getSenderStatistics(streamID);
                senderStatistics->SentBytes += msgSize;
                senderStatistics->SentMessages++;
            }
#endif
            // ====== UDP ===================================================
            else if (TransportProtocol == UDP) {
                auto cmsg = new Packet("NetPerfMeterDataMessage");
                auto dataMessage = makeShared<BytesChunk>();
                std::vector<uint8_t> vec;
                vec.resize(msgSize);
                for (uint32_t i = 0; i < msgSize; i++)
                    vec[i] = ((i & 1) ? 'D' : 'T');
                dataMessage->setBytes(vec);
                dataMessage->addTag<CreationTimeTag>()->setCreationTime(simTime());
                cmsg->insertAtBack(dataMessage);

                SocketUDP->send(cmsg);

                SenderStatistics *senderStatistics = getSenderStatistics(0);
                senderStatistics->SentBytes += msgSize;
                senderStatistics->SentMessages++;
            }

            newlyQueuedBytes += msgSize;
            bytesToSend -= msgSize;
        } while (bytesToSend > 0);
    }
    return newlyQueuedBytes;
}

// ###### Get frame rate for next frame to be sent on given stream ##########
double NetPerfMeter::getFrameRate(const unsigned int streamID)
{
    double frameRate;
    if (FrameRateExpressionVector.size() == 0) {
        frameRate = par("frameRate");
    }
    else {
        frameRate = FrameRateExpressionVector[streamID % FrameRateExpressionVector.size()].doubleValue(this, "Hz");
        if (frameRate < 0) {
            frameRate = par("frameRate");
        }
    }
    return frameRate;
}

// ###### Get frame rate for next frame to be sent on given stream ##########
unsigned long NetPerfMeter::getFrameSize(const unsigned int streamID)
{
    unsigned long frameSize;
    if (FrameSizeExpressionVector.size() == 0) {
        frameSize = par("frameSize");
    }
    else {
        double doubleSize = FrameSizeExpressionVector[streamID % FrameSizeExpressionVector.size()].doubleValue(this, "B");
        frameSize = (doubleSize >= 0.0) ? (long)doubleSize : par("frameSize");
    }
    return frameSize;
}

// ###### Send data of saturated streams ####################################
void NetPerfMeter::sendDataOfSaturatedStreams(const unsigned long long bytesAvailableInQueue,
                                              const Ptr<const SctpSendQueueAbatedReq>& sendQueueAbatedIndication)
{
    if (OnTimer != nullptr) {
        // We are in Off mode -> nothing to send!
        return;
    }

    // ====== Is sending allowed (i.e. space in the queue)? ==================
    if (SendingAllowed) {
        // ====== SCTP tells current queue occupation for each stream =========
        unsigned long long contingent;
        unsigned long long queued[ActualOutboundStreams];
        if (sendQueueAbatedIndication == nullptr) {
            // At the moment, the actual queue size is unknown.
            // => Assume it to be bytesAvailableInQueue.
            contingent = bytesAvailableInQueue / ActualOutboundStreams;
            for (unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
                queued[streamID] = 0;
            }
        }
        else {
            assert(ActualOutboundStreams <= sendQueueAbatedIndication->getQueuedForStreamArraySize());
            for (unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
                queued[streamID] = sendQueueAbatedIndication->getQueuedForStream(streamID);
            }
            contingent = sendQueueAbatedIndication->getBytesLimit() / ActualOutboundStreams;
        }

        // ====== Send, but care for per-stream contingent ====================
        LastStreamID = (LastStreamID + 1) % ActualOutboundStreams;
        unsigned int startStreamID = LastStreamID;
        unsigned long long newlyQueuedBytes = 0;
        bool progress;
        do {
            // ====== Perform one round ========================================
            progress = false; // Will be set to true on any transmission progress
                              // during the next round
            do {
                const double frameRate = getFrameRate(LastStreamID);
                if (frameRate < 0.0000001) { // Saturated stream
                    if ((DecoupleSaturatedStreams == false) || (queued[LastStreamID] < contingent)) {
                        // ====== Send one frame ==================================
                        const unsigned long frameSize = getFrameSize(LastStreamID);
                        if (frameSize >= 1) {
                            const unsigned long long sent = transmitFrame(frameSize, LastStreamID);
                            newlyQueuedBytes += sent;
                            queued[LastStreamID] += sent;
                            progress = true;
                        }
                    }
                }
                LastStreamID = (LastStreamID + 1) % ActualOutboundStreams;
            } while (LastStreamID != startStreamID);
        } while ((newlyQueuedBytes < bytesAvailableInQueue) && (progress == true));
    }
}

// ###### Send data of non-saturated streams ################################
void NetPerfMeter::sendDataOfNonSaturatedStreams(const unsigned long long bytesAvailableInQueue,
                                                 const unsigned int       streamID)
{
    assert(OnTimer == nullptr);

    // ====== Is there something to send? ====================================
    const double frameRate = getFrameRate(streamID);
    if (frameRate <= 0.0) {
        // No non-saturated transmission on this stream
        // => no need to re-schedule timer!
        return;
    }

    // ====== Is sending allowed (i.e. space in the queue)? ==================
    if (SendingAllowed) {
        const long frameSize = getFrameSize(streamID);
        if (frameSize < 1) {
            return;
        }
        if ((frameRate <= 0.0) && ((TransportProtocol == TCP) || (TransportProtocol == UDP))) {
            throw cRuntimeError("TCP and UDP do not support \"send as much as possible\" mode (frameRate=0)!");
        }

        // ====== Transmit frame ==============================================
        /*
         EV << simTime() << ", " << getFullPath() << ": Stream #" << streamID
         << ":\tframeRate=" << frameRate
         << "\tframeSize=" << frameSize << endl;
         */
        transmitFrame(frameSize, streamID);
    }
    else {
        // No transmission allowed -> skip this frame!
    }

    // ====== Schedule next frame transmission ===============================
    assert(TransmitTimerVector[streamID] == nullptr);
    TransmitTimerVector[streamID] = new NetPerfMeterTransmitTimer("TransmitTimer", TIMER_TRANSMIT);
    TransmitTimerVector[streamID]->setStreamID(streamID);
    const double nextFrameTime = 1.0 / frameRate;
    /*
     EV << simTime() << ", " << getFullPath()
     << ": Next on stream #" << streamID << " in " << nextFrameTime << "s" << endl;
     */
    scheduleAfter(nextFrameTime, TransmitTimerVector[streamID]);
}

// ###### Send data of non-saturated streams ################################
void NetPerfMeter::sendDataOfTraceFile(const unsigned long long bytesAvailableInQueue)
{
    if (TraceIndex < TraceVector.size()) {
        const unsigned int frameSize = TraceVector[TraceIndex].FrameSize;
        unsigned int streamID = TraceVector[TraceIndex].StreamID;
        if (streamID >= ActualOutboundStreams) {
            if (TransportProtocol == SCTP) {
                throw cRuntimeError("Invalid streamID in trace");
            }
            streamID = 0;
        }
        transmitFrame(frameSize, streamID);
        TraceIndex++;
    }

    if (TraceIndex >= TraceVector.size()) {
        TraceIndex = 0;
    }

    // ====== Schedule next frame transmission ===============================
    if (TraceIndex < TraceVector.size()) {
        const double nextFrameTime = TraceVector[TraceIndex].InterFrameDelay;
        assert(TransmitTimerVector[0] == nullptr);
        TransmitTimerVector[0] = new NetPerfMeterTransmitTimer("TransmitTimer", TIMER_TRANSMIT);
        TransmitTimerVector[0]->setStreamID(0);

//        std::cout << simTime() << ", " << getFullPath()
//                  << ": Next in " << nextFrameTime << "s" << endl;

        scheduleAfter(nextFrameTime, TransmitTimerVector[0]);
    }
}

// ###### Receive data ######################################################
void NetPerfMeter::receiveMessage(cMessage *msg)
{
    if (const Packet *dataMessage = dynamic_cast<const Packet *>(msg)) {
        unsigned int streamID = 0;
        const auto& smsg = dataMessage->peekData();

        if (TransportProtocol == SCTP) {
            auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
            auto& receiveCommand = tags.findTag<SctpRcvReq>();
            streamID = receiveCommand->getSid();
        }

        ReceiverStatistics *receiverStatistics = getReceiverStatistics(streamID);
        receiverStatistics->ReceivedMessages++;
        receiverStatistics->ReceivedBytes += B(smsg->getChunkLength()).get();
        for (auto& region : smsg->getAllTags<CreationTimeTag>())
            receiverStatistics->ReceivedDelayHistogram.collect(simTime() - region.getTag()->getCreationTime());
    }
}

// ###### SCTP queue length configuration ###################################
void NetPerfMeter::sendSCTPQueueRequest(const unsigned int queueSize)
{
    assert(SocketSCTP != nullptr);

    // Tell SCTP to limit the send queue to the number of bytes specified.
    // When the queue is able accept more data again, it will be indicated by
    // SCTP_I_SENDQUEUE_ABATED!

    Request *cmsg = new Request("QueueRequest", SCTP_C_QUEUE_BYTES_LIMIT);
    auto queueInfo = cmsg->addTag<SctpInfoReq>();
    queueInfo->setText(queueSize);
    queueInfo->setSocketId(ConnectionID);

    if (IncomingSocketSCTP) {
        IncomingSocketSCTP->sendRequest(cmsg);
    }
    else {
        SocketSCTP->sendRequest(cmsg);
    }
}

// ###### TCP queue length configuration ####################################
void NetPerfMeter::sendTCPQueueRequest(const unsigned int queueSize)
{
    assert(SocketTCP != nullptr);

    // Tell TCP to limit the send queue to the number of bytes specified.
    // When the queue is able accept more data again, it will be indicated by
    // TCP_I_SEND_MSG!

    TcpCommand *queueInfo = new TcpCommand();
    queueInfo->setUserId(queueSize);

    auto request = new Request("QueueRequest", TCP_C_QUEUE_BYTES_LIMIT);
    request->setControlInfo(queueInfo);
    request->addTag<SocketReq>()->setSocketId(ConnectionID);
    if (IncomingSocketTCP) {
        IncomingSocketTCP->sendCommand(request);
    }
    else {
        SocketTCP->sendCommand(request);
    }
}

// ###### Return sprintf-formatted string ####################################
opp_string NetPerfMeter::format(const char *formatString, ...)
{
    char str[1024];
    va_list args;
    va_start(args, formatString);
    vsnprintf(str, sizeof(str), formatString, args);
    va_end(args);
    return opp_string(str);
}

} // namespace inet

