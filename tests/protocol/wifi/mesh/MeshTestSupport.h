// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_MESHTESTSUPPORT_H
#define __INET_PROTOCOLTEST_MESHTESTSUPPORT_H

#include "WifiTestSupport.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtMesh.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiver.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet::protocoltest {

inline cModule *meshInterface(int index)
{
    return getSimulation()->getSystemModule()->getSubmodule("station", index)->getSubmodule("wlan", 0);
}

inline ieee80211::Ieee80211MgmtMesh *meshManagement(int index)
{
    return check_and_cast<ieee80211::Ieee80211MgmtMesh *>(meshInterface(index)->getSubmodule("mgmt"));
}

inline ieee80211::Ieee80211Mib *meshMib(int index)
{
    return check_and_cast<ieee80211::Ieee80211Mib *>(meshInterface(index)->getSubmodule("mib"));
}

inline std::pair<Hz, Hz> meshListeningBand(int index)
{
    auto radio = check_and_cast<physicallayer::IRadio *>(meshInterface(index)->getSubmodule("radio"));
    std::unique_ptr<const physicallayer::IListening> listening(radio->getReceiver()->createListening(
        radio, simTime(), simTime(), Coord(0,0,0), Coord(0,0,0)));
    auto band = check_and_cast<const physicallayer::BandListening *>(listening.get());
    return {band->getCenterFrequency(), band->getBandwidth()};
}

inline void mutateMeshOpen(Packet *packet, const std::function<void(ieee80211::Ieee80211MeshPeeringOpenFrame&)>& mutation)
{
    using namespace ieee80211;
    std::vector<Ptr<Chunk>> prefix;
    while (!dynamicPtrCast<const Ieee80211MacHeader>(packet->peekAtFront<Chunk>()))
        prefix.push_back(packet->removeAtFront<Chunk>(packet->peekAtFront<Chunk>()->getChunkLength()));
    auto header = packet->removeAtFront<Ieee80211MeshActionFrame>();
    auto body = packet->removeAtFront<Ieee80211MeshPeeringOpenFrame>();
    auto trailer = packet->removeAtFront<Ieee80211MacTrailer>(B(4));
    mutation(*body);
    Packet encoded("updated-mac", header);
    encoded.insertAtBack(body);
    auto bytes = encoded.peekAllAsBytes()->getBytes();
    trailer->setFcsMode(FCS_COMPUTED);
    trailer->setFcs(ethernetFcs(bytes.data(), bytes.size()));
    packet->insertAtFront(trailer);
    packet->insertAtFront(body);
    packet->insertAtFront(header);
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        packet->insertAtFront(*it);
}

inline void truncateMeshPeeringElement(Packet *packet)
{
    using namespace ieee80211;
    std::vector<Ptr<Chunk>> prefix;
    while (!dynamicPtrCast<const Ieee80211MacHeader>(packet->peekAtFront<Chunk>()))
        prefix.push_back(packet->removeAtFront<Chunk>(packet->peekAtFront<Chunk>()->getChunkLength()));
    auto header = packet->removeAtFront<Ieee80211MeshActionFrame>();
    auto body = packet->removeAtFront<Ieee80211MeshPeeringOpenFrame>();
    auto trailer = packet->removeAtFront<Ieee80211MacTrailer>(B(4));
    Packet encodedBody("body", body);
    auto bytes = encodedBody.peekAllAsBytes()->getBytes();
    ASSERT(bytes.at(bytes.size() - 6) == 117);
    bytes.at(bytes.size() - 5) = 5; // one more than the four available element octets
    Packet encoded("updated-mac", header);
    encoded.insertAtBack(makeShared<BytesChunk>(bytes));
    auto macBytes = encoded.peekAllAsBytes()->getBytes();
    trailer->setFcsMode(FCS_COMPUTED);
    trailer->setFcs(ethernetFcs(macBytes.data(), macBytes.size()));
    packet->insertAtFront(trailer);
    packet->insertAtFront(makeShared<BytesChunk>(bytes));
    packet->insertAtFront(header);
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        packet->insertAtFront(*it);
}

} // namespace inet::protocoltest
#endif
