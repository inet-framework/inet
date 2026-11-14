// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSERVICEPERIODTABLESUPPORT_H
#define INET_TEST_MESHSERVICEPERIODTABLESUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshPowerResponseTag_m.h"

class EndingResponseTxFixture : public Tx
{
    bool cancelled=false;
  public:
    void resumePendingTransmission() override {
        Enter_Method("resumePendingTransmission");
        if (!cancelled && frame && !transmitting && admittedStart<=simTime()
                && getContainingNode(this)->getIndex()==1
                && getSimulation()->getSystemModule()->par("scenario").intValue()==20
                && frame->findTag<Ieee80211MeshPowerResponseTag>()) {
            cancelled=true;
            cancelUnstartedTransmission(); // Existing detach-before-callback owner.
            emit(registerSignal("endingCancelled"),1L);
            return;
        }
        Tx::resumePendingTransmission();
    }
};
Define_Module(EndingResponseTxFixture);

class ActiveResponseTxFixture : public Tx
{
    bool invoked=false;
  public:
    void resumePendingTransmission() override {
        Enter_Method("resumePendingTransmission");
        auto policy=check_and_cast<MeshPowerSave *>(getParentModule()->getParentModule()->getSubmodule("powerSavePolicy"));
        bool ending=false;
        for (const auto& period : policy->getServicePeriods())
            ending|=period.direction==MeshPeriodDirection::RECIPIENT && period.ending;
        if (!invoked && getContainingNode(this)->getIndex()==1 && !transmitting && frame
                && admittedStart<=simTime() && frame->findTag<Ieee80211MeshPowerResponseTag>() && ending) {
            invoked=true;
            emit(registerSignal("activeResponseHeld"),1L);
        }
        Tx::resumePendingTransmission();
    }
};
namespace active_response_registration { Define_Module(ActiveResponseTxFixture); }

class TablePowerObserver : public MeshPowerSave
{
  public:
    bool bufferedHint(MacAddress peer) const {
        auto wake=peerWake.find(peer); return wake!=peerWake.end() && wake->second.individualBuffered;
    }
    bool retrievalRequested(MacAddress peer) const {
        auto wake=peerWake.find(peer); return wake!=peerWake.end() && wake->second.retrievalRequested;
    }
    std::pair<int64_t,simtime_t> opening(MacAddress peer,MeshPeriodDirection direction) const {
        auto period=findPeriod(peer,direction); ASSERT(period);
        return {period->openingId,period->openingTime};
    }
    void originalAcknowledged(const Packet *packet) override {
        MeshPowerSave::originalAcknowledged(packet);
        if (std::string(packet->getName())=="table-trigger") emit(registerSignal("tableAck"),const_cast<Packet *>(packet));
    }
};
namespace table_power_registration { Define_Module(TablePowerObserver); }

class ReplayEndingMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName())=="mesh-period-ending"
                && !driver->par("releaseEnding").boolValue()) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
Define_Module(ReplayEndingMac);

class ActiveEndingMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        if (getContainingNode(this)->getIndex()==1 && std::string(packet->getName())=="mesh-period-ending"
                && !driver->par("releaseEnding").boolValue()) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace active_ending_registration { Define_Module(ActiveEndingMac); }

inline void setReceivedNullRetry(Packet *packet)
{
    auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    Packet member("retry1-null-wire-control",packet->peekDataAt(phy->getChunkLength(),phy->getLengthField()));
    auto header=member.removeAtFront<Ieee80211MeshDataHeader>();
    ASSERT(header->getType()==ST_QOS_NULL && header->getSequenceNumber().get()==0);
    header->setRetry(true); member.insertAtFront(header);
    member.removeAtBack<Ieee80211MacTrailer>(B(4));
    auto bytes=member.peekDataAsBytes()->getBytes();
    auto trailer=makeShared<Ieee80211MacTrailer>();
    trailer->setFcsMode(FCS_COMPUTED); trailer->setFcs(ethernetFcs(bytes.data(),bytes.size()));
    member.insertAtBack(trailer);
    packet->replaceDataAt(member.peekData(),phy->getChunkLength(),phy->getLengthField());
}

