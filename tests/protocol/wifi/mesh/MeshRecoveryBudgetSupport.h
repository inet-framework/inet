// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHRECOVERYBUDGETSUPPORT_H
#define INET_TEST_MESHRECOVERYBUDGETSUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPowerBuffer.h"

// Diagnostic-only instances reject before allocating their boundary timer.
// They never own protocol work; skip the full initialized-policy withdrawal
// path on test disposal. This is not an invalid-module lifecycle claim.
class InvalidPeriodLimitProbe : public MeshPowerSave
{
  public:
    void preDelete(cComponent *root) override { OperationalBase::preDelete(root); }
};
namespace invalidPeriodLimitRegistration { Define_Module(InvalidPeriodLimitProbe); }

// Observation only: all decisions/debits remain with MeshPowerSave. No period,
// acknowledgment, counter, agreement, opportunity or deadline is installed.
class BudgetPowerObserver : public MeshPowerSave
{
    mutable int refusedPairs = 0;
    mutable std::set<int64_t> refusedIds;
    std::map<int64_t,int> authorizedStarts;
    std::vector<int64_t> lastIds;

    static bool sameViews(const std::vector<MeshServicePeriodView>& a,
            const std::vector<MeshServicePeriodView>& b) {
        if (a.size()!=b.size()) return false;
        for (size_t i=0;i<a.size();i++)
            if (a[i].peer!=b[i].peer || a[i].direction!=b[i].direction || a[i].ending!=b[i].ending
                    || a[i].cap!=b[i].cap || a[i].spent!=b[i].spent || a[i].cohort!=b[i].cohort)
                return false;
        return true;
    }
    std::vector<MeshBufferedFrame> copiedBuffers() const {
        auto hcf=getParentModule()->getSubmodule("mac")->getSubmodule("hcf");
        return check_and_cast<IMeshPowerBuffer *>(hcf)->getMeshBufferedFrames();
    }
    static bool sameBuffers(const std::vector<MeshBufferedFrame>& a,
            const std::vector<MeshBufferedFrame>& b) {
        if (a.size()!=b.size()) return false;
        for (const auto& first : a) {
            auto second=std::find_if(b.begin(),b.end(),[&](const MeshBufferedFrame& x) { return x.id==first.id; });
            if (second==b.end() || first.receiver!=second->receiver || first.length!=second->length
                    || first.deadline!=second->deadline || first.accessCategory!=second->accessCategory || first.kind!=second->kind)
                return false;
        }
        return true;
    }
  public:
    int refusedPairCount() const { return refusedPairs; }
    const std::set<int64_t>& refusedOriginals() const { return refusedIds; }
    const std::vector<int64_t>& lastAuthorizedOriginals() const { return lastIds; }
    int starts(int64_t id) const { auto it=authorizedStarts.find(id); return it==authorizedStarts.end() ? 0 : it->second; }
    int64_t requestedControl(MacAddress peer) const {
        auto it=transitions.find(peer); return it==transitions.end() ? -1 : it->second.originalId;
    }
    int64_t ownerOpening(MacAddress peer) const {
        auto period=findPeriod(peer,MeshPeriodDirection::OWNER); ASSERT(period); return period->openingId;
    }
    bool canTransmitSet(const std::vector<MeshPowerTxMember>& members,simtime_t start) const override {
        auto before=getServicePeriods();
        bool pair=false;
        std::set<int64_t> ids;
        if (getContainingNode(this)->getIndex()==0 && members.size()==2)
            for (const auto& view : before)
                if (view.direction==MeshPeriodDirection::OWNER && view.ending && view.cap==1 && view.spent==0
                        && members[0].peer==view.peer && members[1].peer==view.peer
                        && members[0].eosp && members[1].eosp
                        && view.cohort.count(members[0].id) && view.cohort.count(members[1].id)
                        && starts(members[0].id)==1 && starts(members[1].id)==1) {
                    pair=true; ids={members[0].id,members[1].id};
                }
        auto buffers=pair ? copiedBuffers() : std::vector<MeshBufferedFrame>();
        bool allowed=MeshPowerSave::canTransmitSet(members,start);
        if (pair) {
            ASSERT(!allowed); // Two already-attempted originals need two slots; only one remains.
            ASSERT(sameViews(before,getServicePeriods()) && sameBuffers(buffers,copiedBuffers()));
            // Repeat the same read-only owner query: it must not consume a slot or identity.
            ASSERT(!MeshPowerSave::canTransmitSet(members,start));
            ASSERT(sameViews(before,getServicePeriods()) && sameBuffers(buffers,copiedBuffers()));
            refusedPairs++; refusedIds=ids;
        }
        return allowed;
    }
    bool authorizeTransmissionStart(const std::vector<MeshPowerTxMember>& members,simtime_t start) override {
        Enter_Method("authorizeTransmissionStart");
        bool allowed=MeshPowerSave::authorizeTransmissionStart(members,start);
        if (allowed) {
            lastIds.clear();
            for (const auto& member : members) { lastIds.push_back(member.id); authorizedStarts[member.id]++; }
        }
        return allowed;
    }
    void originalAcknowledged(const Packet *packet) override {
        MeshPowerSave::originalAcknowledged(packet);
        if (std::string(packet->getName())=="budget-opening")
            emit(registerSignal("budgetOpeningAck"),const_cast<Packet *>(packet));
    }
};
Define_Module(BudgetPowerObserver);

