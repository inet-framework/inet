// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHMANAGEMENTPERIODSUPPORT_H
#define INET_TEST_MESHMANAGEMENTPERIODSUPPORT_H

#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtMesh.h"

class PeriodManagementFixture : public Ieee80211MgmtMesh
{
  public:
    void queueReport(MacAddress peer) {
        Enter_Method("queueReport");
        auto body=makeShared<Ieee80211MeshLinkMetricFrame>();
        body->setRequest(false); body->setMetric(7); body->setChunkLength(B(7));
        sendBody(body,ST_ACTION,peer);
    }
};
namespace period_management_registration { Define_Module(PeriodManagementFixture); }

class ManagementPeriodDriver : public cSimpleModule, public cListener
{
    int trigger=0, reportAir=0, reportRx=0, ending=0, confirmed=0, samples=0, terminal=0;
    MeshPowerSave *power(int i) const { return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("management-period",kind)); }
    bool owner() const {
        for (const auto& view : power(0)->getServicePeriods())
            if (view.peer==meshMib(1)->address && view.direction==MeshPeriodDirection::OWNER) return true;
        return false;
    }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,Hcf::meshDataAttemptCompletedSignal,
                Hcf::meshTerminalRecoverySignal,MeshPowerSave::meshServicePeriodOutcomeSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1);
        at(SimTime(80,SIMTIME_MS),2); at(SimTime(130,SIMTIME_MS),3); at(SimTime(380,SIMTIME_MS),4);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==Hcf::meshDataAttemptCompletedSignal) { samples++; return; }
        if (signal==Hcf::meshTerminalRecoverySignal) { terminal++; return; }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->outcome==MeshPeriodOutcome::CONFIRMED) confirmed++;
            return;
        }
        if (signal==transmissionStartedSignal && source==meshInterface(0)->getSubmodule("radio")) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            auto name=std::string(packet->getName());
            if (name=="mesh-period-trigger") { ASSERT(trigger==0 && reportAir==0); trigger++; }
            if (name=="mesh-period-ending") { ASSERT(owner() && reportAir==1); ending++; }
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            auto header=dynamicPtrCast<const Ieee80211MeshActionFrame>(packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength()));
            if (header && header->getMeshAction()==MESH_LINK_METRIC_REPORT && simTime()>SimTime(130,SIMTIME_MS)) {
                ASSERT(trigger==1 && owner() && ending==0); reportAir++;
            }
            return;
        }
        if (signal==receptionEndedSignal && source==meshInterface(1)->getSubmodule("radio")) {
            auto packet=dynamic_cast<Packet *>(details);
            if (!packet || packet->hasBitError()) return;
            auto header=dynamicPtrCast<const Ieee80211MeshActionFrame>(packet->peekAtFront<Ieee80211MacHeader>());
            if (header && header->getMeshAction()==MESH_LINK_METRIC_REPORT && simTime()>SimTime(130,SIMTIME_MS)) reportRx++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) {
            ASSERT(power(0)->getServicePeriods().empty() && power(1)->getServicePeriods().empty());
            check_and_cast<PeriodManagementFixture *>(meshManagement(0))->queueReport(meshMib(1)->address);
        }
        else {
            ASSERT(trigger==1 && reportAir==1 && reportRx==1 && ending==1 && confirmed==2 && samples==0 && terminal==0);
            ASSERT(power(0)->getServicePeriods().empty() && power(1)->getServicePeriods().empty());
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
namespace management_period_registration { Define_Module(ManagementPeriodDriver); }
#endif
