// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_TEST_MESHFORWARDINGSUPPORT_H
#define __INET_TEST_MESHFORWARDINGSUPPORT_H

#include "mesh/MeshTestSupport.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPathSelection.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshForwarding.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MeshMac.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"

using namespace inet;
using namespace inet::ieee80211;
using namespace inet::protocoltest;

class MeshFixturePaths : public cSimpleModule, public IMeshPathSelection
{
  public:
    std::map<MacAddress, MeshPath> paths;
    std::map<std::pair<MacAddress, MacAddress>, simtime_t> precursors;
    std::map<MacAddress, simtime_t> pathRefreshTimes;
    simtime_t lifetime = SimTime(200, SIMTIME_MS);
    std::optional<MeshPath> findPath(MacAddress destination) const override {
        auto it = paths.find(destination);
        return it == paths.end() ? std::nullopt : std::optional<MeshPath>(it->second);
    }
    simtime_t getPrecursorExpiry(MacAddress destination, MacAddress precursor) const override {
        auto it = precursors.find({destination, precursor});
        return it == precursors.end() ? SIMTIME_ZERO : it->second;
    }
    void refreshPath(MacAddress destination) override {
        Enter_Method("refreshPath");
        auto it = paths.find(destination);
        if (it != paths.end()) {
            it->second.expiry = simTime() + lifetime;
            pathRefreshTimes[destination] = simTime();
        }
    }
    void refreshPrecursor(MacAddress destination, MacAddress precursor) override {
        Enter_Method("refreshPrecursor");
        auto& expiry = precursors[{destination, precursor}];
        expiry = std::max(expiry, simTime() + lifetime);
    }
    void startDiscovery(MacAddress destination) override {
        Enter_Method("startDiscovery");
        // This fixture provides only explicitly installed paths, never discovery.
        check_and_cast<IMeshForwarding *>(getParentModule()->getSubmodule("mac")->getSubmodule("ds"))->discoveryFailed(destination);
    }
    void cancelDiscovery(MacAddress) override {}
    void processHwmp(const Ptr<const Ieee80211HwmpFrame>&, MacAddress, MacAddress) override {}
};
Define_Module(MeshFixturePaths);

class MeshFixtureLlc : public cSimpleModule, public IIeee80211Llc
{
    void handleMessage(cMessage *message) override { delete message; }
  public:
    const Protocol *getProtocol() const override { return &Protocol::ieee8022llc; }
};
Define_Module(MeshFixtureLlc);

inline MeshFixturePaths *fixturePaths(int station) {
    return check_and_cast<MeshFixturePaths *>(meshInterface(station)->getSubmodule("pathSelection"));
}
inline void fixturePath(int station, int destination, int nextHop, simtime_t expiry) {
    fixturePaths(station)->paths[meshMib(destination)->address] = {meshMib(nextHop)->address, expiry};
}
inline void fixturePrecursor(int station, int destination, int precursor, simtime_t expiry) {
    fixturePaths(station)->precursors[{meshMib(destination)->address, meshMib(precursor)->address}] = expiry;
}
inline Packet *fixtureMsdu(const char *name, MacAddress destination, int tid = 0) {
    auto packet = new Packet(name, makeShared<BytesChunk>(std::vector<uint8_t>{0x42,0x42,0x03,0xde,0xad,0xbe,0xef}));
    packet->addTag<MacAddressReq>()->setDestAddress(destination);
    packet->addTag<UserPriorityReq>()->setUserPriority(tid);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022llc);
    return packet;
}
inline void reconstructMeshMpdu(Packet *packet) {
    std::vector<Ptr<Chunk>> prefix;
    while (!dynamicPtrCast<const Ieee80211MacHeader>(packet->peekAtFront<Chunk>()))
        prefix.push_back(packet->removeAtFront<Chunk>(packet->peekAtFront<Chunk>()->getChunkLength()));
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    auto length = header->getChunkLength() + B(6 + 7 + 4);
    auto bytes = packet->peekAtFront<BytesChunk>(length)->getBytes();
    packet->popAtFront(length);
    packet->insertAtFront(makeShared<BytesChunk>(bytes));
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        packet->insertAtFront(*it);
}

#endif
