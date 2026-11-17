// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSERVICEINTENTSUPPORT_H
#define INET_TEST_MESHSERVICEINTENTSUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"

class IntentPowerObserver : public MeshPowerSave
{
  public:
    int64_t intentId(MacAddress peer) const { auto found=triggers.find(peer); return found==triggers.end() ? -1 : found->second.queuedId; }
    bool failedIntent(MacAddress peer) const { auto found=triggers.find(peer); return found!=triggers.end() && found->second.failed; }
    simtime_t awakeEnd(MacAddress peer) const { auto found=awakeUntil.find(peer); return found==awakeUntil.end() ? SIMTIME_ZERO : found->second; }
};
Define_Module(IntentPowerObserver);

class IntentHcfFixture : public Hcf
{
  public:
    void expireNamedNow(const char *name) {
        Enter_Method("expireNamedNow");
        for (int ac=0;ac<AC_NUMCATEGORIES;ac++) {
            auto queue=edca->getEdcaf(static_cast<AccessCategory>(ac))->getPendingQueue();
            for (int i=0;i<queue->getNumPackets();i++) {
                auto packet=queue->getPacket(i);
                if (std::string(packet->getName())!=name) continue;
                edca->getLifetimeHandler()->forgetFrame(packet->getId());
                edca->getLifetimeHandler()->registerFragments({packet},simTime());
                expireDataFrames(false); return;
            }
        }
        throw cRuntimeError("Expected queued intent fixture DATA");
    }
};
Define_Module(IntentHcfFixture);

class IntentMacFixture : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        if (std::string(packet->getName()).find("intent-data-")==0 && !driver->par("releasePayload").boolValue()) return false;
        if (std::string(packet->getName())=="mesh-period-trigger" && driver->par("scenario").intValue()==23
                && (getContainingNode(this)->getIndex()==1 || !driver->par("releaseTrigger").boolValue())) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
Define_Module(IntentMacFixture);

