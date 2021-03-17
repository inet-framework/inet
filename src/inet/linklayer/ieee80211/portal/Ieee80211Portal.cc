//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/basic/EthernetEncapsulation.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#endif // ifdef INET_WITH_ETHERNET

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/portal/Ieee80211Portal.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211Portal);

void Ieee80211Portal::initialize(int stage)
{
    Ieee8022Llc::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        upperLayerOutConnected = gate("upperLayerOut")->getPathEndGate()->isConnected();
    }
}

void Ieee80211Portal::processPacketFromMac(Packet *packet)
{
    decapsulate(packet);
    if (upperLayerOutConnected)
        send(packet, "upperLayerOut");
    else
        delete packet;
}

} // namespace ieee80211
} // namespace inet

