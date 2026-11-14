// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_HWMP_TEST_SUPPORT_H
#define INET_TEST_HWMP_TEST_SUPPORT_H
#include "mesh/MeshForwardingSupport.h"
#include "inet/linklayer/ieee80211/mesh/hwmp/Hwmp.h"
#include "inet/linklayer/ieee80211/mesh/MeshForwarding.h"
#include "inet/linklayer/ieee80211/mesh/AirtimeLinkMetric.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

class HwmpTestManagement : public Ieee80211MgmtMesh
{
    bool malformedNext = false;
    bool suppressAll = false;
  protected:
    void sendDown(Packet *packet) override {
        if (auto body = dynamicPtrCast<const Ieee80211HwmpFrame>(packet->peekAtFront<Chunk>())) {
            emit(registerSignal("hwmpTestHandoff"),packet);
            if (suppressAll) { delete packet; return; }
            if (par("suppressOwnPreq").boolValue() && body->getPreqsArraySize()
                    && body->getPreqs(0).getOriginator() == mib->address) {
                delete packet;
                return;
            }
            if (malformedNext) {
                auto bytes = packet->peekDataAsBytes()->getBytes();
                size_t offset = 28+11*body->getPreqs(0).getTargetsArraySize();
                ASSERT(body->getPreqsArraySize() == 1 && body->getPrepsArraySize() == 1 && bytes.at(offset) == 131);
                bytes.at(offset+1)++; // PREP advertises one unavailable octet; MAC computes a valid new FCS
                packet->popAtFront(packet->getDataLength()); packet->insertAtFront(makeShared<BytesChunk>(bytes));
                malformedNext = false;
            }
        }
        Ieee80211MgmtMesh::sendDown(packet);
    }
  public:
    void suppressHwmp(bool value) { Enter_Method("suppressHwmp"); suppressAll = value; }
    void sendTestBody(const Ptr<Ieee80211HwmpFrame>& body, MacAddress receiver, bool malformed = false) {
        Enter_Method("sendTestBody"); malformedNext = malformed; sendHwmp(body,receiver);
    }
};
Define_Module(HwmpTestManagement);

inline Hwmp *hwmp(int station) { return check_and_cast<Hwmp *>(meshInterface(station)->getSubmodule("pathSelection")); }
inline MeshForwarding *meshForwarding(int station) { return check_and_cast<MeshForwarding *>(meshInterface(station)->getSubmodule("mac")->getSubmodule("ds")); }
inline uint32_t outgoingMetric(int station, int peer) {
    return check_and_cast<IMeshLinkMetric *>(meshInterface(station)->getSubmodule("linkMetric"))->getMetric(meshMib(peer)->address);
}
inline HwmpPreq testPreq(MacAddress originator, MacAddress target, uint32_t id, uint32_t sequence, uint8_t targetFlags = 4) {
    HwmpPreq preq; preq.setOriginator(originator); preq.setOriginatorSequenceNumber(sequence);
    preq.setDiscoveryId(id); preq.setLifetime(100); preq.setTtl(8); preq.setTargetsArraySize(1);
    HwmpPreqTarget entry; entry.address = target; entry.flags = targetFlags; preq.setTargets(0,entry); return preq;
}
inline Ptr<Ieee80211HwmpFrame> testBody(const HwmpPreq& preq) {
    auto body = makeShared<Ieee80211HwmpFrame>(); body->setPreqsArraySize(1); body->setPreqs(0,preq);
    body->setChunkLength(B(28+11*preq.getTargetsArraySize()+((preq.getFlags()&64) ? 6 : 0))); return body;
}
inline Ptr<Ieee80211HwmpFrame> testBody(const HwmpPrep& prep) {
    auto body = makeShared<Ieee80211HwmpFrame>(); body->setPrepsArraySize(1); body->setPreps(0,prep);
    body->setChunkLength(B(33+((prep.flags&64) ? 6 : 0))); return body;
}
inline void sendHwmpTest(int station, const Ptr<Ieee80211HwmpFrame>& body, MacAddress receiver, bool malformed = false) {
    check_and_cast<HwmpTestManagement *>(meshManagement(station))->sendTestBody(body,receiver,malformed);
}
inline HwmpPrep testPrep(MacAddress target, MacAddress originator, uint32_t sequence, uint32_t metric = 0, int lifetime = 100) {
    HwmpPrep prep; prep.target = target; prep.originator = originator; prep.targetSequenceNumber = sequence;
    prep.metric = metric; prep.ttl = 8; prep.lifetime = lifetime; return prep;
}
inline Ptr<Ieee80211HwmpFrame> testBody(const std::vector<HwmpPerrDestination>& destinations, uint8_t ttl = 8) {
    auto body = makeShared<Ieee80211HwmpFrame>();
    for (size_t offset = 0; offset < destinations.size(); offset += 19) {
        HwmpPerr perr; perr.setTtl(ttl); auto count = std::min(size_t(19),destinations.size()-offset); perr.setDestinationsArraySize(count);
        for (size_t i = 0; i < count; i++) perr.setDestinations(i,destinations[offset+i]);
        auto index = body->getPerrsArraySize(); body->setPerrsArraySize(index+1); body->setPerrs(index,perr);
    }
    size_t bytes = 4*body->getPerrsArraySize()+13*destinations.size();
    for (const auto& entry : destinations) if (entry.flags&64) bytes += 6;
    body->setChunkLength(B(bytes)); return body;
}
inline HwmpPerrDestination testError(MacAddress destination, uint32_t sn, uint16_t reason = RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION) {
    HwmpPerrDestination entry; entry.address = destination; entry.sequenceNumber = sn; entry.reasonCode = reason; return entry;
}
#endif
