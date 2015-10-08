//
// Copyright (C) 2006 Andras Varga and Levente M�sz�ros
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

#ifndef IEEE_80211_MAC_H
#define IEEE_80211_MAC_H

// uncomment this if you do not want to log state machine transitions
#define FSM_DEBUG

#include <list>

#include "inet/common/INETDefs.h"

#include "inet_old/linklayer/ieee80211/mac/WirelessMacBase.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet_old/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet_old/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet_old/base/NotificationBoard.h"
#include "inet_old/linklayer/contract/RadioState.h"

namespace inet {

class Ieee80211UpperMac;
class Ieee80211MacReception;
class Ieee80211MacTransmission;

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

class Ieee80211UpperMac;
class Ieee80211MacReception;
class Ieee80211MacTransmission;

class INET_API Ieee80211NewMac : public WirelessMacBase, public INotifiable
{
  public:

    Ieee80211UpperMac *upperMac = nullptr;
    Ieee80211MacReception *reception = nullptr;
    Ieee80211MacTransmission *transmission = nullptr;

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


    /**
     * Maximum number of transmissions for a message.
     * This includes the initial transmission and all subsequent retransmissions.
     * Thus a value 0 is invalid and a value 1 means no retransmissions.
     * See: dot11ShortRetryLimit on page 484.
     *   'This attribute shall indicate the maximum number of
     *    transmission attempts of a frame, the length of which is less
     *    than or equal to dot11RTSThreshold, that shall be made before a
     *    failure condition is indicated. The default value of this
     *    attribute shall be 7'
     */
    int transmissionLimit;

    int rtsThreshold;

  protected:
    /**
     * The last change channel message received and not yet sent to the physical layer, or NULL.
     * The message will be sent down when the state goes to IDLE or DEFER next time.
     */
    cMessage *pendingRadioConfigMsg;
    //@}

  protected:

    cMessage *endImmediateIFS = nullptr;
    cMessage *immediateFrameDuration = nullptr;
    Ieee80211Frame *immediateFrame = nullptr;

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
    static simsignal_t stateSignal;
    static simsignal_t radioStateSignal;
    //@}

  public:
    /**
     * @name Construction functions
     */
    //@{
    Ieee80211NewMac();
    virtual ~Ieee80211NewMac();
    //@}

    MACAddress getAddress() const { return address; }
    void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t deferDuration);

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);
    virtual void registerInterface();
    //@}
  protected:
    /**
     * @name Message handing functions
     * @brief Functions called from other classes to notify about state changes and to handle messages.
     */
    //@{
    /** @brief Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic * details);

    /** @brief Handle commands (msg kind+control info) coming from upper layers */
    virtual void handleCommand(cMessage *msg);

    /** @brief Handle timer self messages */
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cPacket *msg);

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerMsg(cPacket *msg);

    bool isForUs(Ieee80211Frame *frame) const;

  public:

    virtual void sendDataFrame(Ieee80211Frame *frameToSend);
   /** @brief Send down the change channel message to the physical layer if there is any. */
    virtual void sendDownPendingRadioConfigMsg();
    //@}
  public:
    virtual simtime_t getSlotTime() const;
    Ieee80211UpperMac *getUpperMac() const { return upperMac; }
    Ieee80211MacReception *getReception() const { return reception; }
    Ieee80211MacTransmission *getTransmission() const { return transmission; }
};

} // namespace inet

#endif