// A bounded fixture eligibility hold after the proof's first confirmation.
// This prevents the unmatched original beginning a separate later period while
// the test observes its retained source identity. It does not alter the PPDU
// already completed, the ACK/BA, the allowance or the natural preview refusal.
class BudgetMacFixture : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName())=="mesh-period-ending"
                && (getSimulation()->getSystemModule()->getSubmodule("driver")->par("budgetCase").intValue()==6
                    || getSimulation()->getSystemModule()->getSubmodule("driver")->par("budgetCase").intValue()==8))
            return false; // Exercise its actual five-second queued control lifetime.
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName())=="mesh-period-ending"
                && getSimulation()->getSystemModule()->getSubmodule("driver")->par("budgetCase").intValue()==7
                && getSimulation()->getSystemModule()->getSubmodule("driver")->par("holdRemainder").boolValue())
            return false; // Retain the failed initial on-air original for one controlled collision callback.
        if (getContainingNode(this)->getIndex()==1 && std::string(packet->getName())=="mesh-period-ending"
                && getSimulation()->getSystemModule()->getSubmodule("driver")->par("budgetCase").intValue()==1)
            return false; // Isolate genuine BA expiry from the independent reverse ending.
        if (getContainingNode(this)->getIndex()==0
                && getSimulation()->getSystemModule()->getSubmodule("driver")->par("holdRemainder").boolValue()
                && std::string(packet->getName()).find("service-payload")==0) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
Define_Module(BudgetMacFixture);

