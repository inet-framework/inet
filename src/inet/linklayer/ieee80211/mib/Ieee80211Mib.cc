//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Mib);

void Ieee80211Mib::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH(address);
        WATCH(mode);
        WATCH(qos);
        WATCH(bssData.bssid);
        WATCH(bssStationData.stationType);
        WATCH(bssStationData.isAssociated);
        WATCH_MAP(bssAccessPointData.stations);
    }
}

void Ieee80211Mib::refreshDisplay() const
{
    std::string modeString;
    std::string ssidString;
    switch (mode) {
        case INFRASTRUCTURE: modeString = "Infrastructure"; ssidString = "\nSSID: " + bssData.ssid + ", " + bssData.bssid.str(); break;
        case INDEPENDENT: modeString = "Ad-hoc"; break;
        case MESH: modeString = "Mesh"; break;
    }
    std::string bssStationTypeString;
    std::string associatedString;
    switch (bssStationData.stationType) {
        case ACCESS_POINT: bssStationTypeString = ", AP"; break;
        case STATION: bssStationTypeString = ", STA"; associatedString = bssStationData.isAssociated ? "\nAssociated" : "\nNot associated"; break;
    }
    auto text = std::string("Address: ") + address.str() + ssidString + "\n" + modeString + bssStationTypeString + (qos ? ", QoS" : ", Non-QoS") + associatedString;
    getDisplayString().setTagArg("t", 0, text.c_str());
}

} // namespace ieee80211

} // namespace inet

