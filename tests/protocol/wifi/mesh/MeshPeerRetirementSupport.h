// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPEERRETIREMENTSUPPORT_H
#define INET_TEST_MESHPEERRETIREMENTSUPPORT_H

#include "mesh/MeshActiveBatchSupport.h"
#include "mesh/MeshServicePeriodTableSupport.h"

// The replay below calls the policy's existing response-completion API with a
// copy of a production-stamped controlled API BA. It is an identity-boundary
// test, not a claim that an old RF response is transmitted after re-peering.
class PeerRetirementDriver : public cSimpleModule, public cListener
{
    bool first=false, second=false, retired=false, reopened=false, replacement=false, replayed=false;
    int oldConfirmations=0, otherOutcomes=0;
    uint16_t oldLocal=0, oldPeer=0;
    std::unique_ptr<Packet> oldResponse;
    std::optional<std::pair<int64_t,simtime_t>> oldOpening, otherOpening;
    std::optional<MeshServicePeriodView> otherView;
    TablePowerObserver *power(int node) const {
        return check_and_cast<TablePowerObserver *>(meshInterface(node)->getSubmodule("powerSavePolicy"));
    }
    bool role(int peer, MeshPeriodDirection direction) const {
        for (const auto& view : power(1)->getServicePeriods())
            if (view.peer==meshMib(peer)->address && view.direction==direction) return true;
        return false;
    }
    bool lightPair(int peer) const {
        auto central=meshMib(1)->findMeshPeerPowerState(meshMib(peer)->address);
        auto neighbor=meshMib(peer)->findMeshPeerPowerState(meshMib(1)->address);
        return central && neighbor && central->local==MeshPowerMode::LIGHT_SLEEP && central->remote==MeshPowerMode::LIGHT_SLEEP
            && neighbor->local==MeshPowerMode::LIGHT_SLEEP && neighbor->remote==MeshPowerMode::LIGHT_SLEEP;
    }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("peer-retirement",kind)); }
    void trigger(int sender,const char *name) {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_QOS_NULL); header->setChunkLength(B(32));
        header->setToDS(true); header->setFromDS(true);
        header->setReceiverAddress(meshMib(1)->address); header->setTransmitterAddress(meshMib(sender)->address);
        header->setAddress3(meshMib(1)->address); header->setAddress4(meshMib(sender)->address);
        header->setAckPolicy(NORMAL_ACK); header->setRspi(true); header->setEosp(false);
        auto packet=new Packet(name,header); packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
        check_and_cast<IMeshDataMac *>(meshInterface(sender)->getSubmodule("mac"))->sendMeshData(packet);
    }
    // Controlled policy API inputs, not a claim of an RF DATA/BA exchange.
    void markEnding() {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_DATA_WITH_QOS); header->setChunkLength(B(32)); header->setTransmitterAddress(meshMib(0)->address);
        header->setReceiverAddress(meshMib(1)->address); header->setPowerMgmt(true);
        header->setAckPolicy(NORMAL_ACK); header->setEosp(true); header->setRspi(false);
        header->setTid(0); header->setSequenceNumber(SequenceNumberCyclic(7));
        Packet data("controlled-api-ending",header);
        power(1)->recipientMemberReceived(&data,header,false);
        bool ending=false;
        for (const auto& view : power(1)->getServicePeriods())
            if (view.peer==meshMib(0)->address && view.direction==MeshPeriodDirection::RECIPIENT) ending=view.ending;
        ASSERT(ending);
    }
    std::unique_ptr<Packet> makeQualifiedResponse() {
        auto ba=makeShared<Ieee80211CompressedBlockAck>();
        ba->setChunkLength(B(32)); ba->setReceiverAddress(meshMib(0)->address); ba->setTidInfo(0);
        ba->setStartingSequenceNumber(SequenceNumberCyclic(7));
        BitVector bitmap(0,64); bitmap.setBit(0,true); ba->setBlockAckBitmap(bitmap);
        auto response=std::make_unique<Packet>("controlled-api-ba",ba);
        power(1)->prepareRecipientResponse(response.get(),nullptr,nullptr,MeshPowerBaWindow{SequenceNumberCyclic(7),64});
        auto stamp=response->getTag<Ieee80211MeshPowerResponseTag>();
        ASSERT(stamp->getQualifyingBlockAck());
        return response;
    }
    void requireOther() const {
        ASSERT(otherOpening && otherView && role(2,MeshPeriodDirection::RECIPIENT));
        ASSERT(power(1)->opening(meshMib(2)->address,MeshPeriodDirection::RECIPIENT)==*otherOpening);
        for (const auto& view : power(1)->getServicePeriods())
            if (view.peer==otherView->peer && view.direction==otherView->direction)
                ASSERT(view.ending==otherView->ending && view.cap==otherView->cap
                    && view.spent==otherView->spent && view.cohort==otherView->cohort);
    }
  protected:
    void initialize() override {
        auto net=getSimulation()->getSystemModule();
        for (auto signal : {receptionEndedSignal,MeshPowerSave::meshServicePeriodOutcomeSignal,registerSignal("batchResponseHeld")})
            net->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(20,SIMTIME_MS),1);
        at(SimTime(50,SIMTIME_MS),2); at(SimTime(70,SIMTIME_MS),3);
        at(SimTime(95,SIMTIME_MS),4); at(SimTime(115,SIMTIME_MS),5);
        at(SimTime(2100,SIMTIME_MS),14);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("batchResponseHeld")) {
            ASSERT(!retired && source==meshInterface(1)->getSubmodule("mac")->getSubmodule("tx"));
            auto response=check_and_cast<Packet *>(object);
            auto stamp=response->getTag<Ieee80211MeshPowerResponseTag>();
            ASSERT(stamp->getPeer()==meshMib(0)->address && stamp->getOpeningId()!=-1);
            ASSERT(role(0,MeshPeriodDirection::RECIPIENT) && role(0,MeshPeriodDirection::OWNER)
                && role(2,MeshPeriodDirection::RECIPIENT));
            oldOpening=power(1)->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT);
            ASSERT(*oldOpening==std::make_pair(stamp->getOpeningId(),stamp->getOpeningTime()));
            otherOpening=power(1)->opening(meshMib(2)->address,MeshPeriodDirection::RECIPIENT);
            for (const auto& view : power(1)->getServicePeriods())
                if (view.peer==meshMib(2)->address && view.direction==MeshPeriodDirection::RECIPIENT) otherView=view;
            ASSERT(otherView);
            oldLocal=meshManagement(1)->getLocalLinkId(meshMib(0)->address);
            oldPeer=meshManagement(1)->getPeerLinkId(meshMib(0)->address);
            markEnding();
            auto qualified=makeQualifiedResponse();
            oldResponse.reset(qualified->dup());
            ASSERT(oldResponse->getTag<Ieee80211MeshPowerResponseTag>()->getOpeningId()==oldOpening->first);
            retired=true;
            meshManagement(1)->closePeer(meshMib(0)->address);
            ASSERT(!meshMib(1)->isMeshPeerEstablished(meshMib(0)->address));
            ASSERT(!role(0,MeshPeriodDirection::RECIPIENT) && !role(0,MeshPeriodDirection::OWNER));
            requireOther(); ASSERT(otherOutcomes==0);
            par("releaseFirstEnding").setBoolValue(false);
            at(simTime()+SimTime(120,SIMTIME_MS),10);
            return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power(1)) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (retired && fact->peer==meshMib(2)->address) otherOutcomes++;
            if (retired && fact->peer==meshMib(0)->address && fact->direction==MeshPeriodDirection::RECIPIENT
                    && fact->outcome==MeshPeriodOutcome::CONFIRMED) oldConfirmations++;
            return;
        }
        if (signal!=receptionEndedSignal) return;
        auto packet=dynamic_cast<Packet *>(details);
        if (!packet || packet->hasBitError()) return;
        if (source==meshInterface(0)->getSubmodule("radio") && simTime()>SimTime(350,SIMTIME_MS)) {
            auto header=packet->peekAtFront<Ieee80211MacHeader>();
            if (header->getType()!=ST_BEACON) return;
            auto beacon=packet->peekAtFront<Ieee80211TwoAddressHeader>();
            if (beacon->getTransmitterAddress()!=meshMib(1)->address) return;
            if (!first && lightPair(0) && lightPair(2)) { first=true; at(simTime()+SimTime::fromRaw(1),6); }
            else if (reopened && !replacement && lightPair(0)) { replacement=true; at(simTime()+SimTime::fromRaw(1),12); }
        }
        if (source==meshInterface(1)->getSubmodule("radio") && std::string(packet->getName())=="retire-trigger-2")
            at(simTime()+SimTime(1,SIMTIME_MS),8);
        if (source==meshInterface(1)->getSubmodule("radio") && std::string(packet->getName())=="retire-trigger-new")
            at(simTime()+SimTime::fromRaw(1),13);
    }
    void handleMessage(cMessage *message) override {
        int kind=message->getKind(); delete message;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) { meshManagement(2)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(2)->address); }
        else if (kind==2) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==4) power(2)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==5) power(1)->requestMode(meshMib(2)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==6) { trigger(0,"retire-trigger-0"); at(simTime()+SimTime(2,SIMTIME_MS),7); }
        else if (kind==7) { ASSERT(role(0,MeshPeriodDirection::RECIPIENT)); second=true; trigger(2,"retire-trigger-2"); }
        else if (kind==8) {
            ASSERT(role(0,MeshPeriodDirection::RECIPIENT) && role(2,MeshPeriodDirection::RECIPIENT));
            par("releaseFirstEnding").setBoolValue(true);
            check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else if (kind==10) {
            requireOther(); ASSERT(otherOutcomes==0);
            ASSERT(meshManagement(1)->getPeerState(meshMib(0)->address)==Ieee80211MgmtMesh::IDLE);
            ASSERT(meshManagement(0)->getPeerState(meshMib(1)->address)==Ieee80211MgmtMesh::IDLE);
            meshManagement(1)->openPeer(meshMib(0)->address); meshManagement(0)->openPeer(meshMib(1)->address);
            at(simTime()+SimTime(80,SIMTIME_MS),11);
        }
        else if (kind==11) {
            ASSERT(meshMib(1)->isMeshPeerEstablished(meshMib(0)->address));
            ASSERT(meshManagement(1)->getLocalLinkId(meshMib(0)->address)!=oldLocal);
            ASSERT(meshManagement(1)->getPeerLinkId(meshMib(0)->address)!=oldPeer);
            requireOther();
            power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
            power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
            reopened=true;
        }
        else if (kind==12) trigger(0,"retire-trigger-new");
        else if (kind==13) {
            ASSERT(oldResponse && oldOpening && role(0,MeshPeriodDirection::RECIPIENT));
            auto current=power(1)->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT);
            ASSERT(current!=*oldOpening); requireOther();
            markEnding(); // A matching qualifying BA would retire this replacement.
            power(1)->recipientResponseComplete(oldResponse.get()); // API-only old-stamp replay.
            ASSERT(role(0,MeshPeriodDirection::RECIPIENT)
                && power(1)->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT)==current);
            requireOther(); ASSERT(otherOutcomes==0 && oldConfirmations==0); replayed=true;
        }
        else {
            ASSERT(first && second && retired && reopened && replacement && replayed && otherOutcomes==0 && oldConfirmations==0);
            requireOther(); oldResponse.reset();
            meshManagement(1)->closePeer(meshMib(0)->address); meshManagement(1)->closePeer(meshMib(2)->address);
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
namespace peer_retirement_registration { Define_Module(PeerRetirementDriver); }
#endif
