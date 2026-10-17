//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RX_H
#define __INET_RX_H

#include <optional>
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosAckPolicy.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {
namespace ieee80211 {

class ITx;
class IContention;
class IStatistics;

/**
 * The default implementation of IRx.
 */
class INET_API Rx : public ModeSetListener, public IRx
{
  public:
    static simsignal_t navChangedSignal;

  protected:
    std::vector<IContention *> contentions;

    MacAddress address;
    cMessage *endNavTimer = nullptr;
    cMessage *endTxnavTimer = nullptr;
    physicallayer::IRadio::ReceptionState receptionState = physicallayer::IRadio::RECEPTION_STATE_UNDEFINED;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;
    physicallayer::IRadioSignal::SignalPart receivedPart = physicallayer::IRadioSignal::SIGNAL_PART_NONE;
    bool mediumFree = true; // cached state
    bool contentionFree = true;
    bool contentionBlocked = false;

    struct PrecedingFrame {
        simtime_t end;
        bool ack = false;
        bool ht = false;
        MacAddress transmitter;
        MacAddress receiver;
        bool noSifsResponse = false;
        simtime_t extension = -1;
    };
    struct ReceptionEvidence {
        simtime_t start;
        simtime_t end;
        std::optional<PrecedingFrame> preceding;
        simtime_t sifs;
        simtime_t tolerance;
        bool mayBecomePredecessor = true;
        bool rifsPermitted = false;
        simtime_t rifs;
    };
    opp_component_ptr<cModule> networkInterface;
    std::optional<PrecedingFrame> precedingFrame;
    std::optional<ReceptionEvidence> activeReception;
    std::optional<ReceptionEvidence> completedReception;
    simtime_t ambiguityEnd = -1;
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<IRecipientQosAckPolicy> recipientAckPolicy;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void setOrExtendNav(simtime_t navInterval);
    virtual bool isFcsOk(Packet *packet) const;
    virtual void recomputeMediumFree();
    std::string getRxStatusTxt() const;
    void invalidateFutureInterframeHistory();
    void discardExpiredReception();
    void observeReceptionStart(const physicallayer::IReception *reception);
    void observeReceptionEnd(const physicallayer::IReception *reception);
    bool isCompletedAck(const physicallayer::ITransmission *transmission) const;
    void attachInterframeResult(Packet *packet, bool accepted);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

  public:
    Rx();
    ~Rx();

    virtual bool isReceptionInProgress() const override;
    virtual bool isMediumFree() const override { return mediumFree; }
    virtual void receptionStateChanged(physicallayer::IRadio::ReceptionState newReceptionState) override;
    virtual void transmissionStateChanged(physicallayer::IRadio::TransmissionState transmissionState) override;
    virtual void receivedSignalPartChanged(physicallayer::IRadioSignal::SignalPart part) override;
    virtual bool lowerFrameReceived(Packet *packet) override;
    virtual void invalidateInterframeHistory() override;
    virtual void frameTransmitted(simtime_t durationField) override;
    virtual simtime_t getTxnavRemaining() const override;
    virtual void successfulFrameTransmitted(simtime_t ppduEnd, simtime_t durationField) override;
    virtual void setContentionBlocked(bool blocked) override;
    virtual void registerContention(IContention *contention) override;
};

} // namespace ieee80211
} // namespace inet

#endif