inline void setReceivedNullMoreData(Packet *packet)
{
    auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    Packet member("ending-more-data-wire-control",packet->peekDataAt(phy->getChunkLength(),phy->getLengthField()));
    auto header=member.removeAtFront<Ieee80211MeshDataHeader>();
    ASSERT(header->getType()==ST_QOS_NULL && header->getEosp());
    header->setMoreData(true); member.insertAtFront(header);
    member.removeAtBack<Ieee80211MacTrailer>(B(4));
    auto bytes=member.peekDataAsBytes()->getBytes();
    auto trailer=makeShared<Ieee80211MacTrailer>();
    trailer->setFcsMode(FCS_COMPUTED); trailer->setFcs(ethernetFcs(bytes.data(),bytes.size()));
    member.insertAtBack(trailer);
    packet->replaceDataAt(member.peekData(),phy->getChunkLength(),phy->getLengthField());
}

class ServicePeriodTableDriver : public cSimpleModule, public cListener
{
    bool submitted=false;
    int beforeAck=0, afterAck=0, air=0, confirmed=0, failed=0, payloadSamples=0;
    int endingAir=0, cancelled=0, lostAsymmetry=0;
    int moreDataEndings=0, quietChecks=0, targetTriggerAir=0, laterBeacons=0;
    bool recipientEnded=false;
    int activeWithdrawn=0, activeConfirmed=0, activeNotices=0, activeResponseAir=0;
    bool activeCommand=false, nestedCommand=false;
    int64_t endingTree=-1;
    int submittedCount=0, retry1Receipts=0;
    bool secondScheduled=false;
    int64_t triggerTree=-1;
    std::optional<std::pair<int64_t,simtime_t>> firstOwner,firstRecipient;
    int scenario() const { return par("scenario").intValue()-4; }
    bool replay() const { return scenario()==21 || scenario()==22; }
    bool ongoingReplay() const { return scenario()==22; }
    bool lostTrigger() const { return scenario()==23; }
    bool moreDataOnly() const { return scenario()==33; }
    bool activeWithdrawal() const { return scenario()>=34 && scenario()<=36; }
    int combination() const { return activeWithdrawal() ? 2 : (scenario()>=16 ? 0 : scenario()%4); }
    bool rspi() const { return combination()>=2; }
    bool eosp() const { return combination()%2; }
    MeshPowerMode mode(int i) const {
        if (scenario()/4==3) return MeshPowerMode::DEEP_SLEEP;
        return (scenario()/4==1 && i==0) || (scenario()/4==2 && i==1) ? MeshPowerMode::ACTIVE : MeshPowerMode::LIGHT_SLEEP;
    }
    MeshPowerSave *power(int i) const { return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    std::set<MeshPeriodDirection> roles(int i) const {
        std::set<MeshPeriodDirection> result;
        for (const auto& view : power(i)->getServicePeriods()) result.insert(view.direction);
        return result;
    }
    std::set<MeshPeriodDirection> expected(int i) const {
        std::set<MeshPeriodDirection> result;
        if (!eosp() && mode(1)!=MeshPowerMode::ACTIVE)
            result.insert(i==0 ? MeshPeriodDirection::OWNER : MeshPeriodDirection::RECIPIENT);
        if (rspi() && mode(0)!=MeshPowerMode::ACTIVE)
            result.insert(i==0 ? MeshPeriodDirection::RECIPIENT : MeshPeriodDirection::OWNER);
        return result;
    }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("table-check",kind)); }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,
                MeshPowerSave::meshServicePeriodOutcomeSignal,Hcf::meshDataAttemptCompletedSignal,registerSignal("tableAck")})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        getSimulation()->getSystemModule()->subscribe(registerSignal("endingCancelled"),this);
        getSimulation()->getSystemModule()->subscribe(registerSignal("activeResponseHeld"),this);
        getSimulation()->getSystemModule()->subscribe(Ieee80211Mib::meshPowerModeChangedSignal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1);
        at(SimTime(80,SIMTIME_MS),2);
        at(scenario()/4==3 ? SimTime(1500,SIMTIME_MS) : SimTime(replay() ? 420 : 260,SIMTIME_MS),6);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,long value,cObject *) override {
        Enter_Method_Silent();
        if (signal==registerSignal("endingCancelled")) {
            ASSERT(value==1 && scenario()==16 && roles(0).count(MeshPeriodDirection::OWNER)
                && roles(1).count(MeshPeriodDirection::RECIPIENT));
            ASSERT(confirmed==0); cancelled++; return;
        }
        if (signal==Ieee80211Mib::meshPowerModeChangedSignal && activeCommand && source==meshMib(1)
                && value==long(MeshPowerMode::ACTIVE)) {
            ASSERT(!roles(1).count(MeshPeriodDirection::RECIPIENT) && roles(1).count(MeshPeriodDirection::OWNER));
            activeNotices++;
            if (scenario()==36 && !nestedCommand) {
                nestedCommand=true;
                power(1)->requestMode(meshMib(0)->address,MeshPowerMode::ACTIVE);
            }
            return;
        }
        if (signal!=registerSignal("activeResponseHeld")) return;
        ASSERT(activeWithdrawal() && value==1 && !activeCommand);
        ASSERT(roles(1).count(MeshPeriodDirection::RECIPIENT) && roles(1).count(MeshPeriodDirection::OWNER));
        activeCommand=true;
        if (scenario()==35) power(1)->requestActiveForAllPeers();
        else power(1)->requestMode(meshMib(0)->address,MeshPowerMode::ACTIVE);
        ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->local==MeshPowerMode::ACTIVE);
        ASSERT(!roles(1).count(MeshPeriodDirection::RECIPIENT) && roles(1).count(MeshPeriodDirection::OWNER));
        ASSERT(activeWithdrawn==1 && activeNotices==1);
        at(simTime()+SimTime(1,SIMTIME_MS),8);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("tableAck") && source==meshInterface(0)->getSubmodule("powerSavePolicy")) {
            ASSERT(roles(0)==expected(0)); afterAck++;
            if (replay()) {
                auto owner=check_and_cast<TablePowerObserver *>(power(0))->opening(meshMib(1)->address,MeshPeriodDirection::OWNER);
                auto recipient=check_and_cast<TablePowerObserver *>(power(1))->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT);
                if (afterAck==1) {
                    firstOwner=owner; firstRecipient=recipient;
                    if (ongoingReplay()) { secondScheduled=true; at(simTime()+SimTime(1,SIMTIME_MS),3); }
                }
                else {
                    ASSERT(afterAck==2 && firstOwner && firstRecipient);
                    ASSERT((owner==*firstOwner)==ongoingReplay());
                    ASSERT((recipient==*firstRecipient)==ongoingReplay());
                    if (ongoingReplay()) {
                        par("releaseEnding").setBoolValue(true);
                        check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
                    }
                }
            }
            return;
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            confirmed+=fact->outcome==MeshPeriodOutcome::CONFIRMED;
            failed+=fact->outcome==MeshPeriodOutcome::FAILED;
            if (activeWithdrawal() && source==power(1) && fact->direction==MeshPeriodDirection::RECIPIENT) {
                if (fact->outcome==MeshPeriodOutcome::WITHDRAWN) {
                    ASSERT(activeCommand && meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->local==MeshPowerMode::ACTIVE);
                    activeWithdrawn++;
                }
                if (activeCommand && fact->outcome==MeshPeriodOutcome::CONFIRMED) activeConfirmed++;
            }
            if (scenario()==17 && fact->direction==MeshPeriodDirection::RECIPIENT && fact->outcome==MeshPeriodOutcome::CONFIRMED) {
                ASSERT(roles(0).count(MeshPeriodDirection::OWNER)); lostAsymmetry++;
            }
            if (moreDataOnly() && source==meshInterface(1)->getSubmodule("powerSavePolicy")
                    && fact->direction==MeshPeriodDirection::RECIPIENT && fact->outcome==MeshPeriodOutcome::CONFIRMED) {
                ASSERT(!recipientEnded && moreDataEndings==1);
                recipientEnded=true; at(simTime()+SimTime(20,SIMTIME_MS),7);
            }
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal) { payloadSamples++; return; }
        const Packet *packet=nullptr;
        if (signal==receptionEndedSignal) packet=dynamic_cast<const Packet *>(details);
        if (signal==transmissionStartedSignal) packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if (!packet) return;
        b offset=signal==receptionEndedSignal ? b(0) : Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet)->getChunkLength();
        auto header=packet->peekDataAt<Ieee80211MacHeader>(offset);
        if (activeWithdrawal() && signal==transmissionStartedSignal && source==meshInterface(1)->getSubmodule("radio")
                && activeCommand && packet->findTag<Ieee80211MeshPowerResponseTag>()) activeResponseAir++;
        if (moreDataOnly() && signal==receptionEndedSignal && source==meshInterface(1)->getSubmodule("radio")) {
            if (recipientEnded && header->getType()==ST_BEACON) laterBeacons++;
            if (std::string(packet->getName())=="mesh-period-ending") {
                auto ending=packet->peekAtFront<Ieee80211MeshDataHeader>();
                ASSERT(!packet->hasBitError() && ending->getEosp() && ending->getMoreData());
                moreDataEndings++;
            }
        }
        if (moreDataOnly() && signal==transmissionStartedSignal && source==meshInterface(1)->getSubmodule("radio")
                && std::string(packet->getName())=="mesh-period-trigger") targetTriggerAir++;
        if ((scenario()==16 || scenario()==17) && signal==transmissionStartedSignal && source==meshInterface(0)->getSubmodule("radio")
                && std::string(packet->getName())=="mesh-period-ending") {
            auto ending=packet->peekDataAt<Ieee80211MeshDataHeader>(offset);
            ASSERT(ending->getEosp() && !ending->getRspi() && ending->getSequenceNumber().get()==0);
            ASSERT(ending->getRetry()==(endingAir!=0));
            if (endingTree==-1) { endingTree=packet->getTreeId(); emit(registerSignal("endingStart"),1L); }
            ASSERT(endingTree==packet->getTreeId());
            endingAir++;
        }
        if (signal==receptionEndedSignal && source==meshInterface(0)->getSubmodule("radio")
                && header->getType()==ST_BEACON && simTime()>SimTime(150,SIMTIME_MS)
                && (!submitted || (replay() && !ongoingReplay() && afterAck==1 && !secondScheduled && roles(0).empty() && roles(1).empty()))) {
            if (submitted) secondScheduled=true;
            submitted=true; at(simTime()+SimTime::fromRaw(1),3);
        }
        if (std::string(packet->getName())!="table-trigger") return;
        if (signal==transmissionStartedSignal) {
            auto data=packet->peekDataAt<Ieee80211MeshDataHeader>(offset);
            ASSERT(data->getRspi()==rspi() && data->getEosp()==eosp());
            ASSERT(data->getSequenceNumber().get()==0 && data->getFragmentNumber()==0);
            ASSERT(roles(0)==(ongoingReplay() && air==1 ? expected(0) : std::set<MeshPeriodDirection>{}));
            if (lostTrigger()) {
                ASSERT(data->getRetry()==(air!=0));
                if (triggerTree==-1) { triggerTree=packet->getTreeId(); emit(registerSignal("triggerStarted"),1L); }
                ASSERT(triggerTree==packet->getTreeId());
            }
            air++;
        }
        else if (source==meshInterface(1)->getSubmodule("radio")) {
            ASSERT(!packet->hasBitError());
            if (replay() && submittedCount==2) {
                ASSERT(packet->peekAtFront<Ieee80211MeshDataHeader>()->getRetry()); retry1Receipts++;
            }
            at(simTime()+SimTime::fromRaw(1),4);
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,mode(0));
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,mode(1));
        else if (kind==3) {
            ASSERT(roles(0)==(ongoingReplay() && submittedCount==1 ? expected(0) : std::set<MeshPeriodDirection>{}));
            ASSERT(roles(1)==(ongoingReplay() && submittedCount==1 ? expected(1) : std::set<MeshPeriodDirection>{}));
            submittedCount++;
            auto header=makeShared<Ieee80211MeshDataHeader>();
            header->setType(ST_QOS_NULL); header->setChunkLength(B(32));
            header->setToDS(true); header->setFromDS(true);
            header->setReceiverAddress(meshMib(1)->address); header->setTransmitterAddress(meshMib(0)->address);
            header->setAddress3(meshMib(1)->address); header->setAddress4(meshMib(0)->address);
            header->setAckPolicy(NORMAL_ACK); header->setRspi(rspi()); header->setEosp(eosp());
            auto packet=new Packet("table-trigger",header);
            packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
            packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
            check_and_cast<IMeshDataMac *>(meshInterface(0)->getSubmodule("mac"))->sendMeshData(packet);
        }
        else if (kind==4) {
            ASSERT(roles(1)==expected(1));
            ASSERT(roles(0)==(ongoingReplay() && submittedCount==2 ? expected(0) : std::set<MeshPeriodDirection>{}));
            if (lostTrigger()) {
                auto recipient=check_and_cast<TablePowerObserver *>(power(1))->opening(meshMib(0)->address,MeshPeriodDirection::RECIPIENT);
                if (firstRecipient) ASSERT(*firstRecipient==recipient); else firstRecipient=recipient;
            }
            beforeAck++;
        }
        else if (kind==7) {
            auto observer=check_and_cast<TablePowerObserver *>(power(1));
            ASSERT(moreDataOnly() && recipientEnded && laterBeacons==0 && moreDataEndings==1);
            ASSERT(observer->bufferedHint(meshMib(0)->address) && !observer->retrievalRequested(meshMib(0)->address));
            ASSERT(roles(1).empty() && targetTriggerAir==0 && !observer->requiresAwake());
            ASSERT(check_and_cast<IRadio *>(meshInterface(1)->getSubmodule("radio"))->getRadioMode()==IRadio::RADIO_MODE_SLEEP);
            quietChecks++;
        }
        else if (kind==8) {
            ASSERT(activeWithdrawal() && activeWithdrawn==1 && activeConfirmed==0 && activeResponseAir==1);
            par("releaseEnding").setBoolValue(true);
            check_and_cast<IMeshPowerBuffer *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else {
            std::cout << "table scenario=" << scenario() << " preACK=" << beforeAck << " postACK=" << afterAck
                << " air=" << air << " confirmed=" << confirmed << " failed=" << failed << std::endl;
            int count=replay() ? 2 : 1;
            ASSERT(submitted && beforeAck==(lostTrigger() ? 2 : count) && afterAck==count
                && air==(lostTrigger() ? 2 : count) && payloadSamples==0 && failed==0);
            ASSERT(confirmed==2*int(expected(0).size())*(replay() && !ongoingReplay() ? 2 : 1)-int(activeWithdrawal())
                && roles(0).empty() && roles(1).empty());
            if (replay()) ASSERT(retry1Receipts==1);
            if (moreDataOnly()) ASSERT(moreDataEndings==1 && quietChecks==1 && targetTriggerAir==0);
            if (activeWithdrawal()) ASSERT(activeCommand && activeWithdrawn==1 && activeConfirmed==0 && activeNotices==1
                && activeResponseAir==1 && nestedCommand==(scenario()==36));
            if (scenario()==16 || scenario()==17) {
                ASSERT(endingAir==2 && cancelled==(scenario()==16 ? 1 : 0));
                ASSERT(lostAsymmetry==(scenario()==17 ? 1 : 0));
            }
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
Define_Module(ServicePeriodTableDriver);
#endif
