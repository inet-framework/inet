// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHENDINGRESPONSESUPPORT_H
#define INET_TEST_MESHENDINGRESPONSESUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshPowerResponseTag_m.h"

class StampPowerObserver : public MeshPowerSave
{
  public:
    bool requested(MacAddress peer) const {
        auto found=peerWake.find(peer);
        return found!=peerWake.end() && found->second.retrievalRequested;
    }
    std::pair<int64_t,simtime_t> opening(MacAddress peer,MeshPeriodDirection direction) const {
        auto period=findPeriod(peer,direction); ASSERT(period);
        return {period->openingId,period->openingTime};
    }
};
Define_Module(StampPowerObserver);

class StampDisposition : public cObject
{
  public:
    bool cancelled;
    int64_t openingId;
    simtime_t openingTime;
    StampDisposition(bool cancelled,int64_t id,simtime_t time) : cancelled(cancelled),openingId(id),openingTime(time) {}
};

class EndingResponseHcf : public Hcf
{
  public:
    std::pair<int,int> recipientWindows(MacAddress peer) const {
        auto agreement=recipientBlockAckAgreementHandler->getAgreement(0,peer);
        ASSERT(agreement);
        return {agreement->getStartingSequenceNumber().get(),agreement->getBlockAckRecord()->getWindowStart().get()};
    }
    bool hasRecipient(MacAddress peer) const { return recipientBlockAckAgreementHandler->getAgreement(0,peer)!=nullptr; }
    void withdrawRecipient(MacAddress peer) { Enter_Method("withdrawRecipient"); retireAgreement(peer,0,false); }
    void expireSentinel() {
        Enter_Method("expireSentinel");
        auto queue=edca->getEdcaf(AC_BE)->getPendingQueue();
        for (int i=0;i<queue->getNumPackets();i++) {
            auto packet=queue->getPacket(i);
            if (std::string(packet->getName())!="stamp-sentinel") continue;
            edca->getLifetimeHandler()->forgetFrame(packet->getId());
            edca->getLifetimeHandler()->registerFragments({packet},simTime());
            expireDataFrames(false); return;
        }
        throw cRuntimeError("Expected unsent stamp sentinel");
    }
};
Define_Module(EndingResponseHcf);

class EndingResponseMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        if (std::string(packet->getName())=="stamp-sentinel") return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
Define_Module(EndingResponseMac);

class EndingResponseTx : public Tx
{
    bool withdrawn=false;
  public:
    void resumePendingTransmission() override {
        Enter_Method("resumePendingTransmission");
        auto stamp=frame ? frame->findTag<Ieee80211MeshPowerResponseTag>() : nullptr;
        if (!withdrawn && getContainingNode(this)->getIndex()==1 && !transmitting && frame
                && admittedStart<=simTime() && stamp && stamp->getQualifyingBlockAck()) {
            auto hcf=check_and_cast<EndingResponseHcf *>(getParentModule()->getSubmodule("hcf"));
            auto windows=hcf->recipientWindows(meshMib(0)->address);
            auto response=frame->peekAtFront<Ieee80211CompressedBlockAck>();
            ASSERT(windows.second>windows.first && response->getStartingSequenceNumber().get()==windows.second);
            ASSERT(stamp->getReceiptId()==-1 && stamp->getOpeningId()!=-1);
            auto openingId=stamp->getOpeningId(); auto openingTime=stamp->getOpeningTime();
            withdrawn=true;
            hcf->withdrawRecipient(meshMib(0)->address);
            ASSERT(!hcf->hasRecipient(meshMib(0)->address));
            if (getSimulation()->getSystemModule()->getSubmodule("driver")->par("cancelResponse").boolValue()) {
                cancelUnstartedTransmission();
                StampDisposition fact(true,openingId,openingTime); emit(registerSignal("stampDisposition"),&fact);
                return;
            }
            ASSERT(frame->getTag<Ieee80211MeshPowerResponseTag>()->getQualifyingBlockAck());
            StampDisposition fact(false,openingId,openingTime); emit(registerSignal("stampDisposition"),&fact);
        }
        Tx::resumePendingTransmission();
    }
};
Define_Module(EndingResponseTx);

