//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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

