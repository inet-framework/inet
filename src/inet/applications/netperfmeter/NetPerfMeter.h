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

#include <assert.h>

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
    void successfullyEstablishedConnection(cMessage *msg, const unsigned int queueSize);
    void teardownConnection(const bool stopTimeReached = false);
    void receiveMessage(cMessage *msg);
    void resetStatistics();
    void writeStatistics();

    void sendSCTPQueueRequest(const unsigned int queueSize);
    void sendTCPQueueRequest(const unsigned int queueSize);

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

    Protocol TransportProtocol = static_cast<Protocol>(-1);
    bool ActiveMode = false;
    bool SendingAllowed = false;
    bool HasFinished = false;
    unsigned int MaxMsgSize = 0;
    unsigned int QueueSize = 0;
    double UnorderedMode = NAN;
    double UnreliableMode = NAN;
    bool DecoupleSaturatedStreams = false;
    simtime_t ConnectTime;
    simtime_t StartTime;
    simtime_t ResetTime;
    simtime_t StopTime;
    cMessage *ConnectTimer = nullptr;
    cMessage *StartTimer = nullptr;
    cMessage *StopTimer = nullptr;
    cMessage *ResetTimer = nullptr;
    cMessage *OffTimer = nullptr;
    cMessage *OnTimer = nullptr;
    unsigned int OnOffCycleCounter = 0;
    int MaxOnOffCycles = 0;
    std::vector<NetPerfMeterTransmitTimer *> TransmitTimerVector;

    unsigned int RequestedOutboundStreams = 0;
    unsigned int MaxInboundStreams = 0;
    unsigned int ActualOutboundStreams = 0;
    unsigned int ActualInboundStreams = 0;
    std::vector<cDynamicExpression> FrameRateExpressionVector;
    std::vector<cDynamicExpression> FrameSizeExpressionVector;

    // ====== Sockets and Connection Information =============================
    SctpSocket *SocketSCTP = nullptr;
    SctpSocket *IncomingSocketSCTP = nullptr;
    TcpSocket *SocketTCP = nullptr;
    TcpSocket *IncomingSocketTCP = nullptr;
    UdpSocket *SocketUDP = nullptr;
    int ConnectionID = 0;
    L3Address PrimaryPath;

    // ====== Trace File Handling ============================================
    struct TraceEntry {
        double InterFrameDelay;
        unsigned int FrameSize;
        unsigned int StreamID;
    };
    std::vector<TraceEntry> TraceVector; // Frame trace from file
    size_t TraceIndex = 0; // Position in trace file

    // ====== Timers =========================================================
    simtime_t TransmissionStartTime; // Absolute transmission start time
    simtime_t ConnectionEstablishmentTime; // Absolute connection establishment time
    simtime_t StatisticsResetTime; // Absolute statistics reset time

    // ====== Variables ======================================================
    unsigned int LastStreamID = 0; // Stream number of last message being sent

    // ====== Statistics =====================================================
    simtime_t StatisticsStartTime; // Absolute start time of statistics recording

  private:
    class SenderStatistics {
      public:
        SenderStatistics() { reset(); }

        inline void reset() { SentBytes = 0; SentMessages = 0; }

        unsigned long long SentBytes;
        unsigned long long SentMessages;
    };

    class ReceiverStatistics {
      public:
        ReceiverStatistics() {
            ReceivedDelayHistogram.setName("Received Message Delay");
            reset();
        }

        inline void reset() {
            ReceivedBytes = 0;
            ReceivedMessages = 0;
            ReceivedDelayHistogram.clear();
        }

        unsigned long long ReceivedBytes = 0;
        unsigned long long ReceivedMessages = 0;
        cHistogram ReceivedDelayHistogram;
    };

    std::map<unsigned int, SenderStatistics *> SenderStatisticsMap;
    std::map<unsigned int, ReceiverStatistics *> ReceiverStatisticsMap;

    inline SenderStatistics *getSenderStatistics(const unsigned int streamID) {
        auto found = SenderStatisticsMap.find(streamID);
        assert(found != SenderStatisticsMap.end());
        return found->second;
    }

    inline ReceiverStatistics *getReceiverStatistics(const unsigned int streamID) {
        auto found = ReceiverStatisticsMap.find(streamID);
        assert(found != ReceiverStatisticsMap.end());
        return found->second;
    }

    double getFrameRate(const unsigned int streamID);
    unsigned long getFrameSize(const unsigned int streamID);
    void startSending();
    void stopSending();
    void sendDataOfTraceFile(const unsigned long long bytesAvailableInQueue);
    void sendDataOfSaturatedStreams(const unsigned long long bytesAvailableInQueue,
            const Ptr<const SctpSendQueueAbatedReq>& sendQueueAbatedIndication);

    void sendDataOfNonSaturatedStreams(const unsigned long long bytesAvailableInQueue, const unsigned int streamID);
    unsigned long transmitFrame(const unsigned int frameSize, const unsigned int streamID);
    static opp_string format(const char *formatString, ...);
    static void parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
            const char *string, const char *delimiters = nullptr);
    void createAndBindSocket();
    void handleTimer(cMessage *msg);
};

} // namespace inet

#endif

