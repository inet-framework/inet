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

#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qEncap.h"

namespace inet {

Define_Module(Ieee8021qEncap);

void Ieee8021qEncap::initialize(int stage)
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

void Ieee8021qEncap::parseParameters(const char *filterParameterName, const char *mapParameterName, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap)
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

Ieee8021qHeader *Ieee8021qEncap::findVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader)
{
    if (*vlanTagType == 'c')
        return ethernetMacHeader->getCTagForUpdate();
    else if (*vlanTagType == 's')
        return ethernetMacHeader->getSTagForUpdate();
    else
        throw cRuntimeError("Unknown VLAN tag type");
}

Ieee8021qHeader *Ieee8021qEncap::addVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader)
{
    auto vlanTag = new Ieee8021qHeader();
    ethernetMacHeader->addChunkLength(B(4));
    if (*vlanTagType == 'c')
        ethernetMacHeader->setCTag(vlanTag);
    else if (*vlanTagType == 's')
        ethernetMacHeader->setSTag(vlanTag);
    else
        throw cRuntimeError("Unknown VLAN tag type");
    return vlanTag;
}

Ieee8021qHeader *Ieee8021qEncap::removeVlanTag(const Ptr<EthernetMacHeader>& ethernetMacHeader)
{
    ethernetMacHeader->addChunkLength(B(-4));
    if (*vlanTagType == 'c')
        return ethernetMacHeader->removeCTag();
    else if (*vlanTagType == 's')
        return ethernetMacHeader->removeSTag();
    else
        throw cRuntimeError("Unknown VLAN tag type");
}

void Ieee8021qEncap::processPacket(Packet *packet, std::vector<int>& vlanIdFilter, std::map<int, int>& vlanIdMap, cGate *gate)
{
    packet->trimFront();
    const auto& ethernetMacHeader = packet->removeAtFront<EthernetMacHeader>();
    Ieee8021qHeader *vlanTag = findVlanTag(ethernetMacHeader);
    auto oldVlanId = vlanTag != nullptr ? vlanTag->getVid() : -1;
    auto vlanReq = packet->removeTagIfPresent<VlanReq>();
    auto newVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : oldVlanId;
    delete vlanReq;
    bool acceptPacket = vlanIdFilter.empty() || std::find(vlanIdFilter.begin(), vlanIdFilter.end(), newVlanId) != vlanIdFilter.end();
    if (acceptPacket) {
        auto it = vlanIdMap.find(newVlanId);
        if (it != vlanIdMap.end())
            newVlanId = it->second;
        if (newVlanId != oldVlanId) {
            EV_WARN << "Changing VLAN ID: new = " << newVlanId << ", old = " << oldVlanId << ".\n";
            if (oldVlanId == -1 && newVlanId != -1)
                addVlanTag(ethernetMacHeader)->setVid(newVlanId);
            else if (oldVlanId != -1 && newVlanId == -1)
                delete removeVlanTag(ethernetMacHeader);
            else
                vlanTag->setVid(newVlanId);
            packet->insertAtFront(ethernetMacHeader);
        }
        else
            packet->insertAtFront(ethernetMacHeader);
        packet->addTagIfAbsent<VlanInd>()->setVlanId(newVlanId);
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

void Ieee8021qEncap::handleMessage(cMessage *message)
{
    if (message->getArrivalGate()->isName("upperLayerIn"))
        processPacket(check_and_cast<Packet *>(message), outboundVlanIdFilter, outboundVlanIdMap, gate("lowerLayerOut"));
    else if (message->getArrivalGate()->isName("lowerLayerIn"))
        processPacket(check_and_cast<Packet *>(message), inboundVlanIdFilter, inboundVlanIdMap, gate("upperLayerOut"));
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

