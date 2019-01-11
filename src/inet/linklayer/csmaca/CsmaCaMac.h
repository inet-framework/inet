//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_CSMAMAC_H
#define __INET_CSMAMAC_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/queue/PacketQueue.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

class INET_API CsmaCaMac : public MacProtocolBase
{
  protected:
    /**
     * @name Configuration parameters
     */
    //@{
    FcsMode fcsMode;
    bool useAck = true;
    double bitrate = NaN;
    b headerLength = b(-1);
    b ackLength = b(-1);
    simtime_t ackTimeout = -1;
    simtime_t slotTime = -1;
    simtime_t sifsTime = -1;
    simtime_t difsTime = -1;
    int maxQueueSize = -1;
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

    physicallayer::IRadio *radio = nullptr;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;

    cFSM fsm;

    /** Remaining backoff period in seconds */
    simtime_t backoffPeriod = -1;

    /** Number of frame retransmission attempts. */
    int retryCounter = -1;

    /** Messages received from upper layer and to be transmitted later */
    PacketQueue transmissionQueue;

    /** Passive queue module to request messages from */
    IPassiveQueue *queueModule = nullptr;
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
    virtual void initializeQueueModule();
    virtual void finish() override;
    virtual void configureInterfaceEntry() override;
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

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details) override;

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
    virtual void popTransmissionQueue();
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
    virtual void handleStartOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}    //TODO implementation
};

} // namespace inet

#endif // ifndef __INET_CSMAMAC_H
