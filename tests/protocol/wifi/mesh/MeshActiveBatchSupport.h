// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHACTIVEBATCHSUPPORT_H
#define INET_TEST_MESHACTIVEBATCHSUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshPowerResponseTag_m.h"

class BatchHoldingMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        std::string name=packet->getName();
        int node=getContainingNode(this)->getIndex();
        if (name=="mesh-period-ending" && !(node==0 && driver->par("releaseFirstEnding").boolValue()))
            return false;
        if (node==1 && name=="mesh-power-mode" && driver->par("batchStarted").boolValue()
                && !driver->par("releaseNested").boolValue()) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace batch_mac_registration { Define_Module(BatchHoldingMac); }

class BatchResponseTx : public Tx
{
    bool observed=false;
  public:
    void resumePendingTransmission() override {
        Enter_Method("resumePendingTransmission");
        auto stamp=frame ? frame->findTag<Ieee80211MeshPowerResponseTag>() : nullptr;
        if (!observed && getContainingNode(this)->getIndex()==1 && frame && !transmitting
                && admittedStart<=simTime() && stamp && stamp->getPeer()==meshMib(0)->address) {
            auto policy=check_and_cast<MeshPowerSave *>(getParentModule()->getParentModule()->getSubmodule("powerSavePolicy"));
            bool ending=false;
            for (const auto& role : policy->getServicePeriods())
                ending |= role.peer==meshMib(0)->address && role.direction==MeshPeriodDirection::RECIPIENT && role.ending;
            if (ending) {
                observed=true;
                emit(registerSignal("batchResponseHeld"),frame);
            }
        }
        Tx::resumePendingTransmission();
    }
};
namespace batch_tx_registration { Define_Module(BatchResponseTx); }