class RecoveryBudgetDriver : public ServicePeriodDriver
{
    bool triggerArmed=false, submitted=false, proofScheduled=false;
    int endingAir=0, retryPpdus=0, targetConfirmed=0, targetFailed=0;
    int64_t endingTree=-1, retryId=-1, lateId=-1;
    std::set<int64_t> cohort;
    std::set<int64_t> allowedControl;
    std::map<int64_t,int> sequence;
    int terminalFacts=0, invalidLimits=0;
    int collisionDrops=0, collisionLinks=0;
    int modeOldAir=0, modeReplacementAir=0;
    int budgetCase() const { return par("budgetCase"); }
    bool scalar() const { return budgetCase()!=1; }
    bool expectFailure() const { return (budgetCase()>=2 && budgetCase()<=4) || budgetCase()>=6; }
    int expectedAir() const { return budgetCase()==6 ? 0 : budgetCase()==2 || budgetCase()==5 || budgetCase()>=7 ? 1 : 2; }
    int expectedCap() const { return budgetCase()==2 ? 0 : budgetCase()>=4 ? 2 : 1; }
    BudgetPowerObserver *observer() const { return check_and_cast<BudgetPowerObserver *>(power(0)); }
    MeshServicePeriodView owner() const {
        for (const auto& view : power(0)->getServicePeriods())
            if (view.peer==meshMib(1)->address && view.direction==MeshPeriodDirection::OWNER) return view;
        throw cRuntimeError("Expected live source OWNER during actual ending start");
    }
    void submitNull() {
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
    void observeEnding(const Ieee80211Transmission *transmission) {
        auto packet=transmission->getPacket();
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        if (scalar()) {
            bool mode=budgetCase()==8 && std::string(packet->getName())=="mesh-power-mode" && simTime()>SimTime(150,SIMTIME_MS);
            if (mode && modeOldAir) { modeReplacementAir++; return; }
            if (std::string(packet->getName())!="mesh-period-ending" && !mode) return;
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
            ASSERT(header->getType()==ST_QOS_NULL && header->getEosp() && !header->getRspi());
            auto view=owner();
            ASSERT(view.ending && view.cap==expectedCap() && view.spent==endingAir && view.cohort.size()==1);
            ASSERT(targetFailed==0 && endingAir<expectedAir() && header->getRetry()==(endingAir!=0));
            if (endingAir==0) { cohort=view.cohort; endingTree=packet->getTreeId(); }
            ASSERT(view.cohort==cohort && packet->getTreeId()==endingTree);
            endingAir++;
            if (mode) {
                ASSERT(header->getPowerMgmt() && header->getMeshPowerSaveLevel());
                modeOldAir++;
                for (const auto& frame : check_and_cast<IMeshPowerBuffer *>(originator())->getMeshBufferedFrames())
                    if (frame.kind==MeshBufferedFrame::Kind::POWER_CONTROL) allowedControl.insert(frame.id);
                power(0)->requestMode(meshMib(1)->address,MeshPowerMode::ACTIVE);
                auto replacement=observer()->requestedControl(meshMib(1)->address);
                ASSERT(replacement!=-1); allowedControl.insert(replacement);
                ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local==MeshPowerMode::ACTIVE);
            }
            if (budgetCase()==7 && endingAir==1) {
                par("holdRemainder").setBoolValue(true);
                at(simTime()+SimTime(2,SIMTIME_MS),15);
            }
            if (endingAir==1) emit(registerSignal("budgetEndingStarted"),1L);
            return;
        }
        if (!transmission->getTxVector().ampdu) return;
        Ieee80211AmpduLayout layout;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),false,layout));
        auto view=owner();
        auto ids=observer()->lastAuthorizedOriginals();
        ASSERT(ids.size()==layout.members.size() && view.ending && view.cap==1);
        if (cohort.empty()) {
            // Fixture reachability precondition: the established agreement has
            // one real initial two-member ending PPDU, not an injected retry set.
            ASSERT(layout.members.size()==2 && view.spent==0 && view.cohort.size()==2);
            cohort=view.cohort;
            for (size_t i=0;i<ids.size();i++) {
                auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+layout.members[i].offset);
                ASSERT(header->getEosp() && !header->getRspi() && !header->getRetry());
                ASSERT(cohort.count(ids[i]) && observer()->starts(ids[i])==1);
                sequence[ids[i]]=header->getSequenceNumber().get();
            }
            // Insert a new MSDU after the actual ending PPDU has frozen its
            // cohort, while its BA outcome is still unresolved.
            at(simTime()+SimTime::fromRaw(1),14);
        }
        else {
            ASSERT(view.cohort==cohort && view.spent==1 && targetFailed==0);
            ASSERT(layout.members.size()==1 && observer()->refusedPairCount()>0);
            ASSERT(observer()->refusedOriginals()==cohort && cohort.count(ids.front()));
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+layout.members.front().offset);
            ASSERT(header->getEosp() && header->getRetry() && !header->getRspi());
            ASSERT(header->getSequenceNumber().get()==sequence.at(ids.front()));
            ASSERT(observer()->starts(ids.front())==2);
            retryId=ids.front(); retryPpdus++; ASSERT(retryPpdus==1);
        }
        endingAir++;
    }
  protected:
    void initialize() override {
        ServicePeriodDriver::initialize();
        getSimulation()->getSystemModule()->subscribe(Hcf::meshTerminalRecoverySignal,this);
        getSimulation()->getSystemModule()->subscribe(packetDroppedSignal,this);
        getSimulation()->getSystemModule()->subscribe(linkBrokenSignal,this);
        getSimulation()->getSystemModule()->subscribe(registerSignal("budgetOpeningAck"),this);
        if (budgetCase()==5) at(SimTime(2,SIMTIME_MS),30);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
        if (budgetCase()==8 && signal==registerSignal("budgetOpeningAck") && source==power(0)) {
            ASSERT(owner().cohort.empty() && !owner().ending);
            power(0)->requestMode(meshMib(1)->address,MeshPowerMode::DEEP_SLEEP);
            return;
        }
        if (budgetCase()==7 && source==originator() && (signal==packetDroppedSignal || signal==linkBrokenSignal)) {
            auto packet=check_and_cast<Packet *>(object);
            if (cohort.count(packet->getId())) {
                if (signal==packetDroppedSignal) collisionDrops++; else collisionLinks++;
            }
            return;
        }
        if (signal==Hcf::meshTerminalRecoverySignal) { terminalFacts++; return; }
        if (signal==transmissionStartedSignal && from(source,"radio")==0)
            observeEnding(check_and_cast<const Ieee80211Transmission *>(object));
        if (scalar() && triggerArmed && !submitted && signal==receptionEndedSignal && from(source,"radio")==0) {
            auto packet=dynamic_cast<const Packet *>(details);
            if (packet && packet->peekAtFront<Ieee80211MacHeader>()->getType()==ST_BEACON && simTime()>SimTime(150,SIMTIME_MS)) {
                submitted=true; at(simTime()+SimTime::fromRaw(1),10);
            }
        }
        if (signal!=MeshPowerSave::meshServicePeriodOutcomeSignal || from(source,"powerSavePolicy")!=0 || cohort.empty()) return;
        auto outcome=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
        if (outcome->peer!=meshMib(1)->address || outcome->direction!=MeshPeriodDirection::OWNER) return;
        if (outcome->outcome==MeshPeriodOutcome::FAILED) {
            targetFailed++; ASSERT(expectFailure() && targetFailed==1 && targetConfirmed==0 && endingAir==expectedAir());
            if (budgetCase()==7) {
                ASSERT(collisionDrops==1 && collisionLinks==1);
                meshManagement(0)->closePeer(meshMib(1)->address);
            }
            ASSERT(!proofScheduled); proofScheduled=true; at(simTime()+SimTime::fromRaw(1),12);
        }
        if (outcome->outcome==MeshPeriodOutcome::CONFIRMED) {
            ASSERT(!expectFailure() && targetFailed==0 && endingAir==expectedAir());
            targetConfirmed++; ASSERT(targetConfirmed==1);
            if (budgetCase()==1) par("holdRemainder").setBoolValue(true);
            ASSERT(!proofScheduled); proofScheduled=true;
            at(simTime()+SimTime::fromRaw(1),12); // Let all ordinary ACK/drop siblings finish first.
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind==3 && scalar()) {
            delete event;
            for (int i=0;i<2;i++) {
                auto state=meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address);
                ASSERT(state && state->local==MeshPowerMode::LIGHT_SLEEP && state->remote==MeshPowerMode::LIGHT_SLEEP);
            }
            triggerArmed=true; return;
        }
        if (kind==10) {
            delete event; submitNull();
            if (budgetCase()==6) at(simTime()+SimTime(2,SIMTIME_MS),11);
            return;
        }
        if (kind==11) {
            delete event; ASSERT(budgetCase()==6 && owner().cohort.empty() && !owner().ending && endingAir==0);
            int controls=0;
            for (const auto& frame : check_and_cast<IMeshPowerBuffer *>(originator())->getMeshBufferedFrames())
                if (frame.receiver==meshMib(1)->address && frame.kind==MeshBufferedFrame::Kind::POWER_CONTROL) {
                    ASSERT(frame.deadline>simTime()+SimTime(4,SIMTIME_S) && frame.deadline<=simTime()+SimTime(5,SIMTIME_S));
                    cohort.insert(frame.id); controls++;
                }
            ASSERT(controls==1); emit(registerSignal("budgetControlHeld"),1L); return;
        }
        if (kind==30) {
            delete event;
            for (int limit : {-1,101}) {
                auto type=cModuleType::get("InvalidPeriodLimitProbe");
                auto probe=type->create("invalidLimitProbe",meshInterface(0));
                probe->par("missingAckRetryLimit").setIntValue(limit);
                probe->finalizeParameters(); probe->buildInside();
                bool rejected=false;
                try { probe->callInitialize(INITSTAGE_LOCAL); }
                catch (const cRuntimeError& error) { rejected=std::string(error.what()).find("missingAckRetryLimit")!=std::string::npos; }
                ASSERT(rejected); invalidLimits++; probe->deleteModule();
            }
            return;
        }
        if (kind==12) {
            delete event;
            ASSERT(targetConfirmed==int(!expectFailure()) && targetFailed==int(expectFailure()) && endingAir==expectedAir());
            verifySampleOrder();
            if (scalar()) {
                ASSERT(submitted && retryPpdus==0 && samples==0 && terminalFacts==0);
                if (budgetCase()==5) ASSERT(invalidLimits==2);
                if (expectFailure()) {
                    for (const auto& period : power(0)->getServicePeriods()) ASSERT(period.direction!=MeshPeriodDirection::OWNER);
                    at(simTime()+SimTime(2,SIMTIME_MS),13); return;
                }
            }
            else {
                ASSERT(retryPpdus==1 && observer()->refusedPairCount()>0 && failures==2);
                ASSERT(payloadAir==4 && samples==4 && delivered==2);
                int retained=0;
                auto inventory=check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->getMeshBufferedFrames();
                for (auto id : cohort) if (id!=retryId) {
                    ASSERT(observer()->starts(id)==1);
                    for (const auto& original : inventory) retained+=original.id==id && original.deadline>simTime();
                }
                ASSERT(retained==1);
                int lateRetained=0;
                for (const auto& original : inventory)
                    lateRetained+=original.id==lateId && original.deadline>simTime();
                ASSERT(lateId!=-1 && !cohort.count(lateId) && lateRetained==1 && observer()->starts(lateId)==0);
            }
            std::cout << "recovery budget case=" << budgetCase() << " confirmed=" << targetConfirmed
                << " failed=" << targetFailed << " endingAir=" << endingAir
                << " refusedPairs=" << observer()->refusedPairCount() << std::endl;
            emit(registerSignal("budgetComplete"),1L);
            return;
        }
        if (kind==13) {
            delete event;
            ASSERT(expectFailure() && endingAir==expectedAir() && targetFailed==1 && targetConfirmed==0 && terminalFacts==0 && samples==0);
            for (const auto& frame : check_and_cast<IMeshPowerBuffer *>(originator())->getMeshBufferedFrames())
                if (budgetCase()==8 && frame.kind==MeshBufferedFrame::Kind::POWER_CONTROL)
                    ASSERT(!cohort.count(frame.id) && allowedControl.count(frame.id));
                else ASSERT(!cohort.count(frame.id));
            if (budgetCase()==8) ASSERT(modeOldAir==1 && modeReplacementAir==1
                && meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local==MeshPowerMode::ACTIVE);
            std::cout << "ending control failed case=" << budgetCase() << " attempts=" << endingAir
                << " no extra control/PERR/data sample" << std::endl;
            emit(registerSignal("budgetComplete"),1L); return;
        }
        if (kind==14) {
            delete event;
            ASSERT(budgetCase()==1 && endingAir==1 && lateId==-1);
            auto buffer=check_and_cast<IMeshPowerBuffer *>(originator());
            std::set<int64_t> before;
            for (const auto& frame : buffer->getMeshBufferedFrames()) before.insert(frame.id);
            auto forwarding=check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"));
            forwarding->processSourceMsdu(phyMsdu("service-payload-late",meshMib(1)->address));
            for (const auto& frame : buffer->getMeshBufferedFrames())
                if (!before.count(frame.id) && frame.receiver==meshMib(1)->address && frame.kind==MeshBufferedFrame::Kind::PAYLOAD) {
                    ASSERT(lateId==-1); lateId=frame.id;
                }
            auto view=owner();
            ASSERT(lateId!=-1 && !cohort.count(lateId) && view.cohort==cohort && view.spent==0);
            return;
        }
        if (kind==15) {
            delete event;
            ASSERT(budgetCase()==7 && endingAir==1 && cohort.size()==1 && owner().spent==0);
            par("holdRemainder").setBoolValue(false); // Expose the retained original to HCF's collision selection.
            check_and_cast<RecoveryHcfFixture *>(originator())->collideEndingRetry(*cohort.begin());
            ASSERT(targetFailed==1 && collisionDrops==1 && collisionLinks==1);
            return;
        }
        if (kind==4) {
            delete event;
            if (budgetCase()==6) { ASSERT(targetFailed==0 && endingAir==0); return; }
            throw cRuntimeError("Budget draft proof did not reach confirmation before its bounded final check");
        }
        ServicePeriodDriver::handleMessage(event);
    }
};
Define_Module(RecoveryBudgetDriver);

