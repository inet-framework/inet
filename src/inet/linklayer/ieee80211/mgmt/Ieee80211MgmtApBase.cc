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
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"

namespace inet {

namespace ieee80211 {

void Ieee80211MgmtApBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib->mode = Ieee80211Mib::INFRASTRUCTURE;
        mib->bssStationData.stationType = Ieee80211Mib::ACCESS_POINT;
        mib->bssData.ssid = par("ssid").stdstringValue();
        auto radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(physicallayer::Ieee80211Radio::radioChannelChangedSignal, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        mib->bssData.bssid = mib->address;
}

void Ieee80211MgmtApBase::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == physicallayer::Ieee80211Radio::radioChannelChangedSignal) {
        EV << "Updating AP primary channel to " << value << ".\n";
        mib->setPrimaryChannel(value);
    }
}

} // namespace ieee80211

} // namespace inet