class ActiveBatchDriver : public cSimpleModule, public cListener
{
    bool firstSubmitted=false, secondSubmitted=false, batch=false, nested=false;
    int notices=0, withdrawals=0, obsoleteConfirmations=0, responseAir=0, lowerCommits=0;
    int64_t oldOpening=-1;
    simtime_t oldOpeningTime=-1;
    MeshPowerSave *power(int node) const {
        return check_and_cast<MeshPowerSave *>(meshInterface(node)->getSubmodule("powerSavePolicy"));
    }
    bool role(int peer,MeshPeriodDirection direction) const {
        for (const auto& view : power(1)->getServicePeriods())
            if (view.peer==meshMib(peer)->address && view.direction==direction) return true;
        return false;
    }
    bool lightPair(int peer) const {
        auto a=meshMib(1)->findMeshPeerPowerState(meshMib(peer)->address);
        auto b=meshMib(peer)->findMeshPeerPowerState(meshMib(1)->address);
        return a && b && a->local==MeshPowerMode::LIGHT_SLEEP && a->remote==MeshPowerMode::LIGHT_SLEEP
            && b->local==MeshPowerMode::LIGHT_SLEEP && b->remote==MeshPowerMode::LIGHT_SLEEP;
    }
    void at(simtime_t when,int kind) { scheduleAt(when,new cMessage("active-batch",kind)); }
    void trigger(int sender,const char *name) {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_QOS_NULL); header->setChunkLength(B(32));
        header->setToDS(true); header->setFromDS(true);
        header->setReceiverAddress(meshMib(1)->address); header->setTransmitterAddress(meshMib(sender)->address);
        header->setAddress3(meshMib(1)->address); header->setAddress4(meshMib(sender)->address);
        header->setAckPolicy(NORMAL_ACK); header->setRspi(true); header->setEosp(false);
        auto packet=new Packet(name,header);
        packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
        check_and_cast<IMeshDataMac *>(meshInterface(sender)->getSubmodule("mac"))->sendMeshData(packet);
    }
  protected:
    void initialize() override {
        auto net=getSimulation()->getSystemModule();
        for (auto signal : {receptionEndedSignal,transmissionStartedSignal,
                MeshPowerSave::meshServicePeriodOutcomeSignal,registerSignal("batchResponseHeld")}) net->subscribe(signal,this);
        net->subscribe(Ieee80211Mib::meshPowerModeChangedSignal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(20,SIMTIME_MS),1);
        at(SimTime(50,SIMTIME_MS),2); at(SimTime(70,SIMTIME_MS),3);
        at(SimTime(95,SIMTIME_MS),4); at(SimTime(115,SIMTIME_MS),5);
        at(SimTime(1400,SIMTIME_MS),12);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,long value,cObject *) override {
        Enter_Method_Silent();
        if (source!=meshMib(1) || signal!=Ieee80211Mib::meshPowerModeChangedSignal || !batch) return;
        if (value==long(MeshPowerMode::ACTIVE)) {
            ASSERT(!role(0,MeshPeriodDirection::RECIPIENT) && !role(2,MeshPeriodDirection::RECIPIENT));
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(2,MeshPeriodDirection::OWNER));
            ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->local==MeshPowerMode::ACTIVE);
            ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(2)->address)->local==MeshPowerMode::ACTIVE);
            notices++;
            if (!nested) {
                nested=true;
                power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
                ASSERT(power(1)->getRequestedMode(meshMib(0)->address)==MeshPowerMode::LIGHT_SLEEP);
            }
        }
        else if (value==long(MeshPowerMode::LIGHT_SLEEP)) lowerCommits++;
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("batchResponseHeld")) {
            ASSERT(!batch && source==meshInterface(1)->getSubmodule("mac")->getSubmodule("tx"));
            auto packet=check_and_cast<Packet *>(object);
            auto stamp=packet->getTag<Ieee80211MeshPowerResponseTag>();
            ASSERT(stamp->getPeer()==meshMib(0)->address && stamp->getOpeningId()!=-1);
            oldOpening=stamp->getOpeningId(); oldOpeningTime=stamp->getOpeningTime();
            ASSERT(role(0,MeshPeriodDirection::RECIPIENT) && role(2,MeshPeriodDirection::RECIPIENT));
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(2,MeshPeriodDirection::OWNER));
            batch=true; par("batchStarted").setBoolValue(true);
            power(1)->requestActiveForAllPeers();
            ASSERT(notices==2 && nested && withdrawals==2);
            ASSERT(power(1)->getRequestedMode(meshMib(0)->address)==MeshPowerMode::LIGHT_SLEEP);
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(2,MeshPeriodDirection::OWNER));
            at(simTime()+SimTime(1,SIMTIME_MS),10);
            return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power(1)) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (batch && fact->direction==MeshPeriodDirection::RECIPIENT) {
                if (fact->outcome==MeshPeriodOutcome::WITHDRAWN) withdrawals++;
                if (fact->outcome==MeshPeriodOutcome::CONFIRMED && fact->peer==meshMib(0)->address
                        && fact->openingId==oldOpening && fact->openingTime==oldOpeningTime) obsoleteConfirmations++;
            }
            return;
        }
        if (signal==transmissionStartedSignal && source==meshInterface(1)->getSubmodule("radio")) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            auto stamp=packet->findTag<Ieee80211MeshPowerResponseTag>();
            if (batch && stamp && stamp->getPeer()==meshMib(0)->address && stamp->getOpeningId()==oldOpening
                    && stamp->getOpeningTime()==oldOpeningTime) responseAir++;
            return;
        }
        if (signal!=receptionEndedSignal) return;
        auto packet=dynamic_cast<Packet *>(details);
        if (!packet || packet->hasBitError()) return;
        if (source==meshInterface(0)->getSubmodule("radio") && !firstSubmitted && simTime()>SimTime(350,SIMTIME_MS)) {
            auto header=packet->peekAtFront<Ieee80211MacHeader>();
            if (header->getType()==ST_BEACON && lightPair(0) && lightPair(2)) {
                auto beacon=packet->peekAtFront<Ieee80211TwoAddressHeader>();
                if (beacon->getTransmitterAddress()==meshMib(1)->address) {
                    firstSubmitted=true; at(simTime()+SimTime::fromRaw(1),6);
                }
            }
        }
        if (source==meshInterface(1)->getSubmodule("radio") && std::string(packet->getName())=="batch-trigger-2")
            at(simTime()+SimTime(1,SIMTIME_MS),8);
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) { meshManagement(2)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(2)->address); }
        else if (kind==2) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==4) power(2)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==5) power(1)->requestMode(meshMib(2)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==6) { trigger(0,"batch-trigger-0"); at(simTime()+SimTime(2,SIMTIME_MS),7); }
        else if (kind==7) { ASSERT(role(0,MeshPeriodDirection::RECIPIENT)); secondSubmitted=true; trigger(2,"batch-trigger-2"); }
        else if (kind==8) {
            ASSERT(role(0,MeshPeriodDirection::RECIPIENT) && role(2,MeshPeriodDirection::RECIPIENT));
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(2,MeshPeriodDirection::OWNER));
            par("releaseFirstEnding").setBoolValue(true);
            check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else if (kind==10) {
            ASSERT(batch && responseAir==1 && obsoleteConfirmations==0 && withdrawals==2);
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(2,MeshPeriodDirection::OWNER));
            ASSERT(power(1)->getRequestedMode(meshMib(0)->address)==MeshPowerMode::LIGHT_SLEEP);
            par("releaseNested").setBoolValue(true);
            check_and_cast<IMeshPowerBuffer *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else {
            ASSERT(firstSubmitted && secondSubmitted && batch && notices==2 && withdrawals==2);
            ASSERT(responseAir==1 && obsoleteConfirmations==0 && lowerCommits>=1);
            ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->local==MeshPowerMode::LIGHT_SLEEP);
            emit(registerSignal("batchComplete"),1L);
        }
    }
};
namespace active_batch_driver_registration { Define_Module(ActiveBatchDriver); }

#endif
