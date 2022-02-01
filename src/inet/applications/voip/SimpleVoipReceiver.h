//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SIMPLEVOIPRECEIVER_H
#define __INET_SIMPLEVOIPRECEIVER_H

#include <string.h>

#include <list>

#include "inet/common/INETMath.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

class SimpleVoipPacket;

/**
 * Implements a simple VoIP source. See the NED file for more information.
 */
class INET_API SimpleVoipReceiver : public cSimpleModule, public LifecycleUnsupported, public UdpSocket::ICallback
{
  private:
    class VoipPacketInfo {
      public:
        unsigned int packetID = 0;
        simtime_t creationTime;
        simtime_t arrivalTime;
        simtime_t playoutTime;
    };

    typedef std::list<VoipPacketInfo *> PacketsList;
    typedef std::vector<VoipPacketInfo> PacketsVector;

    class TalkspurtInfo {
      public:
        enum Status {
            EMPTY,
            ACTIVE,
            FINISHED
        };
        Status status = EMPTY;
        unsigned int talkspurtID = -1;
        unsigned int talkspurtNumPackets = 0;
        simtime_t voiceDuration;
        PacketsVector packets;

      public:
        TalkspurtInfo() {}
        void startTalkspurt(const SimpleVoipPacket *pk);
        void finishTalkspurt() { status = FINISHED; packets.clear(); }
        bool checkPacket(const SimpleVoipPacket *pk);
        void addPacket(const SimpleVoipPacket *pk);
        bool isActive() { return status == ACTIVE; }
    };

    // parameters
    double emodelRo = NaN;
    unsigned int bufferSpace = 0;
    int emodelIe = -1;
    int emodelBpl = -1;
    int emodelA = -1;
    simtime_t playoutDelay;
    simtime_t mosSpareTime; // spare time before calculating MOS (after calculated playout time of last packet)

    // state
    UdpSocket socket;
    cMessage *selfTalkspurtFinished = nullptr;
    TalkspurtInfo currentTalkspurt;

    static simsignal_t voipPacketLossRateSignal;
    static simsignal_t voipPacketDelaySignal;
    static simsignal_t voipPlayoutDelaySignal;
    static simsignal_t voipPlayoutLossRateSignal;
    static simsignal_t voipMosRateSignal;
    static simsignal_t voipTaildropLossRateSignal;

    double eModel(double delay, double loss);
    void evaluateTalkspurt(bool finish);
    void startTalkspurt(Packet *packet);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // UdpSocket::ICallback methods
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {}

  public:
    SimpleVoipReceiver();
    ~SimpleVoipReceiver();
};

} // namespace inet

#endif