class ServiceIntentDriver : public cSimpleModule, public cListener
{
    int overflows=0, controlExpiries=0, dataExpiries=0, triggerAir=0, delivered=0, samples=0, errors=0;
    int quiescentChecks=0, reentryChecks=0;
    int64_t droppedIntent=-1, replacement=-1;
    simtime_t failedAt=-1;
    int which() const { return par("scenario").intValue()-22; }
    int remaining() const { return which()==1 ? 1 : 63; }
    IntentPowerObserver *power() const { return check_and_cast<IntentPowerObserver *>(meshInterface(0)->getSubmodule("powerSavePolicy")); }
    IntentHcfFixture *hcf() const { return check_and_cast<IntentHcfFixture *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("intent-check",kind)); }
    int count(MeshBufferedFrame::Kind kind) const {
        int result=0; for (const auto& frame : hcf()->getMeshBufferedFrames()) result+=frame.receiver==meshMib(1)->address && frame.kind==kind;
        return result;
    }
    void releasePayload() {
        bool owner=false; for (const auto& period : power()->getServicePeriods()) owner|=period.direction==MeshPeriodDirection::OWNER;
        ASSERT(owner); par("releasePayload").setBoolValue(true); hcf()->meshPowerEligibilityChanged();
    }
  protected:
    void initialize() override {
        for (auto signal : {packetDroppedSignal,transmissionStartedSignal,packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1); at(SimTime(80,SIMTIME_MS),2);
        at(SimTime(130,SIMTIME_MS),3);
        at(which()==1 ? SimTime(5400,SIMTIME_MS) : SimTime(450,SIMTIME_MS),20);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==packetDroppedSignal && source==hcf()) {
            auto packet=check_and_cast<Packet *>(object); auto drop=check_and_cast<PacketDropDetails *>(details);
            if (std::string(packet->getName()).find("intent-data-")==0 && drop->getReason()==LIFETIME_EXPIRED) dataExpiries++;
            if (std::string(packet->getName())!="mesh-period-trigger") return;
            ASSERT(overflows+controlExpiries==0); droppedIntent=packet->getId(); failedAt=simTime();
            if (drop->getReason()==QUEUE_OVERFLOW) {
                overflows++;
                if (which()==2) {
                    hcf()->expireNamedNow("intent-data-0");
                    // Explicit accepted management-role input for this
                    // synchronous reentry probe, not a second RF beacon.
                    power()->beaconReceived(meshMib(1)->address,false,false,SimTime(10240,SIMTIME_US));
                    power()->reviewServicePeriods();
                    replacement=power()->intentId(meshMib(1)->address);
                    ASSERT(replacement!=-1 && replacement!=droppedIntent && !power()->failedIntent(meshMib(1)->address));
                    at(simTime()+SimTime::fromRaw(1),4);
                }
                else { at(simTime()+SimTime(1,SIMTIME_MS),5); at(simTime()+SimTime(12,SIMTIME_MS),8); }
            }
            else { ASSERT(which()==1 && drop->getReason()==LIFETIME_EXPIRED); controlExpiries++; at(simTime()+SimTime(1,SIMTIME_MS),9); }
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal) {
            auto packet=check_and_cast<Packet *>(object);
            if (source!=hcf() || std::string(packet->getName()).find("intent-data-")!=0) return;
            auto fact=check_and_cast<MeshDataAttempt *>(details); samples++; errors+=!fact->acknowledged; return;
        }
        if (signal==packetSentToUpperSignal && source==meshInterface(1)->getSubmodule("mac")) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && std::string(packet->getName()).find("intent-data-")==0) delivered++;
            return;
        }
        if (signal==transmissionStartedSignal && source==meshInterface(0)->getSubmodule("radio")) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if (std::string(packet->getName())=="mesh-period-trigger") {
                ASSERT(triggerAir==0 && simTime()>failedAt);
                triggerAir++; at(simTime()+SimTime(1,SIMTIME_MS),7);
            }
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) check_and_cast<MeshPowerSave *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==2) check_and_cast<MeshPowerSave *>(meshInterface(1)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(0)->address,MeshPowerMode::DEEP_SLEEP);
        else if (kind==3) {
            fixturePath(0,1,1,SimTime(7,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(7,SIMTIME_S));
            auto forwarding=check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"));
            int frames=which()==1 ? 1 : 64;
            for (int i=0;i<frames;i++) forwarding->processSourceMsdu(phyMsdu(("intent-data-"+std::to_string(i)).c_str(),meshMib(1)->address));
            ASSERT(count(MeshBufferedFrame::Kind::PAYLOAD)==frames);
        }
        else if (kind==4) {
            ASSERT(power()->intentId(meshMib(1)->address)==replacement && !power()->failedIntent(meshMib(1)->address));
            ASSERT(count(MeshBufferedFrame::Kind::POWER_CONTROL)==1 && count(MeshBufferedFrame::Kind::PAYLOAD)==63);
            reentryChecks++;
        }
        else if (kind==5) { hcf()->expireNamedNow("intent-data-0"); at(simTime()+SimTime(100,SIMTIME_US),6); }
        else if (kind==6 || kind==10) {
            ASSERT(power()->failedIntent(meshMib(1)->address) && power()->intentId(meshMib(1)->address)==droppedIntent);
            ASSERT(count(MeshBufferedFrame::Kind::PAYLOAD)==remaining() && count(MeshBufferedFrame::Kind::POWER_CONTROL)==0 && triggerAir==0);
            if (kind==6) ASSERT(simTime()<power()->awakeEnd(meshMib(1)->address));
            quiescentChecks++;
        }
        else if (kind==7) releasePayload();
        else if (kind==8 || kind==9) {
            ASSERT(check_and_cast<IRadio *>(meshInterface(0)->getSubmodule("radio"))->getRadioMode()==IRadio::RADIO_MODE_SLEEP);
            ASSERT(!power()->requiresAwake());
            if (kind==9) { par("releaseTrigger").setBoolValue(true); hcf()->meshPowerEligibilityChanged(); at(simTime()+SimTime(100,SIMTIME_US),10); }
        }
        else {
            ASSERT(overflows==(which()==1 ? 0 : 1) && controlExpiries==(which()==1 ? 1 : 0));
            ASSERT(dataExpiries==(which()==1 ? 0 : 1) && triggerAir==1 && delivered==remaining() && samples==remaining() && errors==0);
            ASSERT(quiescentChecks==(which()==2 ? 0 : 1) && reentryChecks==(which()==2 ? 1 : 0));
            ASSERT(count(MeshBufferedFrame::Kind::PAYLOAD)==0 && power()->getServicePeriods().empty());
            std::cout << "intent case=" << which() << " overflow=" << overflows << " expiry=" << controlExpiries
                << " delivered=" << delivered << " quiescent=" << quiescentChecks << " reentry=" << reentryChecks << std::endl;
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
Define_Module(ServiceIntentDriver);
#endif
