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
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

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
        mib->configureBssRole(Ieee80211Mib::ACCESS_POINT, par("ssid").stdstringValue());
        radio = getModuleFromPar<cModule>(par("radioModule"), this);
        radio->subscribe(ieee80211RadioChannelChangedSignal, this);
    }

}

void Ieee80211MgmtApBase::prepareBss()
{
    Enter_Method("prepareBss");
    if (!isUp())
        throw cRuntimeError("Cannot prepare a BSS while AP management is down");
    prepareConfiguration();
    prepareLocalOperation();
}

void Ieee80211MgmtApBase::installSimplifiedPeer(const MacAddress& address, const Ieee80211HtCapabilities *capabilities)
{
    Enter_Method("installSimplifiedPeer");
    prepareBss();
    // This method is the explicit no-air association completion boundary.
    mib->setPeerAssociationStatus(address, Ieee80211Mib::ASSOCIATED);
    if (capabilities != nullptr && mib->isLocalHtCapable())
        mib->setPeerHtCapabilities(address, *capabilities);
    else
        mib->removePeerHtCapabilities(address);
    mib->publishStateChange();
}

void Ieee80211MgmtApBase::removeSimplifiedPeer(const MacAddress& address)
{
    Enter_Method("removeSimplifiedPeer");
    mib->removePeerAssociation(address);
    mib->publishStateChange();
}

void Ieee80211MgmtApBase::prepareLocalOperation()
{
    if (mib->isLocalHtCapable()) {
        int channel = radioChannel;
        if (channel < 0)
            throw cRuntimeError("IEEE 802.11 primary channel is unavailable");
        const auto *band = getHtOperationBand();
        band->getStandardChannelNumber(channel);
        auto operation = computeLocalHtOperation(channel, band);
        mib->commitBss(mib->getBssData().ssid, mib->address, band, channel, &operation);
    }
    else {
        mib->commitBss(mib->getBssData().ssid, mib->address, radioBand,
                radioChannel, nullptr);
    }
}

void Ieee80211MgmtApBase::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (source == radio && signalID == ieee80211RadioChannelChangedSignal) {
        EV << "Updating AP primary channel to " << value << ".\n";
        const auto *channelDetails = dynamic_cast<const physicallayer::Ieee80211RadioChannelChangedDetails *>(details);
        const auto *band = channelDetails == nullptr ? nullptr : channelDetails->getBand();
        if (value < 0 || value > 255)
            throw cRuntimeError("IEEE 802.11 primary channel must be in the range 0..255");
        if (mib->isLocalHtCapable()) {
            if (band == nullptr)
                throw cRuntimeError("HT Operation channel conversion requires radioChannelChanged with IEEE 802.11 band details");
            band->getStandardChannelNumber(value);
        }
        // Physical context is retained while down; it does not activate a BSS.
        radioBand = band;
        radioChannel = value;
        if (mib->hasPreparedLocalCapabilities() && isUp()) {
            prepareLocalOperation();
            mib->publishStateChange();
        }
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
    try {
        return radioBand->getStandardChannelNumber(channelIndex);
    }
    catch (const cRuntimeError&) {
        if (mib->isLocalHtCapable())
            throw;
        // Modeling simplification: nonstandard legacy bands can operate without
        // a standards channel mapping. Omit DSSS rather than invent a wire value.
        // Channel-index validity was checked by getCenterFrequency() above.
        return -1;
    }
}

} // namespace ieee80211

} // namespace inet
