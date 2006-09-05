//
// Copyright (C) 2006 Andras Varga and Levente Mészáros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef IEEE_80211_MAC_H
#define IEEE_80211_MAC_H

// uncomment this if you do not want to log state machine transitions
#define FSM_DEBUG

#include <list>
#include "WirelessMacBase.h"
#include "IPassiveQueue.h"
#include "Ieee80211Frame_m.h"
#include "Ieee80211Consts.h"
#include "NotificationBoard.h"
#include "RadioState.h"
#include "FSMA.h"

/**
 * IEEE 802.11b Media Access Control Layer.
 *
 * Various comments in the code refer to the Wireless LAN Medium Access
 * Control (MAC) and Physical Layer(PHY) Specifications
 * ANSI/IEEE Std 802.11, 1999 Edition (R2003)
 *
 * For more info, see the NED file.
 *
 * TODO: support fragmentation
 * TODO: PCF mode
 * TODO: CF period
 * TODO: pass radio power to upper layer
 * TODO: transmission complete notification to upper layer
 * TODO: STA TCF timer syncronization, see Chapter 11 pp 123
 *
 * Parts of the implementation have been taken over from the
 * Mobility Framework's Mac80211 module.
 *
 * @ingroup macLayer
 */
class INET_API Ieee80211Mac : public WirelessMacBase, public INotifiable
{
  typedef std::list<Ieee80211DataOrMgmtFrame*> Ieee80211DataOrMgmtFrameList;

  /**
   * This is used to populate fragments and identify duplicated messages. See spec 9.2.9.
   */
  struct Ieee80211ASFTuple
  {
      MACAddress address;
      int sequenceNumber;
      int fragmentNumber;
  };

  typedef std::list<Ieee80211ASFTuple*> Ieee80211ASFTupleList;

  protected:
    /**
     * @name Configuration parameters
     * These are filled in during the initialization phase and not supposed to change afterwards.
     */
    //@{
    /** MAC address */
    MACAddress address;

    /** The bitrate is used to send data and mgmt frames; be sure to use a valid 802.11 bitrate */
    double bitrate;

    /** The basic bitrate (1 or 2 Mbps) is used to transmit control frames */
    double basicBitrate;

    /** Maximum number of frames in the queue; should be set in the omnetpp.ini */
    int maxQueueSize;

    /**
     * The minimum length of MPDU to use RTS/CTS mechanism. 0 means always, extremely
     * large value means never. See spec 9.2.6 and 361.
     */
    int rtsThreshold;

    /** Maximum number of retries. */
    int retryLimit;

    /** Minimum contention window. */
    int cwMinData;

    /** Contention window size for broadcast messages. */
    int cwMinBroadcast;

    /** Messages longer than this threshold will be sent in multiple fragments. see spec 361 */
    static const int fragmentationThreshold = 2346;
    //@}

  protected:
    /**
     * @name Ieee80211Mac state variables
     * Various state information checked and modified according to the state machine.
     */
    //@{
    /** the 80211 MAC state machine */
    enum State {
        IDLE,
        DEFER,
        WAITDIFS,
        BACKOFF,
        WAITACK,
        WAITBROADCAST,
        WAITCTS,
        WAITSIFS,
        RECEIVE,
    };
    cFSM fsm;

    /** 80211 MAC operation modes */
    enum Mode {
        DCF,  ///< Distributed Coordination Function
        PCF,  ///< Point Coordination Function
    };
    Mode mode;

    /** Sequence number to be assigned to the next frame */
    int sequenceNumber;

    /**
     * Indicates that the last frame received had bit errors in it or there was a
     * collision during receiving the frame. If this flag is set, then the MAC
     * will wait EIFS instead of DIFS period of time in WAITDIFS state.
     */
    bool lastReceiveFailed;

    /** True if backoff is enabled */
    bool backoff;

    /** True during network allocation period. This flag is present to be able to watch this state. */
    bool nav;

    /** Remaining backoff period in seconds */
    double backoffPeriod;

    /**
     * Number of frame retransmission attempts, this is a simpification of
     * SLRC and SSRC, see 9.2.4 in the spec
     */
    int retryCounter;

    /** Physical radio (medium) state copied from physical layer */
    RadioState::State radioState;

    /** Messages received from upper layer and to be transmitted later */
    Ieee80211DataOrMgmtFrameList transmissionQueue;

    /**
     * A list of last sender, sequence and fragment number tuples to identify
     * duplicates, see spec 9.2.9.
     * TODO: this is not yet used
     */
    Ieee80211ASFTupleList asfTuplesList;

    /** Passive queue module to request messages from */
    IPassiveQueue *queueModule;

    /**
     * The last change channel message received and not yet sent to the physical layer, or NULL.
     * The message will be sent down when the state goes to IDLE or DEFER next time.
     */
    cMessage *pendingRadioConfigMsg;
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

    /** Timeout after the transmission of an RTS, a CTS, or a DATA frame */
    cMessage *endTimeout;

    /** End of medium reserve period (NAV) when two other nodes were communicating on the channel */
    cMessage *endReserve;

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
    cOutVector stateVector;
    cOutVector radioStateVector;
    //@}