class EndingResponseDriver : public cSimpleModule, public cListener
{
    int delivered=0, samples=0, errors=0, ppdus=0, nChecks=0;
    int disposition=0, qualifiedAir=0, ownerConfirmed=0, recipientConfirmed=0, newerRequest=0;
    double expectedN=1;
    std::optional<std::pair<int64_t,simtime_t>> sourceOpening,recipientOpening;
    bool cancel() const { return par("cancelResponse"); }
    MeshPowerSave *power(int i) const { return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    EndingResponseHcf *hcf(int i) const { return check_and_cast<EndingResponseHcf *>(meshInterface(i)->getSubmodule("mac")->getSubmodule("hcf")); }
    bool role(int i,MeshPeriodDirection direction) const {
        for (const auto& period : power(i)->getServicePeriods()) if (period.direction==direction) return true;
        return false;
    }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("ending-response-check",kind)); }
    void source(const char *name) {
        check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
            ->processSourceMsdu(phyMsdu(name,meshMib(1)->address));
    }
    void batch(int first,int count) { for (int i=first;i<first+count;i++) source(("stamp-data-"+std::to_string(i)).c_str()); }
  protected:
    void initialize() override {
        for (auto signal : {packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal,Hcf::meshPayloadTransmittedSignal,
                transmissionStartedSignal,receptionEndedSignal,MeshPowerSave::meshServicePeriodOutcomeSignal,registerSignal("stampDisposition")})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(50,SIMTIME_MS),1);
        at(SimTime(60,SIMTIME_MS),2); at(SimTime(80,SIMTIME_MS),3);
        at(SimTime(180,SIMTIME_MS),4); at(SimTime(185,SIMTIME_MS),5);
        at(SimTime(190,SIMTIME_MS),6); at(SimTime(195,SIMTIME_MS),7);
        at(SimTime(205,SIMTIME_MS),8); at(SimTime(240,SIMTIME_MS),20);
    }
    void receiveSignal(cComponent *module,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("stampDisposition")) {
            auto fact=check_and_cast<StampDisposition *>(object);
            ASSERT(fact->cancelled==cancel() && disposition==0); disposition++;
            sourceOpening=check_and_cast<StampPowerObserver *>(power(0))->opening(meshMib(1)->address,MeshPeriodDirection::OWNER);
            recipientOpening=std::make_pair(fact->openingId,fact->openingTime);
            ASSERT(*recipientOpening==check_and_cast<StampPowerObserver *>(power(1))->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT));
            ASSERT(newerRequest==1 && check_and_cast<StampPowerObserver *>(power(1))->requested(meshMib(0)->address));
            at(simTime()+SimTime::fromRaw(1),10);
            if (cancel()) at(simTime()+SimTime::fromRaw(2),11);
            return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            auto outcome=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (outcome->outcome==MeshPeriodOutcome::CONFIRMED) {
                auto opening=std::make_pair(outcome->openingId,outcome->openingTime);
                ownerConfirmed+=module==power(0) && outcome->direction==MeshPeriodDirection::OWNER && sourceOpening && opening==*sourceOpening;
                recipientConfirmed+=module==power(1) && outcome->direction==MeshPeriodDirection::RECIPIENT && recipientOpening && opening==*recipientOpening;
                if (module==power(1) && recipientOpening && opening==*recipientOpening)
                    ASSERT(check_and_cast<StampPowerObserver *>(power(1))->requested(meshMib(0)->address));
            }
            return;
        }
        if (signal==receptionEndedSignal) {
            if (module!=meshInterface(1)->getSubmodule("radio") || simTime()<SimTime(204,SIMTIME_MS)
                    || simTime()>=SimTime(205,SIMTIME_MS)) return;
            auto received=dynamic_cast<Packet *>(details);
            if (received && received->peekAtFront<Ieee80211MacHeader>()->getType()==ST_BEACON)
                at(simTime()+SimTime::fromRaw(1),13);
            return;
        }
        if (signal==transmissionStartedSignal) {
            if (module!=meshInterface(1)->getSubmodule("radio")) return;
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            auto stamp=packet->findTag<Ieee80211MeshPowerResponseTag>();
            if (stamp && stamp->getQualifyingBlockAck()) { ASSERT(disposition==1); qualifiedAir++; }
            return;
        }
        auto packet=dynamic_cast<Packet *>(object);
        if (!packet || std::string(packet->getName()).find("stamp-data-")!=0) return;
        if (signal==packetSentToUpperSignal && module==meshInterface(1)->getSubmodule("mac")) delivered++;
        if (signal==Hcf::meshDataAttemptCompletedSignal && module==hcf(0)) {
            auto attempt=check_and_cast<MeshDataAttempt *>(details); samples++; errors+=!attempt->acknowledged;
        }
        if (signal==Hcf::meshPayloadTransmittedSignal && module==hcf(0)) {
            static const int expected[]={20,20,20,10,2};
            ASSERT(ppdus<5 && check_and_cast<MeshPayloadTransmission *>(details)->msduCount==expected[ppdus]);
            expectedN=expectedN*0.875+expected[ppdus]*0.125; ppdus++;
            at(simTime()+SimTime::fromRaw(1),12);
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) {
            fixturePath(0,1,1,SimTime(1,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(1,SIMTIME_S));
            source("stamp-prime"); source("stamp-sentinel");
        }
        else if (kind==2) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==4) batch(0,20);
        else if (kind==5) batch(20,20);
        else if (kind==6) batch(40,20);
        else if (kind==7) batch(60,10);
        else if (kind==8) { batch(70,2); hcf(0)->expireSentinel(); }
        else if (kind==10) {
            ASSERT(!hcf(1)->hasRecipient(meshMib(0)->address));
            if (cancel()) ASSERT(qualifiedAir==0 && role(1,MeshPeriodDirection::RECIPIENT) && role(0,MeshPeriodDirection::OWNER));
        }
        else if (kind==11) { meshManagement(0)->closePeer(meshMib(1)->address); meshManagement(1)->closePeer(meshMib(0)->address); }
        else if (kind==12) {
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getPayloadEstimate(meshMib(1)->address)==expectedN); nChecks++;
        }
        else if (kind==13) {
            ASSERT(role(1,MeshPeriodDirection::RECIPIENT));
            ASSERT(check_and_cast<StampPowerObserver *>(power(1))->requested(meshMib(0)->address));
            newerRequest++;
        }
        else {
            std::cout << "stamp totals cancel=" << cancel() << " delivered=" << delivered << " samples=" << samples
                << " errors=" << errors << " ppdus=" << ppdus << " nChecks=" << nChecks << std::endl;
            ASSERT(delivered==72 && samples==(cancel() ? 70 : 72) && errors==0 && ppdus==5 && nChecks==5);
            ASSERT(disposition==1 && qualifiedAir==(cancel() ? 0 : 1));
            ASSERT(ownerConfirmed==(cancel() ? 0 : 1) && recipientConfirmed==(cancel() ? 0 : 1));
            ASSERT(power(0)->getServicePeriods().empty() && power(1)->getServicePeriods().empty());
            std::cout << "stamp cancel=" << cancel() << " delivered=" << delivered << " samples=" << samples
                << " air=" << qualifiedAir << " confirmed=" << ownerConfirmed << "/" << recipientConfirmed << std::endl;
            emit(registerSignal("stampComplete"),1L);
        }
    }
};
Define_Module(EndingResponseDriver);
#endif
