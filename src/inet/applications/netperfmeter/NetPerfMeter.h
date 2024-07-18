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


#ifndef __INET_NETPERFMETER_H
#define __INET_NETPERFMETER_H

#include <cassert>
#include <fstream>

#include "inet/applications/netperfmeter/NetPerfMeter_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * Implementation of NetPerfMeter. See NED file for more details.
 */
class INET_API NetPerfMeter : public cSimpleModule
{
  public:
    NetPerfMeter();
    ~NetPerfMeter();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void refreshDisplay() const override;

    void establishConnection();
    void successfullyEstablishedConnection(cMessage *msg, unsigned int queueSize);
    void teardownConnection(bool stopTimeReached = false);
    void receiveMessage(cMessage *msg);
    void resetStatistics();
    void writeStatistics();

    void sendSCTPQueueRequest(unsigned int queueSize);
    void sendTCPQueueRequest(unsigned int queueSize);

  protected:
    // ====== Parameters =====================================================
    enum Protocol {
        SCTP = 0, TCP = 1, UDP = 2
    };
    enum TimerType {
        TIMER_CONNECT  = 1,
        TIMER_START    = 2,
        TIMER_RESET    = 3,
        TIMER_STOP     = 4,
        TIMER_TRANSMIT = 5,
        TIMER_OFF      = 6,
        TIMER_ON       = 7
    };

    Protocol transportProtocol = static_cast<Protocol>(-1);
    bool activeMode = false;
    bool sendingAllowed = false;
    bool hasFinished = false;
    unsigned int maxMsgSize = 0;
    unsigned int queueSize = 0;
    double unorderedMode = NAN;
    double unreliableMode = NAN;
    bool decoupleSaturatedStreams = false;
    simtime_t connectTime;
    simtime_t startTime;
    simtime_t resetTime;
    simtime_t stopTime;
    cMessage *connectTimer = nullptr;
    cMessage *startTimer = nullptr;
    cMessage *stopTimer = nullptr;
    cMessage *resetTimer = nullptr;
    cMessage *offTimer = nullptr;
    cMessage *onTimer = nullptr;
    unsigned int onOffCycleCounter = 0;
    int maxOnOffCycles = 0;
    std::vector<NetPerfMeterTransmitTimer *> transmitTimerVector;

    unsigned int requestedOutboundStreams = 0;
    unsigned int maxInboundStreams = 0;
    unsigned int actualOutboundStreams = 0;
    unsigned int actualInboundStreams = 0;
    std::vector<cDynamicExpression> frameRateExpressionVector;
    std::vector<cDynamicExpression> frameSizeExpressionVector;

    // ====== Sockets and Connection Information =============================
    SctpSocket *socketSCTP = nullptr;
    SctpSocket *incomingSocketSCTP = nullptr;
    TcpSocket *socketTCP = nullptr;
    TcpSocket *incomingSocketTCP = nullptr;
    UdpSocket *socketUDP = nullptr;
    int connectionID = 0;
    L3Address primaryPath;

    // ====== Trace File Handling ============================================
    struct TraceEntry {
        double interFrameDelay;
        unsigned int frameSize;
        unsigned int streamID;
    };
    std::vector<TraceEntry> traceVector; // Frame trace from file
    size_t traceIndex = 0; // Position in trace file

    // ====== Timers =========================================================
    simtime_t transmissionStartTime; // Absolute transmission start time
    simtime_t connectionEstablishmentTime; // Absolute connection establishment time
    simtime_t statisticsResetTime; // Absolute statistics reset time

    // ====== Variables ======================================================
    unsigned int lastStreamID = 0; // Stream number of last message being sent

    // ====== Statistics =====================================================
    simtime_t statisticsStartTime; // Absolute start time of statistics recording

  private:
    class SenderStatistics {
      public:
        SenderStatistics() { reset(); }

        void reset() { sentBytes = 0; sentMessages = 0; }

        unsigned long long sentBytes = 0;
        unsigned long long sentMessages = 0;
    };

    class ReceiverStatistics {
      public:
        ReceiverStatistics() {
            receivedDelayHistogram.setName("Received Message Delay");
            reset();
        }

        void reset() {
            receivedBytes = 0;
            receivedMessages = 0;
            receivedDelayHistogram.clear();
        }

        unsigned long long receivedBytes = 0;
        unsigned long long receivedMessages = 0;
        cHistogram receivedDelayHistogram;
    };

    std::map<unsigned int, SenderStatistics *> senderStatisticsMap;
    std::map<unsigned int, ReceiverStatistics *> receiverStatisticsMap;

    SenderStatistics *getSenderStatistics(unsigned int streamID) {
        auto found = senderStatisticsMap.find(streamID);
        ASSERT(found != senderStatisticsMap.end());
        return found->second;
    }

    ReceiverStatistics *getReceiverStatistics(unsigned int streamID) {
        auto found = receiverStatisticsMap.find(streamID);
        ASSERT(found != receiverStatisticsMap.end());
        return found->second;
    }

    double getFrameRate(unsigned int streamID);
    unsigned long getFrameSize(unsigned int streamID);
    void startSending();
    void stopSending();
    void sendDataOfTraceFile(unsigned long long bytesAvailableInQueue);
    void sendDataOfSaturatedStreams(unsigned long long bytesAvailableInQueue,
                                    const Ptr<const SctpSendQueueAbatedReq>& sendQueueAbatedIndication);

    void sendDataOfNonSaturatedStreams(unsigned long long bytesAvailableInQueue, unsigned int streamID);
    unsigned long transmitFrame(unsigned int frameSize, unsigned int streamID);
    static opp_string format(const char *formatString, ...);
    static void parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
                                      const char *string, const char *delimiters = nullptr);
    void createAndBindSocket();
    void handleTimer(cMessage *msg);
};

} // namespace inet

#endif