  public:
    /**
     * @name Construction functions
     */
    //@{
    Ieee80211Mac();
    virtual ~Ieee80211Mac();
    //@}

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);
    virtual void registerInterface();
    virtual void initializeQueueModule();
    //@}

  protected:
    /**
     * @name Message handing functions
     * @brief Functions called from other classes to notify about state changes and to handle messages.
     */
    //@{
    /** @brief Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, cPolymorphic* details);

    /** @brief Handle commands (msg kind+control info) coming from upper layers */
    virtual void handleCommand(cMessage *msg);

    /** @brief Handle timer self messages */
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage *msg);

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerMsg(cMessage *msg);

    /** @brief Handle all kinds of messages and notifications with the state machine */
    virtual void handleWithFSM(cMessage *msg);
    //@}

  protected:
    /**
     * @name Timing functions
     * @brief Calculate various timings based on transmission rate and physical layer charactersitics.
     */
    //@{
    simtime_t SIFSPeriod();
    simtime_t SlotPeriod();
    simtime_t DIFSPeriod();
    simtime_t EIFSPeriod();
    simtime_t PIFSPeriod();
    simtime_t BackoffPeriod(Ieee80211Frame *msg, int r);
    //@}

  protected:
    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
    void scheduleSIFSPeriod(Ieee80211Frame *frame);

    void scheduleDIFSPeriod();
    void cancelDIFSPeriod();

    void scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frame);
    void scheduleBroadcastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frame);
    void cancelTimeoutPeriod();

    void scheduleCTSTimeoutPeriod();

    /** @brief Schedule network allocation period according to 9.2.5.4. */
    void scheduleReservePeriod(Ieee80211Frame *frame);

    /** @brief Generates a new backoff period based on the contention window. */
    void invalidateBackoffPeriod();
    bool isInvalidBackoffPeriod();
    void generateBackoffPeriod();
    void decreaseBackoffPeriod();
    void scheduleBackoffPeriod();
    void cancelBackoffPeriod();
    //@}

  protected:
    /**
     * @name Frame transmission functions
     */
    //@{
    void sendACKFrameOnEndSIFS();
    void sendACKFrame(Ieee80211DataOrMgmtFrame *frame);
    void sendRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    void sendCTSFrameOnEndSIFS();
    void sendCTSFrame(Ieee80211RTSFrame *rtsFrame);
    void sendDataFrameOnEndSIFS(Ieee80211DataOrMgmtFrame *frameToSend);
    void sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    void sendBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    //@}

  protected:
    /**
     * @name Frame builder functions
     */
    //@{
    Ieee80211DataOrMgmtFrame *buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    Ieee80211ACKFrame *buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK);
    Ieee80211RTSFrame *buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    Ieee80211CTSFrame *buildCTSFrame(Ieee80211RTSFrame *rtsFrame);
    Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    //@}

    /**
     * @brief Attaches a PhyControlInfo to the frame which will cause it to be sent at
     * basicBitrate not bitrate (e.g. 2Mbps instead of 11Mbps). Used with ACK, CTS, RTS.
     */
    Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame);

  protected:
    /**
     * @name Utility functions
     */
    //@{
    void finishCurrentTransmission();
    void giveUpCurrentTransmission();
    void retryCurrentTransmission();

   /** @brief Send down the change channel message to the physical layer if there is any. */
    void sendDownPendingRadioConfigMsg();

    /** @brief Change the current MAC operation mode. */
    void setMode(Mode mode);

    /** @brief Returns the current frame being transmitted */
    Ieee80211DataOrMgmtFrame *currentTransmission();

    /** @brief Reset backoff, backoffPeriod and retryCounter for IDLE state */
    void resetStateVariables();

    /** @brief Used by the state machine to identify medium state change events.
        This message is currently optimized away and not sent through the kernel. */
    bool isMediumStateChange(cMessage *msg);

    /** @brief Tells if the medium is free according to the physical and virtual carrier sense algorithm. */
    bool isMediumFree();

    /** @brief Returns true if message is a broadcast message */
    bool isBroadcast(Ieee80211Frame *msg);

    /** @brief Returns true if message destination address is ours */
    bool isForUs(Ieee80211Frame *msg);

    /** @brief Checks if the frame is a data or management frame */
    bool isDataOrMgmtFrame(Ieee80211Frame *frame);

    /** @brief Returns the last frame received before the SIFS period. */
    Ieee80211Frame *frameReceivedBeforeSIFS();

    /** @brief Deletes frame at the front of queue. */
    void popTransmissionQueue();

    /**
     * @brief Computes the duration (in seconds) of the transmission of a frame
     * over the physical channel. 'bits' should be the total length of the MAC frame
     * in bits, but excluding the physical layer framing (preamble etc.)
     */
    double frameDuration(Ieee80211Frame *msg);
    double frameDuration(int bits, double bitrate);

    /** @brief Logs all state information */
    void logState();

    /** @brief Produce a readable name of the given MAC operation mode */
    const char *modeName(int mode);
    //@}
};

#endif

