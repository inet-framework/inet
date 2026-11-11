// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHDOZEREADINESSSUPPORT_H
#define INET_TEST_MESHDOZEREADINESSSUPPORT_H

#include "mesh/MeshDozeSupport.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"

// These adapters expose existing retained-copy/sequence ownership. They do
// not replace radio readiness, MAC final-start admission or normal recovery.
class ManagedTxFixture : public Tx
{
  public:
    const Packet *pending() const { return !transmitting ? frame : nullptr; }
    simtime_t start() const { return admittedStart; }
};
Define_Module(ManagedTxFixture);

class ManagedHcfFixture : public Hcf
{
  public:
    int cancellations = 0;
    std::vector<Packet *> originals() const {
        if (!frameSequenceHandler->isSequenceRunning()) return {};
        auto step=dynamic_cast<TransmitStep *>(frameSequenceHandler->getContext()->getLastStep());
        if (!step) return {};
        if (auto aggregate=dynamic_cast<AggregateTransmitStep *>(step)) return aggregate->members;
        if (auto bar=dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(step->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>())) {
            std::vector<Packet *> result;
            for (int ac=0;ac<AC_NUMCATEGORIES;ac++) {
                auto frames=edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
                for (int i=0;i<frames->getLength();i++) {
                    auto packet=frames->getFrames(i);
                    auto data=dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
                    if (data && data->getType()==ST_DATA_WITH_QOS && data->getReceiverAddress()==bar->getReceiverAddress()
                            && data->getTid()==bar->getTidInfo()) result.push_back(packet);
                }
            }
            return result;
        }
        return {step->getFrameToTransmit()};
    }
    Packet *findOriginal(int64_t id) const {
        for (int ac=0;ac<AC_NUMCATEGORIES;ac++) {
            auto frames=edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
            for (int i=0;i<frames->getLength();i++) if (frames->getFrames(i)->getId()==id) return frames->getFrames(i);
        }
        return nullptr;
    }
    simtime_t ppduStart() const {
        ASSERT(frameSequenceHandler->isSequenceRunning());
        return check_and_cast<TransmitStep *>(frameSequenceHandler->getContext()->getLastStep())->getPpduStart();
    }
    simtime_t deadline(Packet *packet) const { return edca->getLifetimeHandler()->getDeadline(packet); }
    bool noRetry(Packet *packet) const {
        try {
            edca->getEdcaf(AC_BE)->getRecoveryProcedure()->getRetryCount(packet,packet->peekAtFront<Ieee80211DataHeader>());
        }
        catch (const cRuntimeError& error) {
            ASSERT(strstr(error.what(),"retry counter entry doesn't exist")); return true;
        }
        return false;
    }
    bool released() const { return !frameSequenceHandler->isSequenceRunning() && !edca->getChannelOwner(); }
    const FrameSequenceContext *sequenceContext() const { return frameSequenceHandler->getContext(); }
    bool waiting() const {
        return frameSequenceHandler->isSequenceRunning() && dynamic_cast<IReceiveStep *>(sequenceContext()->getLastStep())
            && startRxTimer->isScheduled();
    }
    simtime_t responseDeadline() const { ASSERT(startRxTimer->isScheduled()); return startRxTimer->getArrivalTime(); }
    int retries(Packet *packet) const {
        if (noRetry(packet)) return -1;
        return edca->getEdcaf(AC_BE)->getRecoveryProcedure()->getRetryCount(packet,packet->peekAtFront<Ieee80211DataHeader>());
    }
    std::string recovery() const { return edca->getEdcaf(AC_BE)->getRecoveryProcedure()->describeBlockAckRecoveries(); }
    void disposeNeverAir(MacAddress peer) {
        Enter_Method("disposeNeverAir");
        auto agreement=originatorBlockAckAgreementHandler ? originatorBlockAckAgreementHandler->getAgreement(peer,0) : nullptr;
        discardAgreementData(peer,0,false,agreement ? agreement->getHighestTransmittedDataIndex() : -1);
    }
    void transmissionCancelled(const Packet *packet) override {
        Enter_Method("transmissionCancelled");
        auto mode=packet->findTag<Ieee80211ModeReq>();
        bool target=std::string(packet->getName())=="managed-target" || (mode && mode->getTxVector().ampdu)
            || dynamicPtrCast<const Ieee80211BlockAckReq>(packet->peekAtFront<Ieee80211MacHeader>())!=nullptr;
        Hcf::transmissionCancelled(packet);
        if (target) { cancellations++; emit(registerSignal("managedCancelled"),const_cast<Packet *>(packet)); }
    }
};
Define_Module(ManagedHcfFixture);

