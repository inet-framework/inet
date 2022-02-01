//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CSMACAMAC_H
#define __INET_CSMACAMAC_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

class INET_API CsmaCaMac : public MacProtocolBase, public IMacProtocol, public queueing::IActivePacketSink
{
  protected:
    /**
     * @name Configuration parameters
     */
    //@{
    FcsMode fcsMode;
    bool useAck = true;
    double bitrate = NaN;
    B headerLength = B(-1);
    B ackLength = B(-1);
    simtime_t ackTimeout = -1;
    simtime_t slotTime = -1;
    simtime_t sifsTime = -1;
    simtime_t difsTime = -1;
    int retryLimit = -1;
    int cwMin = -1;
    int cwMax = -1;
    int cwMulticast = -1;
    //@}

    /**
     * @name CsmaCaMac state variables
     * Various state information checked and modified according to the state machine.
     */
    //@{
    enum State {
        IDLE,
        DEFER,
        WAITDIFS,
        BACKOFF,
        TRANSMIT,
        WAITACK,
        RECEIVE,
        WAITSIFS,
    };

    ModuleRefByPar<physicallayer::IRadio> radio;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;

    cFSM fsm;

    /** Remaining backoff period in seconds */
    simtime_t backoffPeriod = -1;

    /** Number of frame retransmission attempts. */
    int retryCounter = -1;
    //@}

    /** @name Timer messages */
    //@{
    /** End of the Short Inter-Frame Time period */
    cMessage *endSifs = nullptr;

    /** End of the Data Inter-Frame Time period */
    cMessage *endDifs = nullptr;

    /** End of the backoff period */
    cMessage *endBackoff = nullptr;

    /** End of the ack timeout */
    cMessage *endAckTimeout = nullptr;

    /** Timeout after the transmission of a Data frame */
    cMessage *endData = nullptr;

    /** Radio state change self message. Currently this is optimized away and sent directly */
    cMessage *mediumStateChange = nullptr;
    //@}

    /** @name Statistics */
    //@{
    long numRetry;
    long numSentWithoutRetry;
    long numGivenUp;
    long numCollision;
    long numSent;
    long numReceived;
    long numSentBroadcast;
    long numReceivedBroadcast;
    //@}

  public:
    /**
     * @name Construction functions
     */
    //@{
    virtual ~CsmaCaMac();
    //@}

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void configureNetworkInterface() override;
    //@}

    /**
     * @name Message handing functions
     * @brief Functions called from other classes to notify about state changes and to handle messages.
     */
    //@{
    virtual void handleSelfMessage(cMessage *msg) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;
    virtual void handleWithFsm(cMessage *msg);

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);
    //@}

    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
    virtual void scheduleSifsTimer(Packet *frame);

    virtual void scheduleDifsTimer();
    virtual void cancelDifsTimer();

    virtual void scheduleAckTimeout(Packet *frame);
    virtual void cancelAckTimer();

    virtual void invalidateBackoffPeriod();
    virtual bool isInvalidBackoffPeriod();
    virtual void generateBackoffPeriod();
    virtual void decreaseBackoffPeriod();
    virtual void scheduleBackoffTimer();
    virtual void cancelBackoffTimer();
    //@}

    /**
     * @name Frame transmission functions
     */
    //@{
    virtual void sendDataFrame(Packet *frameToSend);
    virtual void sendAckFrame();
    //@}

    /**
     * @name Utility functions
     */
    //@{
    virtual void finishCurrentTransmission();
    virtual void giveUpCurrentTransmission();
    virtual void retryCurrentTransmission();
    virtual Packet *getCurrentTransmission();
    virtual void resetTransmissionVariables();
    virtual void emitPacketDropSignal(Packet *frame, PacketDropReason reason, int limit = -1);

    virtual bool isMediumFree();
    virtual bool isReceiving();
    virtual bool isAck(Packet *frame);
    virtual bool isBroadcast(Packet *frame);
    virtual bool isForUs(Packet *frame);
    virtual bool isFcsOk(Packet *frame);

    virtual uint32_t computeFcs(const Ptr<const BytesChunk>& bytes);
    //@}

    // OperationalBase:
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void processUpperPacket();

  public:
    // IActivePacketSink:
    virtual queueing::IPassivePacketSource *getProvider(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif

