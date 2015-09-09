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
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {

namespace ieee80211 {

using namespace physicallayer;

class IIeee80211MacContext;

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
class ITransmissionCompleteCallback;
class IIeee80211MacContext;
class Ieee80211MacImmediateTx;

class INET_API Ieee80211NewMac : public MACProtocolBase
{
  public:

    Ieee80211UpperMac *upperMac = nullptr;
    Ieee80211MacReception *reception = nullptr;
    Ieee80211MacTransmission *transmission = nullptr;
    Ieee80211MacImmediateTx *immediateTx = nullptr;

    IIeee80211MacContext *context = nullptr;  // owned here

  protected:
    /**
     * The last change channel message received and not yet sent to the physical layer, or NULL.
     * The message will be sent down when the state goes to IDLE or DEFER next time.
     */
    cMessage *pendingRadioConfigMsg = nullptr;
    //@}

  protected:
    IRadio *radio = nullptr;
    IRadio::TransmissionState transmissionState = IRadio::TransmissionState::TRANSMISSION_STATE_UNDEFINED;

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

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual int numInitStages() const {return NUM_INIT_STAGES;}
    virtual void initialize(int);
    void receiveSignal(cComponent *source, simsignal_t signalID, long value);
    void configureRadioMode(IRadio::RadioMode radioMode);
    virtual InterfaceEntry *createInterfaceEntry() override;
    virtual const MACAddress& isInterfaceRegistered();
    void transmissionStateChanged(IRadio::TransmissionState transmissionState);

    //@}
  protected:

    /** @brief Handle commands (msg kind+control info) coming from upper layers */
    virtual void handleUpperCommand(cMessage *msg);

    /** @brief Handle timer self messages */
    virtual void handleSelfMessage(cMessage *msg);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *msg);

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerPacket(cPacket *msg);

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

  public:

    virtual void sendFrame(Ieee80211Frame *frameToSend);
   /** @brief Send down the change channel message to the physical layer if there is any. */
    virtual void sendDownPendingRadioConfigMsg();
    //@}
  public:
    virtual simtime_t getSlotTime() const;
    Ieee80211UpperMac *getUpperMac() const { return upperMac; }
    Ieee80211MacReception *getReception() const { return reception; }
    Ieee80211MacTransmission *getTransmission() const { return transmission; }
};

} // namespace ieee80211

} // namespace inet

#endif

