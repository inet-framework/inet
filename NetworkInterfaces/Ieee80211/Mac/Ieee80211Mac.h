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
 * @brief An implementation of the 802.11b MAC.
 *
 * For more info, see the NED file.
 *
 * @ingroup macLayer
 */
class INET_API Ieee80211Mac : public WirelessMacBase, public INotifiable
{
  typedef std::list<Ieee80211DataOrMgmtFrame*> MacPktList;

  protected:
    /**
     * @name Parameters
     * @brief These are filled in during the initialization phase and not supposed to change afterwards.
     */
    //@{
    /** @brief MAC address */
    MACAddress address;

    /** @brief The bitrate should be set in omnetpp.ini; be sure to use a valid 802.11 bitrate */
    double bitrate;

    /** @brief Maximal number of frames in the queue; should be set in the omnetpp.ini */
    int maxQueueSize;
    //@}

  protected:
    /**
     * @name Ieee80211Mac state variables
     * @brief Various state information checked and modified according to the state machine.
     */
    //@{
    /** @brief the 80211 MAC state machine */
    enum State {
        IDLE,
        DEFER,
        WAITDIFS,
        BACKOFF,
        TRANSMITTING,
        WAITSIFS,
        TRANSMITTING_RTS_CTS,
        RECEIVING_RTS_CTS,
        RESERVE,
    };
    cFSM fsm;

    /** @brief 80211 MAC operation modes */
    enum Mode {
        DCF,  // Distributed Coordination Function
        MACA, // DCF with RTS/CTS (Request To Send / Clear To Send)
        PCF,  // Point Coordination Function
    };
    Mode mode;

    /** @brief true if backoff is enabled */
    bool backoff;

    /** @brief Remaining backoff period in seconds */
    double backoffPeriod;

    /** @brief Number of frame retransmission attempts */
    int retryCounter;

    /** @brief Physical radio (medium) state copied from physical layer */
    RadioState::States radioState;

    /** @brief Messages received from upper layer and to be transmitted later */
    MacPktList transmissionQueue;

    /** @brief Passive queue module to request messages from */
    IPassiveQueue *queueModule;
    //@}

  protected:
    /**
     * @name Timer self messages
     */
    //@{
    /** @brief End of the Short Inter-Frame Time period */
    cMessage *endSIFS;

    /** @brief End of the Data Inter-Frame Time period */
    cMessage *endDIFS;

    /** @brief End of the backoff period */
    cMessage *endBackoff;

    /** @brief Timeout after the transmission of an RTS, a CTS, or a DATA frame */
    cMessage *endTimeout;

    /** @brief End of medium reserve period when two other nodes were communicating on the channel */
    cMessage *endReserve;

    /** @brief Radio state change self message. Currently this is optimized away and sent directly */
    cMessage *radioStateChange;
    //@}

  protected:
    /**
     * @name Statistics
     */
    //@{
    long numRetry;
    long numSentWithoutRetry;
    long numGivenUp;
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
     * @name Timer functions
     */
    //@{
    void scheduleSIFSPeriod(Ieee80211Frame *frame);
    void scheduleDIFSPeriod();
    void scheduleBackoffPeriod();
    void scheduleTimeoutPeriod(Ieee80211Frame *frame);
    void scheduleRTSTimeoutPeriod();
    void scheduleReservePeriod(Ieee80211Frame *frame);
    //@}


  protected:
    /**
     * @name Frame transmission functions
     */
    //@{
    void sendACKFrame(Ieee80211DataOrMgmtFrame *frame);
    void sendRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    void sendCTSFrame(Ieee80211RTSFrame *rtsFrame);
    void sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    void sendBroadcastFrame(Ieee80211Frame *frameToSend);
    //@}

  protected:
    /**
     * @name Frame builder functions
     */
    //@{
    Ieee80211DataFrame *encapsulate(cMessage *frame);
    cMessage *decapsulate(Ieee80211DataFrame *frame);
    Ieee80211DataOrMgmtFrame *buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    Ieee80211ACKFrame *buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK);
    Ieee80211RTSFrame *buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    Ieee80211CTSFrame *buildCTSFrame(Ieee80211RTSFrame *rtsFrame);
    Ieee80211Frame *buildBroadcastFrame(Ieee80211Frame *frameToSend);
    //@}

  protected:
    /**
     * @name Utility functions
     */
    //@{
    /** @brief Change the current MAC operation mode. */
    void setMode(Mode mode);

    /** @brief Reset backoff, backoffPeriod and retryCounter for IDLE state */
    void resetStateVariables();

    /** @brief Generates a new backoff period based on the contention window. */
    void generateBackoffPeriod();

    /** @brief Compute the contention window with the binary backoff algorithm. */
    int contentionWindow();

    /** @brief Used by the state machine to identify radio state change events. This message is currently optimized away. */
    bool isRadioStateChange(cMessage *msg);

    /** @brief Returns true if message is a broadcast message */
    bool isBroadcast(Ieee80211DataOrMgmtFrame *msg);

    /** @brief Returns true if message destination address is ours */
    bool isForUs(Ieee80211Frame *msg);

    /** @brief Deletes frame from front of queue. */
    void popTransmissionQueue();

    /** @brief Computes the duration (in seconds) of the transmission of a frame over the physical channel.
               'bits' should be the total length of the MAC frame in bits. */
    double frameDuration(int bits);

    /** @brief Logs all state information */
    void logState();

    /** @brief Produce a readable name of the given frame type */
    const char *frameTypeName(int type);

    /** @brief Produce a readable name of the given MAC operation mode */
    const char *modeName(int mode);
    //@}
};

#endif