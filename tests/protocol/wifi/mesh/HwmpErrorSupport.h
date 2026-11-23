// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_HWMP_ERROR_SUPPORT_H
#define INET_TEST_HWMP_ERROR_SUPPORT_H
#include "mesh/HwmpTestSupport.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

class HwmpErrorProbe : public Hwmp
{
  public:
    size_t pendingCount() const { return pendingErrors.size(); }
    size_t pendingCount(MacAddress peer) const {
        return std::count_if(pendingErrors.begin(),pendingErrors.end(),[&](const auto& error) { return error.recipient == peer; });
    }
    std::vector<HwmpPerrDestination> pendingFor(MacAddress destination) const {
        std::vector<HwmpPerrDestination> result;
        for (const auto& error : pendingErrors) if (error.destination.address == destination) result.push_back(error.destination);
        return result;
    }
    simtime_t errorDeadline() const { return perrTimer->isScheduled() ? perrTimer->getArrivalTime() : SIMTIME_ZERO; }
    std::optional<simtime_t> lastErrorHandoff() const { return lastPerrHandoff; }
};
Define_Module(HwmpErrorProbe);
inline HwmpErrorProbe *errorProbe(int station) { return check_and_cast<HwmpErrorProbe *>(hwmp(station)); }
inline void sourceHwmp(cSimpleModule *driver, int station, MacAddress destination, const char *name, int tid = 0) {
    driver->sendDirect(fixtureMsdu(name,destination,tid),meshInterface(station)->getSubmodule("llc")->gate("lowerLayerOut"));
}
inline void injectMeshTransit(int station, int receiver, MacAddress source, MacAddress destination, int sequence, ITx::ICallback *callback, const char *name = "unknown-transit", int fault = 0) {
    auto header = makeShared<Ieee80211MeshDataHeader>(); header->setType(ST_DATA_WITH_QOS); header->setChunkLength(B(32));
    header->setDurationField(SIMTIME_ZERO); header->setToDS(true); header->setFromDS(true); header->setMeshControlPresent(true);
    header->setReceiverAddress(meshMib(receiver)->address); header->setTransmitterAddress(meshMib(station)->address);
    header->setAddress3(destination); header->setAddress4(source); header->setSequenceNumber(SequenceNumberCyclic(600+sequence));
    if (fault == 3) { header->setChunkLength(B(26)); header->setToDS(false); header->setReceiverAddress(MacAddress("01:00:5e:00:00:01")); header->setAddress3(source); }
    auto control = makeShared<Ieee80211MeshControl>(); control->setTtl(8); control->setSequenceNumber(sequence);
    if (fault == 1) { control->setAddressExtensionMode(1); control->setAddress4(source); control->setChunkLength(B(12)); }
    auto packet = new Packet(name,header);
    if (fault == 2) packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>{3,8,uint8_t(sequence),uint8_t(sequence>>8),0,0}));
    else packet->insertAtBack(control);
    packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>{0x42,0x42,3,0xde,0xad,0xbe,0xef}));
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    packet->addTag<Ieee80211ModeReq>()->setMode(&Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz);
    check_and_cast<ITx *>(meshInterface(station)->getSubmodule("mac")->getSubmodule("tx"))->transmitFrame(packet,header,false,callback); delete packet;
}
#endif
