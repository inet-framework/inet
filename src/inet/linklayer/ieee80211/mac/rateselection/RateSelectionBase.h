//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATESELECTIONBASE_H
#define __INET_RATESELECTIONBASE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

class INET_API RateSelectionBase : public ModeSetListener
{
  protected:
    IRateControl *dataOrMgmtRateControl = nullptr;
    ModuleRefByPar<Ieee80211Mib> mib;
    const physicallayer::IIeee80211Mode *fastestMandatoryMode = nullptr;

    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    const Ieee80211RateSetState& getBssRateSetForReceiver(const MacAddress& receiver) const;
    const MacAddress& getResponsePeer(const Ptr<const Ieee80211TwoAddressHeader>& header) const;

    const physicallayer::IIeee80211Mode *getPeerCompatibleMode(const MacAddress& peerAddress,
            const physicallayer::IIeee80211Mode *mode) const;
    const physicallayer::IIeee80211Mode *selectAllowedMode(const MacAddress& peerAddress,
            const physicallayer::IIeee80211Mode *mode, bool groupAddressed = false) const;
    const physicallayer::IIeee80211Mode *selectBasicMode(const physicallayer::IIeee80211Mode *upperBoundMode,
            const MacAddress& receiver, bool allowHtBasicMcs) const;
    const physicallayer::IIeee80211Mode *computePrimaryResponseMode(
            const physicallayer::IIeee80211Mode *elicitingMode,
            Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver) const;
    const physicallayer::IIeee80211Mode *validateResponseOverride(
            const physicallayer::IIeee80211Mode *computedMode,
            const physicallayer::IIeee80211Mode *configuredMode,
            Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver) const;

    bool isAllowedByRateState(const physicallayer::IIeee80211Mode *mode,
            const MacAddress& receiver, bool groupAddressed) const;
    const physicallayer::IIeee80211Mode *selectGroupMode(const physicallayer::IIeee80211Mode *configuredMode,
            const physicallayer::IIeee80211Mode *preferredMode = nullptr) const;
    const physicallayer::IIeee80211Mode *validateConfiguredMode(const physicallayer::IIeee80211Mode *mode,
            const MacAddress& receiver, const char *frameClass) const;

  public:
    virtual ~RateSelectionBase() {}
};

} // namespace ieee80211
} // namespace inet

#endif
