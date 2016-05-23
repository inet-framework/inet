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
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/newcsma/CsmaFrame_m.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

using namespace inet::physicallayer;

class INET_API CsmaMac : public MACProtocolBase
{
  protected:
    /**
     * @name Configuration parameters
     */
    //@{
    /** MAC address */
    MACAddress address;

    bool useAck = true;

    /** The bitrate is used to send data frames */
    double bitrate = NaN;

    /** The header size of frames */
    int headerLength = -1;

    simtime_t slotTime = -1;
    simtime_t sifsTime = -1;
    simtime_t difsTime = -1;

    /** Maximum number of frames in the queue; should be set in the omnetpp.ini */
    int maxQueueSize = -1;

    /**
     * Maximum number of transmissions for a message.
     * This includes the initial transmission and all subsequent retransmissions.
     * Thus a value 0 is invalid and a value 1 means no retransmissions.
     */
    int retryLimit = -1;

    /** Minimum contention window. */
    int cwMin = -1;

    /** Maximum contention window. */
    int cwMax = -1;
    //@}

    /**
     * @name CsmaMac state variables
     * Various state information checked and modified according to the state machine.
     */
    //@{
    enum State {
        IDLE,
        DEFER,
        WAITDIFS,
        BACKOFF,
        WAITACK,
        WAITTRANSMIT,
        WAITSIFS,
        RECEIVE,
    };

    IRadio *radio = nullptr;
    IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

    cFSM fsm;

    /** True if backoff is enabled */
    bool backoff = false;

    /** Remaining backoff period in seconds */
    simtime_t backoffPeriod = -1;

    /** Number of frame retransmission attempts. */
    int retryCounter = -1;

    /** Messages received from upper layer and to be transmitted later */
    std::list<CsmaDataFrame*> transmissionQueue;

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

    /** End of the ack period */
    cMessage *endAck = nullptr;

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
    virtual ~CsmaMac();
    //@}

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual void initialize(int stage) override;
    virtual void initializeQueueModule();
    virtual InterfaceEntry *createInterfaceEntry() override;
    //@}

  protected:
    /**
     * @name Message handing functions
     * @brief Functions called from other classes to notify about state changes and to handle messages.
     */
    //@{
    /** @brief Handle timer self messages */
    virtual void handleSelfMessage(cMessage *msg) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *msg) override;

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerPacket(cPacket *msg) override;

    /** @brief Handle all kinds of messages and notifications with the state machine */
    virtual void handleWithFsm(cMessage *msg);

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG) override;

    virtual CsmaDataFrame *encapsulate(cPacket *msg);
    virtual cPacket *decapsulate(CsmaDataFrame *frame);
    //@}

  protected:
    /**
     * @name Timing functions
     * @brief Calculate various timings based on transmission rate and physical layer charactersitics.
     */
    //@{
    virtual simtime_t getSlotTime();
    virtual simtime_t getSifsTime();
    virtual simtime_t getDifsTime();
    virtual simtime_t computeBackoffPeriod(int r);
    //@}

  protected:
    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
    virtual void scheduleSifsPeriod(CsmaFrame *frame);

    virtual void scheduleDifsPeriod();
    virtual void cancelDifsPeriod();

    virtual void scheduleAckTimeoutPeriod(CsmaDataFrame *frame);
    virtual void cancelAckTimeoutPeriod();

    /** @brief Generates a new backoff period based on the contention window. */
    virtual void invalidateBackoffPeriod();
    virtual bool isInvalidBackoffPeriod();
    virtual void generateBackoffPeriod();
    virtual void decreaseBackoffPeriod();
    virtual void scheduleBackoffPeriod();
    virtual void cancelBackoffPeriod();
    //@}

  protected:
    /**
     * @name Frame transmission functions
     */
    //@{
    virtual void sendAckFrame();
    virtual void sendAckFrame(CsmaDataFrame *frame);
    virtual void sendDataFrame(CsmaDataFrame *frameToSend);
    virtual void sendBroadcastFrame(CsmaDataFrame *frameToSend);
    //@}

  protected:
    /**
     * @name Frame builder functions
     */
    //@{
    virtual CsmaDataFrame *buildDataFrame(CsmaDataFrame *frameToSend);
    virtual CsmaAckFrame *buildAckFrame(CsmaDataFrame *frameToACK);
    virtual CsmaDataFrame *buildBroadcastFrame(CsmaDataFrame *frameToSend);
    //@}

  protected:
    /**
     * @name Utility functions
     */
    //@{
    virtual void finishCurrentTransmission();
    virtual void giveUpCurrentTransmission();
    virtual void retryCurrentTransmission();

    /** @brief Returns the current frame being transmitted */
    virtual CsmaDataFrame *getCurrentTransmission();

    /** @brief Deletes frame at the front of queue. */
    virtual void popTransmissionQueue();

    /** @brief Reset backoff, backoffPeriod and retryCounter for IDLE state */
    virtual void resetStateVariables();

    /** @brief Tells if the medium is free according to the physical and virtual carrier sense algorithm. */
    virtual bool isMediumFree();

    /** @brief Returns true if message is a broadcast message */
    virtual bool isBroadcast(CsmaFrame *msg);

    /** @brief Returns true if message destination address is ours */
    virtual bool isForUs(CsmaFrame *msg);
    //@}
};

} // namespace inet

#endif // ifndef __INET_CSMAMAC_H
