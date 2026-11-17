// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPERIODDISPOSALSUPPORT_H
#define INET_TEST_MESHPERIODDISPOSALSUPPORT_H

class PeriodDisposalDriver : public ServicePeriodDriver
{
    int failed=0, confirmed=0, bars=0, controls=0, drops=0, delbaAir=0;
    std::set<int64_t> originalCohort, discarded;
    int64_t originalOpening=-1;
    bool wireDelba() const { return getSimulation()->getSystemModule()->par("recoveryCase").intValue()==18; }
    bool all() const { return getSimulation()->getSystemModule()->par("recoveryCase").intValue()>=16; }
    RecoveryHcfFixture *hcf() const { return check_and_cast<RecoveryHcfFixture *>(originator()); }
    MeshServicePeriodView owner() const {
        for (const auto& view : power(0)->getServicePeriods())
            if (view.peer==meshMib(1)->address && view.direction==MeshPeriodDirection::OWNER) return view;
        throw cRuntimeError("Expected live ending owner");
    }
  protected:
    void initialize() override {
        ServicePeriodDriver::initialize();
        getSimulation()->getSystemModule()->subscribe(packetDroppedSignal,this);
    }
    void observedAggregate(const Ieee80211Transmission *) override {
        ASSERT(aggregateAir==1);
        auto view=owner(); ASSERT(view.ending && view.cohort.size()==2 && view.spent==0);
        originalCohort=view.cohort;
        originalOpening=check_and_cast<BudgetPowerObserver *>(power(0))->ownerOpening(meshMib(1)->address);
        at(simTime()+SimTime(2,SIMTIME_MS),10);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
        if (signal==packetDroppedSignal && source==hcf()) {
            auto packet=check_and_cast<Packet *>(object);
            if (originalCohort.count(packet->getId())) {
                ASSERT(check_and_cast<PacketDropDetails *>(details)->getReason()==OTHER_PACKET_DROP);
                drops++;
            }
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power(0)) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->direction==MeshPeriodDirection::OWNER && fact->openingId==originalOpening) {
                failed+=fact->outcome==MeshPeriodOutcome::FAILED;
                confirmed+=fact->outcome==MeshPeriodOutcome::CONFIRMED;
            }
        }
        if (signal==transmissionStartedSignal && from(source,"radio")==0) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if (std::string(packet->getName())=="mesh-period-ending" && !confirmed && !failed) controls++;
            if (!packet->findTag<Ieee80211ModeReq>()->getTxVector().ampdu) {
                auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
                bars+=bool(dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())));
            }
        }
        if (wireDelba() && signal==transmissionStartedSignal && from(source,"radio")==1) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            auto header=dynamicPtrCast<const Ieee80211Delba>(packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength()));
            if (header) { ASSERT(!header->getInitiator() && header->getTid()==0); delbaAir++; }
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind!=4 && kind<10) { ServicePeriodDriver::handleMessage(event); return; }
        delete event;
        if (kind==10) {
            ASSERT(hcf()->recovering(meshMib(1)->address) && owner().cohort==originalCohort);
            ASSERT(samples==3 && failures==2 && failed==0 && confirmed==0);
            if (wireDelba()) {
                check_and_cast<RecoveryHcfFixture *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))
                    ->sendRecipientDelba(meshMib(0)->address);
                at(simTime()+SimTime(10,SIMTIME_MS),12); return;
            }
            discarded=hcf()->discardCovered(all());
            ASSERT(discarded.size()==(all() ? 2U : 1U) && drops==int(discarded.size()));
            if (all()) {
                ASSERT(failed==1 && confirmed==0 && controls==0 && bars==0);
                meshManagement(0)->closePeer(meshMib(1)->address);
                meshManagement(1)->closePeer(meshMib(0)->address);
            }
            else {
                ASSERT(failed==0 && owner().cohort==originalCohort && hcf()->recovering(meshMib(1)->address));
                check_and_cast<RecoveryMacFixture *>(meshInterface(0)->getSubmodule("mac"))->releaseBar=true;
                hcf()->meshPowerEligibilityChanged();
                at(simTime()+SimTime(5,SIMTIME_MS),11);
            }
        }
        else if (kind==11) {
            check_and_cast<RecoveryMacFixture *>(meshInterface(1)->getSubmodule("mac"))->releaseBar=true;
            check_and_cast<Hcf *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else if (kind==12) {
            ASSERT(wireDelba() && delbaAir==1 && drops==2 && failed==1 && confirmed==0 && controls==0 && bars==0);
            ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
            discarded=originalCohort;
            check_and_cast<RecoveryMacFixture *>(meshInterface(1)->getSubmodule("mac"))->releaseBar=true;
            check_and_cast<Hcf *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
        }
        else {
            ASSERT(discarded.size()==(all() ? 2U : 1U) && samples==3 && failures==2);
            ASSERT(failed==int(all()) && confirmed==int(!all()) && bars==int(!all()) && controls==0);
            emit(registerSignal("disposalComplete"),1L);
        }
    }
};
namespace period_disposal_registration { Define_Module(PeriodDisposalDriver); }
#endif
