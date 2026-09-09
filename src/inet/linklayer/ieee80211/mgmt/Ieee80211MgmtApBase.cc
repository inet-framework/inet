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
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"
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
        if (mib->isHtOperationSupported()) {
            const auto *radioContract = check_and_cast<const physicallayer::IRadio *>(radio);
            const auto *receiverWidths = check_and_cast<const physicallayer::Ieee80211Receiver *>(radioContract->getReceiver());
            const auto *transmitterWidths = check_and_cast<const physicallayer::Ieee80211Transmitter *>(radioContract->getTransmitter());
            mib->setPrimaryChannel(value, getHtOperationBand(),
                    receiverWidths->isHtChannelWidthSupported(MHz(40)) && transmitterWidths->isHtChannelWidthSupported(MHz(40)));
        }
        else
            mib->setPrimaryChannel(value);
    }
}

const physicallayer::IIeee80211Band *Ieee80211MgmtApBase::getHtOperationBand() const
{
    if (radio == nullptr)
        throw cRuntimeError("HT Operation channel conversion requires a configured radioModule");
    const auto *radioContract = dynamic_cast<const physicallayer::IRadio *>(radio);
    if (radioContract == nullptr)
        throw cRuntimeError("HT Operation channel conversion requires radioModule to reference a radio, got %s", radio->getClassName());
    const auto *transmitter = dynamic_cast<const physicallayer::Ieee80211Transmitter *>(radioContract->getTransmitter());
    if (transmitter == nullptr)
        throw cRuntimeError("HT Operation channel conversion requires radioModule's transmitter to provide an IEEE 802.11 channel");
    const auto *channel = transmitter->getChannel();
    if (channel == nullptr || channel->getBand() == nullptr)
        throw cRuntimeError("HT Operation channel conversion requires radioModule's IEEE 802.11 transmitter to have a configured channel and band");
    return channel->getBand();
}

} // namespace ieee80211

} // namespace inet
