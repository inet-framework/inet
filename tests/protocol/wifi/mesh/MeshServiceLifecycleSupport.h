// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSERVICELIFECYCLESUPPORT_H
#define INET_TEST_MESHSERVICELIFECYCLESUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/common/lifecycle/LifecycleController.h"

// Isolated policy-owner lifecycle operations; radios, peers and MAC queues
// survive. Operations occur while a real trigger/DATA/response is on air.
class ServiceLifecycleDriver : public cSimpleModule, public cListener, public LifecycleController
{
    bool armed=false, removed=false, restarted=false;
    int targetId=-1, operations=0, delivered=0, samples=0, errors=0, terminal=0, afterRemoval=0;
    int which() const { return par("scenario").intValue()-28; }
    int phase() const { return which()/3; }
    int action() const { return which()%3; }
    int target() const { return phase()==2 ? 1 : 0; }
    MeshPowerSave *power(int i) const { return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    Hcf *hcf(int i) const { return check_and_cast<Hcf *>(meshInterface(i)->getSubmodule("mac")->getSubmodule("hcf")); }
    bool role(int i,MeshPeriodDirection direction) const {
        for (const auto& view : power(i)->getServicePeriods()) if (view.direction==direction) return true;
        return false;
    }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("service-lifecycle",kind)); }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,packetSentToUpperSignal,
                Hcf::meshDataAttemptCompletedSignal,Hcf::meshTerminalRecoverySignal,MeshPowerSave::meshServicePeriodOutcomeSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        targetId=power(target())->getId();
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1); at(SimTime(80,SIMTIME_MS),2);
        at(SimTime(130,SIMTIME_MS),3); at(SimTime(250,SIMTIME_MS),5);
        at(SimTime(300,SIMTIME_MS),6); at(SimTime(380,SIMTIME_MS),7);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            if (removed && source->getId()==targetId) {
                // Any post-restart future role must be independently opened;
                // this fixture provides no new request before its checkpoint.
                afterRemoval++;
            }
            return;
        }
        if (signal==Hcf::meshTerminalRecoverySignal) { terminal++; return; }
        if (signal==Hcf::meshDataAttemptCompletedSignal) {
            if (source==hcf(0) && std::string(check_and_cast<Packet *>(object)->getName())=="lifecycle-payload") {
                samples++; errors+=!check_and_cast<MeshDataAttempt *>(details)->acknowledged;
            }
            return;
        }
        if (signal==packetSentToUpperSignal) {
            auto packet=dynamic_cast<Packet *>(object);
            delivered+=source==meshInterface(1)->getSubmodule("mac") && packet && std::string(packet->getName())=="lifecycle-payload";
            return;
        }
        if (armed) return;
        if (signal==transmissionStartedSignal && source==meshInterface(0)->getSubmodule("radio")) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            bool match=phase()==0 ? std::string(packet->getName())=="mesh-period-trigger"
                : phase()==1 && std::string(packet->getName())=="lifecycle-payload";
            if (match) { armed=true; at(simTime()+SimTime::fromRaw(1),4); }
        }
        if (phase()==2 && signal==receptionEndedSignal && source==meshInterface(1)->getSubmodule("radio")) {
            auto packet=dynamic_cast<Packet *>(details);
            if (packet && std::string(packet->getName())=="lifecycle-payload") {
                armed=true; at(simTime()+SimTime::fromRaw(1),4);
            }
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) {
            fixturePath(0,1,1,SimTime(2,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("lifecycle-payload",meshMib(1)->address));
        }
        else if (kind==4) {
            ASSERT(armed && !removed && operations==0);
            if (phase()==0) ASSERT(!role(0,MeshPeriodDirection::OWNER));
            else ASSERT(role(target(),phase()==1 ? MeshPeriodDirection::OWNER : MeshPeriodDirection::RECIPIENT));
            removed=true; operations++;
            if (action()==2) power(target())->deleteModule();
            else {
                auto operation=action()==1 ? static_cast<LifecycleOperation *>(new ModuleCrashOperation())
                    : static_cast<LifecycleOperation *>(new ModuleStopOperation());
                LifecycleOperation::StringMap parameters; operation->initialize(power(target()),parameters);
                ASSERT(initiateOperation(operation)); ASSERT(power(target())->getServicePeriods().empty());
            }
            at(simTime()+SimTime(1,SIMTIME_MS),8);
        }
        else if (kind==8) {
            ASSERT(afterRemoval==0 && terminal==0);
            auto radio=check_and_cast<IRadio *>(meshInterface(target())->getSubmodule("radio"));
            ASSERT(radio->getRadioMode()==IRadio::RADIO_MODE_TRANSCEIVER);
        }
        else if (kind==5) {
            ASSERT(removed && afterRemoval==0);
            if (action()!=2) {
                auto operation=new ModuleStartOperation(); LifecycleOperation::StringMap parameters;
                operation->initialize(power(target()),parameters); ASSERT(initiateOperation(operation)); restarted=true;
                ASSERT(power(target())->getServicePeriods().empty());
            }
        }
        else if (kind==6) {
            ASSERT(afterRemoval==0);
            if (action()!=2) ASSERT(restarted && power(target())->getServicePeriods().empty());
            // Explicit relationship cleanup is distinct from confirming the
            // other endpoint's potentially still-open asymmetric period.
            meshManagement(0)->closePeer(meshMib(1)->address); meshManagement(1)->closePeer(meshMib(0)->address);
        }
        else {
            std::cout << "lifecycle phase=" << phase() << " action=" << action() << " delivered=" << delivered
                << " samples=" << samples << " errors=" << errors << " lateFacts=" << afterRemoval << std::endl;
            ASSERT(operations==1 && delivered==1 && samples==1 && errors==0 && terminal==0 && afterRemoval==0);
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
Define_Module(ServiceLifecycleDriver);
#endif
