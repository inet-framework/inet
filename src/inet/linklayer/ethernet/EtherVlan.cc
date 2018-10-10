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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherVlan.h"

namespace inet {

Define_Module(EtherVlan);

void EtherVlan::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        vlanTagType = par("vlanTagType");
        parseParameters("inboundVlanIdFilter", "inboundVlanIdMap", inboundVlanIdFilter, inboundVlanIdMap);
        parseParameters("outboundVlanIdFilter", "outboundVlanIdMap", outboundVlanIdFilter, outboundVlanIdMap);
        WATCH_VECTOR(inboundVlanIdFilter);
        WATCH_MAP(inboundVlanIdMap);
        WATCH_VECTOR(outboundVlanIdFilter);
        WATCH_MAP(outboundVlanIdMap);
    }
}

void EtherVlan::parseParameters(const char *filterParameterName, const char *mapParameterName, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap)
{
    cStringTokenizer filterTokenizer(par(filterParameterName));
    while (filterTokenizer.hasMoreTokens())
        vlanIdFilter.push_back(atoi(filterTokenizer.nextToken()));
    cStringTokenizer mapTokenizer(par(mapParameterName));
    while (mapTokenizer.hasMoreTokens()) {
        auto fromVlanId = atoi(mapTokenizer.nextToken());
        auto toVlanId = atoi(mapTokenizer.nextToken());
        vlanIdMap[fromVlanId] = toVlanId;
    }
}

void EtherVlan::processPacket(Packet *packet, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap, cGate *gate)
{
    packet->trimFront();
    const auto& ethernetMacHeader = packet->removeAtFront<EthernetMacHeader>();
    Ieee8021QTag *vlanTag;
    if (*vlanTagType == 'c')
        vlanTag = &ethernetMacHeader->getCTagForUpdate();
    else if (*vlanTagType == 's')
        vlanTag = &ethernetMacHeader->getSTagForUpdate();
    else
        throw cRuntimeError("Unknown VLAN tag type");
    auto oldVlanId = vlanTag != nullptr ? vlanTag->getVid() : -1;
    bool acceptPacket = vlanIdFilter.empty() || std::find(vlanIdFilter.begin(), vlanIdFilter.end(), oldVlanId) != vlanIdFilter.end();
    if (acceptPacket) {
        auto newVlanId = oldVlanId;
        auto it = vlanIdMap.find(oldVlanId);
        B chunkLengthDelta(0);
        if (it != vlanIdMap.end()) {
            newVlanId = it->second;
            if (oldVlanId == -1 && newVlanId != -1)
                chunkLengthDelta = B(4);
            else if (oldVlanId != -1 && newVlanId == -1)
                chunkLengthDelta = -B(4);
        }
        ethernetMacHeader->setChunkLength(ethernetMacHeader->getChunkLength() + chunkLengthDelta);
        packet->insertAtFront(ethernetMacHeader);
        if (oldVlanId != newVlanId) {
            EV_WARN << "Changing VLAN ID: new = " << newVlanId << ", old = " << oldVlanId << ".\n";
            vlanTag->setVid(newVlanId);
            auto oldFcs = packet->removeAtBack<EthernetFcs>();
            EtherEncap::addFcs(packet, oldFcs->getFcsMode());
        }
        send(packet, gate);
    }
    else {
        EV_WARN << "Received VLAN ID = " << oldVlanId << " is not accepted, dropping packet.\n";
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void EtherVlan::handleMessage(cMessage *message)
{
    if (message->getArrivalGate()->isName("upperLayerIn"))
        processPacket(check_and_cast<Packet *>(message), outboundVlanIdFilter, outboundVlanIdMap, gate("lowerLayerOut"));
    else if (message->getArrivalGate()->isName("lowerLayerIn"))
        processPacket(check_and_cast<Packet *>(message), inboundVlanIdFilter, inboundVlanIdMap, gate("upperLayerOut"));
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

