//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_ETHERNETCSMAMAC_H
#define __INET_ETHERNETCSMAMAC_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/base/EthernetMacBase.h"

namespace inet {

/**
 * Ethernet MAC module which supports both half-duplex (CSMA/CD) and full-duplex
 * operation. (See also EthernetMac which has a considerably smaller
 * code with all the CSMA/CD complexity removed.)
 *
 * See NED file for more details.
 */
class INET_API EthernetCsmaMac : public EthernetMacBase
{
  public:
    EthernetCsmaMac() {}
    virtual ~EthernetCsmaMac();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void initializeFlags() override;
    virtual void initializeStatistics() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

  protected:
    class RxSignal {
      public:
        long transmissionId = -1;
        EthernetSignalBase *signal = nullptr;
        simtime_t endRxTime;
        RxSignal(long transmissionId, EthernetSignalBase *signal, simtime_t_cref endRxTime) : transmissionId(transmissionId), signal(signal), endRxTime(endRxTime) {}
    };
    std::vector<RxSignal> rxSignals;

  protected:
    // states
    int backoffs = 0; // value of backoff for exponential back-off algorithm

    cMessage *endRxTimer = nullptr;
    cMessage *endBackoffTimer = nullptr;
    cMessage *endJammingTimer = nullptr;

    // list of receptions during reconnect state; an additional special entry (with packetTreeId=-1)
    // stores the end time of the reconnect state
    struct PkIdRxTime {
        long packetTreeId; // >=0: tree ID of packet being received; -1: this is a special entry that stores the end time of the reconnect state
        simtime_t endTime; // end of reception
        PkIdRxTime(long id, simtime_t time) { packetTreeId = id; endTime = time; }
    };

    // statistics
    simtime_t totalCollisionTime; // total duration of collisions on channel
    simtime_t totalSuccessfulRxTxTime; // total duration of successful transmissions on channel
    simtime_t channelBusySince; // needed for computing totalCollisionTime/totalSuccessfulRxTxTime
    unsigned long numCollisions = 0; // collisions (NOT number of collided frames!) sensed
    unsigned long numBackoffs = 0; // number of retransmissions
    int framesSentInBurst = 0; // Number of frames send out in current frame burst
    B bytesSentInBurst = B(0); // Number of bytes transmitted in current frame burst

    static simsignal_t collisionSignal;
    static simsignal_t backoffSlotsGeneratedSignal;

  protected:
    // event handlers
    virtual void handleSelfMessage(cMessage *msg) override;
    virtual void handleEndIFGPeriod();
    virtual void handleEndPausePeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndRxPeriod();
    virtual void handleEndBackoffPeriod();
    virtual void handleEndJammingPeriod();
    virtual void handleRetransmission();

    // helpers
    virtual void readChannelParameters(bool errorWhenAsymmetric) override;
    virtual void handleUpperPacket(Packet *msg) override;
    virtual void processMsgFromNetwork(EthernetSignalBase *msg);
    virtual void scheduleEndIFGPeriod();
    virtual void fillIFGIfInBurst();
    virtual void scheduleEndPausePeriod(int pauseUnits);
    virtual void beginSendFrames();
    virtual void sendJamSignal();
    virtual void startFrameTransmission();
    virtual void frameReceptionComplete();
    virtual void processReceivedDataFrame(Packet *frame);
    virtual void processReceivedControlFrame(Packet *packet);
    virtual void processConnectDisconnect() override;
    virtual void processDetectedCollision();
    virtual void sendSignal(EthernetSignalBase *signal, simtime_t_cref duration);
    virtual void handleSignalFromNetwork(EthernetSignalBase *signal);
    virtual void updateRxSignals(EthernetSignalBase *signal, simtime_t endRxTime);
    virtual void dropCurrentTxFrame(PacketDropDetails& details) override;

    B calculateMinFrameLength();
    B calculatePaddedFrameLength(Packet *frame);

    virtual void printState();
};

} // namespace inet

#endif