class ManagedMacFixture : public Ieee80211MeshMac
{
  public:
    bool armed = false, holdTarget = false, armBar = false;
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t time) const override {
        if (holdTarget && std::string(packet->getName()).find("managed-target")==0) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,time);
    }
    void requestTransmissionReadiness() override {
        Ieee80211MeshMac::requestTransmissionReadiness();
        if (!armed) return;
        auto pending=check_and_cast<ManagedTxFixture *>(getSubmodule("tx"))->pending();
        if (!pending) return;
        auto mode=pending->findTag<Ieee80211ModeReq>();
        bool aggregate=mode && mode->getTxVector().ampdu;
        bool match=armBar ? !aggregate && dynamicPtrCast<const Ieee80211BlockAckReq>(pending->peekAtFront<Ieee80211MacHeader>())!=nullptr :
            std::string(pending->getName()).find("managed-target")==0 || aggregate;
        if (match) {
            armed=false;
            emit(registerSignal("managedPending"),const_cast<Packet *>(pending));
        }
    }
};
Define_Module(ManagedMacFixture);

class ManagedDriver : public cSimpleModule, public cListener, public LifecycleController
{
    struct Original {
        int64_t id;
        int sequence;
        simtime_t creation, deadline;
        std::vector<uint8_t> bytes;
        int retries;
    };
    std::vector<Original> originals;
    int actual=0, delivered=0, samples=0, failures=0, cancelled=0, terminalFacts=0, freshPreqs=0, trainingPpdus=0, bars=0, recipientChecks=0;
    std::string initialRecovery;
    double heldN=0, heldError=0;
    bool captured=false, checked=false, replacementQueued=false;
    ManagedMacFixture *mac() const { return check_and_cast<ManagedMacFixture *>(meshInterface(0)->getSubmodule("mac")); }
    ManagedHcfFixture *hcf() const { return check_and_cast<ManagedHcfFixture *>(mac()->getSubmodule("hcf")); }
    ManagedTxFixture *tx() const { return check_and_cast<ManagedTxFixture *>(mac()->getSubmodule("tx")); }
    IRadio *radio() const { return check_and_cast<IRadio *>(meshInterface(0)->getSubmodule("radio")); }
    int scenario() const { return par("scenario"); }
    void at(simtime_t time,int kind,short priority=0) {
        auto event=new cMessage("managed-check",kind); event->setSchedulingPriority(priority); scheduleAt(time,event);
    }
    void source(const char *name,int extra=0) {
        check_and_cast<IMeshForwarding *>(mac()->getSubmodule("ds"))->processSourceMsdu(phyMsdu(name,meshMib(1)->address,extra));
    }
    void remoteMode(bool active) {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_QOS_NULL); header->setReceiverAddress(meshMib(0)->address);
        header->setTransmitterAddress(meshMib(1)->address); header->setPowerMgmt(!active); header->setMeshPowerSaveLevel(!active);
        // A committed receive-owner fact injected between accepted IFS and
        // actual start. This is not claimed as an RF Null exchange.
        check_and_cast<IMeshPowerSave *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->receivePowerFields(nullptr,header);
    }
  protected:
    void initialize() override {
        auto network=getSimulation()->getSystemModule();
        for (auto signal : {registerSignal("managedPending"),registerSignal("managedCancelled"),
                transmissionStartedSignal,transmissionEndedSignal,packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal,
                Hcf::meshTerminalRecoverySignal}) network->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(50,SIMTIME_MS),1);
        at(SimTime(200,SIMTIME_MS),2); at(SimTime(215,SIMTIME_MS),5);
        at(SimTime(220,SIMTIME_MS),6); at(SimTime(240,SIMTIME_MS),7);
        at(SimTime(350,SIMTIME_MS),8);
    }
    void receiveSignal(cComponent *sourceModule,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==Hcf::meshTerminalRecoverySignal) { terminalFacts++; return; }
        if (signal==registerSignal("managedPending") && sourceModule==mac()) {
            ASSERT(!captured); captured=true; at(simTime(),3,-100); return;
        }
        if (signal==registerSignal("managedCancelled") && sourceModule==hcf()) {
            cancelled++;
            ASSERT(!tx()->hasTransmission()); // Registration detached before callbacks.
            if (scenario()==3) {
                source("managed-replacement"); replacementQueued=true;
            }
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal && sourceModule==hcf()) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && std::string(packet->getName()).find("managed-")==0) {
                samples++; failures+=!check_and_cast<MeshDataAttempt *>(details)->acknowledged;
            }
            return;
        }
        if (signal==packetSentToUpperSignal && sourceModule==meshInterface(1)->getSubmodule("mac")) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && std::string(packet->getName()).find("managed-")==0) delivered++;
            return;
        }
        if (signal==transmissionEndedSignal && sourceModule==meshInterface(0)->getSubmodule("radio")
                && (checked || (scenario()==3 && replacementQueued))) {
            auto transmission=check_and_cast<const Ieee80211Transmission *>(object);
            auto packet=transmission->getPacket();
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            bool target=std::string(packet->getName()).find("managed-")==0 || transmission->getTxVector().ampdu;
            if (!target && !transmission->getTxVector().ampdu)
                target=dynamicPtrCast<const Ieee80211BlockAckReq>(packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength()))!=nullptr;
            if (target) {
                ASSERT(hcf()->waiting() && !tx()->hasTransmission());
                auto context=hcf()->sequenceContext(); auto step=context->getLastStep();
                auto deadline=hcf()->responseDeadline();
                Packet response("detached-unrelated-recipient",makeShared<Ieee80211AckFrame>());
                hcf()->transmissionCancelled(&response); // Owner injection, no cancelled RF response is claimed.
                ASSERT(hcf()->sequenceContext()==context && context->getLastStep()==step && hcf()->waiting());
                ASSERT(hcf()->responseDeadline()==deadline);
                recipientChecks++;
            }
            return;
        }
        if (signal==transmissionStartedSignal && sourceModule==meshInterface(0)->getSubmodule("radio")) {
            auto transmission=check_and_cast<const Ieee80211Transmission *>(object);
            auto packet=transmission->getPacket();
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            bool aggregate=transmission->getTxVector().ampdu;
            if ((scenario()==10 || scenario()==15) && aggregate && simTime()<SimTime(205,SIMTIME_MS)) { trainingPpdus++; return; }
            if (scenario()==15 && !aggregate && dynamicPtrCast<const Ieee80211BlockAckReq>(
                    packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength()))) {
                ASSERT(checked); bars++;
            }
            if (scenario()==7 && simTime()>=SimTime(240,SIMTIME_MS) && !aggregate) {
                auto header=packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
                auto action=dynamicPtrCast<const Ieee80211MeshActionFrame>(header);
                if (action && action->getCategory()==MESH_ACTION && action->getMeshAction()==MESH_HWMP_PATH_SELECTION)
                    freshPreqs+=packet->peekDataAt<Ieee80211HwmpFrame>(phy->getChunkLength()+header->getChunkLength())->getPreqsArraySize();
            }
            if (!aggregate && std::string(packet->getName()).find("managed-")!=0) return;
            ASSERT(checked || (scenario()==3 && replacementQueued) || scenario()==15);
            if (aggregate) {
                Ieee80211AmpduLayout layout;
                ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),false,layout));
                if (scenario()==15) { ASSERT(layout.members.size()==3); actual++; return; }
                ASSERT(layout.members.size()==originals.size() && originals.size()==3);
                for (size_t i=0;i<layout.members.size();i++) {
                    auto header=packet->peekDataAt<Ieee80211DataHeader>(phy->getChunkLength()+layout.members[i].offset);
                    ASSERT(header->getSequenceNumber().get()==originals[i].sequence && !header->getRetry());
                }
                actual++; return;
            }
            auto header=packet->peekDataAt<Ieee80211DataHeader>(phy->getChunkLength());
            if (std::string(packet->getName())=="managed-target") {
                ASSERT(originals.size()==1 && header->getSequenceNumber().get()==originals.front().sequence && !header->getRetry());
            }
            actual++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) source("prime-readiness");
        else if (kind==2) {
            if (scenario()==10 || scenario()==15) {
                mac()->holdTarget=true;
                source("agreement-primer");
                for (int i=0;i<3;i++) source(("managed-target"+std::to_string(i)).c_str());
                at(simTime()+SimTime(5,SIMTIME_MS),9);
            }
            else {
                mac()->armed=true;
                if (scenario()==11) source("readiness-first");
                source("managed-target",scenario()==11 ? 200 : 0);
            }
        }
        else if (kind==3) {
            ASSERT(tx()->pending() && hcf()->ppduStart()<0 && actual==(scenario()==15 ? 1 : 0));
            for (auto packet : hcf()->originals()) {
                auto header=packet->peekAtFront<Ieee80211DataHeader>();
                ASSERT(!header->getRetry() && hcf()->retries(packet)==(scenario()==15 ? 1 : -1));
                originals.push_back({packet->getId(),header->getSequenceNumber().get(),packet->getCreationTime(),
                    hcf()->deadline(packet),packet->peekDataAt<BytesChunk>(b(0),packet->getDataLength()-B(4))->getBytes(),hcf()->retries(packet)});
            }
            ASSERT(!originals.empty());
            if (scenario()==11 || scenario()==15) {
                if (scenario()==11) ASSERT(tx()->start()-simTime()==SimTime(16,SIMTIME_US));
                if (scenario()==15) {
                    initialRecovery=hcf()->recovery(); ASSERT(initialRecovery.find("expirations=0")!=std::string::npos);
                    auto metric=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
                    heldN=metric->getPayloadEstimate(meshMib(1)->address); heldError=metric->getErrorEstimate(meshMib(1)->address);
                    ASSERT(heldN==1.25 && heldError==169.0/512);
                }
                remoteMode(false);
            }
            else {
                mac()->holdTarget=true;
                radio()->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            }
            if (scenario()==9 || scenario()==10) {
                hcf()->disposeNeverAir(meshMib(1)->address);
                for (const auto& original : originals) ASSERT(hcf()->findOriginal(original.id));
            }
            if (scenario()>=4 && scenario()<=7) at(simTime()+SimTime(10,SIMTIME_US),4);
        }
        else if (kind==4) {
            auto policy=meshInterface(0)->getSubmodule("powerSavePolicy");
            if (scenario()==6) policy->deleteModule();
            else if (scenario()==7) meshManagement(0)->closePeer(meshMib(1)->address);
            else {
                auto operation=scenario()==5 ? static_cast<LifecycleOperation *>(new ModuleCrashOperation())
                    : static_cast<LifecycleOperation *>(new ModuleStopOperation());
                LifecycleOperation::StringMap parameters; operation->initialize(policy,parameters); ASSERT(initiateOperation(operation));
            }
        }
        else if (kind==5) {
            ASSERT(captured && actual==(scenario()==3 || scenario()==15 ? 1 : 0)
                && samples==(scenario()==15 ? 3 : scenario()==3 ? 1 : 0)
                && failures==(scenario()==15 ? 3 : 0) && !tx()->hasTransmission());
            ASSERT(hcf()->released());
            auto state=radio()->getRadioModeState();
            ASSERT(state && !state->transition && state->mode==IRadio::RADIO_MODE_TRANSCEIVER);
            for (const auto& saved : originals) {
                auto packet=hcf()->findOriginal(saved.id);
                if (scenario()==7 || scenario()==8) ASSERT(!packet);
                else {
                    ASSERT(packet && packet->getCreationTime()==saved.creation && hcf()->deadline(packet)==saved.deadline);
                    ASSERT(packet->peekDataAt<BytesChunk>(b(0),packet->getDataLength()-B(4))->getBytes()==saved.bytes
                        && hcf()->retries(packet)==saved.retries);
                }
            }
            if (scenario()!=7 && scenario()!=8) ASSERT(cancelled==1);
            if (scenario()==15) {
                ASSERT(bars==0 && hcf()->recovery()==initialRecovery && delivered==3);
                auto metric=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
                ASSERT(metric->getPayloadEstimate(meshMib(1)->address)==heldN && metric->getErrorEstimate(meshMib(1)->address)==heldError);
            }
            checked=true;
        }
        else if (kind==6) {
            if (scenario()==4 || scenario()==5) {
                auto operation=new ModuleStartOperation(); LifecycleOperation::StringMap parameters;
                operation->initialize(meshInterface(0)->getSubmodule("powerSavePolicy"),parameters); ASSERT(initiateOperation(operation));
            }
            if (scenario()==7) meshManagement(0)->openPeer(meshMib(1)->address);
            if (scenario()==11 || scenario()==15) remoteMode(true);
            mac()->holdTarget=false; hcf()->meshPowerEligibilityChanged();
        }
        else if (kind==7 && (scenario()==7 || scenario()==8)) source("managed-after-retirement");
        else if (kind==9) {
            mac()->armed=true; mac()->armBar=scenario()==15; mac()->holdTarget=false; hcf()->meshPowerEligibilityChanged();
        }
        else if (kind==8) {
            int expected=scenario()==3 ? 2 : 1;
            int expectedMembers=scenario()==10 || scenario()==15 ? 3 : expected;
            std::cout << "managed final scenario=" << scenario() << " checked=" << checked << " actual=" << actual
                << " delivered=" << delivered << " samples=" << samples << " failures=" << failures
                << " cancelled=" << cancelled << " peer=" << meshManagement(0)->getPeerState(meshMib(1)->address) << std::endl;
            ASSERT(checked && actual==expected && delivered==expectedMembers && samples==expectedMembers
                && failures==(scenario()==15 ? 3 : 0) && terminalFacts==0);
            ASSERT(recipientChecks==expected);
            if (scenario()==7) ASSERT(freshPreqs>0);
            if (scenario()==10 || scenario()==15) ASSERT(trainingPpdus==1);
            if (scenario()==15) {
                ASSERT(bars==1 && hcf()->recovery().empty());
                for (const auto& original : originals) ASSERT(!hcf()->findOriginal(original.id));
                auto metric=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
                ASSERT(metric->getPayloadEstimate(meshMib(1)->address)==1 && metric->getErrorEstimate(meshMib(1)->address)==heldError);
            }
            emit(registerSignal("dozeComplete"),1L);
        }
    }
};
Define_Module(ManagedDriver);

