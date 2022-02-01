//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RX_H
#define __INET_RX_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

class ITx;
class IContention;
class IStatistics;

/**
 * The default implementation of IRx.
 */
class INET_API Rx : public cSimpleModule, public IRx
{
  public:
    static simsignal_t navChangedSignal;

  protected:
    std::vector<IContention *> contentions;

    MacAddress address;
    cMessage *endNavTimer = nullptr;
    physicallayer::IRadio::ReceptionState receptionState = physicallayer::IRadio::RECEPTION_STATE_UNDEFINED;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;
    physicallayer::IRadioSignal::SignalPart receivedPart = physicallayer::IRadioSignal::SIGNAL_PART_NONE;
    bool mediumFree = true; // cached state

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void setOrExtendNav(simtime_t navInterval);
    virtual bool isFcsOk(Packet *packet) const;
    virtual void recomputeMediumFree();
    virtual void refreshDisplay() const override;

  public:
    Rx();
    ~Rx();

    virtual bool isReceptionInProgress() const override;
    virtual bool isMediumFree() const override { return mediumFree; }
    virtual void receptionStateChanged(physicallayer::IRadio::ReceptionState newReceptionState) override;
    virtual void transmissionStateChanged(physicallayer::IRadio::TransmissionState transmissionState) override;
    virtual void receivedSignalPartChanged(physicallayer::IRadioSignal::SignalPart part) override;
    virtual bool lowerFrameReceived(Packet *packet) override;
    virtual void frameTransmitted(simtime_t durationField) override;
    virtual void registerContention(IContention *contention) override;
};

} // namespace ieee80211
} // namespace inet

#endif

