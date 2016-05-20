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

#ifndef __INET_NEWCSMA_H
#define __INET_NEWCSMA_H

#include "inet/common/FSMA.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/newcsma/NewCsmaFrame_m.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

using namespace inet::physicallayer;

// frame lengths in bits
const unsigned int LENGTH_ACK = 112;

const_simtime_t MAX_PROPAGATION_DELAY = 2E-6;  // 300 meters at the speed of light

const int PHY_HEADER_LENGTH = 192;
const double BITRATE_HEADER = 1E+6;
const double BANDWIDTH = 2E+6;

class INET_API NewCsmaMac : public MACProtocolBase
{
  typedef std::list<NewCsmaDataFrame*> NewCsmaDataFrameList;

  protected:
    IRadio *radio;

    /**
     * @name Configuration parameters
     */
    //@{
    /** MAC address */
    MACAddress address;

    /** The bitrate is used to send data frames; be sure to use a valid 802.11 bitrate */
    double bitrate;

    /** Maximum number of frames in the queue; should be set in the omnetpp.ini */
    int maxQueueSize;

    /**
     * Maximum number of transmissions for a message.
     * This includes the initial transmission and all subsequent retransmissions.
     * Thus a value 0 is invalid and a value 1 means no retransmissions.
     */
    int retryLimit;

    /** Minimum contention window. */
    int cwMin;

    /** Maximum contention window. */
    int cwMax;
    //@}

  public:
    /**
     * @name NewCsmaMac state variables
     * Various state information checked and modified according to the state machine.
     */
    //@{
    enum State {
        IDLE,
        DEFER,
        WAITDIFS,
        BACKOFF,
        WAITACK,
        WAITBROADCAST,
        WAITSIFS,
        RECEIVE,
    };
  protected:
    cFSM fsm;

  protected:
    /** True if backoff is enabled */
    bool backoff;

    /** Remaining backoff period in seconds */
    simtime_t backoffPeriod;

    /**
     * Number of frame retransmission attempts.
     */
    int retryCounter;

    /** Messages received from upper layer and to be transmitted later */
    NewCsmaDataFrameList transmissionQueue;

    /** Passive queue module to request messages from */
    IPassiveQueue *queueModule;
    //@}

  protected:
    /** @name Timer messages */
    //@{
    /** End of the Short Inter-Frame Time period */
    cMessage *endSIFS;

    /** End of the Data Inter-Frame Time period */
    cMessage *endDIFS;

    /** End of the backoff period */
    cMessage *endBackoff;

    /** Timeout after the transmission of a DATA frame */
    cMessage *endTimeout;

    /** Radio state change self message. Currently this is optimized away and sent directly */
    cMessage *mediumStateChange;
    //@}

  protected:
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
    NewCsmaMac();
    virtual ~NewCsmaMac();
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
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG) override;

    /** @brief Handle timer self messages */
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *msg) override;

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerPacket(cPacket *msg) override;

    /** @brief Handle all kinds of messages and notifications with the state machine */
    virtual void handleWithFSM(cMessage *msg);

    virtual NewCsmaDataFrame *encapsulate(cPacket *msg);
    virtual cPacket *decapsulate(NewCsmaDataFrame *frame);
    //@}

  protected:
    /**
     * @name Timing functions
     * @brief Calculate various timings based on transmission rate and physical layer charactersitics.
     */
    //@{
    virtual simtime_t getSIFS();
    virtual simtime_t getSlotTime();
    virtual simtime_t getDIFS();
    virtual simtime_t computeBackoffPeriod(NewCsmaFrame *msg, int r);
    //@}

  protected:
    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
    virtual void scheduleSIFSPeriod(NewCsmaFrame *frame);

    virtual void scheduleDIFSPeriod();
    virtual void cancelDIFSPeriod();

    virtual void scheduleDataTimeoutPeriod(NewCsmaDataFrame *frame);
    virtual void scheduleBroadcastTimeoutPeriod(NewCsmaDataFrame *frame);
    virtual void cancelTimeoutPeriod();

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
    virtual void sendACKFrame();
    virtual void sendACKFrame(NewCsmaDataFrame *frame);
    virtual void sendDataFrame(NewCsmaDataFrame *frameToSend);
    virtual void sendBroadcastFrame(NewCsmaDataFrame *frameToSend);
    //@}

  protected:
    /**
     * @name Frame builder functions
     */
    //@{
    virtual NewCsmaDataFrame *buildDataFrame(NewCsmaDataFrame *frameToSend);
    virtual NewCsmaAckFrame *buildACKFrame(NewCsmaDataFrame *frameToACK);
    virtual NewCsmaDataFrame *buildBroadcastFrame(NewCsmaDataFrame *frameToSend);
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
    virtual NewCsmaDataFrame *getCurrentTransmission();

    /** @brief Reset backoff, backoffPeriod and retryCounter for IDLE state */
    virtual void resetStateVariables();

    /** @brief Used by the state machine to identify medium state change events.
        This message is currently optimized away and not sent through the kernel. */
    virtual bool isMediumStateChange(cMessage *msg);

    /** @brief Tells if the medium is free according to the physical and virtual carrier sense algorithm. */
    virtual bool isMediumFree();

    /** @brief Returns true if message is a broadcast message */
    virtual bool isBroadcast(NewCsmaFrame *msg);

    /** @brief Returns true if message destination address is ours */
    virtual bool isForUs(NewCsmaFrame *msg);

    /** @brief Deletes frame at the front of queue. */
    virtual void popTransmissionQueue();

    /**
     * @brief Computes the duration (in seconds) of the transmission of a frame
     * over the physical channel. 'bits' should be the total length of the MAC frame
     * in bits, but excluding the physical layer framing (preamble etc.)
     */
    virtual double computeFrameDuration(NewCsmaFrame *msg);
    virtual double computeFrameDuration(int bits, double bitrate);
    //@}
};

} // namespace inet

#endif // ifndef __INET_NEWCSMA_H