class NaturalPolicyDriver : public cSimpleModule, public cListener, public LifecycleController
{
    int actual=0, delivered=0, samples=0;
    IRadio *radio(int i) const { return check_and_cast<IRadio *>(meshInterface(i)->getSubmodule("radio")); }
    DozePowerFixture *power(int i) const { return check_and_cast<DozePowerFixture *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    void at(int ms,int kind) { scheduleAt(SimTime(ms,SIMTIME_MS),new cMessage("natural-policy-check",kind)); }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(1,0); at(60,1); at(65,2); at(130,3); at(150,4); at(151,5);
        at(160,6); at(165,7); at(180,8); at(300,9);
    }
    void receiveSignal(cComponent *sourceModule,simsignal_t signal,cObject *object,cObject *details) override {
        if (signal==transmissionStartedSignal) {
            auto transmission=dynamic_cast<const Ieee80211Transmission *>(object);
            if (transmission && std::string(transmission->getPacket()->getName())=="natural-after-policy") {
                ASSERT(radio(0)->getRadioMode()==IRadio::RADIO_MODE_TRANSCEIVER && radio(1)->getRadioMode()==IRadio::RADIO_MODE_TRANSCEIVER);
                actual++;
            }
            return;
        }
        auto packet=dynamic_cast<Packet *>(object);
        if (!packet || std::string(packet->getName())!="natural-after-policy") return;
        if (signal==packetSentToUpperSignal && sourceModule==meshInterface(1)->getSubmodule("mac")) delivered++;
        if (signal==Hcf::meshDataAttemptCompletedSignal) {
            ASSERT(check_and_cast<MeshDataAttempt *>(details)->acknowledged); samples++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        int scenario=par("scenario");
        if (kind==0) {
            meshManagement(0)->openPeer(meshMib(1)->address);
            meshManagement(1)->openPeer(meshMib(0)->address);
        }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if (kind==3) {
            for (int i=0;i<2;i++) {
                ASSERT(meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address)->local==MeshPowerMode::LIGHT_SLEEP);
                ASSERT(radio(i)->getRadioMode()==IRadio::RADIO_MODE_SLEEP);
            }
        }
        else if (kind==4) {
            ASSERT(radio(0)->getRadioMode()==IRadio::RADIO_MODE_SLEEP);
            if (scenario==14) power(0)->deleteModule();
            else {
                auto operation=scenario==13 ? static_cast<LifecycleOperation *>(new ModuleCrashOperation())
                    : static_cast<LifecycleOperation *>(new ModuleStopOperation());
                LifecycleOperation::StringMap parameters; operation->initialize(power(0),parameters); ASSERT(initiateOperation(operation));
            }
        }
        else if (kind==5) {
            auto state=radio(0)->getRadioModeState();
            ASSERT(state && !state->transition && state->mode==IRadio::RADIO_MODE_TRANSCEIVER);
            ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local==MeshPowerMode::LIGHT_SLEEP);
            ASSERT(actual==0 && samples==0);
        }
        else if (kind==6) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::ACTIVE);
        else if (kind==7 && scenario!=14) {
            auto operation=new ModuleStartOperation(); LifecycleOperation::StringMap parameters;
            operation->initialize(power(0),parameters); ASSERT(initiateOperation(operation));
            power(0)->requestMode(meshMib(1)->address,MeshPowerMode::ACTIVE);
        }
        else if (kind==8) {
            ASSERT(radio(0)->getRadioMode()==IRadio::RADIO_MODE_TRANSCEIVER && radio(1)->getRadioMode()==IRadio::RADIO_MODE_TRANSCEIVER);
            check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("natural-after-policy",meshMib(1)->address));
        }
        else if (kind==9) {
            ASSERT(actual==1 && delivered==1 && samples==1);
            emit(registerSignal("dozeComplete"),1L);
        }
    }
};
Define_Module(NaturalPolicyDriver);

#endif
