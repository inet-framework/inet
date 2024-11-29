//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ETHERNETCSMAMAC_H
#define __INET_ETHERNETCSMAMAC_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ethernet/base/EthernetModes.h"
#include "inet/linklayer/ethernet/basic/IEthernetCsmaMac.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/physicallayer/wired/ethernet/IEthernetCsmaPhy.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

/**
 * Ethernet MAC module which supports both half-duplex (CSMA/CD) and full-duplex
 * operation. (See also EthernetMacPhy which has a considerably smaller
 * code with all the CSMA/CD complexity removed.)
 *
 * See NED file for more details.
 */
class INET_API EthernetCsmaMac : public MacProtocolBase, public virtual IEthernetCsmaMac, public virtual IActivePacketSink
{
  public:
    static simsignal_t carrierSenseChangedSignal;
    static simsignal_t collisionChangedSignal;
    static simsignal_t stateChangedSignal;

    enum State {
        IDLE,        // neither transmitting nor receiving
        WAIT_IFG,    // waiting for inter-frame gap after transmission or reception
        TRANSMITTING,
        JAMMING,     // collision detected sending jam signal
        BACKOFF,     // waiting for a random period before subsequent channel access
        RECEIVING,
    };

  protected:
    enum Event {
        UPPER_PACKET,
        LOWER_PACKET,
        CARRIER_SENSE_START,
        CARRIER_SENSE_END,
        COLLISION_START,
        END_TX_TIMER,
        END_IFG_TIMER,
        END_JAM_TIMER,
        END_BACKOFF_TIMER,
    };

  protected:
    // parameters
    FcsMode fcsMode;
    bool promiscuous = false;
    EthernetModes::EthernetMode mode;

    // environment
    opp_component_ptr<IEthernetCsmaPhy> phy = nullptr;

    // state
    Fsm fsm;
    int numRetries = 0; // for exponential back-off algorithm
    bool carrierSense = false;
    bool collision = false;

    // timers
    cMessage *txTimer = nullptr;
    cMessage *ifgTimer = nullptr;
    cMessage *jamTimer = nullptr;
    cMessage *backoffTimer = nullptr;

    // statistics

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual void configureNetworkInterface() override;

    virtual void handleWithFsm(int event, cMessage *message);

    virtual void setCurrentTransmission(Packet *packet);
    virtual void startTransmission();
    virtual void endTransmission();
    virtual void abortTransmission();
    virtual void retryTransmission();
    virtual void giveUpTransmission();

    virtual void processReceivedFrame(Packet *packet);

    virtual bool isFrameNotForUs(const Ptr<const EthernetMacHeader>& header);
    virtual MacAddress getMacAddress() { return networkInterface ? networkInterface->getMacAddress() : MacAddress::UNSPECIFIED_ADDRESS; }

    virtual void addPaddingAndSetFcs(Packet *packet, B requiredMinBytes) const;

    virtual void scheduleTxTimer(Packet *packet);
    virtual void scheduleIfgTimer();
    virtual void scheduleJamTimer();
    virtual void scheduleBackoffTimer();

  public:
    virtual ~EthernetCsmaMac();

    virtual void handleSelfMessage(cMessage *message) override;

    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

    virtual void handleCarrierSenseStart() override;
    virtual void handleCarrierSenseEnd() override;

    virtual void handleCollisionStart() override;
    virtual void handleCollisionEnd() override;

    virtual void handleReceptionStart(EthernetSignalType signalType, Packet *packet) override;
    virtual void handleReceptionError(EthernetSignalType signalType, Packet *packet) override;
    virtual void handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1) override;

    virtual IPassivePacketSource *getProvider(const cGate *gate) override;
    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override { throw cRuntimeError("Not supported"); }
};

} // namespace inet

#endif

