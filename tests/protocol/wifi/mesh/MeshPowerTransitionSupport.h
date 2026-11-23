// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERTRANSITIONSUPPORT_H
#define INET_TEST_MESHPOWERTRANSITIONSUPPORT_H

#include "inet/common/lifecycle/LifecycleController.h"

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

class PowerHoldMac : public Ieee80211MeshMac
{
  public:
    bool holdNull = false;
    bool holdData = false;
    bool holdReports = false;
    bool holdOutside = false;
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t start) const override {
        auto type=packet->peekAtFront<Ieee80211MacHeader>()->getType();
        if(holdOutside&&std::string(packet->getName())=="outside-selected")return false;
        if (holdReports) {
            auto action=dynamicPtrCast<const Ieee80211MeshActionFrame>(packet->peekAtFront<Ieee80211MacHeader>());
            if (action && action->getCategory()==MESH_ACTION && action->getMeshAction()==MESH_LINK_METRIC_REPORT)return false;
        }
        return !(holdNull && type==ST_QOS_NULL) && !(holdData && type==ST_DATA_WITH_QOS)
            && Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace power_transition_registration { Define_Module(::PowerHoldMac); }

class PowerSupersessionDriver : public cSimpleModule, public cListener, public LifecycleController
{
    bool superseded = false;
    bool oldOnAir = false;
    int nullPpdus = 0;
    int samples = 0;
    int pendingData=0,pendingReports=0;
    uint16_t oldLink = 0;
    int64_t oldId = -1;
    int scenario() const { return par("scenario"); }
    MacAddress peer() const { return meshMib(1)->address; }
    AwakePowerFixture *power() { return check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy")); }
    PowerHoldMac *mac() { return check_and_cast<PowerHoldMac *>(meshInterface(0)->getSubmodule("mac")); }
    Hcf *hcf() { return check_and_cast<Hcf *>(mac()->getSubmodule("hcf")); }
    Edca *edca() { return check_and_cast<Edca *>(hcf()->getSubmodule("edca")); }
    MeshPowerMode local() { return meshMib(0)->findMeshPeerPowerState(peer())->local; }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("transition-step",kind)); }
    Packet *original() {
        for(int ac=0;ac<AC_NUMCATEGORIES;++ac) {
            auto owner=edca()->getEdcaf(static_cast<AccessCategory>(ac));
            auto queue=owner->getPendingQueue();
            for(int i=0;i<queue->getNumPackets();++i)
                if(queue->getPacket(i)->peekAtFront<Ieee80211MacHeader>()->getType()==ST_QOS_NULL) return queue->getPacket(i);
            auto frames=owner->getInProgressFrames();
            for(int i=0;i<frames->getLength();++i)
                if(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>()->getType()==ST_QOS_NULL) return frames->getFrames(i);
        }
        return nullptr;
    }
    void initialize() override {
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionStarted"),this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionEnded"),this);
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
        mac()->subscribe(registerSignal("packetSentToLower"),this);
        at(SimTime(5,SIMTIME_MS),90); at(SimTime(10,SIMTIME_MS),91);
        at(SimTime(100,SIMTIME_MS),0);
        if(scenario()==1) at(SimTime(100800,SIMTIME_US),1);
        if(scenario()==3) { at(SimTime(175,SIMTIME_MS),2); at(SimTime(200,SIMTIME_MS),3); }
        if(scenario()==4) { at(SimTime(5100,SIMTIME_MS),4); at(SimTime(5100,SIMTIME_MS)+SimTime(1,SIMTIME_NS),5); }
        if(scenario()==5) {at(SimTime(50,SIMTIME_MS),7);at(SimTime(95,SIMTIME_MS),8);at(SimTime(102,SIMTIME_MS),9);}
        if(scenario()>=6){at(SimTime(110,SIMTIME_MS),10);if(scenario()!=8){at(SimTime(120,SIMTIME_MS),11);at(SimTime(130,SIMTIME_MS),12);}}
        at(scenario()==4 ? SimTime(5200,SIMTIME_MS) : SimTime(240,SIMTIME_MS),6);
    }
    void receiveSignal(cComponent *, simsignal_t signal,cObject *object,cObject *) override {
        Enter_Method_Silent();
        if(signal==Hcf::meshDataAttemptCompletedSignal) { if(simTime()>=SimTime(100,SIMTIME_MS))++samples; return; }
        if(signal==registerSignal("packetSentToLower")) {
            auto packet=check_and_cast<Packet *>(object);
            if(scenario()==5&&simTime()>=SimTime(100,SIMTIME_MS)&&simTime()<SimTime(102,SIMTIME_MS)) {
                auto header=packet->peekAtFront<Ieee80211MacHeader>();
                if(std::string(packet->getName())=="pending-before-null") {
                    ASSERT(!header->getPowerMgmt()&&!packet->peekAtFront<Ieee80211MeshDataHeader>()->getMeshPowerSaveLevel());++pendingData;
                }
                auto action=dynamicPtrCast<const Ieee80211MeshActionFrame>(header);
                if(action&&action->getMeshAction()==MESH_LINK_METRIC_REPORT){ASSERT(!header->getPowerMgmt());++pendingReports;}
            }
            return;
        }
        auto transmission=check_and_cast<const physicallayer::Ieee80211Transmission *>(object);
        auto packet=transmission->getPacket();
        if(std::string(packet->getName())!="mesh-power-mode") return;
        auto phy=physicallayer::Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
        if(signal==registerSignal("transmissionStarted")) {
            ++nullPpdus;
            if(!superseded && scenario()==2) {
                ASSERT(header->getPowerMgmt() && header->getMeshPowerSaveLevel());
                oldOnAir=true; superseded=true;
                power()->requestMode(peer(),MeshPowerMode::ACTIVE);
                ASSERT(local()==MeshPowerMode::ACTIVE);
                for(const auto& frame:hcf()->getMeshBufferedFrames()) ASSERT(frame.id!=oldId);
            }
            if(!superseded && scenario()==3) {
                oldOnAir=true; superseded=true;
                meshManagement(0)->closePeer(peer());
                ASSERT(!meshMib(0)->isMeshPeerEstablished(peer()));
            }
        }
        else if(scenario()==1 && !superseded) {
            // Let the real missing ACK consume one retry, then prevent a new
            // grant until the higher request inspects and withdraws it.
            mac()->holdNull=true;
        }
    }
    void handleMessage(cMessage *message) override {
        int kind=message->getKind();delete message;
        if(kind==90) { meshManagement(0)->openPeer(peer()); return; }
        if(kind==91) { meshManagement(0)->openPeer(meshMib(2)->address); return; }
        if(kind==0) {
            ASSERT(meshMib(0)->isMeshPeerEstablished(peer()));
            oldLink=meshManagement(0)->getLocalLinkId(peer());
            mac()->holdNull=scenario()==0||scenario()==4||scenario()==5||scenario()>=6;
            emit(registerSignal("powerStart"),1L);
            power()->requestMode(peer(),MeshPowerMode::DEEP_SLEEP);
            auto pending=original(); ASSERT(pending); oldId=pending->getId();
            if(scenario()==0) {
                power()->requestMode(peer(),MeshPowerMode::ACTIVE); superseded=true;
                for(const auto& frame:hcf()->getMeshBufferedFrames()) ASSERT(frame.id!=oldId);
                mac()->holdNull=false; hcf()->meshPowerEligibilityChanged();
            }
            if(scenario()==5){mac()->holdReports=false;mac()->holdData=false;hcf()->meshPowerEligibilityChanged();}
        }
        else if(kind==1) {
            auto pending=original(); ASSERT(pending && pending->getId()==oldId);
            std::unique_ptr<Packet> snapshot(pending->dup());
            auto header=snapshot->peekAtFront<Ieee80211DataHeader>();
            auto recovery=edca()->getEdcaf(AC_BE)->getRecoveryProcedure();
            ASSERT(recovery->getRetryCount(snapshot.get(),header)>0);
            superseded=true; power()->requestMode(peer(),MeshPowerMode::ACTIVE);
            bool removed=false;
            try { recovery->getRetryCount(snapshot.get(),header); }
            catch (const cRuntimeError& error) { removed=std::string(error.what()).find("retry counter entry doesn't exist")!=std::string::npos; }
            ASSERT(removed); // Existing query rejects an absent entry, rather than returning zero.
            for(const auto& frame:hcf()->getMeshBufferedFrames()) ASSERT(frame.id!=oldId);
            mac()->holdNull=false;hcf()->meshPowerEligibilityChanged();
        }
        else if(kind==2) { meshManagement(0)->openPeer(peer()); }
        else if(kind==3) {
            ASSERT(meshMib(0)->isMeshPeerEstablished(peer()));
            ASSERT(meshManagement(0)->getLocalLinkId(peer())!=oldLink);
            ASSERT(local()==MeshPowerMode::ACTIVE);
            power()->requestMode(peer(),MeshPowerMode::LIGHT_SLEEP);
        }
        else if(kind==4) {
            for(const auto& frame:hcf()->getMeshBufferedFrames()) ASSERT(frame.id!=oldId);
            ASSERT(local()==MeshPowerMode::ACTIVE); // deadline query precedes timer callback
        }
        else if(kind==5) {
            ASSERT(power()->getRequestedMode(peer())==MeshPowerMode::ACTIVE && !original());
            mac()->holdNull=false; power()->requestMode(peer(),MeshPowerMode::LIGHT_SLEEP);
        }
        else if(kind==7)sendDirect(phyMsdu("pending-training",peer()),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
        else if(kind==8){
            mac()->holdData=true;mac()->holdReports=true;
            sendDirect(phyMsdu("pending-before-null",peer()),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
            check_and_cast<PowerManagementFixture *>(meshManagement(0))->queueReport(peer());
        }
        else if(kind==9){
            ASSERT(pendingData==1&&pendingReports>=1&&local()==MeshPowerMode::ACTIVE&&power()->getRequestedMode(peer())==MeshPowerMode::DEEP_SLEEP);
            mac()->holdNull=false;hcf()->meshPowerEligibilityChanged();
        }
        else if(kind==10){
            if(scenario()==8)power()->deleteModule();
            else{
                auto operation=scenario()==7?static_cast<LifecycleOperation *>(new ModuleCrashOperation()):static_cast<LifecycleOperation *>(new ModuleStopOperation());
                LifecycleOperation::StringMap parameters;operation->initialize(power(),parameters);ASSERT(initiateOperation(operation));
            }
            ASSERT(!original()&&meshMib(0)->isMeshPeerEstablished(peer()));
            ASSERT(nullPpdus==0);
            mac()->holdNull=false;hcf()->meshPowerEligibilityChanged();
        }
        else if(kind==11){
            auto operation=new ModuleStartOperation();LifecycleOperation::StringMap parameters;operation->initialize(power(),parameters);ASSERT(initiateOperation(operation));
            ASSERT(power()->isEnabled()&&power()->getRequestedMode(peer())==MeshPowerMode::ACTIVE);
        }
        else if(kind==12)power()->requestMode(peer(),MeshPowerMode::LIGHT_SLEEP);
        else {
            ASSERT(samples==(scenario()==5?1:0));
            if(scenario()==3||scenario()==4||scenario()==6||scenario()==7) ASSERT(local()==MeshPowerMode::LIGHT_SLEEP);
            else if(scenario()==8)ASSERT(local()==MeshPowerMode::ACTIVE&&!meshInterface(0)->getSubmodule("powerSavePolicy"));
            else if(scenario()==5)ASSERT(local()==MeshPowerMode::DEEP_SLEEP&&pendingData==1&&pendingReports>=1);
            else ASSERT(local()==MeshPowerMode::ACTIVE && superseded);
            if(scenario()==2||scenario()==3) ASSERT(oldOnAir);
            int expected=scenario()==8?0:(scenario()==4||scenario()>=5?1:2);
            ASSERT(nullPpdus==expected && !original());
            if(scenario()>=6)ASSERT(check_and_cast<PowerForwardingFixture *>(mac()->getSubmodule("ds"))->nextIdentity()==0);
            emit(registerSignal("powerComplete"),1L);
        }
    }
};
namespace power_transition_registration { Define_Module(::PowerSupersessionDriver); }
#endif
