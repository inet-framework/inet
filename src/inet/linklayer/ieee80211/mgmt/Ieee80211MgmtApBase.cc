//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#endif // ifdef INET_WITH_ETHERNET

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtApBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211RadioChannelChangedDetails.h"

namespace inet {

namespace {

const simsignal_t ieee80211RadioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

}

namespace ieee80211 {

void Ieee80211MgmtApBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib->mode = Ieee80211Mib::INFRASTRUCTURE;
        mib->bssStationData.stationType = Ieee80211Mib::ACCESS_POINT;
        mib->bssData.ssid = par("ssid").stdstringValue();
        radio = getModuleFromPar<cModule>(par("radioModule"), this);
        radio->subscribe(ieee80211RadioChannelChangedSignal, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        mib->bssData.bssid = mib->address;
    else if (stage == INITSTAGE_LAST && mib->isHtOperationSupported()) {
        mib->setPrimaryChannel(mib->requirePrimaryChannel(), getHtOperationBand());
        const auto& operation = mib->getHtOperation();
        if (operation.operatingChannelWidth == MHz(40) &&
                !getHtOperationBand()->isHt40OperationSupported(operation.primaryChannel, operation.secondaryChannelOffset))
            throw cRuntimeError("Invalid 40 MHz HT operation for band '%s', primary channel index %d, secondary channel offset %d",
                    getHtOperationBand()->getName(), operation.primaryChannel, operation.secondaryChannelOffset);
    }
}

void Ieee80211MgmtApBase::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (source == radio && signalID == ieee80211RadioChannelChangedSignal) {
        EV << "Updating AP primary channel to " << value << ".\n";
        const auto *channelDetails = dynamic_cast<const physicallayer::Ieee80211RadioChannelChangedDetails *>(details);
        const auto *band = channelDetails == nullptr ? nullptr : channelDetails->getBand();
        if (mib->isHtOperationSupported()) {
            if (band == nullptr)
                throw cRuntimeError("HT Operation channel conversion requires radioChannelChanged with IEEE 802.11 band details");
            mib->setPrimaryChannel(value, band);
        }
        else
            mib->setPrimaryChannel(value);
        radioBand = band;
    }
}

const physicallayer::IIeee80211Band *Ieee80211MgmtApBase::getHtOperationBand() const
{
    if (radioBand == nullptr)
        throw cRuntimeError("HT Operation channel conversion requires radioChannelChanged with IEEE 802.11 band details");
    return radioBand;
}

int Ieee80211MgmtApBase::getDsssParameterSetChannel() const
{
    // IEEE Std 802.11-2024, Tables 9-62 and 9-69, 9.4.2.4:
    // advertise DSSS Current Channel for the modeled 2.4 GHz operation.
    // Omit it for other bands and generic radios without an IEEE channel.
    if (radioBand == nullptr || !mib->hasPrimaryChannel())
        return -1;
    int channelIndex = mib->requirePrimaryChannel();
    auto frequency = radioBand->getCenterFrequency(channelIndex);
    if (frequency < GHz(2.4) || frequency >= GHz(2.5))
        return -1;
    return radioBand->getStandardChannelNumber(channelIndex);
}

} // namespace ieee80211

} // namespace inet