namespace inet::protocoltest {
Define_ProtocolTest(mesh_power_save_budget) {
    ProtocolTest test("mesh_power_save_budget");
    int which=getSimulation()->getSystemModule()->par("budgetCase");
    if (which==6) {
        test.once(on("driver").signal("budgetControlHeld").assertValue(1).within(0.35));
        test.once(on("driver").signal("budgetComplete").assertValue(1).within(6));
    }
    else if (which!=1) {
        test.once(on("driver").signal("budgetEndingStarted").assertValue(1).within(0.35));
        if (which==0) test.meanwhile(tap("radioMedium").from("station[1]").to("station[0]")
            .filterPacket("ieee80211mac.type == 29").nth(1).drop()
            .describe("lose first actual ending Normal ACK; last allowed retry ACK is untouched"));
        else if (which<5 || which==7) test.meanwhile(tap("radioMedium").from("station[1]").to("station[0]")
            .filterPacket("ieee80211mac.type == 29").fromNth(1).drop()
            .describe("all ending control ACKs are lost until its exact mesh or ordinary limit"));
        test.once(on("driver").signal("budgetComplete").assertValue(1).within(0.6));
    }
    else {
        auto corrupted=std::make_shared<bool>(false);
        test.intercept(tap("radioMedium").from("station[0]").to("station[1]")
            .mutate([corrupted](Packet *packet) {
                if (*corrupted) return;
                auto indication=packet->findTag<Ieee80211ModeInd>();
                if (!indication || !indication->getTxVector().ampdu) return;
                auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
                Ieee80211AmpduLayout layout;
                ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),false,layout));
                if (layout.members.size()!=2) return;
                for (const auto& member : layout.members)
                    if (!packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+member.offset)->getEosp()) return;
                ASSERT(corruptReceivedAmpduMember(packet,0) && corruptReceivedAmpduMember(packet,1));
                *corrupted=true;
            }).describe("only the first actual two-member ending PSDU has two stale member FCS values"));
        test.once(on("driver").signal("budgetComplete").assertValue(1).within(0.6));
    }
    return test;
}
}

#endif
