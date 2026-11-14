// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPARTIALTRIGGERSUPPORT_H
#define INET_TEST_MESHPARTIALTRIGGERSUPPORT_H

inline void retainFirstPositiveBaBit(Packet *packet)
{
    auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    Packet member("partial-trigger-BA",packet->peekDataAt(phy->getChunkLength(),phy->getLengthField()));
    auto header=member.removeAtFront<Ieee80211CompressedBlockAck>();
    BitVector selected(0,64);
    int count=0;
    for (int i=0;i<64;i++) if (header->getBlockAckBitmap().getBit(i)) {
        if (count==0) selected.setBit(i,true);
        count++;
    }
    ASSERT(count==2);
    header->setBlockAckBitmap(selected);
    member.insertAtFront(header);
    member.removeAtBack<Ieee80211MacTrailer>(B(4));
    auto bytes=member.peekDataAsBytes()->getBytes();
    auto trailer=makeShared<Ieee80211MacTrailer>();
    trailer->setFcsMode(FCS_COMPUTED); trailer->setFcs(ethernetFcs(bytes.data(),bytes.size()));
    member.insertAtBack(trailer);
    packet->replaceDataAt(member.peekData(),phy->getChunkLength(),phy->getLengthField());
}

class PartialInitialDriver : public ServicePeriodDriver
{
    int endingStarts=0, pendingChecks=0, heldChecks=0, failedPeriods=0;
    simtime_t failedAt=-1, lastPeerBeacon=-1, lastPeerTrigger=-1;
    std::map<int64_t,std::pair<int,uint32_t>> originalIdentities;
    int terminalFacts=0;
    bool endingLoss() const { return par("endingLoss"); }
    RecoveryHcfFixture *hcf() const { return check_and_cast<RecoveryHcfFixture *>(originator()); }
    bool hasOwner() const {
        for (const auto& view : power(0)->getServicePeriods()) if (view.direction==MeshPeriodDirection::OWNER) return true;
        return false;
    }
  protected:
    bool expectAggregateEosp() const override { return aggregateAir>1; }
    void initialize() override {
        ServicePeriodDriver::initialize();
        getSimulation()->getSystemModule()->subscribe(Hcf::meshTerminalRecoverySignal,this);
        at(SimTime(50,SIMTIME_MS),10);
    }
    void observedAggregate(const Ieee80211Transmission *transmission) override {
        Ieee80211AmpduLayout layout;
        auto packet=transmission->getPacket();
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),false,layout));
        if (aggregateAir==1) {
            ASSERT(layout.members.size()==2 && !hasOwner());
            originalIdentities=hcf()->nativeIdentities(); ASSERT(originalIdentities.size()==2);
            check_and_cast<InitialDataPowerFixture *>(power(0))->restoreAutomatic=true;
            // Only the real initial two DATA originals remain when the BA
            // accepts one. The unsent agreement-keeping sentinel expires.
            hcf()->setNamedDeadline("service-sentinel",simTime()+SimTime::fromRaw(1));
            at(simTime()+SimTime(2,SIMTIME_MS),11);
            at(simTime()+SimTime(3,SIMTIME_MS),12);
            at(simTime()+SimTime(5,SIMTIME_MS),13);
        }
        else {
            ASSERT(layout.members.size()==1);
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+layout.members[0].offset);
            ASSERT(header->getEosp() && header->getRetry());
            auto current=hcf()->nativeIdentities(); ASSERT(current.size()==1);
            auto retained=*current.begin();
            ASSERT(originalIdentities.count(retained.first) && originalIdentities.at(retained.first)==retained.second);
            ASSERT(header->getSequenceNumber().get()==retained.second.first);
            auto control=packet->peekDataAt<Ieee80211MeshControl>(phy->getChunkLength()+layout.members[0].offset+header->getChunkLength());
            ASSERT(control->getSequenceNumber()==retained.second.second);
            ASSERT(hcf()->ordinaryRetries(retained.first)==(aggregateAir==2 ? 1 : 2));
            bool ending=false;
            for (const auto& view : power(0)->getServicePeriods()) if (view.direction==MeshPeriodDirection::OWNER) {
                ASSERT(view.ending && view.cap==0 && view.spent==0 && view.cohort.size()==1); ending=true;
            }
            ASSERT(ending); // First EOSP1, despite its existing MAC Retry1, is initial.
            if (aggregateAir==3) ASSERT(endingLoss() && failedAt>=0 &&
                (lastPeerBeacon>failedAt || lastPeerTrigger>failedAt));
            endingStarts++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind<10 || kind>=100) { ServicePeriodDriver::handleMessage(event); return; }
        delete event;
        if (kind==10) {
            fixturePath(0,1,1,SimTime(2,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            auto forwarding=check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"));
            forwarding->processSourceMsdu(phyMsdu("service-prime",meshMib(1)->address));
            forwarding->processSourceMsdu(phyMsdu("service-sentinel",meshMib(1)->address));
            hcf()->setNamedDeadline("service-sentinel",SimTime(320,SIMTIME_MS));
        }
        else if (kind==11 && endingLoss()) {
            ASSERT(aggregateAir==2 && hasOwner() && hcf()->recovering(meshMib(1)->address));
            ASSERT(failedPeriods==0); // Missing immediate BA still has BAR ambiguity.
            pendingChecks++;
        }
        else if (kind==12) {
            for (int i=0;i<2;i++) {
                check_and_cast<RecoveryMacFixture *>(meshInterface(i)->getSubmodule("mac"))->releaseBar=true;
                check_and_cast<Hcf *>(meshInterface(i)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
            }
        }
        else if (kind==13) {
            ASSERT(endingStarts==1 && aggregateAir==2);
            if (endingLoss()) {
                ASSERT(failedPeriods==1 && !hasOwner() && hcf()->liveEnding()==1);
                ASSERT(check_and_cast<InitialDataPowerFixture *>(power(0))->failedIntent(meshMib(1)->address));
                ASSERT(simTime()<check_and_cast<InitialDataPowerFixture *>(power(0))->awakeEnd(meshMib(1)->address));
                heldChecks++;
            }
            else ASSERT(!hasOwner() && ownerClosed>=1);
        }
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
        if (signal==Hcf::meshTerminalRecoverySignal) terminalFacts++;
        if (signal==receptionEndedSignal && source==meshInterface(0)->getSubmodule("radio")) {
            auto packet=dynamic_cast<const Packet *>(details);
            if (packet) {
                auto header=packet->peekAtFront<Ieee80211MacHeader>();
                if (header->getType()==ST_BEACON) lastPeerBeacon=simTime();
                auto data=dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
                if (!packet->hasBitError() && data && data->getType()==ST_QOS_NULL && data->getRspi()
                        && data->getReceiverAddress()==meshMib(0)->address) lastPeerTrigger=simTime();
            }
        }
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal && source==power(0)) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->direction==MeshPeriodDirection::OWNER && fact->outcome==MeshPeriodOutcome::FAILED) {
                failedPeriods++; failedAt=simTime();
            }
        }
    }
    void finishAssertions() override {
        ASSERT(endingStarts==(endingLoss() ? 2 : 1));
        ASSERT(failedPeriods==(endingLoss() ? 1 : 0));
        ASSERT(pendingChecks==(endingLoss() ? 1 : 0) && heldChecks==(endingLoss() ? 1 : 0));
        ASSERT(terminalFacts==0);
    }
};
Define_Module(PartialInitialDriver);
#endif
