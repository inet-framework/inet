//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t IRateSelection::datarateSelectedSignal = cComponent::registerSignal("datarateSelected");

void IRateSelection::emitDatarateSelected(cComponent *emitter, StationLabelCache& stationLabels, const Ptr<const Ieee80211MacHeader>& header, const IIeee80211Mode *mode)
{
    double rate = mode->getDataMode()->getNetBitrate().get<bps>();
    auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
    if (dataHeader != nullptr && !dataHeader->getReceiverAddress().isMulticast()) {
        cNamedObject details(stationLabels.getLabel(dataHeader->getReceiverAddress()).c_str());
        emitter->emit(datarateSelectedSignal, rate, &details);
    }
    else
        emitter->emit(datarateSelectedSignal, rate);
}

} // namespace ieee80211
} // namespace inet

