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
        WATCH(bssAccessPointData.stations);
        WATCH_LAMBDA("modeStr", [this]() -> std::string {
            switch (mode) {
                case INFRASTRUCTURE: return "Infrastructure";
                case INDEPENDENT: return "Ad-hoc";
                case MESH: return "Mesh";
                default: return "?";
            }
        });
        WATCH_LAMBDA("ssidStr", [this]() -> std::string {
            return mode == INFRASTRUCTURE ? "\nSSID: " + bssData.ssid + ", " + bssData.bssid.str() : "";
        });
        WATCH_LAMBDA("stationTypeStr", [this]() -> std::string {
            switch (bssStationData.stationType) {
                case ACCESS_POINT: return ", AP";
                case STATION: return ", STA";
                default: return "";
            }
        });
        WATCH_LAMBDA("qosStr", [this]() -> std::string { return qos ? ", QoS" : ", Non-QoS"; });
        WATCH_LAMBDA("associatedStr", [this]() -> std::string {
            return bssStationData.stationType == STATION ? (bssStationData.isAssociated ? "\nAssociated" : "\nNot associated") : "";
        });
    }
}

} // namespace ieee80211

} // namespace inet

