// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHLOWCAPFALLBACKSUPPORT_H
#define INET_TEST_MESHLOWCAPFALLBACKSUPPORT_H

#include "inet/linklayer/common/UserPriorityTag_m.h"

class LowCapMacFixture : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName())=="lowcap-payload"
                && driver->par("holdPayload").boolValue()) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace lowcap_mac_registration { Define_Module(LowCapMacFixture); }

class LowCapFallbackDriver : public cSimpleModule, public cListener
{
    bool submitted=false;
    int dataStarts=0, fallbackStarts=0, confirmed=0, failed=0, samples=0, failures=0;
    int64_t dataId=-1;
    BudgetPowerObserver *power() const { return check_and_cast<BudgetPowerObserver *>(meshInterface(0)->getSubmodule("powerSavePolicy")); }
    RecoveryHcfFixture *hcf() const { return check_and_cast<RecoveryHcfFixture *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    MeshServicePeriodView owner() const {
        for (const auto& view : power()->getServicePeriods())
            if (view.peer==meshMib(1)->address && view.direction==MeshPeriodDirection::OWNER) return view;
        throw cRuntimeError("Expected low-cap ending owner");
    }
    void at(simtime_t when,int kind) { scheduleAt(when,new cMessage("lowcap",kind)); }
    void opening() {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_QOS_NULL); header->setChunkLength(B(32));
        header->setToDS(true); header->setFromDS(true);
        header->setReceiverAddress(meshMib(1)->address); header->setTransmitterAddress(meshMib(0)->address);
        header->setAddress3(meshMib(1)->address); header->setAddress4(meshMib(0)->address);
        header->setAckPolicy(NORMAL_ACK); header->setRspi(false); header->setEosp(false);
        auto packet=new Packet("budget-opening",header);
        packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
        check_and_cast<IMeshDataMac *>(meshInterface(0)->getSubmodule("mac"))->sendMeshData(packet);
    }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,registerSignal("budgetOpeningAck"),
                Hcf::meshDataAttemptCompletedSignal,MeshPowerSave::meshServicePeriodOutcomeSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1); at(SimTime(80,SIMTIME_MS),2);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("budgetOpeningAck") && source==power()) {
            fixturePath(0,1,1,SimTime(2,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            auto packet=phyMsdu("lowcap-payload",meshMib(1)->address);
            packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(5);
            check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))->processSourceMsdu(packet);
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal && source==hcf()) {
            auto packet=check_and_cast<Packet *>(object);
            if (std::string(packet->getName())=="lowcap-payload") {
                samples++; failures+=!check_and_cast<MeshDataAttempt *>(details)->acknowledged;
            }
            return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power()) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->direction==MeshPeriodDirection::OWNER) {
                failed+=fact->outcome==MeshPeriodOutcome::FAILED;
                confirmed+=fact->outcome==MeshPeriodOutcome::CONFIRMED;
                if (fact->outcome==MeshPeriodOutcome::CONFIRMED) at(simTime()+SimTime::fromRaw(1),5);
            }
            return;
        }
        if (signal==receptionEndedSignal && source==meshInterface(0)->getSubmodule("radio") && !submitted) {
            auto packet=dynamic_cast<Packet *>(details);
            if (packet && packet->peekAtFront<Ieee80211MacHeader>()->getType()==ST_BEACON && simTime()>SimTime(150,SIMTIME_MS)) {
                submitted=true; at(simTime()+SimTime::fromRaw(1),3);
            }
            return;
        }
        if (signal!=transmissionStartedSignal || source!=meshInterface(0)->getSubmodule("radio")) return;
        auto transmission=check_and_cast<const Ieee80211Transmission *>(object); auto packet=transmission->getPacket();
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        if (std::string(packet->getName())=="lowcap-payload") {
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
            auto view=owner(); auto ids=power()->lastAuthorizedOriginals();
            std::cout << "lowcap DATA e=" << view.ending << " cap=" << view.cap << " spent=" << view.spent
                << " starts=" << dataStarts << " cohort=" << view.cohort.size() << " ids=" << ids.size() << std::endl;
            ASSERT(ids.size()==1 && header->getTid()==5 && header->getEosp());
            ASSERT(view.ending && view.cap==4 && view.spent==dataStarts && view.cohort==std::set<int64_t>{ids[0]});
            if (dataId==-1) dataId=ids[0]; else ASSERT(dataId==ids[0]);
            ASSERT(header->getRetry()==(dataStarts!=0)); dataStarts++;
            if (dataStarts==1) emit(registerSignal("lowcapInitial"),1L);
            else if (dataStarts==2) emit(registerSignal("lowcapRetry"),1L);
            else {
                ASSERT(dataStarts==3); par("holdPayload").setBoolValue(true);
                emit(registerSignal("lowcapRetry2"),1L); at(simTime()+SimTime(2,SIMTIME_MS),4);
            }
        }
        else if (std::string(packet->getName())=="mesh-period-ending" && dataStarts==3) {
            auto view=owner();
            ASSERT(view.ending && view.cap==1 && view.spent==2 && view.cohort.count(dataId));
            ASSERT(!failed && !confirmed && fallbackStarts==0); fallbackStarts++;
            emit(registerSignal("lowcapFallback"),1L);
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) check_and_cast<MeshPowerSave *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==2) check_and_cast<MeshPowerSave *>(meshInterface(1)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) opening();
        else if (kind==4) { ASSERT(dataStarts==3 && samples==3 && failures==3); hcf()->expireOriginal(dataId); }
        else {
            ASSERT(dataStarts==3 && fallbackStarts==1 && samples==3 && failures==3 && confirmed==1 && failed==0);
            emit(registerSignal("lowcapComplete"),1L);
        }
    }
};
namespace lowcap_driver_registration { Define_Module(LowCapFallbackDriver); }
#endif
